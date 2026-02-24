# Relic System

Relics are persistent modifiers that affect battle mechanics. A player can have up to 5 active relics at a time, stored in the save block. Each relic has 3 upgrade tiers (Tier 1/2/3) and a rarity level (Common/Uncommon/Rare).

## File Map

| File | Purpose |
|------|---------|
| `include/constants/relics.h` | Relic IDs, enums, slot counts, tier/rarity constants |
| `include/relic.h` | Struct definitions and public API |
| `src/relic.c` | Runtime logic (effect matching, modifier application) |
| `src/data/relics.h` | Relic data tables (effects, names, descriptions per tier) |
| `src/field_specials.c` | `Special_InitRelics` — called from map scripts |
| `test/battle/relic/` | Battle tests for each relic |

## Anatomy of a Relic

A relic is made up of three layers:

```
RelicDef            (name, rarity, per-tier descriptions, per-tier effects)
  -> RelicEffect[]  (what it does: effect type + stacking + pointer to typed params)
       -> Type-specific params struct (when/how it applies)
```

### RelicDef

Top-level definition. One entry per relic in the `sRelicDefs` table.

```c
struct RelicDef
{
    const u8 *name;                          // Display name (use COMPOUND_STRING)
    const u8 *descriptions[RELIC_NUM_TIERS]; // Per-tier tooltip text
    const struct RelicEffect *effects[RELIC_NUM_TIERS]; // Per-tier effects
    u8 effectCount[RELIC_NUM_TIERS];         // Per-tier effect counts
    u8 rarity;                               // RELIC_RARITY_COMMON/UNCOMMON/RARE
};
```

### RelicEffect

A single mechanical effect. A relic can have multiple effects per tier.

```c
struct RelicEffect
{
    u8 type;            // What the effect modifies (see RelicEffectType)
    u8 stacking;        // How it combines with other modifiers (see RelicStacking)
    const void *params; // Pointer to type-specific params struct
};
```

**Effect types** (`RelicEffectType`):

| Value | Meaning |
|-------|---------|
| `RELIC_EFF_DAMAGE_TAKEN_MULT` | Multiplies damage taken by the target |
| `RELIC_EFF_ACCURACY_FLAT_BONUS` | Adds a flat bonus to move base accuracy (capped at 100) |

**Stacking rules** (`RelicStacking`):

| Value | Meaning |
|-------|---------|
| `RELIC_STACK_MULTIPLY` | Multiplies into the running modifier |
| `RELIC_STACK_ADD_PCT` | Adds to the running value (used by accuracy bonuses) |
| `RELIC_STACK_MAX` | Takes the larger value (not yet implemented in engine) |
| `RELIC_STACK_MIN` | Takes the smaller value (not yet implemented in engine) |
| `RELIC_STACK_OVERRIDE` | Replaces the modifier entirely (not yet implemented in engine) |

### Type-specific params

Each effect type owns its own params struct. Current battle effect params:

```c
struct RelicBattleDamageTakenParams
{
    u8 side;
    u8 moveType;
    u8 moveCategory;
    u8 minHpPct;
    u8 typeMatchup;     // see RelicTypeMatchup
    uq4_12_t multiplier;
};

struct RelicBattleAccuracyFlatParams
{
    u8 side;
    u8 moveType;
    u8 moveCategory;
    u8 minHpPct;
    s16 flatBonus;
};
```

**Side values** (`RelicSide`):

| Value | Meaning |
|-------|---------|
| `RELIC_SIDE_PLAYER` | Only when the relevant battler is on the player's side |
| `RELIC_SIDE_OPPONENT` | Only when the relevant battler is on the opponent's side |
| `RELIC_SIDE_BOTH` | Applies regardless of side |

**Type matchup values** (`RelicTypeMatchup`):

| Value | Meaning |
|-------|---------|
| `RELIC_TYPE_MATCHUP_ANY` | Ignore type effectiveness |
| `RELIC_TYPE_MATCHUP_SUPER_EFFECTIVE_ONLY` | Only apply when effectiveness is > 1.0 |
| `RELIC_TYPE_MATCHUP_NOT_VERY_EFFECTIVE_ONLY` | Only apply when effectiveness is > 0 and < 1.0 |
| `RELIC_TYPE_MATCHUP_NEUTRAL_ONLY` | Only apply when effectiveness is exactly 1.0 |

## How to Add a New Relic

### Step 1: Define the ID

