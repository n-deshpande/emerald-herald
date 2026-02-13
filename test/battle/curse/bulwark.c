#include "global.h"
#include "test/battle.h"
#include "curse.h"

SINGLE_BATTLE_TEST("Bulwark reduces damage taken by 20%", s16 damage)
{
    bool32 cursed;
    PARAMETRIZE { cursed = FALSE; }
    PARAMETRIZE { cursed = TRUE; }
    GIVEN {
        if (cursed)
            Curse_SetActiveBoon(0, CURSE_BOON_BULWARK);
        else
            Curse_ClearActive();
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(0.8), results[1].damage);
    }
}
