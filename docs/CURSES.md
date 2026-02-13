# Curse System

Curses are persistent modifiers that affect battle mechanics. They come in two flavors:

- **Boons** — beneficial effects (e.g. damage reduction)
- **Banes** — detrimental effects (e.g. damage increase)

A player can have up to 3 active boons and 3 active banes at a time, stored in the save block.

## File Map

| File | Purpose |
|------|---------|
| `include/constants/curses.h` | Curse IDs, enums, slot counts |
| `include/curse.h` | Struct definitions and public API |
| `src/curse.c` | Runtime logic (selector matching, modifier application) |
| `src/data/curses.h` | Curse data tables (effects, names, descriptions) |
| `src/field_specials.c` | `Special_InitCurses` — called from map scripts |
| `test/battle/curse/` | Battle tests for each curse |

## Anatomy of a Curse

A curse is made up of three layers:

```
CurseDef            (name, description, pointer to effects)
  -> CurseEffect[]  (what it does: effect type, multiplier, stacking rule)
       -> CurseSelector  (when it applies: which side, move type, move category, HP threshold)
```

### CurseDef

Top-level definition. One entry per curse in the `sCurseDefs` table.

```c
struct CurseDef
{
    const u8 *name;                  // Display name (use COMPOUND_STRING)
    const u8 *description;           // Tooltip text (use COMPOUND_STRING)
    const struct CurseEffect *effects; // Array of effects
    u8 effectCount;                  // Length of effects array
};
```

### CurseEffect

A single mechanical effect. A curse can have multiple effects.

```c
struct CurseEffect
{
    u8 type;                    // What the effect modifies (see CurseEffectType)
    u8 stacking;               // How it combines with other modifiers (see CurseStacking)
    struct CurseSelector selector; // Conditions for when it applies
    uq4_12_t multiplier;       // The modifier value (fixed-point 4.12)
};
```

**Effect types** (`CurseEffectType`):

| Value | Meaning |
|-------|---------|
| `CURSE_EFF_DAMAGE_TAKEN_MULT` | Multiplies damage taken by the target |

**Stacking rules** (`CurseStacking`):

| Value | Meaning |
|-------|---------|
| `CURSE_STACK_MULTIPLY` | Multiplies into the running modifier |
| `CURSE_STACK_ADD_PCT` | Adds percentage points (not yet implemented in engine) |
| `CURSE_STACK_MAX` | Takes the larger value (not yet implemented in engine) |
| `CURSE_STACK_MIN` | Takes the smaller value (not yet implemented in engine) |
| `CURSE_STACK_OVERRIDE` | Replaces the modifier entirely (not yet implemented in engine) |

### CurseSelector

Controls which battle situations the effect applies to.

```c
struct CurseSelector
{
    u8 side;            // Who is being hit
    u8 moveType;        // Filter by move type (e.g. TYPE_FIRE), or TYPE_NONE for any
    u8 moveCategory;    // Filter by category (DAMAGE_CATEGORY_PHYSICAL, etc.), or CURSE_MOVE_CATEGORY_ANY
    u8 minHpPct;        // Only applies if defender HP >= this % (0 = always)
};
```

**Side values** (`CurseSide`):

| Value | Meaning |
|-------|---------|
| `CURSE_SIDE_PLAYER` | Only when the player's Pokemon is the target |
| `CURSE_SIDE_OPPONENT` | Only when the opponent's Pokemon is the target |
| `CURSE_SIDE_BOTH` | Applies regardless of which side is hit |

## How to Add a New Curse

### Step 1: Define the ID

In `include/constants/curses.h`, add a new ID and bump `CURSE_COUNT`:

```c
#define CURSE_NONE              0
#define CURSE_BOON_BULWARK      1
#define CURSE_BOON_PYROWARD     2  // <- new
#define CURSE_COUNT             3  // <- bumped
```

Use the `CURSE_BOON_` prefix for boons and `CURSE_BANE_` for banes.

### Step 2: Define the effects array

In `src/data/curses.h`, add an effects array above the `sCurseDefs` table:

