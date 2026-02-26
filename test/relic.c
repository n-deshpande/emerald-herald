#include "global.h"
#include "event_data.h"
#include "new_game.h"
#include "relic.h"
#include "rival_starter.h"
#include "starter_choose.h"
#include "test/test.h"

#include "constants/opponents.h"
#include "constants/pokemon.h"

static void ResetStarterOfferLock(void)
{
    VarSet(VAR_STARTER_OFFER_LOCKED, FALSE);
    VarSet(VAR_STARTER_OFFER_SLOT_0_SPECIES, SPECIES_NONE);
    VarSet(VAR_STARTER_OFFER_SLOT_1_SPECIES, SPECIES_NONE);
    VarSet(VAR_STARTER_OFFER_SLOT_2_SPECIES, SPECIES_NONE);
}

static void SetTestTrainerId(u32 trainerId)
{
    SetTrainerId(trainerId, gSaveBlock2Ptr->playerTrainerId);
}

TEST("Relic_ClearAll clears all relic slots")
{
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_1);
    Relic_SetSlot(1, RELIC_EXPOSED, RELIC_TIER_1);
    Relic_ClearAll();

    EXPECT_EQ(Relic_GetSlotId(0), RELIC_NONE);
    EXPECT_EQ(Relic_GetSlotId(1), RELIC_NONE);
}

TEST("Relic_SetSlot sets the target slot")
{
    Relic_ClearAll();
    Relic_SetSlot(1, RELIC_EXPOSED, RELIC_TIER_2);

    EXPECT_EQ(Relic_GetSlotId(1), RELIC_EXPOSED);
    EXPECT_EQ(Relic_GetSlotTier(1), RELIC_TIER_2);
}

TEST("Relic slot getters return defaults for invalid slots")
{
    Relic_ClearAll();

    EXPECT_EQ(Relic_GetSlotId(RELIC_SLOTS), RELIC_NONE);
    EXPECT_EQ(Relic_GetSlotTier(RELIC_SLOTS), RELIC_TIER_1);
}

TEST("Relic slot getters return set values for valid slots")
{
    Relic_ClearAll();
    Relic_SetSlot(2, RELIC_BULWARK, RELIC_TIER_3);
    Relic_SetSlot(4, RELIC_EXPOSED, RELIC_TIER_1);

    EXPECT_EQ(Relic_GetSlotId(2), RELIC_BULWARK);
    EXPECT_EQ(Relic_GetSlotTier(2), RELIC_TIER_3);
    EXPECT_EQ(Relic_GetSlotId(4), RELIC_EXPOSED);
    EXPECT_EQ(Relic_GetSlotTier(4), RELIC_TIER_1);
}

TEST("Relic_ClearSlot clears a single slot")
{
    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_2);
    Relic_SetSlot(1, RELIC_PRECISION, RELIC_TIER_1);
    Relic_ClearSlot(0);

    EXPECT_EQ(Relic_GetSlotId(0), RELIC_NONE);
    EXPECT_EQ(Relic_GetSlotId(1), RELIC_PRECISION);
}

TEST("Relic_UpgradeSlot increases tier by one")
{
    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_1);
    Relic_UpgradeSlot(0);

    EXPECT_EQ(Relic_GetSlotTier(0), RELIC_TIER_2);
}

TEST("Relic_UpgradeSlot does not exceed max tier")
{
    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_MAX_TIER);
    Relic_UpgradeSlot(0);

    EXPECT_EQ(Relic_GetSlotTier(0), RELIC_MAX_TIER);
}

TEST("Relic_GetMaxActiveSlots returns current slot capacity")
{
    EXPECT_EQ(Relic_GetMaxActiveSlots(), RELIC_SLOTS);
}

TEST("Relic_BuildOffer produces deterministic valid unique candidates")
{
    struct RelicOfferSpec spec = {0};
    struct RelicOffer offerA = {0};
    struct RelicOffer offerB = {0};
    u8 i;
    u8 j;

    Relic_ClearAll();

    spec.reason = RELIC_ACQUIRE_START_IDENTITY;
    spec.choiceCount = 3;
    spec.allowDuplicates = FALSE;
    spec.excludeOwnedRelics = FALSE;
    spec.requiredFlags = 0;

    EXPECT_EQ(Relic_BuildOffer(&spec, &offerA, 12345), TRUE);
    EXPECT_EQ(Relic_BuildOffer(&spec, &offerB, 12345), TRUE);
    EXPECT_EQ(offerA.count, offerB.count);
    EXPECT_EQ(offerA.count, 3);

    for (i = 0; i < offerA.count; i++)
    {
        EXPECT_EQ(Relic_IsOfferChoiceValid(&offerA, i), TRUE);
        EXPECT_EQ(offerA.relicIds[i], offerB.relicIds[i]);
        for (j = i + 1; j < offerA.count; j++)
            EXPECT_NE(offerA.relicIds[i], offerA.relicIds[j]);
    }
}

