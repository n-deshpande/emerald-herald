#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "relic.h"
#include "event_data.h"
#include "move.h"
#include "new_game.h"

#include "constants/relics.h"
#include "constants/pokemon.h"
#include "constants/vars.h"

#include "data/relics.h"

STATIC_ASSERT(ARRAY_COUNT(sRelicDefs) == RELIC_COUNT, RelicDefsCountMismatch);

// 5 active slots * 3 effects per relic = 15 max; round up for safety.
#define MAX_RELIC_MATCHED_EFFECTS 18

#define RELIC_DEFAULT_WEIGHT_COMMON 80
#define RELIC_DEFAULT_WEIGHT_UNCOMMON 15
#define RELIC_DEFAULT_WEIGHT_RARE 5

#ifndef NDEBUG
#define RELIC_DEBUG_ASSERT(condition) AGB_ASSERT(condition)
#else
#define RELIC_DEBUG_ASSERT(condition) ((void)0)
#endif

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

static u8 GetRarityWeightByIndex(const u8 rarityWeights[RELIC_RARITY_COUNT], u8 rarity)
{
    if (rarity >= RELIC_RARITY_COUNT)
        return 0;

    return rarityWeights[rarity];
}

static bool32 GetEffectHook(u8 effectType, enum RelicHook *hook)
{
    switch (effectType)
    {
    case RELIC_EFF_DAMAGE_TAKEN_MULT:
        *hook = RELIC_HOOK_BATTLE_DAMAGE_TAKEN;
        return TRUE;
    case RELIC_EFF_ACCURACY_FLAT_BONUS:
        *hook = RELIC_HOOK_BATTLE_ACCURACY_BONUS;
        return TRUE;
    case RELIC_EFF_STARTER_POOL_FILTER:
        *hook = RELIC_HOOK_STARTER_POOL_BUILD;
        return TRUE;
    case RELIC_EFF_STARTER_SLOT_OVERRIDE:
        *hook = RELIC_HOOK_STARTER_POOL_BUILD;
        return TRUE;
    default:
        return FALSE;
    }
}

static bool32 EffectStackingIsValid(const struct RelicEffect *effect)
{
    switch (effect->type)
    {
    case RELIC_EFF_DAMAGE_TAKEN_MULT:
        return effect->stacking == RELIC_STACK_MULTIPLY;
    case RELIC_EFF_ACCURACY_FLAT_BONUS:
        return effect->stacking == RELIC_STACK_ADD_FLAT;
    case RELIC_EFF_STARTER_POOL_FILTER:
        return effect->stacking == RELIC_STACK_OVERRIDE || effect->stacking == RELIC_STACK_ADD_FLAT;
    case RELIC_EFF_STARTER_SLOT_OVERRIDE:
        return effect->stacking == RELIC_STACK_OVERRIDE;
    default:
        return FALSE;
    }
}

static u32 CollectEffectsByHook(enum RelicHook hook, const struct RelicEffect *matched[], bool32 *overflowed)
{
    u32 numMatched = 0;
    u32 i;

    *overflowed = FALSE;

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
            enum RelicHook effectHook;

            if (!GetEffectHook(effect->type, &effectHook))
            {
                RELIC_DEBUG_ASSERT(FALSE);
                continue;
            }

            if (effectHook != hook)
                continue;

            if (!EffectStackingIsValid(effect))
            {
                RELIC_DEBUG_ASSERT(FALSE);
                continue;
            }

            if (numMatched >= MAX_RELIC_MATCHED_EFFECTS)
            {
                *overflowed = TRUE;
                RELIC_DEBUG_ASSERT(FALSE);
                continue;
            }

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
            modifier = uq4_12_multiply(modifier, params->multiplier);
            break;
        default:
            RELIC_DEBUG_ASSERT(FALSE);
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
        case RELIC_STACK_ADD_FLAT:
            bonus += params->flatBonus;
            break;
        default:
            RELIC_DEBUG_ASSERT(FALSE);
            break;
        }
    }

    return bonus;
}

static bool32 StarterPoolContainsSpecies(const u16 species[], u8 count, u16 target)
{
    u8 i;

    for (i = 0; i < count; i++)
    {
        if (species[i] == target)
            return TRUE;
    }

    return FALSE;
}

static u8 StarterPoolRemoveSpecies(u16 species[], u8 count, u16 target)
{
    u8 write = 0;
    u8 read;

    for (read = 0; read < count; read++)
    {
        if (species[read] != target)
            species[write++] = species[read];
    }

    return write;
}

