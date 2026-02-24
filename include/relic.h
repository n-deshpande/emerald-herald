#ifndef GUARD_RELIC_H
#define GUARD_RELIC_H

#include "gba/types.h"
#include "fpmath.h"
#include "constants/relics.h"

struct DamageContext;

struct RelicBattleDamageTakenParams
{
    u8 side;
    u8 moveType;
    u8 moveCategory;
    u8 minHpPct;
    u8 typeMatchup;
    uq4_12_t multiplier;
};

struct RelicBattleAccuracyFlatParams
{
    u8 side;
    u8 moveType;
    u8 moveCategory;
    u8 minHpPct;
    s16 flatBonus;
};

struct RelicEffect
{
    u8 type;
    u8 stacking;
    const void *params;
};

struct RelicDef
{
    const u8 *name;
    const u8 *descriptions[RELIC_NUM_TIERS];
    const struct RelicEffect *effects[RELIC_NUM_TIERS];
    u8 effectCount[RELIC_NUM_TIERS];
    u8 rarity;
};

struct RelicsSaveData
{
    u8 slotId[RELIC_SLOTS];
    u8 slotTier[RELIC_SLOTS];
    u8 activeMarks;
};

void Relic_InitDefaults(void);
void Relic_ClearAll(void);
void Relic_SetSlot(u8 slot, u8 relicId, u8 tier);
void Relic_ClearSlot(u8 slot);
u8 Relic_GetSlotId(u8 slot);
u8 Relic_GetSlotTier(u8 slot);
void Relic_UpgradeSlot(u8 slot);
uq4_12_t Relic_GetDamageTakenModifier(const struct DamageContext *ctx);
s32 Relic_GetAccuracyBonus(u32 battlerAtk, u32 move);
u16 Relic_GetCount(void);
const struct RelicDef *Relic_GetDef(u8 relicId);
bool32 Relic_IsImplemented(u8 relicId);

#endif // GUARD_RELIC_H
