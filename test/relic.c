#include "global.h"
#include "relic.h"
#include "test/test.h"

TEST("Relic_ClearAll clears all relic slots")
{
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_1);
    Relic_SetSlot(1, RELIC_EXPOSED, RELIC_TIER_1);
    Relic_ClearAll();

    EXPECT_EQ(Relic_GetSlotId(0), RELIC_NONE);
    EXPECT_EQ(Relic_GetSlotId(1), RELIC_NONE);
}

TEST("Relic_SetSlot sets the target slot")
{
    Relic_ClearAll();
    Relic_SetSlot(1, RELIC_EXPOSED, RELIC_TIER_2);

    EXPECT_EQ(Relic_GetSlotId(1), RELIC_EXPOSED);
    EXPECT_EQ(Relic_GetSlotTier(1), RELIC_TIER_2);
}

TEST("Relic slot getters return defaults for invalid slots")
{
    Relic_ClearAll();

    EXPECT_EQ(Relic_GetSlotId(RELIC_SLOTS), RELIC_NONE);
    EXPECT_EQ(Relic_GetSlotTier(RELIC_SLOTS), RELIC_TIER_1);
}

TEST("Relic slot getters return set values for valid slots")
{
    Relic_ClearAll();
    Relic_SetSlot(2, RELIC_BULWARK, RELIC_TIER_3);
    Relic_SetSlot(4, RELIC_EXPOSED, RELIC_TIER_1);

    EXPECT_EQ(Relic_GetSlotId(2), RELIC_BULWARK);
    EXPECT_EQ(Relic_GetSlotTier(2), RELIC_TIER_3);
    EXPECT_EQ(Relic_GetSlotId(4), RELIC_EXPOSED);
    EXPECT_EQ(Relic_GetSlotTier(4), RELIC_TIER_1);
}

TEST("Relic_ClearSlot clears a single slot")
{
    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_2);
    Relic_SetSlot(1, RELIC_PRECISION, RELIC_TIER_1);
    Relic_ClearSlot(0);

    EXPECT_EQ(Relic_GetSlotId(0), RELIC_NONE);
    EXPECT_EQ(Relic_GetSlotId(1), RELIC_PRECISION);
}

TEST("Relic_UpgradeSlot increases tier by one")
{
    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_TIER_1);
    Relic_UpgradeSlot(0);

    EXPECT_EQ(Relic_GetSlotTier(0), RELIC_TIER_2);
}

TEST("Relic_UpgradeSlot does not exceed max tier")
{
    Relic_ClearAll();
    Relic_SetSlot(0, RELIC_BULWARK, RELIC_MAX_TIER);
    Relic_UpgradeSlot(0);

    EXPECT_EQ(Relic_GetSlotTier(0), RELIC_MAX_TIER);
}