TEST("Relic_BuildOffer respects owned exclusion and required flags")
{
    struct RelicOfferSpec spec = {0};
    struct RelicOffer offer = {0};

    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_1);
    Relic_SetSlot(1, RELIC_PRECISION, RELIC_TIER_1);
    Relic_SetSlot(2, RELIC_EXPOSED, RELIC_TIER_1);

    spec.reason = RELIC_ACQUIRE_GYM_PASSIVE;
    spec.choiceCount = 1;
    spec.allowDuplicates = FALSE;
    spec.excludeOwnedRelics = TRUE;
    spec.requiredFlags = RELIC_ACQF_MILESTONE_DRAW;
    EXPECT_EQ(Relic_BuildOffer(&spec, &offer, 7), FALSE);

    spec.excludeOwnedRelics = FALSE;
    spec.requiredFlags = 0x8000;
    EXPECT_EQ(Relic_BuildOffer(&spec, &offer, 7), FALSE);
}

TEST("Relic_IsOfferChoiceValid rejects invalid indices and invalid relic ids")
{
    struct RelicOffer offer =
    {
        .count = 2,
        .relicIds = { RELIC_BULWARK, RELIC_NONE },
    };

    EXPECT_EQ(Relic_IsOfferChoiceValid(&offer, 0), TRUE);
    EXPECT_EQ(Relic_IsOfferChoiceValid(&offer, 1), FALSE);
    EXPECT_EQ(Relic_IsOfferChoiceValid(&offer, 2), FALSE);
}

TEST("BuildStarterOffer returns default three-choice starter offer")
{
    struct StarterOffer offer = {0};
    u8 i;

    ResetStarterOfferLock();
    Relic_ClearAll();
    BuildStarterOffer(&offer);

    EXPECT_EQ(offer.count, STARTER_OFFER_COUNT);
    for (i = 0; i < STARTER_OFFER_COUNT; i++)
        EXPECT_EQ(GetStarterPokemon(i), GetStarterPokemonFromOffer(&offer, i));
}

TEST("BuildStarterOffer is deterministic without starter-pool relic effects")
{
    struct StarterOffer offerA = {0};
    struct StarterOffer offerB = {0};
    u8 i;

    ResetStarterOfferLock();
    Relic_ClearAll();
    BuildStarterOffer(&offerA);
    BuildStarterOffer(&offerB);

    EXPECT_EQ(offerA.count, STARTER_OFFER_COUNT);
    EXPECT_EQ(offerA.count, offerB.count);
    for (i = 0; i < STARTER_OFFER_COUNT; i++)
        EXPECT_EQ(offerA.species[i], offerB.species[i]);
}

TEST("BuildStarterOffer applies Draconic Ancestry with primary species when trainer id bits are clear")
{
    struct StarterOffer offer = {0};

    ResetStarterOfferLock();
    Relic_ClearAll();
    SetTestTrainerId(0);
    Relic_SetSlot(0, RELIC_DRACONIC_ANCESTRY, RELIC_TIER_1);
    BuildStarterOffer(&offer);

    EXPECT_EQ(offer.count, STARTER_OFFER_COUNT);
    EXPECT_EQ(offer.species[0], SPECIES_DREEPY);
    EXPECT_EQ(offer.species[1], SPECIES_JANGMO_O);
    EXPECT_EQ(offer.species[2], SPECIES_DRATINI);
    ResetStarterOfferLock();
}

TEST("BuildStarterOffer applies Draconic Ancestry with secondary species when trainer id bits are set")
{
    struct StarterOffer offer = {0};

    ResetStarterOfferLock();
    Relic_ClearAll();
    SetTestTrainerId(7);
    Relic_SetSlot(0, RELIC_DRACONIC_ANCESTRY, RELIC_TIER_1);
    BuildStarterOffer(&offer);

    EXPECT_EQ(offer.count, STARTER_OFFER_COUNT);
    EXPECT_EQ(offer.species[0], SPECIES_GIBLE);
    EXPECT_EQ(offer.species[1], SPECIES_BAGON);
    EXPECT_EQ(offer.species[2], SPECIES_GOOMY);
    ResetStarterOfferLock();
}

