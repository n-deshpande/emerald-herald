# Export Tools

This directory contains small utilities that export game data from the C source of truth.

## Setup (uv)

```sh
uv venv
uv sync --extra export
```

## Curses Export

Generate JSON + Markdown:

```sh
uv run python tools/export/export_curses.py --json exports/curses.json --md exports/curses.md
```

Validate registry consistency:

```sh
uv run python tools/export/export_curses.py --check
```

### Output schema

- `exports/curses.json`
  - `generated_at`
  - `source_files`
  - `curses[]`

- `exports/curses.md`
  - Human-readable summary per curse
