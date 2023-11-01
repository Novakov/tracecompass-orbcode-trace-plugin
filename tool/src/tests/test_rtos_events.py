from dataclasses import dataclass
from typing import List, Tuple
import pytest
from orbcode.rtos_trace.rtos_events import Collector, TraceEventData


class Runner:
    def __init__(self) -> None:
        self._collector = Collector()
        self._event_map = self._collector.event_map()

    def run(self, event_id: int, payload: List[int]) -> Tuple[str, TraceEventData]:
        handler = self._event_map.get(event_id, None)
        assert handler is not None

        event_data = handler(*payload)
        assert isinstance(event_data, tuple)

        event_type, event_data = event_data
        return event_type, event_data


@pytest.fixture()
def runner() -> Runner:
    return Runner()


@dataclass(frozen=True)
class TestCase:
    __test__ = False

    event_id: int
    payload: List[int]
    event_name: str
    expected: TraceEventData
    needs_task: bool = False


TCB = '0x12345678'
TASK_NAME = 'MyTask1'


TEST_CASES_BINARY_SEMAPHORE_LOCKING = [
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b101],
        event_name='binary_semaphore_locked',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b001],
        event_name='binary_semaphore_lock_failed',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b111],
        event_name='binary_semaphore_locked',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b011],
        event_name='binary_semaphore_lock_failed',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b100],
        event_name='binary_semaphore_unlocked',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b000],
        event_name='binary_semaphore_unlock_failed',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b110],
        event_name='binary_semaphore_unlocked',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=5,
        payload=[0x12345678, 0b010],
        event_name='binary_semaphore_unlock_failed',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    )
]

TEST_CASES_COUNTING_SEMAPHORE_GIVE_TAKE = [
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b000 | (1000 << 16)],
        event_name='counting_semaphore_give_failed',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b001 | (1000 << 16)],
        event_name='counting_semaphore_take_failed',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b010 | (1000 << 16)],
        event_name='counting_semaphore_give_failed',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b011 | (1000 << 16)],
        event_name='counting_semaphore_take_failed',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b100 | (1000 << 16)],
        event_name='counting_semaphore_given',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b101 | (1000 << 16)],
        event_name='counting_semaphore_taken',
        expected={'Semaphore': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b110 | (1000 << 16)],
        event_name='counting_semaphore_given',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=12,
        payload=[0x12345678, 0b111 | (1000 << 16)],
        event_name='counting_semaphore_taken',
        expected={'Semaphore': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedCount': 1000},
        needs_task=True
    )
]

TEST_CASES_MUTEX_LOCKING = [
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b000],
        event_name='mutex_unlock_failed',
        expected={'Mutex': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b001],
        event_name='mutex_lock_failed',
        expected={'Mutex': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b010],
        event_name='mutex_unlock_failed',
        expected={'Mutex': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b011],
        event_name='mutex_lock_failed',
        expected={'Mutex': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b100],
        event_name='mutex_unlocked',
        expected={'Mutex': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b101],
        event_name='mutex_locked',
        expected={'Mutex': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b110],
        event_name='mutex_unlocked',
        expected={'Mutex': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=7,
        payload=[0x12345678, 0b111],
        event_name='mutex_locked',
        expected={'Mutex': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True},
        needs_task=True
    )
]

TEST_CASES_QUEUE_PUSH_POP = [
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b000 | (1000 << 16)],
        event_name='queue_pushed_failed',
        expected={'Queue': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b001 | (1000 << 16)],
        event_name='queue_popped_failed',
        expected={'Queue': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b010 | (1000 << 16)],
        event_name='queue_pushed_failed',
        expected={'Queue': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b011 | (1000 << 16)],
        event_name='queue_popped_failed',
        expected={'Queue': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b100 | (1000 << 16)],
        event_name='queue_pushed',
        expected={'Queue': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b101 | (1000 << 16)],
        event_name='queue_popped',
        expected={'Queue': '0x12345678', 'TCB': TCB, 'TaskName': TASK_NAME, 'ISR': False, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b110 | (1000 << 16)],
        event_name='queue_pushed',
        expected={'Queue': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
    TestCase(
        event_id=9,
        payload=[0x12345678, 0b111 | (1000 << 16)],
        event_name='queue_popped',
        expected={'Queue': '0x12345678', 'TCB': '0x00000000', 'TaskName': '0x00000000', 'ISR': True, 'UpdatedItemsCount': 1000},
        needs_task=True
    ),
]

TEST_CASES_TASK_NOTIFY = [
    TestCase(
        event_id=14,
        payload=[0x12345678, 0xABCD1234, 200],
        event_name='task_notify_received',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'UpdatedValue': 200},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x00001234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'NoAction', 'UpdatedValue': 200 ,'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x00011234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'SetBits', 'UpdatedValue': 200, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x00021234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'Increment', 'UpdatedValue': 200, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x00031234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'SetValueWithOverwrite', 'UpdatedValue': 200, 'ISR': False},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x00041234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'SetValueWithoutOverwrite', 'UpdatedValue': 200, 'ISR': False},
        needs_task=True
    ),

    TestCase(
        event_id=13,
        payload=[0x12345678, 0x01001234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'NoAction', 'UpdatedValue': 200 ,'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x01011234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'SetBits', 'UpdatedValue': 200, 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x01021234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'Increment', 'UpdatedValue': 200, 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x01031234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'SetValueWithOverwrite', 'UpdatedValue': 200, 'ISR': True},
        needs_task=True
    ),
    TestCase(
        event_id=13,
        payload=[0x12345678, 0x01041234, 200],
        event_name='task_notified',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'NotifyIndex': 0x1234, 'NotifyAction': 'SetValueWithoutOverwrite', 'UpdatedValue': 200, 'ISR': True},
        needs_task=True
    ),
]

