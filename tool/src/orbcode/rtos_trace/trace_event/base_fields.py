from orbcode import pyorb
from .definition import Field


class IntField(Field):
    def __init__(self, index: int, *, size: int = 32, offset: int = 0) -> None:
        self.index = index
        self.size = size
        self.offset = offset

    def read(self, data: list[pyorb.TraceMessage]) -> int:
        pkt = data[self.index]
        assert isinstance(pkt, pyorb.swMsg)
        v = int(pkt.value)
        v >>= self.offset
        mask = (1 << self.size) - 1
        return v & mask


class BoolField(Field):
    def __init__(self, index: int, offset: int) -> None:
        self.index = index
        self.offset = offset

    def read(self, data: list[pyorb.TraceMessage]) -> bool:
        arg = data[self.index]
        assert isinstance(arg, pyorb.swMsg)
        v = arg.value
        v >>= self.offset
        return bool(v & 1)


class StringField(Field):
    def __init__(self, index: int) -> None:
        self.index = index

    def read(self, data: list[pyorb.TraceMessage]) -> str:
        pkt = data[self.index:]

        assert len(pkt) >= 2, 'String field must have at least two messages (length and data)'
        assert isinstance(pkt[0], pyorb.swMsg), 'First message must be a ITM message'

        name_length = pkt[0].value
        name = b''

        for p in pkt[1:]:
            assert isinstance(p, pyorb.swMsg), 'Subsequent messages must be ITM messages'
            name += p.value.to_bytes(p.len, 'little')
            assert len(name) <= name_length, 'Name length must not exceed the length specified in the first message'
            if len(name) == name_length:
                break

        return name.decode('utf-8')