In `include/constants/relics.h`, add a new ID and bump `RELIC_COUNT`:

```c
#define RELIC_NONE              0
#define RELIC_BULWARK           1
#define RELIC_PYROWARD          2  // <- new
#define RELIC_COUNT             5  // <- bumped
```

### Step 2: Define per-tier effects arrays

In `src/data/relics.h`, define params + effects for each tier above `sRelicDefs`:

```c
static const struct RelicBattleDamageTakenParams sRelicParams_Pyroward_T1 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_FIRE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = RELIC_TYPE_MATCHUP_ANY,
    .multiplier = RELIC_REDUCTION_PCT(30),
};

// ... repeat for T2 (40%) and T3 (50%)

static const struct RelicEffect sRelicEffects_Pyroward_T1[] =
{
    {
        .type = RELIC_EFF_DAMAGE_TAKEN_MULT,
        .stacking = RELIC_STACK_MULTIPLY,
        .params = &sRelicParams_Pyroward_T1,
    },
};

// ... repeat for T2 and T3
```

**Multiplier helpers** (defined at the top of `src/data/relics.h`):

| Macro | Usage | Example |
|-------|-------|---------|
| `RELIC_REDUCTION_PCT(pct)` | Take `pct`% less damage | `RELIC_REDUCTION_PCT(20)` = 0.8x |
| `RELIC_MULT_PCT(pct)` | Multiply damage by `pct`% | `RELIC_MULT_PCT(150)` = 1.5x |
| `RELIC_FLAT_ACCURACY(n)` | Flat accuracy bonus (raw integer) | `RELIC_FLAT_ACCURACY(5)` = +5 accuracy |

### Step 3: Add the RelicDef entry

In the `sRelicDefs` table in `src/data/relics.h`:

```c
[RELIC_PYROWARD] =
{
    .name = COMPOUND_STRING("Pyroward"),
    .rarity = RELIC_RARITY_UNCOMMON,
    .descriptions = {
        COMPOUND_STRING("Your party takes 30%\nless Fire damage."),
        COMPOUND_STRING("Your party takes 40%\nless Fire damage."),
        COMPOUND_STRING("Your party takes 50%\nless Fire damage."),
    },
    .effects = { sRelicEffects_Pyroward_T1, sRelicEffects_Pyroward_T2, sRelicEffects_Pyroward_T3 },
    .effectCount = { ARRAY_COUNT(sRelicEffects_Pyroward_T1), ARRAY_COUNT(sRelicEffects_Pyroward_T2), ARRAY_COUNT(sRelicEffects_Pyroward_T3) },
},
```

Use `COMPOUND_STRING()` for both `name` and `descriptions` (not `_()`).
The `_()` macro produces a brace initializer that only works for array fields, not pointers.

### Step 4: Write tests

Create `test/battle/relic/pyroward.c` with per-tier parametrize tests:

```c
#include "global.h"
#include "test/battle.h"
#include "relic.h"

SINGLE_BATTLE_TEST("Pyroward T1 reduces Fire damage taken by 30%", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        if (active)
            Relic_SetSlot(0, RELIC_PYROWARD, RELIC_TIER_1);
        else
            Relic_ClearAll();
        ASSUME(GetMoveType(MOVE_EMBER) == TYPE_FIRE);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_EMBER); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.7), results[1].damage);
    }
}
```

### Step 5: Run

```bash
make check TESTS="Pyroward" -j$(nproc)
```

## Test Checklist for New Relics

- [ ] Test each tier (T1, T2, T3) with expected multiplier
- [ ] Negative case: effect does NOT apply when params don't match (wrong type, wrong side, type matchup, etc.)
- [ ] If the relic has `minHpPct`: test at/above and below the threshold

## Gotchas

| Pitfall | Fix |
|---------|-----|
| Using `_("text")` for name/description fields | Use `COMPOUND_STRING("text")` — struct fields are pointers, not arrays |
| Forgetting to bump `RELIC_COUNT` | Build will fail with `RelicDefsCountMismatch` static assert |
| Using `goto_if_set LABEL` in scripts | `goto_if_set` takes two args: `goto_if_set FLAG, LABEL` |
| Adding a new stacking rule | Must handle it in the relevant `Apply*Stacking` function in `src/relic.c` |
| Changing `RelicsSaveData` (adding slots, new fields) | Update `T_SAVEBLOCK2_SIZE` in `test/save.c` to match the new `sizeof(struct SaveBlock2)` |