static u16 ChooseStarterSlotOverrideSpecies(const struct RelicStarterSlotOverrideParams *params)
{
    u32 trainerId;

    if (params == NULL)
        return SPECIES_NONE;

    trainerId = GetTrainerId(gSaveBlock2Ptr->playerTrainerId);
    if (((trainerId >> params->slot) & 1) != 0)
        return params->speciesSecondary;

    return params->speciesPrimary;
}

static u8 ApplyStarterPoolEffects(u16 species[], u8 count, u8 capacity, const struct RelicEffect *matched[], u32 numMatched, bool32 *changed)
{
    u32 i;

    for (i = 0; i < numMatched; i++)
    {
        switch (matched[i]->type)
        {
        case RELIC_EFF_STARTER_POOL_FILTER:
            {
                const struct RelicStarterPoolFilterParams *params = matched[i]->params;

                if (params == NULL)
                    break;

                switch (params->operation)
                {
                case RELIC_STARTER_POOL_ADD:
                    if (count < capacity && !StarterPoolContainsSpecies(species, count, params->species))
                    {
                        species[count++] = params->species;
                        *changed = TRUE;
                    }
                    break;
                case RELIC_STARTER_POOL_REMOVE:
                {
                    u8 prevCount = count;
                    count = StarterPoolRemoveSpecies(species, count, params->species);
                    if (count != prevCount)
                        *changed = TRUE;
                    break;
                }
                default:
                    RELIC_DEBUG_ASSERT(FALSE);
                    break;
                }
            }
            break;
        case RELIC_EFF_STARTER_SLOT_OVERRIDE:
        {
            const struct RelicStarterSlotOverrideParams *params = matched[i]->params;
            u16 replacement;

            if (params == NULL || params->slot >= capacity)
                break;

            replacement = ChooseStarterSlotOverrideSpecies(params);
            if (replacement == SPECIES_NONE)
                break;

            if (params->slot >= count)
                count = params->slot + 1;

            if (species[params->slot] != replacement)
            {
                species[params->slot] = replacement;
                *changed = TRUE;
            }
            break;
        }
        default:
            RELIC_DEBUG_ASSERT(FALSE);
            break;
        }
    }

    return count;
}

static u16 GetAcquireReasonFlag(enum RelicAcquireReason reason)
{
    switch (reason)
    {
    case RELIC_ACQUIRE_START_IDENTITY:
        return RELIC_ACQF_IDENTITY_DRAFT;
    case RELIC_ACQUIRE_GYM_PASSIVE:
        return RELIC_ACQF_MILESTONE_DRAW;
    case RELIC_ACQUIRE_WAGER_TRADE_UP:
        return RELIC_ACQF_TRADE_UP_TARGET;
    case RELIC_ACQUIRE_WAGER_UPGRADE:
    case RELIC_ACQUIRE_OTHER:
    default:
        return 0;
    }
}

static bool32 IsRelicOwned(u8 relicId)
{
    u8 i;

    for (i = 0; i < RELIC_SLOTS; i++)
    {
        if (gSaveBlock2Ptr->relics.slotId[i] == relicId)
            return TRUE;
    }

    return FALSE;
}

static u32 NextOfferSeed(u32 *seed)
{
    if (*seed == 0)
        *seed = 0x9E3779B9;

    *seed ^= *seed << 13;
    *seed ^= *seed >> 17;
    *seed ^= *seed << 5;
    return *seed;
}

static s32 ChooseWeightedCandidateIndex(const u8 candidateIds[], u32 candidateCount, const u8 rarityWeights[RELIC_RARITY_COUNT], u32 *seed)
{
    u32 totalWeight = 0;
    u32 i;
    u32 roll;

    for (i = 0; i < candidateCount; i++)
    {
        const struct RelicDef *def = Relic_GetDef(candidateIds[i]);

        if (def != NULL)
            totalWeight += GetRarityWeightByIndex(rarityWeights, def->rarity);
    }

    if (totalWeight == 0)
        return -1;

    roll = NextOfferSeed(seed) % totalWeight;

    for (i = 0; i < candidateCount; i++)
    {
        const struct RelicDef *def = Relic_GetDef(candidateIds[i]);
        u32 weight = (def == NULL) ? 0 : GetRarityWeightByIndex(rarityWeights, def->rarity);

        if (roll < weight)
            return i;
        roll -= weight;
    }

    return -1;
}

