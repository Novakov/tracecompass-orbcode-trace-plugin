from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Any
from orbcode import pyorb


class TraceEvent(ABC):
    pass


@dataclass(frozen=True)
class TimestampedEvent:
    timestamp: float
    packet: TraceEvent
    extra_information: dict[str, Any]


class Field(ABC):
    @abstractmethod
    def read(self, data: list[pyorb.TraceMessage]) -> Any:
        pass


class ExpandableField(ABC):
    @abstractmethod
    def expand(self) -> dict[str, Any]:
        pass
