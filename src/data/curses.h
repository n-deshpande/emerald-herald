// Balance/editing helpers (percent values).
#define CURSE_MULT_PCT(pct)      UQ_4_12((pct) / 100.0)
#define CURSE_REDUCTION_PCT(pct) UQ_4_12((100 - (pct)) / 100.0)

static const struct CurseEffect sCurseEffects_Bulwark[] =
{
    {
        .type = CURSE_EFF_DAMAGE_TAKEN_MULT,
        .stacking = CURSE_STACK_MULTIPLY,
        .selector =
        {
            .side = CURSE_SIDE_PLAYER,
            .moveType = TYPE_NONE,
            .moveCategory = CURSE_MOVE_CATEGORY_ANY,
            .minHpPct = 0,
        },
        .multiplier = CURSE_REDUCTION_PCT(20),
    },
};

static const struct CurseDef sCurseDefs[CURSE_COUNT] =
{
    [CURSE_NONE] = {0},
    [CURSE_BOON_BULWARK] =
    {
        .name = _("Bulwark"),
        .description = COMPOUND_STRING("Your party takes 20% less damage."),
        .effects = sCurseEffects_Bulwark,
        .effectCount = ARRAY_COUNT(sCurseEffects_Bulwark),
    },
};

#undef CURSE_MULT_PCT
#undef CURSE_REDUCTION_PCT
