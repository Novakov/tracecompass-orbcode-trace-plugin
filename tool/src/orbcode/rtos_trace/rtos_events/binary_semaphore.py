from dataclasses import dataclass
from typing import Annotated
from orbcode.rtos_trace.trace_event import TraceEvent
from orbcode.rtos_trace.trace_event.base_fields import BoolField, IntField

from .symbol_ref import SymbolRef


class BinarySemaphoreReference(SymbolRef):
    pass


@dataclass(frozen=True)
class BinarySemaphoreCreated(TraceEvent):
    semaphore: Annotated[BinarySemaphoreReference, IntField(0)]


@dataclass(frozen=True)
class BinarySemaphoreLocked(TraceEvent):
    semaphore: Annotated[BinarySemaphoreReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class BinarySemaphoreUnlocked(TraceEvent):
    semaphore: Annotated[BinarySemaphoreReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class BinarySemaphoreLockFailed(TraceEvent):
    semaphore: Annotated[BinarySemaphoreReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]


@dataclass(frozen=True)
class BinarySemaphoreUnlockFailed(TraceEvent):
    semaphore: Annotated[BinarySemaphoreReference, IntField(0)]
    isr: Annotated[bool, BoolField(1, 0)]
