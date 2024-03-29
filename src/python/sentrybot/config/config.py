import os
from dataclasses import dataclass
from typing import Any, List, Optional, Mapping


class Config(dict):
    def __init__(self, **entries):
        super().__init__(**entries)

    def __getattr__(self, item: str) -> Any:
        return self[item]

    def __setattr__(self, key: str, value: Any) -> None:
        self[key] = value

    def __str__(self) -> str:
        return '\n'.join([
            '{',
            *self._get_entry_lines(),
            '}'
        ])

    def _get_entry_lines(self) -> List[str]:
        indent = '    '
        lines = []

        for k, v in self.items():
            if isinstance(v, Config):
                lines.append(f'{indent}{k}={{')
                lines.extend(indent + sub_line for sub_line in v._get_entry_lines())
                lines.append(f'{indent}}},')
            else:
                lines.append(f'{indent}{k}={v!r},')

        return lines

    def update(self, other: Mapping, **kwargs) -> None:
        other = {**other, **kwargs}

        for k in other:
            self_v = self.get(k, None)
            other_v = other[k]

            if isinstance(self_v, Config) and isinstance(other_v, Config):
                self_v.update(other_v)
            else:
                self[k] = other_v


@dataclass(repr=False)
class Secret:
    """
    Just a class that wraps a value and does not display it when
    str(...) or repr(...) are used.
    """

    value: Any

    def __repr__(self) -> str:
        return '<SECRET>'


def env_var(name: str, default=None, required: bool = False) -> Optional[str]:
    """
    Returns the value of the environment variable with the given name,
    or ``default`` if it is not set.

    :raises AssertionError: if ``required`` is ``True`` and the environment variable
        is not set.
    """

    value = os.getenv(name)

    if value is not None:
        return value
    elif required:
        raise AssertionError(f'Required env var is not set. name={name}')
    else:
        return default
