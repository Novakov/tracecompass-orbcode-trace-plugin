from typing import IO, Iterable, Sequence

from orbcode import pyorb


def iterate_packets_from_trace(trace: pyorb.Orb) -> Iterable[pyorb.TraceMessage]:
    while True:
        packet = trace.rx()
        if packet is None:
            break

        if isinstance(packet, pyorb.Empty):
            continue

        yield packet


def group_trace_packets_by_timestamp(trace_messages: Iterable[pyorb.TraceMessage]) -> Iterable[tuple[pyorb.TSMsg, Sequence[pyorb.TraceMessage]]]:
    current_ts = None
    current_group: list[pyorb.TraceMessage] = []

    for msg in trace_messages:
        if isinstance(msg, pyorb.TSMsg):
            if current_ts is not None:
                yield current_ts, current_group
            current_ts = msg
            current_group = []
        else:
            current_group.append(msg)

    if current_ts is not None:
        yield current_ts, current_group


def generate_packet_dump(trace_messages: Iterable[pyorb.TraceMessage], output: IO[str]) -> None:
    for msg in trace_messages:
        if not isinstance(msg, pyorb.swMsg):
            continue

        if msg.srcAddr != 2:
            continue

        as_str = hex(msg.value)[2:].rjust(2 * msg.len, '0').upper()
        output.write(f'0x{as_str}\n')


def packet_dump_to_trace_messages(lines: Iterable[str]) -> Iterable[pyorb.TraceMessage]:
    for line in lines:
        line = line.strip()
        if line == '':
            continue

        yield pyorb.swMsg(srcAddr=2, value=int(line, 16), len=(len(line) - 2) // 2)
