#include "global.h"
#include "test/battle.h"
#include "curse.h"

SINGLE_BATTLE_TEST("Precision adds +5 accuracy to player's moves")
{
    ASSUME(GetMoveAccuracy(MOVE_THUNDER) == 70);
    PASSES_RANDOMLY(75, 100, RNG_ACCURACY);
    GIVEN {
        Curse_ClearActive();
        Curse_SetActiveBoon(0, CURSE_BOON_PRECISION);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER, player);
    }
}

SINGLE_BATTLE_TEST("Precision does not affect opponent's moves")
{
    ASSUME(GetMoveAccuracy(MOVE_THUNDER) == 70);
    PASSES_RANDOMLY(70, 100, RNG_ACCURACY);
    GIVEN {
        Curse_ClearActive();
        Curse_SetActiveBoon(0, CURSE_BOON_PRECISION);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER, opponent);
    }
}

SINGLE_BATTLE_TEST("Precision does not exceed 100% accuracy")
{
    ASSUME(GetMoveAccuracy(MOVE_SCRATCH) == 100);
    PASSES_RANDOMLY(100, 100, RNG_ACCURACY);
    GIVEN {
        Curse_ClearActive();
        Curse_SetActiveBoon(0, CURSE_BOON_PRECISION);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
    }
}
