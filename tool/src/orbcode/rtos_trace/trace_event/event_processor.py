from abc import ABC, abstractmethod

from orbcode.rtos_trace.trace_event import TimestampedEvent


class EventProcessor(ABC):
    @abstractmethod
    def process_pass_1(self, packet: TimestampedEvent) -> None:
        pass

    @abstractmethod
    def after_pass_1(self) -> None:
        pass

    @abstractmethod
    def process_pass_2(self, packet: TimestampedEvent) -> None:
        pass
