#ifndef GUARD_CONSTANTS_CURSES_H
#define GUARD_CONSTANTS_CURSES_H

// Curse ids (update CURSE_COUNT when adding new ids)
#define CURSE_NONE 0
#define CURSE_BOON_BULWARK 1
#define CURSE_COUNT 2

// Effect types
enum CurseEffectType
{
    CURSE_EFF_DAMAGE_TAKEN_MULT,
};

// Stacking rules
enum CurseStacking
{
    CURSE_STACK_MULTIPLY,
    CURSE_STACK_ADD_PCT,
    CURSE_STACK_MAX,
    CURSE_STACK_MIN,
    CURSE_STACK_OVERRIDE,
};

// Selector values
enum CurseSide
{
    CURSE_SIDE_PLAYER,
    CURSE_SIDE_OPPONENT,
    CURSE_SIDE_BOTH,
};

#define CURSE_MOVE_CATEGORY_ANY 0xFF

// Active slots
#define CURSE_ACTIVE_BOON_SLOTS 3
#define CURSE_ACTIVE_BANE_SLOTS 3

#endif // GUARD_CONSTANTS_CURSES_H
