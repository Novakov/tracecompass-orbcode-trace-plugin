from typing import Any
from orbcode.rtos_trace.trace_event.definition import ExpandableField


class SymbolRef(ExpandableField):
    def __init__(self, address: int, field_name: str = 'address') -> None:
        self.address = address
        self.field_name = field_name
        self.name = f'0x{address:08X}'

    def __repr__(self) -> str:
        return f'{self.__class__.__name__}({self.field_name}=0x{self.address:08X}, name={self.address})'

    def expand(self) -> dict[str, Any]:
        return {
            self.field_name: f'0x{self.address:08X}',
            'name': self.name,
        }

    def set_name(self, name: str) -> None:
        self.name = name
