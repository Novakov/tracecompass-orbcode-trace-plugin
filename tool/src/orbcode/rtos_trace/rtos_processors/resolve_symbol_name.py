
import typing
from orbcode.rtos_trace.address_resolver import SymbolNameResolver
from orbcode.rtos_trace.rtos_events.symbol_ref import SymbolRef
from orbcode.rtos_trace.trace_event import EventProcessor
from orbcode.rtos_trace.trace_event.definition import TimestampedEvent


class ResolveSymbolNameProcessor(EventProcessor):
    def __init__(self, *, resolver_command: list[str]) -> None:
        self._resolver_command = resolver_command
        self._addresses: set[int] = set()
        self._resolved_names: dict[int, str] = {}

    def process_pass_1(self, packet: TimestampedEvent) -> None:
        # TODO: this is naive approach
        for field_name in typing.get_type_hints(packet.packet).keys():
            field_value = getattr(packet.packet, field_name)

            if isinstance(field_value, SymbolRef):
                self._addresses.add(field_value.address)

    def after_pass_1(self) -> None:
        with SymbolNameResolver(resolver_command=self._resolver_command) as address_resolver:
            for symbol in self._addresses:
                resolved = address_resolver.resolve_symbol_name(symbol)
                if resolved is not None:
                    self._resolved_names[symbol] = resolved

    def process_pass_2(self, packet: TimestampedEvent) -> None:
        for field_name in typing.get_type_hints(packet.packet).keys():
            field_value = getattr(packet.packet, field_name)

            if not isinstance(field_value, SymbolRef):
                continue

            name = self._resolved_names.get(field_value.address, None)
            if name is None:
                continue

            field_value.set_name(name)
