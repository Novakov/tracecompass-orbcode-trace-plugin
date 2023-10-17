from functools import wraps
import inspect
from typing import Callable, Dict, Optional, Tuple, Union


EVENT_TASK_SWITCHED_IN = 1
EVENT_TASK_SWITCHED_OUT = 2
EVENT_TASK_CREATED = 3
EVENT_BINARY_SEM_CREATED = 4
EVENT_BINARY_SEM_LOCKING = 5
EVENT_MUTEX_CREATED = 6
EVENT_MUTEX_LOCKING = 7
EVENT_QUEUE_CREATED = 8
EVENT_QUEUE_PUSH_POP = 9
EVENT_TASK_READIED = 10
EVENT_COUNTING_SEM_CREATED = 11
EVENT_COUNTING_SEM_GIVE_TAKE = 12
EVENT_TASK_NOTIFY = 13
EVENT_TASK_NOTIFY_RECEIVED = 14

EVENT_DWT_EXCEPTION_ENTERED = 201
EVENT_DWT_EXCEPTION_EXITED = 202
EVENT_DWT_EXCEPTION_RETURNED = 203


NOTIFY_ACTIONS = {
    0: 'NoAction',
    1: 'SetBits',
    2: 'Increment',
    3: 'SetValueWithOverwrite',
    4: 'SetValueWithoutOverwrite',
}


TraceEventData = Dict[str, Union[None, str, int, bool]]
TraceEvent = Union[Tuple[str, TraceEventData], TraceEventData]


def event_handler(event_id: int) -> Callable[[Callable[..., TraceEvent]], Callable[..., TraceEvent]]:
    def wrapper(f: Callable[..., TraceEvent]) -> Callable[..., TraceEvent]:
        f.event_id = event_id  # type: ignore

        @wraps(f)
        def wrapped(*args, **kwargs) -> TraceEvent:  # type: ignore
            result = f(*args, **kwargs)

            if not isinstance(result, tuple):
                return f.__name__, result
            else:
                return result

        return wrapped

    return wrapper


