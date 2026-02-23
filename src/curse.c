#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "curse.h"
#include "event_data.h"
#include "move.h"

#include "constants/curses.h"
#include "constants/pokemon.h"

#include "data/curses.h"

STATIC_ASSERT(ARRAY_COUNT(sCurseDefs) == CURSE_COUNT, CurseDefsCountMismatch);

static bool32 CurseSelectorMatches(const struct CurseSelector *selector, const struct DamageContext *ctx)
{
    enum BattleSide side = GetBattlerSide(ctx->battlerDef);
    enum DamageCategory category = GetMoveCategory(ctx->move);

    switch (selector->side)
    {
    case CURSE_SIDE_PLAYER:
        if (side != B_SIDE_PLAYER)
            return FALSE;
        break;
    case CURSE_SIDE_OPPONENT:
        if (side != B_SIDE_OPPONENT)
            return FALSE;
        break;
    case CURSE_SIDE_BOTH:
        break;
    default:
        return FALSE;
    }

    if (selector->moveType != TYPE_NONE && selector->moveType != ctx->moveType)
        return FALSE;

    if (selector->moveCategory != CURSE_MOVE_CATEGORY_ANY && selector->moveCategory != category)
        return FALSE;

    if (selector->minHpPct != 0)
    {
        u32 maxHp = gBattleMons[ctx->battlerDef].maxHP;
        u32 currHp = gBattleMons[ctx->battlerDef].hp;
        u32 hpPct = (maxHp == 0) ? 0 : (currHp * 100) / maxHp;

        if (hpPct < selector->minHpPct)
            return FALSE;
    }

    return TRUE;
}

static uq4_12_t ApplyCurseEffects(uq4_12_t modifier, const struct DamageContext *ctx, const u16 *curseIds, u32 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        u16 curseId = curseIds[i];
        const struct CurseDef *curse;
        u32 j;

        if (curseId == CURSE_NONE)
            continue;

        if (curseId >= ARRAY_COUNT(sCurseDefs))
            continue;

        curse = &sCurseDefs[curseId];
        if (curse->effects == NULL || curse->effectCount == 0)
            continue;

        for (j = 0; j < curse->effectCount; j++)
        {
            const struct CurseEffect *effect = &curse->effects[j];

            if (effect->type != CURSE_EFF_DAMAGE_TAKEN_MULT)
                continue;

            if (!CurseSelectorMatches(&effect->selector, ctx))
                continue;

            switch (effect->stacking)
            {
            case CURSE_STACK_MULTIPLY:
            default:
                modifier = uq4_12_multiply(modifier, effect->multiplier);
                break;
            }
        }
    }

    return modifier;
}

void Curse_ClearActive(void)
{
    u32 i;

    for (i = 0; i < CURSE_ACTIVE_BOON_SLOTS; i++)
        gSaveBlock2Ptr->curses.activeBoons[i] = CURSE_NONE;

    for (i = 0; i < CURSE_ACTIVE_BANE_SLOTS; i++)
        gSaveBlock2Ptr->curses.activeBanes[i] = CURSE_NONE;
}

void Curse_SetActiveBoon(u8 slot, u16 curseId)
{
    if (gSaveBlock2Ptr == NULL)
        return;

    if (slot >= CURSE_ACTIVE_BOON_SLOTS)
        return;

    gSaveBlock2Ptr->curses.activeBoons[slot] = curseId;
}

void Curse_SetActiveBane(u8 slot, u16 curseId)
{
    if (gSaveBlock2Ptr == NULL)
        return;

    if (slot >= CURSE_ACTIVE_BANE_SLOTS)
        return;

    gSaveBlock2Ptr->curses.activeBanes[slot] = curseId;
}

u16 Curse_GetActiveBoon(u8 slot)
{
    if (gSaveBlock2Ptr == NULL || slot >= CURSE_ACTIVE_BOON_SLOTS)
        return CURSE_NONE;

    return gSaveBlock2Ptr->curses.activeBoons[slot];
}

u16 Curse_GetActiveBane(u8 slot)
{
    if (gSaveBlock2Ptr == NULL || slot >= CURSE_ACTIVE_BANE_SLOTS)
        return CURSE_NONE;

    return gSaveBlock2Ptr->curses.activeBanes[slot];
}

void Curse_InitDefaults(void)
{
    Curse_ClearActive();
    Curse_SetActiveBoon(0, CURSE_BOON_BULWARK);
}

uq4_12_t Curse_GetDamageTakenModifier(const struct DamageContext *ctx)
{
    uq4_12_t modifier = UQ_4_12(1.0);

    if (gSaveBlock2Ptr == NULL)
        return modifier;

    modifier = ApplyCurseEffects(modifier, ctx, gSaveBlock2Ptr->curses.activeBoons, CURSE_ACTIVE_BOON_SLOTS);
    modifier = ApplyCurseEffects(modifier, ctx, gSaveBlock2Ptr->curses.activeBanes, CURSE_ACTIVE_BANE_SLOTS);
    return modifier;
}

u16 Curse_GetCount(void)
{
    return CURSE_COUNT;
}

const struct CurseDef *Curse_GetDef(u16 curseId)
{
    if (curseId == CURSE_NONE || curseId >= CURSE_COUNT)
        return NULL;

    if (sCurseDefs[curseId].effects == NULL || sCurseDefs[curseId].effectCount == 0)
        return NULL;

    return &sCurseDefs[curseId];
}

bool32 Curse_IsImplemented(u16 curseId)
{
    return (Curse_GetDef(curseId) != NULL);
}
