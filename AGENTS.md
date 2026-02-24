# AGENTS.md

This file gives Codex quick, high-signal context for working in this repo.

## Project Overview

`pokeemerald-expansion` is a GBA ROM-hack framework built on pret's `pokeemerald` decompilation. It provides modern Pokemon mechanics (Gen 1-9), plus QoL features for ROM-hack developers. It is not a standalone playable game; it is a base for building your own ROM hacks.

## Build Commands

```bash
make                           # Build ROM (pokeemerald.gba)
make -j$(nproc)                # Parallel build
make debug                     # Build with debug symbols (-Og -g)
make release                   # Optimized release build
```

## Testing

```bash
make check                     # Run all tests
make check -j$(nproc)          # Parallel test run
make check TESTS="Spikes"      # Run tests matching prefix
make check TESTS="*effect*"    # Run tests with pattern (infix)
make check TESTS="filename.c"  # Run tests from specific file
make pokeemerald-test.elf TESTS="Spikes"  # Build test ROM for visual inspection in mgba
```

### Test Structure

Tests use a DSL with three blocks:
- GIVEN: Initialize battle state (parties, abilities, items)
- WHEN: Define turns and actions
- SCENE: Verify observable outputs (animations, HP changes, messages)

Test macros: `SINGLE_BATTLE_TEST`, `DOUBLE_BATTLE_TEST`, `AI_SINGLE_BATTLE_TEST`, `WILD_BATTLE_TEST`

Tests auto-rig RNG so moves hit and effects activate unless specified otherwise.

## Repository Layout (high level)

- `src/` - C source files; major systems like `battle_*.c`, `field_*.c`, `pokemon_*.c`
- `include/` - headers; `include/config/` for feature toggles and `include/constants/` for constants
- `data/` - map layouts, encounters, compiled resources
- `graphics/` - sprites and tilesets
- `sound/` - audio assets (M4A, voicegroups, samples)
- `asm/` - assembly sources
- `test/` - battle and compression tests
- `docs/` - extended documentation, style guide, and system-specific notes

## Pokemon System Anatomy

- Core data + APIs: `include/pokemon.h` defines `struct BoxPokemon`, `struct Pokemon`, `struct SpeciesInfo`, `struct Evolution`, `struct LevelUpMove`, `struct FormChange`, and `GetMonData/SetMonData` helpers; creation and evolution logic live in `src/pokemon.c`.
- Species data: `src/data/pokemon/species_info.h` holds `gSpeciesInfo`; per-gen family tables are in `src/data/pokemon/species_info/gen_*_families.h`; shared dex text/anims in `src/data/pokemon/species_info/`.
- Learnsets: level-up lists in `src/data/pokemon/level_up_learnsets/gen_*.h`; egg moves in `src/data/pokemon/egg_moves.h`; TM/tutor teachables in `src/data/pokemon/teachable_learnsets.h` (auto-generated via `tools/learnset_helpers/make_teachables.py` when enabled).
- Forms: `src/data/pokemon/form_species_tables.h`, `src/data/pokemon/form_change_tables.h`, `src/data/pokemon/form_change_table_pointers.h`, plus `include/constants/form_change_types.h`.
- Moves/abilities/items: IDs in `include/constants/moves.h`, `include/constants/abilities.h`, `include/constants/items.h`; move data in `src/data/moves_info.h`; move effects in `src/data/battle_move_effects.h`; ability data in `src/data/abilities.h`; item effects in `src/data/pokemon/item_effects.h` and item data in `src/data/items.h`.
- Growth + storage: experience tables in `src/data/pokemon/experience_tables.h`; party globals in `include/pokemon.h`; storage UI in `src/pokemon_storage_system.c`; Pokedex screens in `src/pokedex*.c`.
- Pokemon configs: `include/config/pokemon.h` for generation rules and breeding/learnset toggles; `include/config/species_enabled.h` to enable/disable families.

## Code Style

### Naming Conventions
- Functions/Structs: `PascalCase`
- Variables/Fields: `camelCase`
- Global variables: prefix `g` (e.g., `gSaveBlock1`)
- Static variables: prefix `s` (e.g., `sMyStaticVar`)
- Macros/Constants: `CAPS_WITH_UNDERSCORES`

### Formatting
- C/H files: 4 spaces (no tabs)
- Assembly/Script files (.s, .inc): tabs
- Opening braces on next line for control structures
- Switch cases align with switch block (no extra indent)
- Single empty line after blocks

### Data Types
- Default to `u32`/`s32` for local variables
- Use the smallest type for: saveblock, EWRAM, and globals
- Prefer enums over magic numbers; use enum types in signatures

### Config Checks
Do config checks inline within normal control flow, not with preprocessor guards inside function bodies:

```c
// Correct
if (!B_VAR_DIFFICULTY)
    return;

// Incorrect
#ifdef B_VAR_DIFFICULTY
    return;
#endif
```

## Key Config Conventions

Generation-based behavior configs in `include/config/battle.h` use the `GEN_LATEST` pattern:

```c
#define B_CRIT_CHANCE GEN_LATEST  // Set to e.g., GEN_3 to lock behavior
```

Config philosophy:
- Save-modifying features: OFF by default, gated behind config
- Developer QoL or modern Pokemon emulation: ON by default
- All other configs: OFF by default

## Relic System (custom persistent battle modifiers)

See `docs/RELICS.md` for full details.

Important rules:
- Strings in data tables: use `COMPOUND_STRING("text")` for pointer fields (`const u8 *`). Never use `_("text")` there.
- Script macros: `goto_if_set` takes two args: `goto_if_set FLAG, LABEL` (do not split).
- Adding relics: update `include/constants/relics.h`, bump `RELIC_COUNT`, add per-tier effects arrays + definition in `src/data/relics.h`. There is a static assert for count mismatch.
- Testing relics: use `PARAMETRIZE`, `captureDamage`, `EXPECT_MUL_EQ` in `test/battle/relic/`. Test all 3 tiers.

## Project Direction (docs/GAMEDESIGN.md)

`docs/GAMEDESIGN.md` is a design bible for "Pokemon Emerald Herald: Bearer of the Curse". It describes the intended future path and priorities for this repo's romhack direction:

- Vision: a vanilla-plus Emerald experience with Soulsborne-inspired difficulty, replayability, and competitive-grade battles.
- Signature mechanic: procedural Relics (curates from a pool, seeded by Trainer ID), with upgrade tiers and rarity levels, plus Legacy Dungeons.
- Battle/encounter goals: competitive trainer teams (items/EVs/movesets), no mid-battle healing item spam, and early access to strong Pokemon.
- QoL priorities: P0/P1/P2 feature lists (e.g., infinite TMs/rare candies, HM removal, early move relearner, EV items).
- Roadmap phases: foundation (relic system), QoL/encounters, trainer teams + AI, legacy dungeons, polish/balance, then postgame.

## Contribution Notes

- Simple trunk-based workflow; main branch is sacred.
- Keep new code minimally invasive; isolate large additions in their own files.
- Mark unused functions with `UNUSED`.
