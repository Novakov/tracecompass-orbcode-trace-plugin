import ctypes
import ctypes.util
import socket
import os
import typing


# Ensure restype is the correct length for platform
class my_void_p(ctypes.c_void_p):
    pass


class TSMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('timeStatus', ctypes.c_int, 8),
        ('timeInc',    ctypes.c_uint, 32)
    ]


class swMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('srcAddr',    ctypes.c_int, 8),
        ('len',        ctypes.c_int, 8),
        ('value',      ctypes.c_uint)
    ]


class nisyncMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('type',       ctypes.c_int, 8),
        ('addr',       ctypes.c_int, 32)
    ]


class pcSampleMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('sleep',      ctypes.c_bool, 1),
        ('pc',         ctypes.c_int)
    ]


class oswMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('comp',       ctypes.c_int, 8),
        ('offset',     ctypes.c_int)
    ]


class wptMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('comp',       ctypes.c_int, 8),
        ('data',       ctypes.c_int)
    ]


class watchMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('comp',       ctypes.c_int, 8),
        ('isWrite',    ctypes.c_bool, 1),
        ('data',       ctypes.c_int)
    ]


class dwtMsg(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong),
        ('event',      ctypes.c_int, 8)
    ]


class Empty(ctypes.Structure):
    _fields_ = [
        ('ts',         ctypes.c_longlong)
    ]


class excMsg(ctypes.Structure):
    _fields_ = [
        ('ts',              ctypes.c_longlong),
        ('exceptionNumber', ctypes.c_int),
        ('eventType',       ctypes.c_int, 8)
    ]


class msgUnion(ctypes.Union):
    _fields_ = [
        ('Unknown',     Empty),
        ('Reserved',    Empty),
        ('Error',       Empty),
        ('None',        Empty),
        ('swMsg',       swMsg),
        ('nisyncMsg',   nisyncMsg),
        ('oswMsg',      oswMsg),
        ('watchMsg',    watchMsg),
        ('wptMsg',      wptMsg),
        ('pcSampleMsg', pcSampleMsg),
        ('dwtMsg',      dwtMsg),
        ('excMsg',      excMsg),
        ('TSMsg',       TSMsg)
    ]


class msg(ctypes.Structure):
    _fields_ = [
        ('msgtype',     ctypes.c_int),
        ('m',           msgUnion)
    ]


TraceMessage = typing.Union[Empty, swMsg, nisyncMsg, oswMsg, watchMsg, wptMsg, pcSampleMsg, dwtMsg, excMsg, TSMsg]


def _load_orb() -> ctypes.CDLL:
    env_path = os.environ.get('LIBORB_PATH')
    if env_path is not None:
        return ctypes.CDLL(env_path)
    else:
        orb_lib = ctypes.util.find_library('liborb-2.1.0')
        if orb_lib is None:
            orb_lib = 'liborb'
        return ctypes.CDLL(orb_lib)


orb = _load_orb()

orb.ITMDecoderCreate.restype = my_void_p
orb.ITMDecoderInit.argtypes = [my_void_p, ctypes.c_bool]
orb.ITMPump.argtypes = [my_void_p, ctypes.c_char]
orb.ITMGetDecodedPacket.argtypes = [my_void_p, my_void_p]
orb.ITMGetDecodedPacket.restype = ctypes.c_bool

ITM_EV_NONE        = 0
ITM_EV_PACKET_RXED = 1
ITM_EV_UNSYNCED    = 2
ITM_EV_SYNCED      = 3
ITM_EV_OVERFLOW    = 4
ITM_EV_ERROR       = 5


class OrbSource(typing.Protocol):
    def __call__(self, __length: int) -> bytes:
        pass


def orb_source_connect(addr: tuple[str, int]) -> OrbSource:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(addr)

    return orb_source_socket(sock)


def orb_source_socket(sock: socket.socket) -> OrbSource:
    def receiver(length: int) -> bytes:
        return sock.recv(length)

    return receiver


def orb_source_io(buffer: typing.IO[bytes]) -> OrbSource:
    def receiver(length: int) -> bytes:
        return buffer.read(length)

    return receiver


class Orb:
    def __init__(self, source: OrbSource, force_sync: bool = True):
        self.source = source
        self.itm = orb.ITMDecoderCreate()
        orb.ITMDecoderInit(self.itm, force_sync)

    def rx(self) -> typing.Optional[TraceMessage]:
        while True:
            c = self.source(1)
            if c == b'':
                return None
            if ITM_EV_PACKET_RXED == orb.ITMPump(self.itm, c[0]):
                p = msg()
                orb.ITMGetDecodedPacket(self.itm, ctypes.byref(p))
                try:
                    return typing.cast(TraceMessage, getattr(p.m, p.m._fields_[p.msgtype][0]))
                except IndexError:
                    return Empty()

    def iterate_all_messages(self) -> typing.Iterable[TraceMessage]:
        while True:
            packet = self.rx()

            if packet is None:
                break
            if isinstance(packet, Empty):
                continue

            yield packet
