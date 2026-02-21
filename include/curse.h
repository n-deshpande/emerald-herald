#ifndef GUARD_CURSE_H
#define GUARD_CURSE_H

#include "gba/types.h"
#include "fpmath.h"
#include "constants/curses.h"

struct DamageContext;

struct CurseSelector
{
    u8 side;
    u8 moveType;
    u8 moveCategory;
    u8 minHpPct;
};

struct CurseEffect
{
    u8 type;
    u8 stacking;
    struct CurseSelector selector;
    uq4_12_t multiplier;
};

struct CurseDef
{
    const u8 *name;
    const u8 *description;
    const struct CurseEffect *effects;
    u8 effectCount;
};

struct CursesSaveData
{
    u16 activeBoons[CURSE_ACTIVE_BOON_SLOTS];
    u16 activeBanes[CURSE_ACTIVE_BANE_SLOTS];
};

void Curse_InitDefaults(void);
void Curse_ClearActive(void);
void Curse_SetActiveBoon(u8 slot, u16 curseId);
uq4_12_t Curse_GetDamageTakenModifier(const struct DamageContext *ctx);
s32 Curse_GetAccuracyBonus(u32 battlerAtk, u32 move);
u16 Curse_GetCount(void);
const struct CurseDef *Curse_GetDef(u16 curseId);
bool32 Curse_IsImplemented(u16 curseId);

#endif // GUARD_CURSE_H