TEST_CASES_TASK_SWITCHED_OUT = [
    TestCase(
        event_id=2,
        payload=[0x12345678, 0, 0, 1],
        event_name='task_switched_out',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'OutState': 'Ready', 'BlockedOn': '', 'BlockingOperation': '', 'BlockedOnObject': ''},
        needs_task=True
    ),
    TestCase(
        event_id=2,
        payload=[0x12345678, 0, 0, 0],
        event_name='task_switched_out',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'OutState': 'Ready', 'BlockedOn': '', 'BlockingOperation': '', 'BlockedOnObject': ''},
        needs_task=True
    ),
    TestCase(
        event_id=2,
        payload=[0x12345678, (1 << 28), 0, 0],
        event_name='task_switched_out',
        expected={'TCB': TCB, 'TaskName': TASK_NAME, 'OutState': 'Delayed', 'BlockedOn': '', 'BlockingOperation': '', 'BlockedOnObject': ''},
        needs_task=True
    ),
    *[
        TestCase(
            event_id=2,
            payload=[0x12345678, (reason << 28), 0xABCD1234, 0],
            event_name='task_switched_out',
            expected={
                'TCB': TCB,
                'TaskName': TASK_NAME,
                'OutState': 'Blocked',
                'BlockedOn': obj,
                'BlockingOperation': op,
                'BlockedOnObject': '0xABCD1234'
            },
            needs_task=True
        )
        for reason, obj, op in [
            (2, 'Mutex', ''),
            (3, 'Queue', 'Push'),
            (4, 'Queue', 'Pop'),
            (5, 'BinarySemaphore', ''),
            (6, 'EventGroup', ''),
            (7, 'CountingSemaphore', 'Give'),
            (8, 'CountingSemaphore', 'Take'),
            (9, 'TaskNotify', 'Wait'),
            (0xF, 'Other', '')
        ]
    ]
]

TEST_CASES = [
    TestCase(
        event_id=1,
        payload=[0x12345678],
        event_name='task_switched_in',
        expected={'TCB': TCB, 'TaskName': '0x12345678'}
    ),
    TestCase(
        event_id=10,
        payload=[0x12345678],
        event_name='task_readied',
        expected={'TCB': TCB, 'TaskName': '0x12345678'}
    ),
    TestCase(
        event_id=10,
        payload=[0x12345678],
        event_name='task_readied',
        expected={'TCB': TCB, 'TaskName': TASK_NAME},
        needs_task=True
    ),
    TestCase(
        event_id=4,
        payload=[0x12345678],
        event_name='binary_semaphore_created',
        expected={'Semaphore': '0x12345678'}
    ),
    TestCase(
        event_id=11,
        payload=[0x12345678, 100, 200],
        event_name='counting_semaphore_created',
        expected={'Semaphore': '0x12345678', 'MaxCount': 100, 'InitialCount': 200}
    ),
    TestCase(
        event_id=6,
        payload=[0x12345678],
        event_name='mutex_created',
        expected={'Mutex': '0x12345678'}
    ),
    TestCase(
        event_id=8,
        payload=[0x12345678, 3000],
        event_name='queue_created',
        expected={'Queue': '0x12345678', 'Capacity': 3000}
    ),
    *TEST_CASES_BINARY_SEMAPHORE_LOCKING,
    *TEST_CASES_COUNTING_SEMAPHORE_GIVE_TAKE,
    *TEST_CASES_MUTEX_LOCKING,
    *TEST_CASES_QUEUE_PUSH_POP,
    *TEST_CASES_TASK_NOTIFY,
    *TEST_CASES_TASK_SWITCHED_OUT
]


@pytest.mark.parametrize('test_case', TEST_CASES)
def test_parse_event(runner: Runner, test_case: TestCase) -> None:
    if test_case.needs_task:
        runner.run(3, [0x12345678, 7, int.from_bytes(b'MyTa', byteorder='little'), int.from_bytes(b'sk1', byteorder='little')])
        runner.run(1, [0x12345678])

    event_type, event_data = runner.run(test_case.event_id, test_case.payload)
    assert event_type == test_case.event_name
    assert event_data == test_case.expected


def test_fallback_to_tcb_address_if_no_name_not_known(runner: Runner) -> None:
    event_type, event_data = runner.run(1, [0x12345678])
    assert event_type == 'task_switched_in'
    assert event_data == {'TCB': '0x12345678', 'TaskName': '0x12345678'}


def test_task_switched_out_unknown_reason(runner: Runner) -> None:
    with pytest.raises(RuntimeError, match='.*'):
        runner.run(2, [0x12345678, (0xE << 28), 0xABCD1234, 0])
