#ifndef GUARD_RELIC_H
#define GUARD_RELIC_H

#include "gba/types.h"
#include "fpmath.h"
#include "constants/relics.h"

struct DamageContext;

enum RelicHook
{
    RELIC_HOOK_BATTLE_DAMAGE_TAKEN,
    RELIC_HOOK_BATTLE_ACCURACY_BONUS,
    RELIC_HOOK_STARTER_POOL_BUILD,
};

enum RelicAcquireReason
{
    RELIC_ACQUIRE_START_IDENTITY,
    RELIC_ACQUIRE_GYM_PASSIVE,
    RELIC_ACQUIRE_WAGER_TRADE_UP,
    RELIC_ACQUIRE_WAGER_UPGRADE,
    RELIC_ACQUIRE_OTHER,
};

enum RelicStarterPoolOp
{
    RELIC_STARTER_POOL_ADD,
    RELIC_STARTER_POOL_REMOVE,
};

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

struct RelicStarterPoolFilterParams
{
    u16 species;
    u8 operation;
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
    u16 acquisitionFlags;
};

struct RelicsSaveData
{
    u8 slotId[RELIC_SLOTS];
    u8 slotTier[RELIC_SLOTS];
    u8 activeMarks;
};

#define RELIC_OFFER_MAX_CHOICES 8

struct RelicOfferSpec
{
    enum RelicAcquireReason reason;
    u8 choiceCount;
    bool8 allowDuplicates;
    bool8 excludeOwnedRelics;
    u16 requiredFlags;
    u8 rarityWeights[RELIC_RARITY_COUNT];
};

struct RelicOffer
{
    u8 count;
    u8 relicIds[RELIC_OFFER_MAX_CHOICES];
};

struct RelicHookContext
{
    union
    {
        struct
        {
            const struct DamageContext *ctx;
        } damageTaken;
        struct
        {
            u32 battlerAtk;
            u32 move;
        } accuracyBonus;
        struct
        {
            u16 *species;
            u8 count;
            u8 capacity;
        } starterPool;
    };
};

struct RelicHookResult
{
    bool32 changed;
    uq4_12_t damageTakenModifier;
    s32 accuracyBonus;
    u8 starterPoolCount;
};

#define RELIC_TYPE_PTR_MATCH(ptr, type) \
    (__builtin_types_compatible_p(__typeof__(ptr), type *) || __builtin_types_compatible_p(__typeof__(ptr), const type *))

#define RELIC_TYPED_PARAMS(ptr, type) \
    ((void)sizeof(char[(RELIC_TYPE_PTR_MATCH((ptr), type)) ? 1 : -1]), (const void *)(ptr))

#define RELIC_EFFECT_DAMAGE_TAKEN(stackingRule, paramsPtr) \
    {                                                       \
        .type = RELIC_EFF_DAMAGE_TAKEN_MULT,               \
        .stacking = (stackingRule),                        \
        .params = RELIC_TYPED_PARAMS((paramsPtr), struct RelicBattleDamageTakenParams), \
    }

#define RELIC_EFFECT_ACCURACY_FLAT(stackingRule, paramsPtr) \
    {                                                        \
        .type = RELIC_EFF_ACCURACY_FLAT_BONUS,              \
        .stacking = (stackingRule),                         \
        .params = RELIC_TYPED_PARAMS((paramsPtr), struct RelicBattleAccuracyFlatParams), \
    }

#define RELIC_EFFECT_STARTER_POOL(stackingRule, paramsPtr) \
    {                                                       \
        .type = RELIC_EFF_STARTER_POOL_FILTER,             \
        .stacking = (stackingRule),                        \
        .params = RELIC_TYPED_PARAMS((paramsPtr), struct RelicStarterPoolFilterParams), \
    }

void Relic_InitDefaults(void);
void Relic_ClearAll(void);
void Relic_SetSlot(u8 slot, u8 relicId, u8 tier);
void Relic_ClearSlot(u8 slot);
u8 Relic_GetSlotId(u8 slot);
u8 Relic_GetSlotTier(u8 slot);
void Relic_UpgradeSlot(u8 slot);
u8 Relic_GetMaxActiveSlots(void);
uq4_12_t Relic_GetDamageTakenModifier(const struct DamageContext *ctx);
s32 Relic_GetAccuracyBonus(u32 battlerAtk, u32 move);
bool32 Relic_ApplyHook(enum RelicHook hook, const struct RelicHookContext *ctx, struct RelicHookResult *out);
u16 Relic_GetCount(void);
const struct RelicDef *Relic_GetDef(u8 relicId);
bool32 Relic_IsImplemented(u8 relicId);
bool32 Relic_BuildOffer(const struct RelicOfferSpec *spec, struct RelicOffer *out, u32 rngSeed);
bool32 Relic_IsOfferChoiceValid(const struct RelicOffer *offer, u8 choiceIndex);

#endif // GUARD_RELIC_H
