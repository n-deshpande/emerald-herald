#include "global.h"
#include "test/battle.h"
#include "relic.h"

SINGLE_BATTLE_TEST("Bulwark T1 reduces damage taken by 15%", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_1);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.85), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Bulwark T2 reduces damage taken by 25%", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_2);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.75), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Bulwark T3 reduces damage taken by 35%", s16 damage)
{
    bool32 active;
    PARAMETRIZE { active = FALSE; }
    PARAMETRIZE { active = TRUE; }
    GIVEN {
        Relic_ClearAll();
        if (active)
            Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_3);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.65), results[1].damage);
    }
}
