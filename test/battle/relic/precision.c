#include "global.h"
#include "test/battle.h"
#include "relic.h"

SINGLE_BATTLE_TEST("Precision T1 adds +3 accuracy to player's moves")
{
    ASSUME(GetMoveAccuracy(MOVE_THUNDER) == 70);
    PASSES_RANDOMLY(73, 100, RNG_ACCURACY);
    GIVEN {
        Relic_ClearAll();
        Relic_SetSlot(0, RELIC_PRECISION, RELIC_TIER_1);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER, player);
    }
}

SINGLE_BATTLE_TEST("Precision T2 adds +5 accuracy to player's moves")
{
    ASSUME(GetMoveAccuracy(MOVE_THUNDER) == 70);
    PASSES_RANDOMLY(75, 100, RNG_ACCURACY);
    GIVEN {
        Relic_ClearAll();
        Relic_SetSlot(0, RELIC_PRECISION, RELIC_TIER_2);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_THUNDER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_THUNDER, player);
    }
}

SINGLE_BATTLE_TEST("Precision T3 adds +8 accuracy to player's moves")
{
    ASSUME(GetMoveAccuracy(MOVE_THUNDER) == 70);
    PASSES_RANDOMLY(78, 100, RNG_ACCURACY);
    GIVEN {
        Relic_ClearAll();
        Relic_SetSlot(0, RELIC_PRECISION, RELIC_TIER_3);
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
        Relic_ClearAll();
        Relic_SetSlot(0, RELIC_PRECISION, RELIC_TIER_2);
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
        Relic_ClearAll();
        Relic_SetSlot(0, RELIC_PRECISION, RELIC_TIER_3);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
    }
}
