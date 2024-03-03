import argparse
from pathlib import Path
from typing import IO, Iterable

from orbcode.rtos_trace.rtos_processors.resolve_symbol_name import ResolveSymbolNameProcessor
from orbcode.rtos_trace.tool.generator import get_trace_events
from orbcode.rtos_trace.trace_event import EventProcessor
from orbcode.rtos_trace.rtos_events import RTOS_TRACE_PACKET_TYPES
from rich.progress import wrap_file

from orbcode.rtos_trace.tracecompass import export_tracecompass_format


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
    parser.add_argument('--trace-file', required=True, help='Trace file (by default: Orbuculum raw trace file without TPIU)',
                        type=open_trace_file)
    parser.add_argument('--output', required=True, help='Output file', type=argparse.FileType('w'))
    parser.add_argument('--symbol-resolver', required=False, help='Command to run symbol name resolver', nargs='+')
    return parser.parse_args()


def packet_dump_lines(raw_file: IO[bytes]) -> Iterable[str]:
    for line in raw_file:
        yield line.decode('utf-8').strip()


def do_main(args: argparse.Namespace) -> None:
    trace_file_size, trace_file_data = args.trace_file
    processors: list[EventProcessor] = []

    if args.symbol_resolver:
        processors.append(ResolveSymbolNameProcessor(
            resolver_command=args.symbol_resolver
        ))

    with wrap_file(trace_file_data, trace_file_size, description='Generating RTOS trace log...') as tracked_file_data:
        trace_events = get_trace_events(
            trace_file=tracked_file_data,
            event_types=RTOS_TRACE_PACKET_TYPES,
            processors=processors
        )

    export_tracecompass_format(trace_events, args.output)

    print('Input file for TraceCompass generated')


def main() -> None:
    do_main(parse_args())
