from typing import IO, Iterable, Type
from orbcode import pyorb
from orbcode.rtos_trace.tool.decoder import trace_messages_to_raw_events

from orbcode.rtos_trace.trace_event import TraceEvent, TimestampedEvent
from orbcode.rtos_trace.trace_event.deserializer import deserialize_trace
from orbcode.rtos_trace.trace_event.event_processor import EventProcessor


def get_trace_events(trace_file: IO[bytes],
                     event_types: dict[int, Type[TraceEvent]], processors: list[EventProcessor]) -> Iterable[TimestampedEvent]:
    trace = pyorb.Orb(source=pyorb.orb_source_io(trace_file))

    trace_packets_raw = trace_messages_to_raw_events(trace.iterate_all_messages())
    trace_events = list(deserialize_trace(trace_packets_raw, event_types))

    for packet in trace_events:
        for processor in processors:
            processor.process_pass_1(packet)

    for processor in processors:
        processor.after_pass_1()

    for packet in trace_events:
        for processor in processors:
            processor.process_pass_2(packet)

    return trace_events
