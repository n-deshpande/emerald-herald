// Balance/editing helpers (percent values).
#define CURSE_MULT_PCT(pct)      UQ_4_12((pct) / 100.0)
#define CURSE_REDUCTION_PCT(pct) UQ_4_12((100 - (pct)) / 100.0)
#define CURSE_FLAT_ACCURACY(n) (n)

static const struct CurseBattleDamageTakenParams sCurseParams_Bulwark =
{
    .side = CURSE_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = CURSE_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = CURSE_TYPE_MATCHUP_ANY,
    .multiplier = CURSE_REDUCTION_PCT(20),
};

static const struct CurseEffect sCurseEffects_Bulwark[] =
{
    {
        .type = CURSE_EFF_DAMAGE_TAKEN_MULT,
        .stacking = CURSE_STACK_MULTIPLY,
        .params = &sCurseParams_Bulwark,
    },
};

static const struct CurseBattleAccuracyFlatParams sCurseParams_Precision =
{
    .side = CURSE_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = CURSE_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .flatBonus = CURSE_FLAT_ACCURACY(5),
};

static const struct CurseEffect sCurseEffects_Precision[] =
{
    {
        .type = CURSE_EFF_ACCURACY_FLAT_BONUS,
        .stacking = CURSE_STACK_ADD_PCT,
        .params = &sCurseParams_Precision,
    },
};

static const struct CurseBattleDamageTakenParams sCurseParams_Exposed =
{
    .side = CURSE_SIDE_PLAYER,
    .moveType = TYPE_NONE,
    .moveCategory = CURSE_MOVE_CATEGORY_ANY,
    .minHpPct = 0,
    .typeMatchup = CURSE_TYPE_MATCHUP_SUPER_EFFECTIVE_ONLY,
    .multiplier = CURSE_MULT_PCT(125),
};

static const struct CurseEffect sCurseEffects_Exposed[] =
{
    {
        .type = CURSE_EFF_DAMAGE_TAKEN_MULT,
        .stacking = CURSE_STACK_MULTIPLY,
        .params = &sCurseParams_Exposed,
    },
};

static const struct CurseDef sCurseDefs[CURSE_COUNT] =
{
    // Curse menu detail pane renders description text as-authored.
    // Dev guidance: prefer 2-3 short lines (use '\n' for manual breaks).
    [CURSE_NONE] = {0},
    [CURSE_BOON_BULWARK] =
    {
        .name = COMPOUND_STRING("Bulwark"),
        .description = COMPOUND_STRING("Your party takes\n20% less damage."),
        .effects = sCurseEffects_Bulwark,
        .effectCount = ARRAY_COUNT(sCurseEffects_Bulwark),
    },
    [CURSE_BOON_PRECISION] =
    {
        .name = COMPOUND_STRING("Precision"),
        .description = COMPOUND_STRING("Your moves gain\n+5 accuracy."),
        .effects = sCurseEffects_Precision,
        .effectCount = ARRAY_COUNT(sCurseEffects_Precision),
    },
    [CURSE_BANE_EXPOSED] =
    {
        .name = COMPOUND_STRING("Exposed"),
        .description = COMPOUND_STRING("Your party takes 25% more\n"
                                       "damage from super-effective\n"
                                       "moves."),
        .effects = sCurseEffects_Exposed,
        .effectCount = ARRAY_COUNT(sCurseEffects_Exposed),
    },
};

#undef CURSE_MULT_PCT
#undef CURSE_REDUCTION_PCT
#undef CURSE_FLAT_ACCURACY
