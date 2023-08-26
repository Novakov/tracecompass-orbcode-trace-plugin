from dataclasses import dataclass
from typing import IO, Iterable

from orbcode import pyorb

from .rtos_events import Collector

@dataclass(frozen=True)
class Packet:
    timestamp: int
    event_id: int
    payload: list[int]


EVENT_ID_SHIFT = 28
EVENT_ID_MASK = 0b1111
CYCCNT_MASK = 0x0FFF_FFFF

def enumerate_packets_from_trace(trace_messages: Iterable[pyorb.TraceMessage]) -> Iterable[Packet]:
    current_chunk = None

    for packet in trace_messages:
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
                cycle_counter = (header & CYCCNT_MASK)

                yield Packet(
                    timestamp=cycle_counter,
                    event_id=event_id,
                    payload=payload
                )
            continue

        if current_chunk is not None:
            current_chunk.append(packet.value)

def generate_rtos_trace_from_trace(trace_messages: Iterable[pyorb.TraceMessage], output: IO[bytes]) -> None:
    packets = enumerate_packets_from_trace(trace_messages)
    generate_rtos_trace(packets, output)


def generate_rtos_trace(packets: Iterable[Packet], output: IO[bytes]) -> None:
    cumulative_timestamp = 0
    last_timestamp = 0

    collector = Collector()

    event_map = collector.event_map()

    for packet in packets:
        if packet.timestamp >= last_timestamp:
            cumulative_timestamp += packet.timestamp - last_timestamp
        else:
            cumulative_timestamp += CYCCNT_MASK - last_timestamp + packet.timestamp

        last_timestamp = packet.timestamp

        timestamp_s = cumulative_timestamp / 50e6

        if packet.event_id in event_map:
            handler = event_map.get(packet.event_id)

            event_type, event_data = handler(*packet.payload)

            output.write(f'Timestamp: {timestamp_s:.6f} Event type: {event_type}\n')
            for k, v in event_data.items():
                output.write(f"    {k}: '{v}'\n")
        else:
            print(f'Unknown event {packet.event_id}')
