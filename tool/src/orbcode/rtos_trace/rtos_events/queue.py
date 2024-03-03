from dataclasses import dataclass
from typing import Annotated
from orbcode.rtos_trace.trace_event.base_fields import BoolField, IntField
from orbcode.rtos_trace.trace_event.definition import TraceEvent

from .symbol_ref import SymbolRef


class QueueReference(SymbolRef):
    pass


@dataclass(frozen=True)
class QueueCreated(TraceEvent):
    queue: Annotated[QueueReference, IntField(0)]
    capacity: Annotated[int, IntField(1)]


@dataclass(frozen=True)
class QueuePushed(TraceEvent):
    queue: Annotated[QueueReference, IntField(0)]
    updated_count: Annotated[int, IntField(1, offset=16)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class QueuePopped(TraceEvent):
    queue: Annotated[QueueReference, IntField(0)]
    updated_count: Annotated[int, IntField(1, offset=16)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class QueuePushFailed(TraceEvent):
    queue: Annotated[QueueReference, IntField(0)]


@dataclass(frozen=True)
class QueuePopFailed(TraceEvent):
    queue: Annotated[QueueReference, IntField(0)]
