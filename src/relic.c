#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "relic.h"
#include "event_data.h"
#include "move.h"

#include "constants/relics.h"
#include "constants/pokemon.h"

#include "data/relics.h"

STATIC_ASSERT(ARRAY_COUNT(sRelicDefs) == RELIC_COUNT, RelicDefsCountMismatch);

// 5 active slots * 3 effects per relic = 15 max; round up for safety.
#define MAX_RELIC_MATCHED_EFFECTS 18

static bool32 SideMatches(u8 sideRule, u32 battler)
{
    switch (sideRule)
    {
    case RELIC_SIDE_PLAYER:
        if (GetBattlerSide(battler) != B_SIDE_PLAYER)
            return FALSE;
        break;
    case RELIC_SIDE_OPPONENT:
        if (GetBattlerSide(battler) != B_SIDE_OPPONENT)
            return FALSE;
        break;
    case RELIC_SIDE_BOTH:
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

    if (moveCategoryRule != RELIC_MOVE_CATEGORY_ANY && moveCategoryRule != GetMoveCategory(move))
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
    case RELIC_TYPE_MATCHUP_ANY:
        return TRUE;
    case RELIC_TYPE_MATCHUP_SUPER_EFFECTIVE_ONLY:
        return (typeEffectivenessModifier > UQ_4_12(1.0));
    case RELIC_TYPE_MATCHUP_NOT_VERY_EFFECTIVE_ONLY:
        return (typeEffectivenessModifier > UQ_4_12(0.0) && typeEffectivenessModifier < UQ_4_12(1.0));
    case RELIC_TYPE_MATCHUP_NEUTRAL_ONLY:
        return (typeEffectivenessModifier == UQ_4_12(1.0));
    default:
        return FALSE;
    }
}

static bool32 BattleDamageTakenParamsMatch(const struct RelicBattleDamageTakenParams *params, const struct DamageContext *ctx)
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

static bool32 BattleAccuracyParamsMatch(const struct RelicBattleAccuracyFlatParams *params, u32 battlerAtk, u32 move, u8 moveType)
{
    if (!SideMatches(params->side, battlerAtk))
        return FALSE;

    if (!MoveConditionsMatch(params->moveType, params->moveCategory, move, moveType))
        return FALSE;

    if (!MinHpMatches(params->minHpPct, battlerAtk))
        return FALSE;

    return TRUE;
}

static u32 CollectEffectsByType(u8 effectType, const struct RelicEffect *matched[])
{
    u32 numMatched = 0;
    u32 i;

    for (i = 0; i < RELIC_SLOTS; i++)
    {
        u8 relicId = gSaveBlock2Ptr->relics.slotId[i];
        u8 tier = gSaveBlock2Ptr->relics.slotTier[i];
        const struct RelicDef *relic;
        u32 j;

        if (relicId == RELIC_NONE || relicId >= ARRAY_COUNT(sRelicDefs))
            continue;

        if (tier > RELIC_MAX_TIER)
            tier = RELIC_MAX_TIER;

        relic = &sRelicDefs[relicId];
        if (relic->effects[tier] == NULL || relic->effectCount[tier] == 0)
            continue;

        for (j = 0; j < relic->effectCount[tier]; j++)
        {
            const struct RelicEffect *effect = &relic->effects[tier][j];

            if (effect->type != effectType)
                continue;

            if (numMatched < MAX_RELIC_MATCHED_EFFECTS)
                matched[numMatched++] = effect;
        }
    }

    return numMatched;
}

static uq4_12_t ApplyDamageStacking(uq4_12_t modifier, const struct RelicEffect *matched[], u32 count, const struct DamageContext *ctx)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        const struct RelicBattleDamageTakenParams *params = matched[i]->params;

        if (params == NULL)
            continue;

        if (!BattleDamageTakenParamsMatch(params, ctx))
            continue;

        switch (matched[i]->stacking)
        {
        case RELIC_STACK_MULTIPLY:
        default:
            modifier = uq4_12_multiply(modifier, params->multiplier);
            break;
        }
    }

    return modifier;
}

