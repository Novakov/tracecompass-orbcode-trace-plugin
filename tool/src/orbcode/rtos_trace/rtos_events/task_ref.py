from .symbol_ref import SymbolRef


class TaskReference(SymbolRef):
    def __init__(self, tcb: int) -> None:
        super().__init__(tcb, field_name='tcb')
