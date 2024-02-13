from dataclasses import dataclass

from .rtos_events import TraceEventData


@dataclass(frozen=True)
class Event:
    timestamp: float
    event_type: str
    event_data: TraceEventData
