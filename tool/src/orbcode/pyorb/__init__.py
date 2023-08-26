from ctypes import *
import ctypes.util
import socket
import os
import typing

# Ensure restype is the correct length for platform
class my_void_p(c_void_p):
    pass

class TSMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("timeStatus", c_int,8),
                 ("timeInc",    c_uint,32)]

class swMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("srcAddr",    c_int,8),
                 ("len",        c_int,8),
                 ("value",      c_uint)]

class nisyncMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("type",       c_int,8),
                 ("addr",       c_int,32)]

class pcSampleMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("sleep",      c_bool,1),
                 ("pc",         c_int)]


class oswMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("comp",       c_int,8),
                 ("offset",     c_int)]

class wptMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("comp",       c_int,8),
                 ("data",       c_int)]

class watchMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("comp",       c_int,8),
                 ("isWrite",    c_bool,1),
                 ("data",       c_int)]

class dwtMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("event",      c_int,8)]

class Empty(Structure):
    _fields_ = [ ("ts",         c_longlong)]


class excMsg(Structure):
    _fields_ = [ ("ts",         c_longlong),
                 ("exceptionNumber",c_int),
                 ("eventType",      c_int,8)]

class msgUnion(Union):
    _fields_ = [ ("Unknown",     Empty),
                 ("Reserved",    Empty),
                 ("Error",       Empty),
                 ("None",        Empty),
                 ("swMsg",       swMsg),
                 ("nisyncMsg",   nisyncMsg),
                 ("oswMsg",      oswMsg),
                 ("watchMsg",    watchMsg),
                 ("wptMsg",      wptMsg),
                 ("pcSampleMsg", pcSampleMsg),
                 ("dwtMsg",      dwtMsg),
                 ("excMsg",      excMsg),
                 ("TSMsg",       TSMsg)]

class msg(Structure):
    _fields_ = [ ("msgtype",     c_int),
                 ("m",           msgUnion) ]

TraceMessage = typing.Union[Empty, swMsg, nisyncMsg, oswMsg, watchMsg, wptMsg, pcSampleMsg, dwtMsg, excMsg, TSMsg]

def _load_orb() -> CDLL:
    env_path = os.environ.get('LIBORB_PATH')
    if env_path is not None:
        return CDLL(env_path)
    else:
        orb_lib = ctypes.util.find_library('liborb-2.1.0')
        if orb_lib is None:
            orb_lib = 'liborb'
        return CDLL(orb_lib)

orb = _load_orb()

orb.ITMDecoderCreate.restype = my_void_p
orb.ITMDecoderInit.argtypes = [ my_void_p, c_bool ]
orb.ITMPump.argtypes = [ my_void_p, c_char ]
orb.ITMGetDecodedPacket.argtypes = [my_void_p, my_void_p]
orb.ITMGetDecodedPacket.restype = c_bool

orb.ITM_EV_NONE       = 0
orb.ITM_EV_PACKET_RXED= 1
orb.ITM_EV_UNSYNCED   = 2
orb.ITM_EV_SYNCED     = 3
orb.ITM_EV_OVERFLOW   = 4
orb.ITM_EV_ERROR      = 5

from typing import IO, Protocol

class OrbSource(Protocol):
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

def orb_source_io(buffer: IO[bytes]) -> OrbSource:
    def receiver(length: int) -> bytes:
        return buffer.read(length)

    return receiver


class Orb:
    def __init__(self, source: OrbSource, withTPIU=False, forceSync=True, sock=None):
        self.source = source
        self.itm = orb.ITMDecoderCreate()
        orb.ITMDecoderInit( self.itm, forceSync )

    def rx(self):
        while True:
            c = self.source(1)
            if c == b'':
                return None
            if orb.ITM_EV_PACKET_RXED == orb.ITMPump(self.itm, c[0]):
                p = msg()
                orb.ITMGetDecodedPacket(self.itm, byref(p))
                try:
                    return getattr(p.m, p.m._fields_[p.msgtype][0])
                except IndexError:
                    return Empty()
