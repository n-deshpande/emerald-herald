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

static bool32 SideMatches(u8 sideRule, u32 battler)
{
    switch (sideRule)
    {
    case CURSE_SIDE_PLAYER:
        if (GetBattlerSide(battler) != B_SIDE_PLAYER)
            return FALSE;
        break;
    case CURSE_SIDE_OPPONENT:
        if (GetBattlerSide(battler) != B_SIDE_OPPONENT)
            return FALSE;
        break;
    case CURSE_SIDE_BOTH:
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

static bool32 MoveConditionsMatch(u8 moveTypeRule, u8 moveCategoryRule, u32 move, u8 moveType)
{
    if (moveTypeRule != TYPE_NONE && moveTypeRule != moveType)
        return FALSE;

    if (moveCategoryRule != CURSE_MOVE_CATEGORY_ANY && moveCategoryRule != GetMoveCategory(move))
        return FALSE;

    return TRUE;
}

static bool32 MinHpMatches(u8 minHpPct, u32 battler)
{
    if (minHpPct != 0)
    {
        u32 maxHp = gBattleMons[battler].maxHP;
        u32 currHp = gBattleMons[battler].hp;
        u32 hpPct = (maxHp == 0) ? 0 : (currHp * 100) / maxHp;

        if (hpPct < minHpPct)
            return FALSE;
    }

    return TRUE;
}

static bool32 TypeMatchupMatches(u8 typeMatchupRule, uq4_12_t typeEffectivenessModifier)
{
    switch (typeMatchupRule)
    {
    case CURSE_TYPE_MATCHUP_ANY:
        return TRUE;
    case CURSE_TYPE_MATCHUP_SUPER_EFFECTIVE_ONLY:
        return (typeEffectivenessModifier > UQ_4_12(1.0));
    case CURSE_TYPE_MATCHUP_NOT_VERY_EFFECTIVE_ONLY:
        return (typeEffectivenessModifier > UQ_4_12(0.0) && typeEffectivenessModifier < UQ_4_12(1.0));
    case CURSE_TYPE_MATCHUP_NEUTRAL_ONLY:
        return (typeEffectivenessModifier == UQ_4_12(1.0));
    default:
        return FALSE;
    }
}

static bool32 BattleDamageTakenParamsMatch(const struct CurseBattleDamageTakenParams *params, const struct DamageContext *ctx)
{
    if (!SideMatches(params->side, ctx->battlerDef))
        return FALSE;

    if (!MoveConditionsMatch(params->moveType, params->moveCategory, ctx->move, ctx->moveType))
        return FALSE;

    if (!MinHpMatches(params->minHpPct, ctx->battlerDef))
        return FALSE;

    if (!TypeMatchupMatches(params->typeMatchup, ctx->typeEffectivenessModifier))
        return FALSE;

    return TRUE;
}

static bool32 BattleAccuracyParamsMatch(const struct CurseBattleAccuracyFlatParams *params, u32 battlerAtk, u32 move, u8 moveType)
{
    if (!SideMatches(params->side, battlerAtk))
        return FALSE;

    if (!MoveConditionsMatch(params->moveType, params->moveCategory, move, moveType))
        return FALSE;

    if (!MinHpMatches(params->minHpPct, battlerAtk))
        return FALSE;

    return TRUE;
}

static u32 CollectEffectsByType(u8 effectType, const u16 *curseIds, u32 count, const struct CurseEffect *matched[])
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

            if (numMatched < MAX_CURSE_MATCHED_EFFECTS)
                matched[numMatched++] = effect;
        }
    }

    return numMatched;
}

static uq4_12_t ApplyDamageStacking(uq4_12_t modifier, const struct CurseEffect *matched[], u32 count, const struct DamageContext *ctx)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        const struct CurseBattleDamageTakenParams *params = matched[i]->params;

        if (params == NULL)
            continue;

        if (!BattleDamageTakenParamsMatch(params, ctx))
            continue;

        switch (matched[i]->stacking)
        {
        case CURSE_STACK_MULTIPLY:
        default:
            modifier = uq4_12_multiply(modifier, params->multiplier);
            break;
        }
    }

    return modifier;
}

static s32 ApplyAccuracyStacking(s32 bonus, const struct CurseEffect *matched[], u32 count, u32 battlerAtk, u32 move, u8 moveType)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        const struct CurseBattleAccuracyFlatParams *params = matched[i]->params;

        if (params == NULL)
            continue;

        if (!BattleAccuracyParamsMatch(params, battlerAtk, move, moveType))
            continue;

        switch (matched[i]->stacking)
        {
        case CURSE_STACK_ADD_PCT:
        default:
            bonus += params->flatBonus;
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
    Curse_SetActiveBoon(1, CURSE_BOON_PRECISION);
    Curse_SetActiveBane(0, CURSE_BANE_EXPOSED);
}

uq4_12_t Curse_GetDamageTakenModifier(const struct DamageContext *ctx)
{
    const struct CurseEffect *matched[MAX_CURSE_MATCHED_EFFECTS];
    uq4_12_t modifier = UQ_4_12(1.0);
    u32 numMatched;

    if (gSaveBlock2Ptr == NULL)
        return modifier;

    numMatched = CollectEffectsByType(CURSE_EFF_DAMAGE_TAKEN_MULT,
                                      gSaveBlock2Ptr->curses.activeBoons, CURSE_ACTIVE_BOON_SLOTS, matched);
    modifier = ApplyDamageStacking(modifier, matched, numMatched, ctx);

    numMatched = CollectEffectsByType(CURSE_EFF_DAMAGE_TAKEN_MULT,
                                      gSaveBlock2Ptr->curses.activeBanes, CURSE_ACTIVE_BANE_SLOTS, matched);
    modifier = ApplyDamageStacking(modifier, matched, numMatched, ctx);

    return modifier;
}

s32 Curse_GetAccuracyBonus(u32 battlerAtk, u32 move)
{
    const struct CurseEffect *matched[MAX_CURSE_MATCHED_EFFECTS];
    s32 bonus = 0;
    u32 numMatched;
    u8 moveType;

    if (gSaveBlock2Ptr == NULL)
        return 0;

    moveType = GetMoveType(move);

    numMatched = CollectEffectsByType(CURSE_EFF_ACCURACY_FLAT_BONUS,
                                      gSaveBlock2Ptr->curses.activeBoons, CURSE_ACTIVE_BOON_SLOTS, matched);
    bonus = ApplyAccuracyStacking(bonus, matched, numMatched, battlerAtk, move, moveType);

    numMatched = CollectEffectsByType(CURSE_EFF_ACCURACY_FLAT_BONUS,
                                      gSaveBlock2Ptr->curses.activeBanes, CURSE_ACTIVE_BANE_SLOTS, matched);
    bonus = ApplyAccuracyStacking(bonus, matched, numMatched, battlerAtk, move, moveType);

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
