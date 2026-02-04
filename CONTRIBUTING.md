# Contributing to Emerald Herald

This is a standalone fork maintained by the project owner. Contributions are welcome, but please start with a short discussion before investing in a large change.

## Before You Start

- Open an issue or discussion describing the change and why it fits the design goals in `docs/GAMEDESIGN.md`.
- Keep changes focused. Large refactors should be proposed first.
- Do not open upstream issues for Emerald Herald changes.

## Bug Reports

Please include:
- Repro steps and expected vs. actual behavior
- Your build target (`modern`), platform, and toolchain
- Any relevant save files or seed/Curse details
- Screenshots or logs when helpful

## Development Guidelines

- Follow the code style in `docs/STYLEGUIDE.md`.
- Use `AGENTS.md` and `CLAUDE.md` for build/test commands and AI-specific guidance.
- Add or update tests when you change battle logic or core systems.
- Prefer small, reviewable commits.

## Pull Requests

- Keep PRs narrow and clearly scoped.
- Describe what changed, why, and any balance implications.
- Include testing notes (`make`, `make check`, or in-game verification).
