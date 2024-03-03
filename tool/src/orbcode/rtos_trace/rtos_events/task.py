from dataclasses import dataclass
from enum import Enum
from typing import Annotated, Any

from orbcode.rtos_trace.trace_event import TraceEvent
from orbcode.rtos_trace.trace_event.base_fields import BoolField, IntField, StringField
from orbcode.rtos_trace.trace_event import ExpandableField

from .task_ref import TaskReference
from .symbol_ref import SymbolRef


@dataclass(frozen=True)
class TaskCreated(TraceEvent):
    task: Annotated[TaskReference, IntField(0)]
    name: Annotated[str, StringField(1)]


@dataclass(frozen=True)
class TaskSwitchedIn(TraceEvent):
    task: Annotated[TaskReference, IntField(0)]


class TaskNotifyAction(Enum):
    NoAction = 0
    SetBits = 1
    Increment = 2
    SetValueWithOverwrite = 3
    SetValueWithoutOverwrite = 4


@dataclass(frozen=True)
class TaskNotify(TraceEvent):
    task: Annotated[TaskReference, IntField(0)]
    index: Annotated[int, IntField(1, offset=0, size=16)]
    action: Annotated[TaskNotifyAction, IntField(1, offset=16, size=4)]
    isr: Annotated[bool, BoolField(1, offset=24)]
    updated_value: Annotated[int, IntField(2)]


@dataclass(frozen=True)
class TaskNotifyReceived(TraceEvent):
    task: Annotated[TaskReference, IntField(0)]
    index: Annotated[int, IntField(1)]
    updated_value: Annotated[int, IntField(2)]


class TaskSwitchReason(Enum):
    Preempted = 0
    Delayed = 1
    TaskNotifyWait = 3

    BlockedBinarySemaphoreUnlock = (1 << 4) | 0
    BlockedBinarySemaphoreLock = (1 << 4) | 1

    BlockedQueuePush = (2 << 4) | 0
    BlockedQueuePop = (2 << 4) | 1

    BlockedOnCountingSemaphoreGive = (3 << 4) | 0
    BlockedOnCountingSemaphoreTake = (3 << 4) | 1

    BlockedOnMutexLock = (4 << 4) | 0
    BlockedOnMutexUnlock = (4 << 4) | 1

    def __str__(self) -> str:
        return self.name


class TaskSwitchReasonObject(Enum):
    Nothing = 0
    BinarySemaphore = 1
    Queue = 2
    CountingSemaphore = 3
    Mutex = 4

    def __str__(self) -> str:
        return self.name


class BlockedOn(ExpandableField):
    OPERATION_NAMES = {
        TaskSwitchReasonObject.BinarySemaphore: {
            0: 'Unlock',
            1: 'Lock'
        },
        TaskSwitchReasonObject.Queue: {
            0: 'Push',
            1: 'Pop'
        },
        TaskSwitchReasonObject.CountingSemaphore: {
            0: 'Give',
            1: 'Take'
        },
        TaskSwitchReasonObject.Mutex: {
            0: 'Lock',
            1: 'Unlock'
        }
    }

    def __init__(self, raw: int) -> None:
        self.object_type = TaskSwitchReasonObject((raw >> 4) & 0xF)
        operation_code = raw & 0xF

        if self.object_type == TaskSwitchReasonObject.Nothing:
            self.operation = None
        else:
            self.operation = self.OPERATION_NAMES[self.object_type][operation_code]

    def expand(self) -> dict[str, Any]:
        return {
            'type': self.object_type if self.object_type != TaskSwitchReasonObject.Nothing else None,
            'operation': self.operation
        }

    def __repr__(self) -> str:
        return f'BlockedOn(object_type={self.object_type}, operation={self.operation})'


@dataclass(frozen=True)
class TaskSwitchedOut(TraceEvent):
    task: Annotated[TaskReference, IntField(0)]
    switch_reason: Annotated[TaskSwitchReason, IntField(1, offset=0, size=8)]
    blocked_on: Annotated[BlockedOn, IntField(1)]
    blocked_on_object: Annotated[SymbolRef, IntField(2)]
    still_ready: Annotated[bool, BoolField(3, 0)]


@dataclass(frozen=True)
class TaskReadied(TraceEvent):
    task: Annotated[TaskReference, IntField(0)]
