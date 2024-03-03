import typing

from orbcode import pyorb

from .definition import TimestampedEvent, TraceEvent, Field


def deserialize_trace_packet(data: list[pyorb.TraceMessage], event_type: type[TraceEvent]) -> TraceEvent:
    fields = {}

    for name, field_type in typing.get_type_hints(event_type, include_extras=True).items():
        field_decoders = [a for a in field_type.__metadata__ if isinstance(a, Field)]
        assert len(field_decoders) == 1, 'Field must be annotated with exactly one Field'
        field_decoder = field_decoders[0]

        retrieved_value = field_decoder.read(data)
        actual_type = typing.get_args(field_type)[0]
        actual_value = actual_type(retrieved_value)

        fields[name] = actual_value

    return event_type(**fields)


def deserialize_trace(
        trace_packets_raw: typing.Iterable[tuple[float, int, list[pyorb.TraceMessage]]],
        types_map: dict[int, type[TraceEvent]]
) -> typing.Iterable[TimestampedEvent]:
    for timestamp, event_type, payload in trace_packets_raw:
        packet_type = types_map.get(event_type)
        if packet_type is None:
            raise ValueError(f'Unknown trace packet type: {event_type}')

        deserialized = deserialize_trace_packet(payload, packet_type)

        yield TimestampedEvent(timestamp, deserialized, extra_information={})