TEST("Starter offer lock keeps species stable after Draconic Ancestry is removed")
{
    struct StarterOffer lockedOffer = {0};
    struct StarterOffer currentOffer = {0};

    ResetStarterOfferLock();
    Relic_ClearAll();
    SetTestTrainerId(7);
    Relic_SetSlot(0, RELIC_DRACONIC_ANCESTRY, RELIC_TIER_1);
    BuildStarterOffer(&lockedOffer);
    LockStarterOffer(&lockedOffer);

    Relic_ClearAll();
    SetTestTrainerId(0);
    BuildStarterOffer(&currentOffer);

    EXPECT_EQ(currentOffer.count, STARTER_OFFER_COUNT);
    EXPECT_EQ(currentOffer.species[0], SPECIES_GIBLE);
    EXPECT_EQ(currentOffer.species[1], SPECIES_BAGON);
    EXPECT_EQ(currentOffer.species[2], SPECIES_GOOMY);
    EXPECT_EQ(GetStarterPokemon(0), SPECIES_GIBLE);
    EXPECT_EQ(GetStarterPokemon(1), SPECIES_BAGON);
    EXPECT_EQ(GetStarterPokemon(2), SPECIES_GOOMY);
    ResetStarterOfferLock();
}

TEST("BuildAndLockStarterOfferIfNeeded freezes starter offer before selection")
{
    struct StarterOffer frozenOffer = {0};
    struct StarterOffer afterRelicChange = {0};

    ResetStarterOfferLock();
    Relic_ClearAll();
    SetTestTrainerId(7);
    Relic_SetSlot(0, RELIC_DRACONIC_ANCESTRY, RELIC_TIER_1);

    BuildAndLockStarterOfferIfNeeded();
    BuildStarterOffer(&frozenOffer);

    Relic_ClearAll();
    SetTestTrainerId(0);
    BuildStarterOffer(&afterRelicChange);

    EXPECT_EQ(frozenOffer.species[0], SPECIES_GIBLE);
    EXPECT_EQ(frozenOffer.species[1], SPECIES_BAGON);
    EXPECT_EQ(frozenOffer.species[2], SPECIES_GOOMY);
    EXPECT_EQ(afterRelicChange.species[0], SPECIES_GIBLE);
    EXPECT_EQ(afterRelicChange.species[1], SPECIES_BAGON);
    EXPECT_EQ(afterRelicChange.species[2], SPECIES_GOOMY);
    ResetStarterOfferLock();
}

TEST("GetStarterPokemonFromOffer clamps invalid selection index to slot zero")
{
    struct StarterOffer offer =
    {
        .species = { SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP },
        .count = STARTER_OFFER_COUNT,
    };

    EXPECT_EQ(GetStarterPokemonFromOffer(&offer, STARTER_OFFER_COUNT), SPECIES_TREECKO);
}

TEST("Relic starter hook keeps starter pool unchanged without effects")
{
    struct RelicHookContext ctx = {0};
    struct RelicHookResult out = {0};
    u16 species[STARTER_OFFER_COUNT] = { SPECIES_TREECKO, SPECIES_TORCHIC, SPECIES_MUDKIP };

    Relic_ClearAll();
    ctx.starterPool.species = species;
    ctx.starterPool.count = STARTER_OFFER_COUNT;
    ctx.starterPool.capacity = STARTER_OFFER_COUNT;

    EXPECT_EQ(Relic_ApplyHook(RELIC_HOOK_STARTER_POOL_BUILD, &ctx, &out), TRUE);
    EXPECT_EQ(out.starterPoolCount, STARTER_OFFER_COUNT);
    EXPECT_EQ(species[0], SPECIES_TREECKO);
    EXPECT_EQ(species[1], SPECIES_TORCHIC);
    EXPECT_EQ(species[2], SPECIES_MUDKIP);
}

TEST("ResolveRivalTrainerId remaps rival variants by actual starter species")
{
    EXPECT_EQ(ResolveRivalTrainerId(TRAINER_MAY_ROUTE_103_TREECKO, SPECIES_TREECKO), TRAINER_MAY_ROUTE_103_TREECKO);
    EXPECT_EQ(ResolveRivalTrainerId(TRAINER_MAY_ROUTE_103_TREECKO, SPECIES_TORCHIC), TRAINER_MAY_ROUTE_103_TORCHIC);
    EXPECT_EQ(ResolveRivalTrainerId(TRAINER_MAY_ROUTE_103_TREECKO, SPECIES_MUDKIP), TRAINER_MAY_ROUTE_103_MUDKIP);
    EXPECT_EQ(ResolveRivalTrainerId(TRAINER_MAY_ROUTE_103_TREECKO, SPECIES_PIKACHU), TRAINER_MAY_ROUTE_103_TREECKO);
}
