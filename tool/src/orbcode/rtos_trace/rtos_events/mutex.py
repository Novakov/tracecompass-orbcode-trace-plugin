from dataclasses import dataclass
from typing import Annotated
from orbcode.rtos_trace.rtos_events.symbol_ref import SymbolRef
from orbcode.rtos_trace.trace_event import TraceEvent
from orbcode.rtos_trace.trace_event.base_fields import BoolField, IntField


class MutexReference(SymbolRef):
    pass


@dataclass(frozen=True)
class MutexCreated(TraceEvent):
    mutex: Annotated[MutexReference, IntField(0)]


@dataclass(frozen=True)
class MutexLocked(TraceEvent):
    mutex: Annotated[MutexReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class MutexUnlocked(TraceEvent):
    mutex: Annotated[MutexReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class MutexLockFailed(TraceEvent):
    mutex: Annotated[MutexReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class MutexUnlockFailed(TraceEvent):
    mutex: Annotated[MutexReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]
