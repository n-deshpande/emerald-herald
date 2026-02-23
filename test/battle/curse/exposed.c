#include "global.h"
#include "test/battle.h"
#include "curse.h"

SINGLE_BATTLE_TEST("Exposed increases super-effective damage taken by 25%", s16 damage)
{
    bool32 cursed;
    PARAMETRIZE { cursed = FALSE; }
    PARAMETRIZE { cursed = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_KARATE_CHOP) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 0) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 1) == TYPE_NORMAL);

        if (cursed)
            Curse_SetActiveBane(0, CURSE_BANE_EXPOSED);
        else
            Curse_ClearActive();

        PLAYER(SPECIES_EEVEE);
        OPPONENT(SPECIES_MACHOP);
    } WHEN {
        TURN { MOVE(opponent, MOVE_KARATE_CHOP); }
    } SCENE {
        HP_BAR(player, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(1.25), results[1].damage);
    }
}

SINGLE_BATTLE_TEST("Exposed does not affect neutral damage", s16 damage)
{
    bool32 cursed;
    PARAMETRIZE { cursed = FALSE; }
    PARAMETRIZE { cursed = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_TACKLE) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_WOBBUFFET, 0) == TYPE_PSYCHIC);

        if (cursed)
            Curse_SetActiveBane(0, CURSE_BANE_EXPOSED);
        else
            Curse_ClearActive();

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
    bool32 cursed;
    PARAMETRIZE { cursed = FALSE; }
    PARAMETRIZE { cursed = TRUE; }
    GIVEN {
        ASSUME(GetMoveType(MOVE_KARATE_CHOP) == TYPE_FIGHTING);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 0) == TYPE_NORMAL);
        ASSUME(GetSpeciesType(SPECIES_EEVEE, 1) == TYPE_NORMAL);

        if (cursed)
            Curse_SetActiveBane(0, CURSE_BANE_EXPOSED);
        else
            Curse_ClearActive();

        PLAYER(SPECIES_MACHOP);
        OPPONENT(SPECIES_EEVEE);
    } WHEN {
        TURN { MOVE(player, MOVE_KARATE_CHOP); }
    } SCENE {
        HP_BAR(opponent, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_EQ(results[0].damage, results[1].damage);
    }
}
