#ifndef GUARD_STARTER_CHOOSE_H
#define GUARD_STARTER_CHOOSE_H

#include "gba/types.h"

extern const u16 gBirchBagGrass_Pal[];
extern const u32 gBirchBagTilemap[];
extern const u32 gBirchGrassTilemap[];
extern const u32 gBirchBagGrass_Gfx[];
extern const u32 gPokeballSelection_Gfx[];

#define STARTER_OFFER_COUNT 3

struct StarterOffer
{
    u16 species[STARTER_OFFER_COUNT];
    u8 count;
};

void BuildStarterOffer(struct StarterOffer *offer);
u16 GetStarterPokemonFromOffer(const struct StarterOffer *offer, u16 chosenStarterId);
u16 GetStarterPokemon(u16 chosenStarterId);
void LockStarterOffer(const struct StarterOffer *offer);
void BuildAndLockStarterOfferIfNeeded(void);
void CB2_ChooseStarter(void);

#endif // GUARD_STARTER_CHOOSE_H
