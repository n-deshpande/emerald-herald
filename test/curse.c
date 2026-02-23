#include "global.h"
#include "curse.h"
#include "test/test.h"

TEST("Curse_ClearActive clears boon and bane slots")
{
    Curse_SetActiveBoon(0, CURSE_BOON_BULWARK);
    Curse_SetActiveBane(0, CURSE_BANE_EXPOSED);
    Curse_ClearActive();

    EXPECT_EQ(Curse_GetActiveBoon(0), CURSE_NONE);
    EXPECT_EQ(Curse_GetActiveBane(0), CURSE_NONE);
}

TEST("Curse_SetActiveBane sets the target slot")
{
    Curse_ClearActive();
    Curse_SetActiveBane(1, CURSE_BANE_EXPOSED);

    EXPECT_EQ(Curse_GetActiveBane(1), CURSE_BANE_EXPOSED);
}

TEST("Curse active getters return CURSE_NONE for invalid slots")
{
    Curse_ClearActive();

    EXPECT_EQ(Curse_GetActiveBoon(CURSE_ACTIVE_BOON_SLOTS), CURSE_NONE);
    EXPECT_EQ(Curse_GetActiveBane(CURSE_ACTIVE_BANE_SLOTS), CURSE_NONE);
}

TEST("Curse active getters return set values for valid slots")
{
    Curse_ClearActive();
    Curse_SetActiveBoon(2, CURSE_BOON_BULWARK);
    Curse_SetActiveBane(2, CURSE_BANE_EXPOSED);

    EXPECT_EQ(Curse_GetActiveBoon(2), CURSE_BOON_BULWARK);
    EXPECT_EQ(Curse_GetActiveBane(2), CURSE_BANE_EXPOSED);
}