static void GetEffectiveOfferWeights(const struct RelicOfferSpec *spec, u8 rarityWeights[RELIC_RARITY_COUNT])
{
    u8 i;
    u32 total = 0;

    for (i = 0; i < RELIC_RARITY_COUNT; i++)
    {
        rarityWeights[i] = spec->rarityWeights[i];
        total += rarityWeights[i];
    }

    if (total == 0)
    {
        rarityWeights[RELIC_RARITY_COMMON] = RELIC_DEFAULT_WEIGHT_COMMON;
        rarityWeights[RELIC_RARITY_UNCOMMON] = RELIC_DEFAULT_WEIGHT_UNCOMMON;
        rarityWeights[RELIC_RARITY_RARE] = RELIC_DEFAULT_WEIGHT_RARE;
    }
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

u8 Relic_GetMaxActiveSlots(void)
{
    return RELIC_SLOTS;
}

void Relic_InitDefaults(void)
{
    Relic_ClearAll();
    VarSet(VAR_STARTER_OFFER_LOCKED, FALSE);
    VarSet(VAR_STARTER_OFFER_SLOT_0_SPECIES, SPECIES_NONE);
    VarSet(VAR_STARTER_OFFER_SLOT_1_SPECIES, SPECIES_NONE);
    VarSet(VAR_STARTER_OFFER_SLOT_2_SPECIES, SPECIES_NONE);
    Relic_SetSlot(0, RELIC_DRACONIC_ANCESTRY, RELIC_TIER_1);
    Relic_SetSlot(1, RELIC_BULWARK, RELIC_TIER_1);
    Relic_SetSlot(2, RELIC_PRECISION, RELIC_TIER_1);
    Relic_SetSlot(3, RELIC_EXPOSED, RELIC_TIER_1);
}

bool32 Relic_ApplyHook(enum RelicHook hook, const struct RelicHookContext *ctx, struct RelicHookResult *out)
{
    const struct RelicEffect *matched[MAX_RELIC_MATCHED_EFFECTS];
    bool32 overflowed = FALSE;
    u32 numMatched;

    if (ctx == NULL || out == NULL)
        return FALSE;

    out->changed = FALSE;
    out->damageTakenModifier = UQ_4_12(1.0);
    out->accuracyBonus = 0;
    out->starterPoolCount = 0;

    if (gSaveBlock2Ptr == NULL)
    {
        switch (hook)
        {
        case RELIC_HOOK_BATTLE_DAMAGE_TAKEN:
        case RELIC_HOOK_BATTLE_ACCURACY_BONUS:
            return TRUE;
        case RELIC_HOOK_STARTER_POOL_BUILD:
            out->starterPoolCount = ctx->starterPool.count;
            return TRUE;
        default:
            return FALSE;
        }
    }

    numMatched = CollectEffectsByHook(hook, matched, &overflowed);
    if (overflowed)
        RELIC_DEBUG_ASSERT(FALSE);

    switch (hook)
    {
    case RELIC_HOOK_BATTLE_DAMAGE_TAKEN:
        if (ctx->damageTaken.ctx == NULL)
            return FALSE;

        out->damageTakenModifier = ApplyDamageStacking(out->damageTakenModifier, matched, numMatched, ctx->damageTaken.ctx);
        out->changed = (out->damageTakenModifier != UQ_4_12(1.0));
        return TRUE;
    case RELIC_HOOK_BATTLE_ACCURACY_BONUS:
    {
        u8 moveType = GetMoveType(ctx->accuracyBonus.move);
        out->accuracyBonus = ApplyAccuracyStacking(out->accuracyBonus, matched, numMatched, ctx->accuracyBonus.battlerAtk, ctx->accuracyBonus.move, moveType);
        out->changed = (out->accuracyBonus != 0);
        return TRUE;
    }
    case RELIC_HOOK_STARTER_POOL_BUILD:
        if (ctx->starterPool.species == NULL || ctx->starterPool.capacity == 0)
            return FALSE;

        out->starterPoolCount = ctx->starterPool.count;
        if (out->starterPoolCount > ctx->starterPool.capacity)
            out->starterPoolCount = ctx->starterPool.capacity;

        out->starterPoolCount = ApplyStarterPoolEffects(ctx->starterPool.species, out->starterPoolCount, ctx->starterPool.capacity, matched, numMatched, &out->changed);
        return TRUE;
    default:
        return FALSE;
    }
}

uq4_12_t Relic_GetDamageTakenModifier(const struct DamageContext *ctx)
{
    struct RelicHookContext hookCtx = {0};
    struct RelicHookResult result = {0};

    if (ctx == NULL)
        return UQ_4_12(1.0);

    hookCtx.damageTaken.ctx = ctx;
    if (!Relic_ApplyHook(RELIC_HOOK_BATTLE_DAMAGE_TAKEN, &hookCtx, &result))
        return UQ_4_12(1.0);

    return result.damageTakenModifier;
}

s32 Relic_GetAccuracyBonus(u32 battlerAtk, u32 move)
{
    struct RelicHookContext hookCtx = {0};
    struct RelicHookResult result = {0};

    hookCtx.accuracyBonus.battlerAtk = battlerAtk;
    hookCtx.accuracyBonus.move = move;

    if (!Relic_ApplyHook(RELIC_HOOK_BATTLE_ACCURACY_BONUS, &hookCtx, &result))
        return 0;

    return result.accuracyBonus;
}

u16 Relic_GetCount(void)
{
    return RELIC_COUNT;
}

const struct RelicDef *Relic_GetDef(u8 relicId)
{
    if (relicId == RELIC_NONE || relicId >= RELIC_COUNT)
        return NULL;

    if (sRelicDefs[relicId].name == NULL)
        return NULL;

    return &sRelicDefs[relicId];
}

bool32 Relic_IsImplemented(u8 relicId)
{
    return (Relic_GetDef(relicId) != NULL);
}

bool32 Relic_BuildOffer(const struct RelicOfferSpec *spec, struct RelicOffer *out, u32 rngSeed)
{
    u8 candidates[RELIC_COUNT];
    u8 rarityWeights[RELIC_RARITY_COUNT] = {0};
    u32 candidateCount = 0;
    u32 i;
    u8 requestedCount;
    u16 acquireFlag;
    u32 seed = rngSeed;

    if (spec == NULL || out == NULL || spec->choiceCount == 0 || spec->choiceCount > RELIC_OFFER_MAX_CHOICES)
        return FALSE;

    out->count = 0;
    for (i = 0; i < RELIC_OFFER_MAX_CHOICES; i++)
        out->relicIds[i] = RELIC_NONE;

    if (gSaveBlock2Ptr == NULL)
        return FALSE;

    acquireFlag = GetAcquireReasonFlag(spec->reason);
    for (i = RELIC_NONE + 1; i < RELIC_COUNT; i++)
    {
        const struct RelicDef *def = Relic_GetDef(i);

        if (def == NULL)
            continue;

        if (acquireFlag != 0 && !(def->acquisitionFlags & acquireFlag))
            continue;

        if (spec->requiredFlags != 0 && (def->acquisitionFlags & spec->requiredFlags) != spec->requiredFlags)
            continue;

        if (spec->excludeOwnedRelics && IsRelicOwned(i))
            continue;

        candidates[candidateCount++] = i;
    }

    if (candidateCount == 0)
        return FALSE;

    GetEffectiveOfferWeights(spec, rarityWeights);
    requestedCount = spec->choiceCount;
    if (!spec->allowDuplicates && requestedCount > candidateCount)
        requestedCount = candidateCount;

    while (out->count < requestedCount)
    {
        s32 selectedIndex = ChooseWeightedCandidateIndex(candidates, candidateCount, rarityWeights, &seed);

        if (selectedIndex < 0)
            break;

        out->relicIds[out->count++] = candidates[selectedIndex];

        if (!spec->allowDuplicates)
        {
            candidateCount--;
            candidates[selectedIndex] = candidates[candidateCount];
            if (candidateCount == 0)
                break;
        }
    }

    return out->count > 0;
}

bool32 Relic_IsOfferChoiceValid(const struct RelicOffer *offer, u8 choiceIndex)
{
    u8 relicId;

    if (offer == NULL)
        return FALSE;

    if (choiceIndex >= offer->count || choiceIndex >= RELIC_OFFER_MAX_CHOICES)
        return FALSE;

    relicId = offer->relicIds[choiceIndex];
    if (relicId == RELIC_NONE || relicId >= RELIC_COUNT)
        return FALSE;

    return Relic_IsImplemented(relicId);
}
