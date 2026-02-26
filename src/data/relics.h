// Balance/editing helpers (percent values).
#define RELIC_MULT_PCT(pct)      UQ_4_12((pct) / 100.0)
#define RELIC_REDUCTION_PCT(pct) UQ_4_12((100 - (pct)) / 100.0)
#define RELIC_FLAT_ACCURACY(n) (n)

// ── Bulwark (damage reduction) ──────────────────────────────────────

static const struct RelicBattleDamageTakenParams sRelicParams_Bulwark_T1 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = RELIC_TYPE_MATCHUP_ANY,
    .multiplier = RELIC_REDUCTION_PCT(15),
};

static const struct RelicBattleDamageTakenParams sRelicParams_Bulwark_T2 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = RELIC_TYPE_MATCHUP_ANY,
    .multiplier = RELIC_REDUCTION_PCT(25),
};

static const struct RelicBattleDamageTakenParams sRelicParams_Bulwark_T3 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = RELIC_TYPE_MATCHUP_ANY,
    .multiplier = RELIC_REDUCTION_PCT(35),
};

static const struct RelicEffect sRelicEffects_Bulwark_T1[] =
{
    RELIC_EFFECT_DAMAGE_TAKEN(RELIC_STACK_MULTIPLY, &sRelicParams_Bulwark_T1),
};

static const struct RelicEffect sRelicEffects_Bulwark_T2[] =
{
    RELIC_EFFECT_DAMAGE_TAKEN(RELIC_STACK_MULTIPLY, &sRelicParams_Bulwark_T2),
};

static const struct RelicEffect sRelicEffects_Bulwark_T3[] =
{
    RELIC_EFFECT_DAMAGE_TAKEN(RELIC_STACK_MULTIPLY, &sRelicParams_Bulwark_T3),
};

// ── Precision (accuracy bonus) ──────────────────────────────────────

static const struct RelicBattleAccuracyFlatParams sRelicParams_Precision_T1 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .flatBonus = RELIC_FLAT_ACCURACY(3),
};

static const struct RelicBattleAccuracyFlatParams sRelicParams_Precision_T2 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .flatBonus = RELIC_FLAT_ACCURACY(5),
};

static const struct RelicBattleAccuracyFlatParams sRelicParams_Precision_T3 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .flatBonus = RELIC_FLAT_ACCURACY(8),
};

static const struct RelicEffect sRelicEffects_Precision_T1[] =
{
    RELIC_EFFECT_ACCURACY_FLAT(RELIC_STACK_ADD_FLAT, &sRelicParams_Precision_T1),
};

static const struct RelicEffect sRelicEffects_Precision_T2[] =
{
    RELIC_EFFECT_ACCURACY_FLAT(RELIC_STACK_ADD_FLAT, &sRelicParams_Precision_T2),
};

static const struct RelicEffect sRelicEffects_Precision_T3[] =
{
    RELIC_EFFECT_ACCURACY_FLAT(RELIC_STACK_ADD_FLAT, &sRelicParams_Precision_T3),
};

// ── Exposed (super-effective damage increase) ───────────────────────

static const struct RelicBattleDamageTakenParams sRelicParams_Exposed_T1 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = RELIC_TYPE_MATCHUP_SUPER_EFFECTIVE_ONLY,
    .multiplier = RELIC_MULT_PCT(120),
};

static const struct RelicBattleDamageTakenParams sRelicParams_Exposed_T2 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = RELIC_TYPE_MATCHUP_SUPER_EFFECTIVE_ONLY,
    .multiplier = RELIC_MULT_PCT(130),
};

static const struct RelicBattleDamageTakenParams sRelicParams_Exposed_T3 =
{
    .side = RELIC_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = RELIC_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = RELIC_TYPE_MATCHUP_SUPER_EFFECTIVE_ONLY,
    .multiplier = RELIC_MULT_PCT(140),
};

static const struct RelicEffect sRelicEffects_Exposed_T1[] =
{
    RELIC_EFFECT_DAMAGE_TAKEN(RELIC_STACK_MULTIPLY, &sRelicParams_Exposed_T1),
};

static const struct RelicEffect sRelicEffects_Exposed_T2[] =
{
    RELIC_EFFECT_DAMAGE_TAKEN(RELIC_STACK_MULTIPLY, &sRelicParams_Exposed_T2),
};

static const struct RelicEffect sRelicEffects_Exposed_T3[] =
{
    RELIC_EFFECT_DAMAGE_TAKEN(RELIC_STACK_MULTIPLY, &sRelicParams_Exposed_T3),
};

// ── Draconic Ancestry (starter slot overrides) ─────────────────────

static const struct RelicStarterSlotOverrideParams sRelicParams_DraconicAncestry_Slot0 =
{
    .slot = 0,
    .speciesPrimary = SPECIES_DREEPY,
    .speciesSecondary = SPECIES_GIBLE,
};

static const struct RelicStarterSlotOverrideParams sRelicParams_DraconicAncestry_Slot1 =
{
    .slot = 1,
    .speciesPrimary = SPECIES_JANGMO_O,
    .speciesSecondary = SPECIES_BAGON,
};

