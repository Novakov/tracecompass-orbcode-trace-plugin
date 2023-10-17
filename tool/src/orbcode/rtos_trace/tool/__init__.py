import argparse
from pathlib import Path
from typing import IO, Iterable
from orbcode import pyorb
from orbcode.rtos_trace.trace_file import iterate_packets_from_trace, packet_dump_to_trace_messages
from orbcode.rtos_trace.generator import generate_rtos_trace_from_trace
from rich.progress import wrap_file


def open_trace_file(file_name: str) -> tuple[int, IO[bytes]]:
    try:
        p = Path(file_name)

        return p.stat().st_size, p.open('rb')
    except OSError as e:
        args = {'filename': file_name, 'error': e}
        message = "can't open '%(filename)s': %(error)s"
        raise argparse.ArgumentTypeError(message % args)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='Orbcode RTOS Trace generator')
    parser.add_argument('--trace-format', default='raw_trace', choices=['raw_trace', 'packet_dump'], help='Trace file format')
    parser.add_argument('--trace-file', required=True, help='Trace file (by default: Orbuculum raw trace file without TPIU)',
                        type=open_trace_file)
    parser.add_argument('--output', required=True, help='Output file', type=argparse.FileType('w'))
    return parser.parse_args()


def packet_dump_lines(raw_file: IO[bytes]) -> Iterable[str]:
    for line in raw_file:
        yield line.decode('utf-8').strip()


def get_trace_messages(format: str, trace_file: IO[bytes]) -> Iterable[pyorb.TraceMessage]:
    if format == 'raw_trace':
        return iterate_packets_from_trace(pyorb.Orb(source=pyorb.orb_source_io(trace_file)))
    elif format == 'packet_dump':
        return packet_dump_to_trace_messages(packet_dump_lines(trace_file))
    else:
        raise ValueError(f'Unknown trace format: {format}')


def do_main(args: argparse.Namespace) -> None:
    trace_file_size, trace_file_data = args.trace_file
    with wrap_file(trace_file_data, trace_file_size, description='Generating RTOS trace log...') as tracked_file_data:
        trace_messages = get_trace_messages(args.trace_format, tracked_file_data)
        generate_rtos_trace_from_trace(trace_messages, args.output)
    print('Input file for TraceCompass generated')


def main() -> None:
    do_main(parse_args())
