#include "global.h"
#include "rival_starter.h"

#include "constants/opponents.h"
#include "constants/pokemon.h"

enum StarterSlot
{
    STARTER_SLOT_TREECKO,
    STARTER_SLOT_TORCHIC,
    STARTER_SLOT_MUDKIP,
    STARTER_SLOT_COUNT,
};

struct RivalTrainerVariantGroup
{
    u16 trainersByStarter[STARTER_SLOT_COUNT];
};

static const struct RivalTrainerVariantGroup sRivalTrainerGroups[] =
{
    // Brendan
    { .trainersByStarter = { TRAINER_BRENDAN_ROUTE_103_TREECKO, TRAINER_BRENDAN_ROUTE_103_TORCHIC, TRAINER_BRENDAN_ROUTE_103_MUDKIP } },
    { .trainersByStarter = { TRAINER_BRENDAN_ROUTE_110_TREECKO, TRAINER_BRENDAN_ROUTE_110_TORCHIC, TRAINER_BRENDAN_ROUTE_110_MUDKIP } },
    { .trainersByStarter = { TRAINER_BRENDAN_ROUTE_119_TREECKO, TRAINER_BRENDAN_ROUTE_119_TORCHIC, TRAINER_BRENDAN_ROUTE_119_MUDKIP } },
    { .trainersByStarter = { TRAINER_BRENDAN_RUSTBORO_TREECKO, TRAINER_BRENDAN_RUSTBORO_TORCHIC, TRAINER_BRENDAN_RUSTBORO_MUDKIP } },
    { .trainersByStarter = { TRAINER_BRENDAN_LILYCOVE_TREECKO, TRAINER_BRENDAN_LILYCOVE_TORCHIC, TRAINER_BRENDAN_LILYCOVE_MUDKIP } },

    // May
    { .trainersByStarter = { TRAINER_MAY_ROUTE_103_TREECKO, TRAINER_MAY_ROUTE_103_TORCHIC, TRAINER_MAY_ROUTE_103_MUDKIP } },
    { .trainersByStarter = { TRAINER_MAY_ROUTE_110_TREECKO, TRAINER_MAY_ROUTE_110_TORCHIC, TRAINER_MAY_ROUTE_110_MUDKIP } },
    { .trainersByStarter = { TRAINER_MAY_ROUTE_119_TREECKO, TRAINER_MAY_ROUTE_119_TORCHIC, TRAINER_MAY_ROUTE_119_MUDKIP } },
    { .trainersByStarter = { TRAINER_MAY_RUSTBORO_TREECKO, TRAINER_MAY_RUSTBORO_TORCHIC, TRAINER_MAY_RUSTBORO_MUDKIP } },
    { .trainersByStarter = { TRAINER_MAY_LILYCOVE_TREECKO, TRAINER_MAY_LILYCOVE_TORCHIC, TRAINER_MAY_LILYCOVE_MUDKIP } },
};

static s32 GetStarterSlotFromSpecies(u16 species)
{
    switch (species)
    {
    case SPECIES_TREECKO:
        return STARTER_SLOT_TREECKO;
    case SPECIES_TORCHIC:
        return STARTER_SLOT_TORCHIC;
    case SPECIES_MUDKIP:
        return STARTER_SLOT_MUDKIP;
    default:
        return -1;
    }
}

u16 ResolveRivalTrainerId(u16 trainerId, u16 playerStarterSpecies)
{
    s32 starterSlot = GetStarterSlotFromSpecies(playerStarterSpecies);
    u32 i;
    u32 j;

    if (starterSlot < 0)
        return trainerId;

    for (i = 0; i < ARRAY_COUNT(sRivalTrainerGroups); i++)
    {
        for (j = 0; j < STARTER_SLOT_COUNT; j++)
        {
            if (sRivalTrainerGroups[i].trainersByStarter[j] == trainerId)
                return sRivalTrainerGroups[i].trainersByStarter[starterSlot];
        }
    }

    return trainerId;
}