static const struct RelicStarterSlotOverrideParams sRelicParams_DraconicAncestry_Slot2 =
{
    .slot = 2,
    .speciesPrimary = SPECIES_DRATINI,
    .speciesSecondary = SPECIES_GOOMY,
};

static const struct RelicEffect sRelicEffects_DraconicAncestry_T1[] =
{
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot0),
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot1),
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot2),
};

static const struct RelicEffect sRelicEffects_DraconicAncestry_T2[] =
{
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot0),
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot1),
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot2),
};

static const struct RelicEffect sRelicEffects_DraconicAncestry_T3[] =
{
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot0),
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot1),
    RELIC_EFFECT_STARTER_SLOT_OVERRIDE(RELIC_STACK_OVERRIDE, &sRelicParams_DraconicAncestry_Slot2),
};

// ── Relic definitions table ─────────────────────────────────────────

// Relic menu detail pane renders description text as-authored.
// Dev guidance: prefer 2-3 short lines (use '\n' for manual breaks).
static const struct RelicDef sRelicDefs[RELIC_COUNT] =
{
    [RELIC_NONE] = {0},
    [RELIC_BULWARK] =
    {
        .name = COMPOUND_STRING("Bulwark"),
        .rarity = RELIC_RARITY_COMMON,
        .acquisitionFlags = RELIC_ACQF_IDENTITY_DRAFT | RELIC_ACQF_MILESTONE_DRAW | RELIC_ACQF_TRADE_UP_TARGET,
        .descriptions = {
            COMPOUND_STRING("Your party takes\n15% less damage."),
            COMPOUND_STRING("Your party takes\n25% less damage."),
            COMPOUND_STRING("Your party takes\n35% less damage."),
        },
        .effects = { sRelicEffects_Bulwark_T1, sRelicEffects_Bulwark_T2, sRelicEffects_Bulwark_T3 },
        .effectCount = { ARRAY_COUNT(sRelicEffects_Bulwark_T1), ARRAY_COUNT(sRelicEffects_Bulwark_T2), ARRAY_COUNT(sRelicEffects_Bulwark_T3) },
    },
    [RELIC_PRECISION] =
    {
        .name = COMPOUND_STRING("Precision"),
        .rarity = RELIC_RARITY_COMMON,
        .acquisitionFlags = RELIC_ACQF_IDENTITY_DRAFT | RELIC_ACQF_MILESTONE_DRAW | RELIC_ACQF_TRADE_UP_TARGET,
        .descriptions = {
            COMPOUND_STRING("Your moves gain\n+3 accuracy."),
            COMPOUND_STRING("Your moves gain\n+5 accuracy."),
            COMPOUND_STRING("Your moves gain\n+8 accuracy."),
        },
        .effects = { sRelicEffects_Precision_T1, sRelicEffects_Precision_T2, sRelicEffects_Precision_T3 },
        .effectCount = { ARRAY_COUNT(sRelicEffects_Precision_T1), ARRAY_COUNT(sRelicEffects_Precision_T2), ARRAY_COUNT(sRelicEffects_Precision_T3) },
    },
    [RELIC_EXPOSED] =
    {
        .name = COMPOUND_STRING("Exposed"),
        .rarity = RELIC_RARITY_COMMON,
        .acquisitionFlags = RELIC_ACQF_IDENTITY_DRAFT | RELIC_ACQF_MILESTONE_DRAW | RELIC_ACQF_TRADE_UP_TARGET,
        .descriptions = {
            COMPOUND_STRING("Your party takes 20% more\n"
                            "damage from super-effective\n"
                            "moves."),
            COMPOUND_STRING("Your party takes 30% more\n"
                            "damage from super-effective\n"
                            "moves."),
            COMPOUND_STRING("Your party takes 40% more\n"
                            "damage from super-effective\n"
                            "moves."),
        },
        .effects = { sRelicEffects_Exposed_T1, sRelicEffects_Exposed_T2, sRelicEffects_Exposed_T3 },
        .effectCount = { ARRAY_COUNT(sRelicEffects_Exposed_T1), ARRAY_COUNT(sRelicEffects_Exposed_T2), ARRAY_COUNT(sRelicEffects_Exposed_T3) },
    },
    [RELIC_DRACONIC_ANCESTRY] =
    {
        .name = COMPOUND_STRING("Draconic Ancestry"),
        .rarity = RELIC_RARITY_UNCOMMON,
        .acquisitionFlags = RELIC_ACQF_IDENTITY_DRAFT,
        .descriptions = {
            COMPOUND_STRING("Starter slots become\n"
                            "dragon descendants."),
            COMPOUND_STRING("Starter slots become\n"
                            "dragon descendants."),
            COMPOUND_STRING("Starter slots become\n"
                            "dragon descendants."),
        },
        .effects = { sRelicEffects_DraconicAncestry_T1, sRelicEffects_DraconicAncestry_T2, sRelicEffects_DraconicAncestry_T3 },
        .effectCount = { ARRAY_COUNT(sRelicEffects_DraconicAncestry_T1), ARRAY_COUNT(sRelicEffects_DraconicAncestry_T2), ARRAY_COUNT(sRelicEffects_DraconicAncestry_T3) },
    },
};

#undef RELIC_MULT_PCT
#undef RELIC_REDUCTION_PCT
#undef RELIC_FLAT_ACCURACY
