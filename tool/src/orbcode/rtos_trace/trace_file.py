from typing import IO, Iterable

from orbcode import pyorb


def iterate_packets_from_trace(trace: pyorb.Orb) -> Iterable[pyorb.TraceMessage]:
    while True:
        packet = trace.rx()
        if packet is None:
            break

        if isinstance(packet, pyorb.Empty):
            continue

        yield packet


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
