from typing import Optional
from orbcode.rtos_trace.rtos_events.task import TaskSwitchedIn, TaskSwitchedOut
from orbcode.rtos_trace.rtos_events.task_ref import TaskReference
from orbcode.rtos_trace.trace_event import EventProcessor
from orbcode.rtos_trace.trace_event.definition import TimestampedEvent


class CurrentTaskProcessor(EventProcessor):
    def __init__(self) -> None:
        self._current_task: Optional[TaskReference] = None

    def process_pass_1(self, packet: TimestampedEvent) -> None:
        if isinstance(packet.packet, TaskSwitchedIn):
            self._current_task = packet.packet.task
        elif isinstance(packet.packet, TaskSwitchedOut):
            self._current_task = None
        else:
            packet.extra_information['current_task'] = self._current_task

    def after_pass_1(self) -> None:
        pass

    def process_pass_2(self, packet: TimestampedEvent) -> None:
        del packet
