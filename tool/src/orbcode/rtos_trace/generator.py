from dataclasses import dataclass
from typing import IO, Iterable, List, Optional

from orbcode import pyorb

from .rtos_events import Collector, TraceEvent
from .trace_file import group_trace_packets_by_timestamp


@dataclass(frozen=True)
class Packet:
    timestamp: int
    event_id: int
    payload: list[int]


EVENT_ID_SHIFT = 16
EVENT_ID_MASK = 0xFFFF_FFFF


def enumerate_packets_from_trace(trace_messages: Iterable[pyorb.TraceMessage]) -> Iterable[Packet]:
    current_chunk: Optional[List[int]] = None
    grouped_by_timestamp = group_trace_packets_by_timestamp(trace_messages)
    cumulative_ts = 0

    for ts, msgs in grouped_by_timestamp:
        cumulative_ts += ts.timeInc

        for packet in msgs:
            if isinstance(packet, pyorb.excMsg):
                yield Packet(
                    timestamp=cumulative_ts,
                    event_id=200 + packet.eventType,
                    payload=[packet.exceptionNumber]
                )

            if not isinstance(packet, pyorb.swMsg):
                continue

            if packet.srcAddr != 2:
                continue

            if packet.value == 0xDEADBEEF:
                current_chunk = []
                continue

            if packet.value == 0xCAFEBABE:
                if current_chunk is not None:
                    [header, *payload] = current_chunk
                    event_id = (header >> EVENT_ID_SHIFT) & EVENT_ID_MASK

                    yield Packet(
                        timestamp=cumulative_ts,
                        event_id=event_id,
                        payload=payload
                    )
                continue

            if current_chunk is not None:
                current_chunk.append(packet.value)


def generate_rtos_trace_from_trace(trace_messages: Iterable[pyorb.TraceMessage], output: IO[str]) -> None:
    packets = enumerate_packets_from_trace(trace_messages)
    generate_rtos_trace(packets, output)


def generate_rtos_trace(packets: Iterable[Packet], output: IO[str]) -> None:
    collector = Collector()

    event_map = collector.event_map()

    for packet in packets:
        timestamp_s = packet.timestamp / 50e6

        handler = event_map.get(packet.event_id, None)
        if handler is not None:
            trace_event: TraceEvent = handler(*packet.payload)
            assert isinstance(trace_event, tuple)
            event_type, event_data = trace_event

            output.write(f'Timestamp: {timestamp_s:.6f} Event type: {event_type}\n')
            for k, v in event_data.items():
                output.write(f"    {k}: '{v}'\n")
        else:
            print(f'Unknown event {packet.event_id}')
