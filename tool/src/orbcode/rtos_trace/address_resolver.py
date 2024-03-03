import subprocess
from typing import Any, Optional


class SymbolNameResolver:
    def __init__(self, *, resolver_command: list[str]) -> None:
        self._resolver_command = resolver_command
        self._addr2field: Optional[subprocess.Popen[str]] = None

    def __enter__(self) -> 'SymbolNameResolver':
        self._addr2field = subprocess.Popen(
            args=self._resolver_command,
            stdout=subprocess.PIPE,
            stdin=subprocess.PIPE,
            encoding='utf-8',
        )
        return self

    def __exit__(self, exc_type: Any, exc_value: Any, traceback: Any) -> None:
        del exc_type, exc_value, traceback
        process = self._addr2field
        assert process is not None
        assert process.stdin is not None
        assert process.stdout is not None
        process.stdin.write('\n')
        process.stdin.flush()
        process.wait()

    def resolve_symbol_name(self, address: int) -> Optional[str]:
        process = self._addr2field
        assert process is not None
        assert process.stdin is not None
        assert process.stdout is not None
        process.stdin.write(f'0x{address:08X}\n')
        process.stdin.flush()
        response = process.stdout.readline().strip()
        name = response.split('-')[1].strip()
        if name == 'Failed to match address to symbol':
            return None
        return name
