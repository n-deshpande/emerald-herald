from __future__ import annotations

from pathlib import Path
import re


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def _find_matching_brace(text: str, open_idx: int) -> int:
    depth = 0
    in_str = None
    escape = False
    for i in range(open_idx, len(text)):
        ch = text[i]
        if in_str is not None:
            if escape:
                escape = False
            elif ch == "\\":
                escape = True
            elif ch == in_str:
                in_str = None
            continue
        if ch in ("\"", "'"):
            in_str = ch
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return i
    raise ValueError("No matching brace found")


def extract_braced_block(text: str, start_idx: int) -> tuple[int, int, str]:
    open_idx = text.find("{", start_idx)
    if open_idx == -1:
        raise ValueError("No opening brace found")
    close_idx = _find_matching_brace(text, open_idx)
    return open_idx, close_idx, text[open_idx : close_idx + 1]


def split_top_level_braces(text: str) -> list[str]:
    blocks: list[str] = []
    i = 0
    while True:
        open_idx = text.find("{", i)
        if open_idx == -1:
            break
        close_idx = _find_matching_brace(text, open_idx)
        blocks.append(text[open_idx : close_idx + 1])
        i = close_idx + 1
    return blocks


def get_simple_field(block: str, field: str) -> str | None:
    pattern = re.compile(r"\\." + re.escape(field) + r"\\s*=\\s*([^,}]+)")
    match = pattern.search(block)
    if not match:
        return None
    return match.group(1).strip()


def extract_string_literal(expr: str) -> str | None:
    if expr is None:
        return None
    start = expr.find('"')
    if start == -1:
        return None
    i = start + 1
    out = []
    escape = False
    while i < len(expr):
        ch = expr[i]
        if escape:
            out.append("\\" + ch)
            escape = False
        elif ch == "\\":
            escape = True
        elif ch == '"':
            return "".join(out)
        else:
            out.append(ch)
        i += 1
    return None


def maybe_int(value: str | None) -> int | None:
    if value is None:
        return None
    value = value.strip()
    if re.fullmatch(r"\d+", value):
        return int(value)
    return None
