from typing import Iterable, Optional

from orbcode import pyorb
from orbcode.rtos_trace.trace_file import group_trace_packets_by_timestamp


def trace_messages_to_raw_events(messages: Iterable[pyorb.TraceMessage]) -> Iterable[tuple[float, int, list[pyorb.TraceMessage]]]:
    grouped_by_timestamp = group_trace_packets_by_timestamp(messages)

    last_timestamp = 0
    current_packet: tuple[Optional[int], Optional[list[pyorb.TraceMessage]]] = (None, None)

    for timestamp_msg, messages_in_group in grouped_by_timestamp:
        last_timestamp += timestamp_msg.timeInc

        for message in messages_in_group:
            if isinstance(message, pyorb.swMsg):
                if message.srcAddr == 3:
                    startstop = (message.value >> 8) & 0xFF
                    event_id = int(message.value & 0xFF)

                    if startstop == 0xBE:
                        # TODO: handle truncated packet (warning)
                        current_packet = (event_id, [])
                    elif startstop == 0xEF:
                        if current_packet[0] != event_id:
                            print(f'Mismatched packet start/stop (current = {current_packet[0]}, stop = {event_id})')
                            current_packet = (None, [])
                            continue

                        assert current_packet[1] is not None
                        yield (last_timestamp, event_id, current_packet[1])
                        current_packet = (None, None)

                if message.srcAddr == 2:
                    if current_packet[0] is None:
                        print('Packet data without start')
                        continue

                    assert current_packet[1] is not None
                    current_packet[1].append(message)

            if isinstance(message, pyorb.excMsg):
                yield (last_timestamp, 256 + message.eventType, [message])
