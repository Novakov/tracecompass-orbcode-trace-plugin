from typing import Type

from orbcode.rtos_trace.trace_event import TraceEvent
from . import task
from . import binary_semaphore
from . import queue
from . import exception
from . import counting_semaphore
from . import mutex


RTOS_TRACE_PACKET_TYPES: dict[int, Type[TraceEvent]] = {
    1: task.TaskSwitchedIn,
    2: task.TaskSwitchedOut,
    3: task.TaskCreated,
    4: task.TaskReadied,

    5: binary_semaphore.BinarySemaphoreCreated,
    6: binary_semaphore.BinarySemaphoreLocked,
    7: binary_semaphore.BinarySemaphoreUnlocked,
    8: binary_semaphore.BinarySemaphoreLockFailed,
    9: binary_semaphore.BinarySemaphoreUnlockFailed,

    10: queue.QueueCreated,
    11: queue.QueuePushed,
    12: queue.QueuePopped,
    13: queue.QueuePushFailed,
    14: queue.QueuePopFailed,

    15: counting_semaphore.CountingSemaphoreCreated,
    16: counting_semaphore.CountingSemaphoreGiven,
    17: counting_semaphore.CountingSemaphoreTaken,
    18: counting_semaphore.CountingSemaphoreGiveFailed,
    19: counting_semaphore.CountingSemaphoreTakeFailed,

    20: task.TaskNotify,
    21: task.TaskNotifyReceived,

    22: mutex.MutexCreated,
    23: mutex.MutexLocked,
    24: mutex.MutexUnlocked,
    25: mutex.MutexLockFailed,
    26: mutex.MutexUnlockFailed,

    256 + 1: exception.ExceptionEntered,
    256 + 2: exception.ExceptionExited,
    256 + 3: exception.ExceptionReturned,
}
