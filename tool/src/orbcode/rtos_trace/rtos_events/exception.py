from dataclasses import dataclass
from typing import Annotated, Any
from orbcode import pyorb
from orbcode.rtos_trace.trace_event import TraceEvent, Field


class ExceptionNumber(Field):
    def read(self, data: list[pyorb.TraceMessage]) -> Any:
        assert len(data) == 1
        [msg] = data
        assert isinstance(msg, pyorb.excMsg)

        return msg.exceptionNumber


@dataclass(frozen=True)
class ExceptionEntered(TraceEvent):
    exception_number: Annotated[int, ExceptionNumber()]


@dataclass(frozen=True)
class ExceptionExited(TraceEvent):
    exception_number: Annotated[int, ExceptionNumber()]


@dataclass(frozen=True)
class ExceptionReturned(TraceEvent):
    exception_number: Annotated[int, ExceptionNumber()]
