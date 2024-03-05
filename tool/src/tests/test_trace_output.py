from pathlib import Path
import pytest
from _pytest.mark.structures import ParameterSet

from orbcode.rtos_trace.rtos_processors.current_task_processor import CurrentTaskProcessor
from orbcode.rtos_trace.tool.generator import get_trace_events
from orbcode.rtos_trace.rtos_events import RTOS_TRACE_PACKET_TYPES

from .snapshot import Snapshot


def get_test_traces() -> list[ParameterSet]:
    base = Path(__file__).parent / 'traces'
    return [
        pytest.param(p, id=str(p.relative_to(base)))
        for p in base.glob('*.trace')
    ]


@pytest.mark.parametrize('trace', get_test_traces())
def test_compare_trace_output(trace: Path, snapshot: Snapshot) -> None:
    with trace.open('rb') as trace_input:
        packets = get_trace_events(
            trace_file=trace_input,
            event_types=RTOS_TRACE_PACKET_TYPES,
            processors=[CurrentTaskProcessor()]
        )
        list(packets)

    assert packets == snapshot
