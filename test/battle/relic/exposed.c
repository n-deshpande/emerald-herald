#include "global.h"
#include "test/battle.h"
#include "relic.h"

SINGLE_BATTLE_TEST("Exposed T1 increases super-effective damage taken by 20%", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_KARATE_CHOP) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 0) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 1) == TYPE_NORMAL);

        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_EXPOSED, RELIC_TIER_1);

        PLAYER(SPECIES_EEVEE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_KARATE_CHOP); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.2), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Exposed T2 increases super-effective damage taken by 30%", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_KARATE_CHOP) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 0) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 1) == TYPE_NORMAL);

        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_EXPOSED, RELIC_TIER_2);

        PLAYER(SPECIES_EEVEE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_KARATE_CHOP); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.3), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Exposed T3 increases super-effective damage taken by 40%", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_KARATE_CHOP) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 0) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 1) == TYPE_NORMAL);

        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_EXPOSED, RELIC_TIER_3);

        PLAYER(SPECIES_EEVEE);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_KARATE_CHOP); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.4), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Exposed does not affect neutral damage", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_TACKLE) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_WOBBUFFET, 0) == TYPE_PSYCHIC);

        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_EXPOSED, RELIC_TIER_1);

        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EEVEE);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TACKLE); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Exposed does not increase damage dealt to opponents", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_KARATE_CHOP) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 0) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 1) == TYPE_NORMAL);

        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_EXPOSED, RELIC_TIER_1);

        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_EEVEE);
    } WHEN {
        TURN { MOVE(player, MOVE_KARATE_CHOP); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}
