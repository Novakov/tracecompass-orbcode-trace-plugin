from io import StringIO
from pathlib import Path
from typing import IO, Iterable

import pytest
from orbcode import pyorb
from orbcode.rtos_trace.trace_file import iterate_packets_from_trace, generate_packet_dump, packet_dump_to_trace_messages
from orbcode.rtos_trace.generator import generate_rtos_trace_from_trace


def generate_output(raw_trace: IO[bytes], output: IO[str]) -> None:
    orb = pyorb.Orb(source=pyorb.orb_source_io(raw_trace))
    generate_rtos_trace_from_trace(iterate_packets_from_trace(orb), output)


def generate_packet_dump_from_trace_file(raw_trace: IO[bytes], output: IO[str]) -> None:
    orb = pyorb.Orb(source=pyorb.orb_source_io(raw_trace))

    trace_messages = iterate_packets_from_trace(orb)
    generate_packet_dump(trace_messages, output)


def generate_packet_dump_from_packet_dump(lines: Iterable[str], output: IO[str]) -> None:
    trace_messages = packet_dump_to_trace_messages(lines)
    generate_packet_dump(trace_messages, output)


@pytest.mark.parametrize('trace_name', ['task_notify'])
def test_generate_expected_trace_output(trace_name: str) -> None:
    raw_trace_path = Path(__file__).parent / 'traces' / trace_name / 'raw.trace'
    expected_output_path = Path(__file__).parent / 'traces' / trace_name / 'output.txt'

    output = StringIO()

    with raw_trace_path.open('rb') as raw_trace:
        generate_output(raw_trace, output)

    assert output.getvalue() == expected_output_path.read_text()


@pytest.mark.parametrize('trace_name', ['task_notify'])
def test_generate_expected_packet_dump_from_trace_file(trace_name: str) -> None:
    raw_trace_path = Path(__file__).parent / 'traces' / trace_name / 'raw.trace'
    expected_output_path = Path(__file__).parent / 'traces' / trace_name / 'packet_dump.txt'

    output = StringIO()

    with raw_trace_path.open('rb') as raw_trace:
        generate_packet_dump_from_trace_file(raw_trace, output)

    output_s = output.getvalue()
    expected_output_s = expected_output_path.read_text()
    assert len(output_s) == len(expected_output_s)
    assert output_s == expected_output_s


@pytest.mark.parametrize('trace_name', ['task_notify'])
def test_generate_expected_packet_dump_from_packet_dump(trace_name: str) -> None:
    packet_dump_path = Path(__file__).parent / 'traces' / trace_name / 'packet_dump.txt'

    output = StringIO()

    with packet_dump_path.open('r') as packet_dump:
        generate_packet_dump_from_packet_dump(packet_dump, output)

    output_s = output.getvalue()
    expected_output_s = packet_dump_path.read_text()
    assert len(output_s) == len(expected_output_s)
    assert output_s == expected_output_s
