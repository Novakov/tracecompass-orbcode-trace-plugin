from typing import Iterable, Sequence

from orbcode import pyorb


def group_trace_packets_by_timestamp(trace_messages: Iterable[pyorb.TraceMessage]) -> Iterable[tuple[pyorb.TSMsg, Sequence[pyorb.TraceMessage]]]:
    current_ts = None
    current_group: list[pyorb.TraceMessage] = []

    for msg in trace_messages:
        if isinstance(msg, pyorb.TSMsg):
            yield msg, current_group
            current_group = []
        else:
            current_group.append(msg)

    if current_ts is not None:
        yield None, current_group
