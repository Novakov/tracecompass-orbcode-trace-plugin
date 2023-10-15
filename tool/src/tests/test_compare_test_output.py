from io import StringIO
from pathlib import Path
from typing import IO

import pytest
from orbcode import pyorb
from orbcode.rtos_trace.trace_file import iterate_packets_from_trace
from orbcode.rtos_trace.generator import generate_rtos_trace_from_trace


def generate_output(raw_trace: IO[bytes], output: IO[str]) -> None:
    orb = pyorb.Orb(source=pyorb.orb_source_io(raw_trace))
    generate_rtos_trace_from_trace(iterate_packets_from_trace(orb), output)


TRACES = [p.name for p in Path(__file__).parent.glob('traces/*') if p.is_dir()]


@pytest.mark.parametrize('trace_name', TRACES)
def test_generate_expected_trace_output(trace_name: str) -> None:
    raw_trace_path = Path(__file__).parent / 'traces' / trace_name / 'raw.trace'
    expected_output_path = Path(__file__).parent / 'traces' / trace_name / 'output.txt'

    output = StringIO()

    with raw_trace_path.open('rb') as raw_trace:
        generate_output(raw_trace, output)

    assert output.getvalue() == expected_output_path.read_text()
