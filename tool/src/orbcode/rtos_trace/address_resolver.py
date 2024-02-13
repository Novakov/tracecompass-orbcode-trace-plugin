import dataclasses
import subprocess
from typing import Optional
from .event import Event
from .rtos_events import AddressReference


class SymbolNameResolver:
    def __init__(self) -> None:
        self._symbol_names = {}
        self._addr2field: Optional[subprocess.Popen] = None

    def __enter__(self) -> 'SymbolNameResolver':
        self._addr2field = subprocess.Popen(
            args=[
                'D:/Coding/addr2field/build/build/execs/addr2field/Debug/addr2field.exe',
                'D:/Coding/orbuculum/tracecompass-rtos/demo/build/execs/binary_semaphore/binary_semaphore.elf',
                '-'
            ],
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            encoding='utf-8',
        )
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        self._addr2field.stdin.write('\n')
        self._addr2field.stdin.flush()
        # self._addr2field.stdout.read()
        self._addr2field.wait()

    def resolve_symbol_name(self, address: int) -> str:
        assert self._addr2field is not None
        self._addr2field.stdin.write(f'0x{address:08X}\n')
        self._addr2field.stdin.flush()
        response = self._addr2field.stdout.readline().strip()
        return response.split('-')[1].strip()
        # return response

class AddressResolver:
    def __init__(self, symbol_name_resolver: SymbolNameResolver) -> None:
        self._task_names = {}
        self._symbol_resolver = symbol_name_resolver

    def learn(self, event: Event) -> None:
        if event.event_type == 'task_created':
            self._task_names[int(event.event_data['TCB'], 16)] = event.event_data['TaskName']

    def resolve(self, event: Event) -> Event:
        result_event_data = {}
        for k, v in event.event_data.items():
            if isinstance(v, AddressReference) and v.object_type == 'TCB':
                result_event_data['TCB'] = f'0x{v.address:08X}'
                result_event_data['TaskName'] = self._task_names.get(v.address, 'Unknown')
            elif isinstance(v, AddressReference) and v.object_type == 'BinarySemaphore':
                result_event_data['Semaphore'] = f'0x{v.address:08X}'
                result_event_data['Semaphore.Name'] = self._symbol_resolver.resolve_symbol_name(v.address)
            else:
                result_event_data[k] = v

        return dataclasses.replace(event, event_data=result_event_data)
