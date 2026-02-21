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

// 6 active slots * 3 effects per curse = 18 max; round up for safety.
#define MAX_CURSE_MATCHED_EFFECTS 18

static bool32 CurseSelectorMatches(const struct CurseSelector *selector, u32 battler, u32 move, u8 moveType)
{
    enum BattleSide side = GetBattlerSide(battler);
    enum DamageCategory category = GetMoveCategory(move);

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

    if (selector->moveType != TYPE_NONE && selector->moveType != moveType)
        return FALSE;

    if (selector->moveCategory != CURSE_MOVE_CATEGORY_ANY && selector->moveCategory != category)
        return FALSE;

    if (selector->minHpPct != 0)
    {
        u32 maxHp = gBattleMons[battler].maxHP;
        u32 currHp = gBattleMons[battler].hp;
        u32 hpPct = (maxHp == 0) ? 0 : (currHp * 100) / maxHp;

        if (hpPct < selector->minHpPct)
            return FALSE;
    }

    return TRUE;
}

static u32 CollectMatchingEffects(u8 effectType, u32 battler, u32 move, u8 moveType,
                                   const u16 *curseIds, u32 count,
                                   const struct CurseEffect *matched[])
{
    u32 numMatched = 0;
    u32 i;

    for (i = 0; i < count; i++)
    {
        u16 curseId = curseIds[i];
        const struct CurseDef *curse;
        u32 j;

        if (curseId == CURSE_NONE || curseId >= ARRAY_COUNT(sCurseDefs))
            continue;

        curse = &sCurseDefs[curseId];
        if (curse->effects == NULL || curse->effectCount == 0)
            continue;

        for (j = 0; j < curse->effectCount; j++)
        {
            const struct CurseEffect *effect = &curse->effects[j];

            if (effect->type != effectType)
                continue;

            if (!CurseSelectorMatches(&effect->selector, battler, move, moveType))
                continue;

            if (numMatched < MAX_CURSE_MATCHED_EFFECTS)
                matched[numMatched++] = effect;
        }
    }

    return numMatched;
}

static uq4_12_t ApplyDamageStacking(uq4_12_t modifier, const struct CurseEffect *matched[], u32 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        switch (matched[i]->stacking)
        {
        case CURSE_STACK_MULTIPLY:
        default:
            modifier = uq4_12_multiply(modifier, matched[i]->multiplier);
            break;
        }
    }

    return modifier;
}

static s32 ApplyAccuracyStacking(s32 bonus, const struct CurseEffect *matched[], u32 count)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        switch (matched[i]->stacking)
        {
        case CURSE_STACK_ADD_PCT:
        default:
            bonus += (s32)matched[i]->multiplier;
            break;
        }
    }

    return bonus;
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
    if (slot >= CURSE_ACTIVE_BOON_SLOTS)
        return;

    gSaveBlock2Ptr->curses.activeBoons[slot] = curseId;
}

void Curse_InitDefaults(void)
{
    Curse_ClearActive();
    Curse_SetActiveBoon(0, CURSE_BOON_BULWARK);
    Curse_SetActiveBoon(1, CURSE_BOON_PRECISION);
}

uq4_12_t Curse_GetDamageTakenModifier(const struct DamageContext *ctx)
{
    const struct CurseEffect *matched[MAX_CURSE_MATCHED_EFFECTS];
    uq4_12_t modifier = UQ_4_12(1.0);
    u32 numMatched;

    if (gSaveBlock2Ptr == NULL)
        return modifier;

    numMatched = CollectMatchingEffects(CURSE_EFF_DAMAGE_TAKEN_MULT, ctx->battlerDef, ctx->move, ctx->moveType,
                                        gSaveBlock2Ptr->curses.activeBoons, CURSE_ACTIVE_BOON_SLOTS, matched);
    modifier = ApplyDamageStacking(modifier, matched, numMatched);

    numMatched = CollectMatchingEffects(CURSE_EFF_DAMAGE_TAKEN_MULT, ctx->battlerDef, ctx->move, ctx->moveType,
                                        gSaveBlock2Ptr->curses.activeBanes, CURSE_ACTIVE_BANE_SLOTS, matched);
    modifier = ApplyDamageStacking(modifier, matched, numMatched);

    return modifier;
}

s32 Curse_GetAccuracyBonus(u32 battlerAtk, u32 move)
{
    const struct CurseEffect *matched[MAX_CURSE_MATCHED_EFFECTS];
    s32 bonus = 0;
    u32 numMatched;

    if (gSaveBlock2Ptr == NULL)
        return 0;

    numMatched = CollectMatchingEffects(CURSE_EFF_ACCURACY_FLAT_BONUS, battlerAtk, move, GetMoveType(move),
                                        gSaveBlock2Ptr->curses.activeBoons, CURSE_ACTIVE_BOON_SLOTS, matched);
    bonus = ApplyAccuracyStacking(bonus, matched, numMatched);

    numMatched = CollectMatchingEffects(CURSE_EFF_ACCURACY_FLAT_BONUS, battlerAtk, move, GetMoveType(move),
                                        gSaveBlock2Ptr->curses.activeBanes, CURSE_ACTIVE_BANE_SLOTS, matched);
    bonus = ApplyAccuracyStacking(bonus, matched, numMatched);

    return bonus;
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