static s32 ApplyAccuracyStacking(s32 bonus, const struct RelicEffect *matched[], u32 count, u32 battlerAtk, u32 move, u8 moveType)
{
    u32 i;

    for (i = 0; i < count; i++)
    {
        const struct RelicBattleAccuracyFlatParams *params = matched[i]->params;

        if (params == NULL)
            continue;

        if (!BattleAccuracyParamsMatch(params, battlerAtk, move, moveType))
            continue;

        switch (matched[i]->stacking)
        {
        case RELIC_STACK_ADD_PCT:
        default:
            bonus += params->flatBonus;
            break;
        }
    }

    return bonus;
}

void Relic_ClearAll(void)
{
    u32 i;

    for (i = 0; i < RELIC_SLOTS; i++)
    {
        gSaveBlock2Ptr->relics.slotId[i] = RELIC_NONE;
        gSaveBlock2Ptr->relics.slotTier[i] = RELIC_TIER_1;
    }

    gSaveBlock2Ptr->relics.activeMarks = 0;
}

void Relic_SetSlot(u8 slot, u8 relicId, u8 tier)
{
    if (gSaveBlock2Ptr == NULL)
        return;

    if (slot >= RELIC_SLOTS)
        return;

    gSaveBlock2Ptr->relics.slotId[slot] = relicId;
    gSaveBlock2Ptr->relics.slotTier[slot] = tier;
}

void Relic_ClearSlot(u8 slot)
{
    if (gSaveBlock2Ptr == NULL)
        return;

    if (slot >= RELIC_SLOTS)
        return;

    gSaveBlock2Ptr->relics.slotId[slot] = RELIC_NONE;
    gSaveBlock2Ptr->relics.slotTier[slot] = RELIC_TIER_1;
}

u8 Relic_GetSlotId(u8 slot)
{
    if (gSaveBlock2Ptr == NULL || slot >= RELIC_SLOTS)
        return RELIC_NONE;

    return gSaveBlock2Ptr->relics.slotId[slot];
}

u8 Relic_GetSlotTier(u8 slot)
{
    if (gSaveBlock2Ptr == NULL || slot >= RELIC_SLOTS)
        return RELIC_TIER_1;

    return gSaveBlock2Ptr->relics.slotTier[slot];
}

void Relic_UpgradeSlot(u8 slot)
{
    if (gSaveBlock2Ptr == NULL || slot >= RELIC_SLOTS)
        return;

    if (gSaveBlock2Ptr->relics.slotTier[slot] < RELIC_MAX_TIER)
        gSaveBlock2Ptr->relics.slotTier[slot]++;
}

void Relic_InitDefaults(void)
{
    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_1);
    Relic_SetSlot(1, RELIC_PRECISION, RELIC_TIER_1);
    Relic_SetSlot(2, RELIC_EXPOSED, RELIC_TIER_1);
}

uq4_12_t Relic_GetDamageTakenModifier(const struct DamageContext *ctx)
{
    const struct RelicEffect *matched[MAX_RELIC_MATCHED_EFFECTS];
    uq4_12_t modifier = UQ_4_12(1.0);
    u32 numMatched;

    if (gSaveBlock2Ptr == NULL)
        return modifier;

    numMatched = CollectEffectsByType(RELIC_EFF_DAMAGE_TAKEN_MULT, matched);
    modifier = ApplyDamageStacking(modifier, matched, numMatched, ctx);

    return modifier;
}

s32 Relic_GetAccuracyBonus(u32 battlerAtk, u32 move)
{
    const struct RelicEffect *matched[MAX_RELIC_MATCHED_EFFECTS];
    s32 bonus = 0;
    u32 numMatched;
    u8 moveType;

    if (gSaveBlock2Ptr == NULL)
        return 0;

    moveType = GetMoveType(move);

    numMatched = CollectEffectsByType(RELIC_EFF_ACCURACY_FLAT_BONUS, matched);
    bonus = ApplyAccuracyStacking(bonus, matched, numMatched, battlerAtk, move, moveType);

    return bonus;
}

u16 Relic_GetCount(void)
{
    return RELIC_COUNT;
}

const struct RelicDef *Relic_GetDef(u8 relicId)
{
    if (relicId == RELIC_NONE || relicId >= RELIC_COUNT)
        return NULL;

    if (sRelicDefs[relicId].effects[RELIC_TIER_1] == NULL || sRelicDefs[relicId].effectCount[RELIC_TIER_1] == 0)
        return NULL;

    return &sRelicDefs[relicId];
}

bool32 Relic_IsImplemented(u8 relicId)
{
    return (Relic_GetDef(relicId) != NULL);
}
