---
name: curse-authoring
description: "Create, edit, and test curse (boon/bane) definitions in this repo. Use when adding a new curse, adjusting curse selectors/multipliers, updating curse data tables, or writing battle tests for curses."
---

# Curse Authoring

## Overview
Add or modify curse definitions, effects, and tests using the existing curse system.

## Workflow
1. Add the curse ID in `include/constants/curses.h` and bump `CURSE_COUNT`.
2. Define the effect array in `src/data/curses.h` with selector and multiplier.
3. Add a `CurseDef` entry in `sCurseDefs` with `COMPOUND_STRING` name/description.
4. Write a battle test in `test/battle/curse/` that covers positive and negative cases.
5. Run `make check TESTS="<CurseName>" -j$(nproc)`.

## Data Locations
- `include/constants/curses.h` (IDs, enums, slots)
- `src/data/curses.h` (effects + definitions)
- `docs/CURSES.md` (canonical guidance and macros)
- `test/battle/curse/` (battle tests)

## Effect / Selector Rules
- Use `CURSE_BOON_` and `CURSE_BANE_` prefixes.
- Use `CURSE_REDUCTION_PCT()` and `CURSE_MULT_PCT()` helpers for multipliers.
- Selector fields: `side`, `moveType`, `moveCategory`, `minHpPct`.
- `TYPE_NONE` and `CURSE_MOVE_CATEGORY_ANY` mean "any".
- Use `ARRAY_COUNT(...)` for `effectCount`.

## Gotchas
- Use `COMPOUND_STRING("text")` for name/description fields (not `_()` macros).
- `CURSE_COUNT` mismatch triggers `CurseDefsCountMismatch`.
- Only `CURSE_STACK_MULTIPLY` is implemented today.
- For banes in tests, either add a `Curse_SetActiveBane` helper (mirroring boons) or set `gSaveBlock2Ptr->curses.activeBanes[]` in `GIVEN`.

## Scripts

### `scripts/new_curse_stub.py`
Generate a stub effect array, `CurseDef` entry, and battle test skeleton. Run with `uv`:

```bash
uv run python scripts/new_curse_stub.py --id CURSE_BOON_PYROWARD --name Pyroward --desc "Your party takes 50% less Fire damage." --move MOVE_EMBER --move-type TYPE_FIRE --reduction-pct 50
```

Optionally write the test file directly:

```bash
uv run python scripts/new_curse_stub.py --id CURSE_BOON_PYROWARD --name Pyroward --write-test test/battle/curse/pyroward.c
```

## Test Pattern (minimal)
```c
SINGLE_BATTLE_TEST("<Name> applies", s16 damage)
{
    bool32 cursed;
    PARAMETRIZE { cursed = FALSE; }
    PARAMETRIZE { cursed = TRUE; }
    GIVEN {
        if (cursed)
            Curse_SetActiveBoon(0, CURSE_BOON_<NAME>);
        else
            Curse_ClearActive();
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_<MOVE>); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(<multiplier>), results[1].damage);
    }
}
```
