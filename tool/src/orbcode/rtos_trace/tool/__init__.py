import argparse
from typing import IO, Iterable
from orbcode import pyorb
from orbcode.rtos_trace.trace_file import iterate_packets_from_trace, packet_dump_to_trace_messages
from orbcode.rtos_trace.generator import generate_rtos_trace_from_trace


def parse_args():
    parser = argparse.ArgumentParser(description='Orbcode RTOS Trace generator')
    parser.add_argument('--trace-format', default='raw_trace', choices=['raw_trace', 'packet_dump'], help='Trace file format')
    parser.add_argument('--trace-file', required=True, help='Trace file (by default: Orbuculum raw trace file without TPIU)', type=argparse.FileType('rb'))
    parser.add_argument('--output', required=True, help='Output file', type=argparse.FileType('w'))
    return parser.parse_args()


def packet_dump_lines(raw_file: IO[bytes]) -> Iterable[str]:
    for line in raw_file:
        yield line.decode('utf-8').strip()

def get_trace_messages(args) -> Iterable[pyorb.TraceMessage]:
    if args.trace_format == 'raw_trace':
        return iterate_packets_from_trace(pyorb.Orb(source=pyorb.orb_source_io(args.trace_file)))
    elif args.trace_format == 'packet_dump':
        return packet_dump_to_trace_messages(packet_dump_lines(args.trace_file))
    else:
        raise ValueError(f'Unknown trace format: {args.trace_format}')

def do_main(args):
    trace_messages = get_trace_messages(args)
    generate_rtos_trace_from_trace(trace_messages, args.output)
    print('Input file for TraceCompass generated')


def main():
    do_main(parse_args())
