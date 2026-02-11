---
name: curse-engine
description: "Extend or refactor the curse runtime: add new effect types, stacking rules, selector logic, save data, or battle hooks. Use when changing Curse_* APIs or integrating curses into new mechanics."
---

# Curse Engine

## Overview
Modify how curses are evaluated and applied at runtime, including new effect types and battle hooks.

## Key Files
- `include/constants/curses.h` (effect types, stacking rules, slots)
- `include/curse.h` (structs, APIs, save data)
- `src/curse.c` (selector matching, effect application)
- `src/battle_util.c` (damage pipeline hook)
- `src/field_specials.c` (default init via `Special_InitCurses`)

## Adding a New Effect Type
1. Add the enum value in `include/constants/curses.h`.
2. Extend `struct CurseEffect` / `struct CurseSelector` in `include/curse.h` if new fields are required.
3. Update `CurseSelectorMatches` or add new matching helpers in `src/curse.c`.
4. Extend `ApplyCurseEffects` to apply the new type and stacking rule.
5. Add a public API in `include/curse.h` (e.g., `Curse_GetAttackModifier`) and implement in `src/curse.c`.
6. Wire the modifier into the correct system (often `src/battle_util.c`).
7. Add battle tests in `test/battle/curse/` for positive and negative cases.

## Stacking Rules
Only `CURSE_STACK_MULTIPLY` is implemented currently. If you add or rely on other rules, implement them in `ApplyCurseEffects` with explicit behavior and tests.

## Fixed-Point Notes
Multipliers use `uq4_12_t` (see `fpmath.h`). Initialize modifiers with `UQ_4_12(1.0)` and use `uq4_12_multiply` or `uq4_12_multiply_by_int_half_down` when combining.

## API / Save Data Conventions
- Keep `struct CursesSaveData` stable unless you are explicitly handling save migration.
- If you change save layout, update init paths and ensure defaults are applied in new games.