```c
static const struct CurseEffect sCurseEffects_Pyroward[] =
{
    {
        .type = CURSE_EFF_DAMAGE_TAKEN_MULT,
        .stacking = CURSE_STACK_MULTIPLY,
        .selector =
        {
            .side = CURSE_SIDE_PLAYER,
            .moveType = TYPE_FIRE,              // only fire moves
            .moveCategory = CURSE_MOVE_CATEGORY_ANY,
            .minHpPct = 0,
        },
        .multiplier = CURSE_REDUCTION_PCT(50),  // 50% less fire damage
    },
};
```

**Multiplier helpers** (defined at the top of `src/data/curses.h`):

| Macro | Usage | Example |
|-------|-------|---------|
| `CURSE_REDUCTION_PCT(pct)` | Take `pct`% less damage | `CURSE_REDUCTION_PCT(20)` = 0.8x |
| `CURSE_MULT_PCT(pct)` | Multiply damage by `pct`% | `CURSE_MULT_PCT(150)` = 1.5x |

### Step 3: Add the CurseDef entry

In the `sCurseDefs` table in `src/data/curses.h`:

```c
[CURSE_BOON_PYROWARD] =
{
    .name = COMPOUND_STRING("Pyroward"),
    .description = COMPOUND_STRING("Your party takes 50% less Fire damage."),
    .effects = sCurseEffects_Pyroward,
    .effectCount = ARRAY_COUNT(sCurseEffects_Pyroward),
},
```

Use `COMPOUND_STRING()` for both `name` and `description` (not `_()`).
The `_()` macro produces a brace initializer that only works for array fields, not pointers.

### Step 4: Write a test

Create `test/battle/curse/pyroward.c`:

```c
#include "global.h"
#include "test/battle.h"
#include "curse.h"

SINGLE_BATTLE_TEST("Pyroward reduces Fire damage taken by 50%", s16 damage)
{
    bool32 cursed;
    PARAMETRIZE { cursed = FALSE; }
    PARAMETRIZE { cursed = TRUE; }
    GIVEN {
        if (cursed)
            Curse_SetActiveBoon(0, CURSE_BOON_PYROWARD);
        else
            Curse_ClearActive();
        ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_EMBER); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.5), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Pyroward does not affect non-Fire damage", s16 damage)
{
    bool32 cursed;
    PARAMETRIZE { cursed = FALSE; }
    PARAMETRIZE { cursed = TRUE; }
    GIVEN {
        if (cursed)
            Curse_SetActiveBoon(0, CURSE_BOON_PYROWARD);
        else
            Curse_ClearActive();
        ASSUME(GetMoveType(MOVE_SCRATCH) != TYPE_FIRE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}
```

The test pattern:
1. `PARAMETRIZE` runs the battle twice (with and without the curse)
2. `GIVEN` sets up curse state via the C API directly (`Curse_SetActiveBoon` / `Curse_ClearActive`)
3. The opponent attacks the player
4. `captureDamage` records the HP lost each run
5. `FINALLY` compares: `EXPECT_MUL_EQ(baseline, expected_multiplier, actual)`

Always test both the positive case (effect applies) and negative case (effect should not apply).

### Step 5: Run

```bash
make check TESTS="Pyroward" -j$(nproc)
```

## Test Checklist for New Curses

- [ ] Positive case: effect applies with expected multiplier
- [ ] Negative case: effect does NOT apply when selector doesn't match (wrong type, wrong side, etc.)
- [ ] If the curse has `minHpPct`: test at/above and below the threshold
- [ ] Banes: use `Curse_ClearActive()` then set via the bane slot API when it exists

## Gotchas

| Pitfall | Fix |
|---------|-----|
| Using `_("text")` for name/description fields | Use `COMPOUND_STRING("text")` — struct fields are pointers, not arrays |
| Forgetting to bump `CURSE_COUNT` | Build will fail with `CurseDefsCountMismatch` static assert |
| Using `goto_if_set LABEL` in scripts | `goto_if_set` takes two args: `goto_if_set FLAG, LABEL` |
| Adding a new stacking rule | Must handle it in `ApplyCurseEffects` in `src/curse.c` |
| Changing `CursesSaveData` (adding slots, new fields) | Update `T_SAVEBLOCK2_SIZE` in `test/save.c` to match the new `sizeof(struct SaveBlock2)` |