class Collector:
    def __init__(self) -> None:
        self.current_tcb = 0
        self.task_names: Dict[int, str] = {}

    def resolve_task_name(self, task_tcb: int) -> str:
        return self.task_names.get(task_tcb, f'0x{task_tcb:08X}')

    @event_handler(EVENT_TASK_SWITCHED_IN)
    def task_switched_in(self, tcb: int) -> TraceEvent:
        self.current_tcb = tcb

        return {
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb)
        }

    @event_handler(EVENT_TASK_SWITCHED_OUT)
    def task_switched_out(self, tcb: int, switch_reason_data: int, blocked_on_object: Optional[int], flags: int) -> TraceEvent:
        self.current_tcb = 0
        still_ready = (flags & (1 << 0)) == (1 << 0)
        switch_reason = (switch_reason_data >> 28) & 0b1111

        out_state = 'Blocked'
        blocked_on = ''
        block_operation = ''

        if still_ready:
            out_state = 'Ready'
            blocked_on_object = None
        elif switch_reason == 0:
            out_state = 'Ready'
            blocked_on_object = None
        elif switch_reason == 1:
            out_state = 'Delayed'
            blocked_on_object = None
        elif switch_reason == 2:
            out_state = 'Blocked'
            blocked_on = 'Mutex'
        elif switch_reason == 3:
            out_state = 'Blocked'
            blocked_on = 'Queue'
            block_operation = 'Push'
        elif switch_reason == 4:
            out_state = 'Blocked'
            blocked_on = 'Queue'
            block_operation = 'Pop'
        elif switch_reason == 5:
            out_state = 'Blocked'
            blocked_on = 'BinarySemaphore'
        elif switch_reason == 6:
            out_state = 'Blocked'
            blocked_on = 'EventGroup'
        elif switch_reason == 7:
            out_state = 'Blocked'
            blocked_on = 'CountingSemaphore'
            block_operation = 'Give'
        elif switch_reason == 8:
            out_state = 'Blocked'
            blocked_on = 'CountingSemaphore'
            block_operation = 'Take'
        elif switch_reason == 9:
            out_state = 'Blocked'
            blocked_on = 'TaskNotify'
            block_operation = 'Wait'
        elif switch_reason == 0xF:
            out_state = 'Blocked'
            blocked_on = 'Other'
        else:
            raise RuntimeError(f'Unknown switch out reason {switch_reason}')

        return {
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'OutState': out_state,
            'BlockedOn': blocked_on,
            'BlockingOperation': block_operation,
            'BlockedOnObject': f'0x{blocked_on_object:08X}' if blocked_on_object is not None else '',
        }

    @event_handler(EVENT_TASK_READIED)
    def task_readied(self, tcb: int) -> TraceEvent:
        return {
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb)
        }

    @event_handler(EVENT_BINARY_SEM_CREATED)
    def binary_semaphore_created(self, queue: int) -> TraceEvent:
        return {'Semaphore': f'0x{queue:08X}'}

    @event_handler(EVENT_COUNTING_SEM_CREATED)
    def counting_semaphore_created(self, semaphore: int, max_count: int, initial_count: int) -> TraceEvent:
        return {'Semaphore': f'0x{semaphore:08X}', 'MaxCount': max_count, 'InitialCount': initial_count}

    @event_handler(EVENT_COUNTING_SEM_GIVE_TAKE)
    def counting_semaphore_give_take(self, queue: int, flags: int) -> TraceEvent:
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_take = (flags & (1 << 0)) == (1 << 0)
        updated_items_count = (flags >> 16) & 0xFFFF

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data: TraceEventData = {
            'Semaphore': f'0x{queue:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr,
            'UpdatedCount': updated_items_count,
        }

        if is_success:
            if is_take:
                return 'counting_semaphore_taken', data
            else:
                return 'counting_semaphore_given', data
        else:
            if is_take:
                return 'counting_semaphore_take_failed', data
            else:
                return 'counting_semaphore_give_failed', data

    @event_handler(EVENT_MUTEX_CREATED)
    def mutex_created(self, mutex: int) -> TraceEvent:
        return {'Mutex': f'0x{mutex:08X}'}

    @event_handler(EVENT_BINARY_SEM_LOCKING)
    def binary_semaphore_locking(self, queue: int, flags: int) -> TraceEvent:
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_lock = (flags & (1 << 0)) == (1 << 0)

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data: TraceEventData = {
            'Semaphore': f'0x{queue:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr
        }

        if is_success:
            if is_lock:
                return 'binary_semaphore_locked', data
            else:
                return 'binary_semaphore_unlocked', data
        else:
            if is_lock:
                return 'binary_semaphore_lock_failed', data
            else:
                return 'binary_semaphore_unlock_failed', data

    @event_handler(EVENT_MUTEX_LOCKING)
    def mutex_locking(self, mutex: int, flags: int) -> TraceEvent:
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_lock = (flags & (1 << 0)) == (1 << 0)

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data: TraceEventData = {
            'Mutex': f'0x{mutex:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr
        }

        if is_success:
            if is_lock:
                return 'mutex_locked', data
            else:
                return 'mutex_unlocked', data
        else:
            if is_lock:
                return 'mutex_lock_failed', data
            else:
                return 'mutex_unlock_failed', data

    @event_handler(EVENT_QUEUE_CREATED)
    def queue_created(self, queue: int, capacity: int) -> TraceEvent:
        return {'Queue': f'0x{queue:08X}', 'Capacity': capacity}

    @event_handler(EVENT_QUEUE_PUSH_POP)
    def queue_push_pop(self, queue: int, flags: int) -> TraceEvent:
        is_success = (flags & (1 << 2)) == (1 << 2)
        is_isr = (flags & (1 << 1)) == (1 << 1)
        is_push = (flags & (1 << 0)) == (0 << 0)
        updated_items_count = (flags >> 16) & 0xFFFF

        if is_isr:
            tcb = 0
        else:
            tcb = self.current_tcb

        data: TraceEventData = {
            'Queue': f'0x{queue:08X}',
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
            'ISR': is_isr,
            'UpdatedItemsCount': updated_items_count
        }

        if is_success:
            if is_push:
                return 'queue_pushed', data
            else:
                return 'queue_popped', data
        else:
            if is_push:
                return 'queue_pushed_failed', data
            else:
                return 'queue_popped_failed', data

    @event_handler(EVENT_TASK_NOTIFY)
    def task_notified(self, task: int, flags: int, updated_value: int) -> TraceEvent:
        index = (flags & 0xFFFF)
        action = (flags >> 16) & 0xFFFF
        return {
            'TCB': f'0x{task:08X}',
            'TaskName': self.resolve_task_name(task),
            'NotifyIndex': index,
            'NotifyAction': NOTIFY_ACTIONS[action],
            'UpdatedValue': updated_value,
        }

    @event_handler(EVENT_TASK_NOTIFY_RECEIVED)
    def task_notify_received(self, task: int, flags: int, updated_value: int) -> TraceEvent:
        index = (flags & 0xFFFF)

        return {
            'TCB': f'0x{task:08X}',
            'TaskName': self.resolve_task_name(task),
            'NotifyIndex': index,
            'UpdatedValue': updated_value,
        }

    @event_handler(EVENT_TASK_CREATED)
    def task_created(self, tcb: int, name_length: int, *name_bytes: int) -> TraceEvent:
        del name_length
        name = b''.join([p.to_bytes(byteorder='little', length=32).strip(b'\0') for p in name_bytes]).decode('utf-8')

        self.task_names[tcb] = name

        return {
            'TCB': f'0x{tcb:08X}',
            'TaskName': self.resolve_task_name(tcb),
        }

    @event_handler(EVENT_DWT_EXCEPTION_ENTERED)
    def exception_entered(self, exception: int) -> TraceEvent:
        return {
            'ExceptionNumber': exception,
        }

    @event_handler(EVENT_DWT_EXCEPTION_EXITED)
    def exception_exited(self, exception: int) -> TraceEvent:
        return {
            'ExceptionNumber': exception,
        }

    @event_handler(EVENT_DWT_EXCEPTION_RETURNED)
    def exception_returned(self, exception: int) -> TraceEvent:
        return {
            'ExceptionNumber': exception,
        }

    def event_map(self) -> Dict[int, Callable[..., TraceEvent]]:
        result: Dict[int, Callable[..., TraceEvent]] = {}
        for _, member in inspect.getmembers(self):
            if not inspect.ismethod(member):
                continue
            if not hasattr(member, 'event_id'):
                continue

            result[member.event_id] = member  # type: ignore

        return result
