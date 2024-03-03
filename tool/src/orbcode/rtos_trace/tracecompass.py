from typing import IO, Any, Iterable, get_type_hints

from orbcode.rtos_trace.trace_event import ExpandableField, TimestampedEvent


def write_field_value(name: str, value: Any, output: IO[str]) -> None:
    if isinstance(value, ExpandableField):
        for k, v in value.expand().items():
            write_field_value(f'{name}.{k}', v, output)
    else:
        output.write(f"    {name}: '{value}'\n")


def export_event(packet: TimestampedEvent, output: IO[str]) -> None:
    event_type_name = type(packet.packet).__name__
    output.write(f'Timestamp: {packet.timestamp / 48e6:.6f} Event type: {event_type_name}\n')

    for name in get_type_hints(type(packet.packet), include_extras=True).keys():
        value = getattr(packet.packet, name)
        write_field_value(name, value, output)

    for name, value in packet.extra_information.items():
        write_field_value(name, value, output)


def export_tracecompass_format(packets: Iterable[TimestampedEvent], output: IO[str]) -> None:
    for packet in packets:
        export_event(packet, output)
