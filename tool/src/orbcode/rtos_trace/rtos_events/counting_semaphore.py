from dataclasses import dataclass
from typing import Annotated
from orbcode.rtos_trace.rtos_events.symbol_ref import SymbolRef
from orbcode.rtos_trace.trace_event.base_fields import IntField
from orbcode.rtos_trace.trace_event.definition import TraceEvent


class CountingSemaphoreReference(SymbolRef):
    pass


@dataclass(frozen=True)
class CountingSemaphoreCreated(TraceEvent):
    semaphore: Annotated[CountingSemaphoreReference, IntField(0)]
    max_count: Annotated[int, IntField(1, size=16, offset=0)]
    initial_count: Annotated[int, IntField(1, size=16, offset=16)]


@dataclass(frozen=True)
class CountingSemaphoreGiven(TraceEvent):
    semaphore: Annotated[CountingSemaphoreReference, IntField(0)]
    updated_count: Annotated[int, IntField(1, offset=16)]
    isr: Annotated[bool, IntField(1, size=1, offset=0)]


@dataclass(frozen=True)
class CountingSemaphoreTaken(TraceEvent):
    semaphore: Annotated[CountingSemaphoreReference, IntField(0)]
    updated_count: Annotated[int, IntField(1, offset=16)]
    isr: Annotated[bool, IntField(1, size=1, offset=0)]


@dataclass(frozen=True)
class CountingSemaphoreGiveFailed(TraceEvent):
    semaphore: Annotated[CountingSemaphoreReference, IntField(0)]
    isr: Annotated[bool, IntField(1, size=1, offset=0)]


@dataclass(frozen=True)
class CountingSemaphoreTakeFailed(TraceEvent):
    semaphore: Annotated[CountingSemaphoreReference, IntField(0)]
    isr: Annotated[bool, IntField(1, size=1, offset=0)]
