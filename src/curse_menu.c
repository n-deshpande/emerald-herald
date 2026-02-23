#include "global.h"
#include "bg.h"
#include "curse.h"
#include "curse_menu.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

enum
{
    CURSE_TAB_BOONS,
    CURSE_TAB_BANES,
    CURSE_TAB_COUNT,
};

enum
{
    WIN_HEADER,
    WIN_TABS,
    WIN_LIST,
    WIN_DETAIL,
    WIN_FOOTER,
    WIN_COUNT,
};

#define CURSE_MENU_MAX_ENTRIES ((CURSE_ACTIVE_BOON_SLOTS > CURSE_ACTIVE_BANE_SLOTS) ? CURSE_ACTIVE_BOON_SLOTS : CURSE_ACTIVE_BANE_SLOTS)
#define CURSE_TAB_GAP 48
#define CURSE_TAB_PILL_PAD_X 8
#define CURSE_TAB_PILL_HEIGHT 14
#define CURSE_TAB_MARKER_BLINK_FRAMES 16
#define CURSE_LIST_ROW_HEIGHT 14
#define CURSE_LIST_ROW_Y_START 2
#define CURSE_TOP_STRIP_HEIGHT 10

struct CurseMenuData
{
    MainCallback callback;
    u8 tab;
    u8 cursor;
    bool8 tabMarkerVisible;
    bool8 closing;
    u8 counts[CURSE_TAB_COUNT];
    u16 entries[CURSE_TAB_COUNT][CURSE_MENU_MAX_ENTRIES];
};

static EWRAM_DATA struct CurseMenuData *sCurseMenu = NULL;

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [WIN_TABS] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 2,
        .width = 30,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 61,
    },
    [WIN_LIST] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 5,
        .width = 12,
        .height = 11,
        .paletteNum = 15,
        .baseBlock = 121,
    },
    [WIN_DETAIL] = {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 5,
        .width = 18,
        .height = 11,
        .paletteNum = 15,
        .baseBlock = 253,
    },
    [WIN_FOOTER] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 17,
        .width = 30,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 451,
    },
    DUMMY_WIN_TEMPLATE,
};

static const u8 sText_Curses[] = _("Curses");
static const u8 sText_Boons[] = _("Boons");
static const u8 sText_Banes[] = _("Banes");
static const u8 sText_NoActiveBoons[] = _("No Active Boons.");
static const u8 sText_NoActiveBanes[] = _("No Active Banes.");
static const u8 sText_DetailsUnavailable[] = _("No Curse Details Available.");
static const u8 sText_Level[] = _("Lv. {STR_VAR_1}");
static const u8 sText_Footer[] = _("L/R: Switch Tab   B: Exit");

static const u8 sTextColors_Primary[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColors_Secondary[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_WHITE};
static const u8 sTextColors_BoonAccent[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_GREEN, TEXT_COLOR_DARK_GRAY};
static const u8 sTextColors_BaneAccent[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_RED, TEXT_COLOR_DARK_GRAY};

static bool8 IsTabMarkerVisible(void)
{
    return ((gMain.vblankCounter1 / CURSE_TAB_MARKER_BLINK_FRAMES) & 1) == 0;
}

static void VBlankCB_CurseMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void BuildEntries(void)
{
    u32 i;

    sCurseMenu->counts[CURSE_TAB_BOONS] = 0;
    sCurseMenu->counts[CURSE_TAB_BANES] = 0;

    for (i = 0; i < CURSE_ACTIVE_BOON_SLOTS; i++)
    {
        u16 curseId = Curse_GetActiveBoon(i);

        if (Curse_GetDef(curseId) != NULL)
            sCurseMenu->entries[CURSE_TAB_BOONS][sCurseMenu->counts[CURSE_TAB_BOONS]++] = curseId;
    }

    for (i = 0; i < CURSE_ACTIVE_BANE_SLOTS; i++)
    {
        u16 curseId = Curse_GetActiveBane(i);

        if (Curse_GetDef(curseId) != NULL)
            sCurseMenu->entries[CURSE_TAB_BANES][sCurseMenu->counts[CURSE_TAB_BANES]++] = curseId;
    }
}

static u8 GetCurrentCount(void)
{
    return sCurseMenu->counts[sCurseMenu->tab];
}

static u16 GetCurrentCurseId(void)
{
    return sCurseMenu->entries[sCurseMenu->tab][sCurseMenu->cursor];
}

static void DrawHeader(void)
{
    s32 x;

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_HEADER, PIXEL_FILL(1), 0, 0, WindowWidthPx(WIN_HEADER), 1);
    FillWindowPixelRect(WIN_HEADER, PIXEL_FILL(1), 0, 15, WindowWidthPx(WIN_HEADER), 1);
    x = GetStringCenterAlignXOffset(FONT_NORMAL, sText_Curses, 30 * 8);
    AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, x, 1, sTextColors_Primary, TEXT_SKIP_DRAW, sText_Curses);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawTabs(void)
{
    u16 boonWidth = GetStringWidth(FONT_NORMAL, sText_Boons, 0);
    u16 baneWidth = GetStringWidth(FONT_NORMAL, sText_Banes, 0);
    u16 totalWidth = boonWidth + CURSE_TAB_GAP + baneWidth;
    s32 boonX = (WindowWidthPx(WIN_TABS) - totalWidth) / 2;
    s32 baneX = boonX + boonWidth + CURSE_TAB_GAP;
    s16 markerX = (sCurseMenu->tab == CURSE_TAB_BOONS) ? boonX - 8 : baneX - 8;
    s16 markerY = 1;

    FillWindowPixelBuffer(WIN_TABS, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_TABS, PIXEL_FILL(1), 0, 15, WindowWidthPx(WIN_TABS), 1);

    if (sCurseMenu->tab == CURSE_TAB_BOONS)
        FillWindowPixelRect(WIN_TABS, PIXEL_FILL(7), boonX - CURSE_TAB_PILL_PAD_X, 1, boonWidth + CURSE_TAB_PILL_PAD_X * 2, CURSE_TAB_PILL_HEIGHT);
    else
        FillWindowPixelRect(WIN_TABS, PIXEL_FILL(5), baneX - CURSE_TAB_PILL_PAD_X, 1, baneWidth + CURSE_TAB_PILL_PAD_X * 2, CURSE_TAB_PILL_HEIGHT);

    AddTextPrinterParameterized3(WIN_TABS, FONT_NORMAL, boonX, 1, sTextColors_Primary, TEXT_SKIP_DRAW, sText_Boons);
    AddTextPrinterParameterized3(WIN_TABS, FONT_NORMAL, baneX, 1, sTextColors_Primary, TEXT_SKIP_DRAW, sText_Banes);

    if (sCurseMenu->tabMarkerVisible)
    {
        const u8 *markerColor = (sCurseMenu->tab == CURSE_TAB_BOONS) ? sTextColors_BoonAccent : sTextColors_BaneAccent;

        AddTextPrinterParameterized3(WIN_TABS, FONT_SMALL, markerX, markerY, markerColor, TEXT_SKIP_DRAW, gText_SelectorArrow);
    }

    CopyWindowToVram(WIN_TABS, COPYWIN_FULL);
}

static void DrawList(void)
{
    u8 count = GetCurrentCount();
    u8 i;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_LIST, PIXEL_FILL(1), 0, 0, WindowWidthPx(WIN_LIST), CURSE_TOP_STRIP_HEIGHT);
    FillWindowPixelRect(WIN_LIST, PIXEL_FILL(2), WindowWidthPx(WIN_LIST) - 1, 0, 1, sWindowTemplates[WIN_LIST].height * 8);

    if (count == 0)
    {
        const u8 *text = (sCurseMenu->tab == CURSE_TAB_BOONS) ? sText_NoActiveBoons : sText_NoActiveBanes;

        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 4, 14, sTextColors_Secondary, TEXT_SKIP_DRAW, text);
        CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
        return;
    }

    for (i = 0; i < count; i++)
    {
        const struct CurseDef *def = Curse_GetDef(sCurseMenu->entries[sCurseMenu->tab][i]);
        const u8 *colors = (i == sCurseMenu->cursor) ? sTextColors_Primary : sTextColors_Secondary;
        u8 y = i * CURSE_LIST_ROW_HEIGHT + CURSE_LIST_ROW_Y_START;

        if (i == sCurseMenu->cursor)
        {
            u8 fill = (sCurseMenu->tab == CURSE_TAB_BOONS) ? PIXEL_FILL(7) : PIXEL_FILL(5);
            const u8 *markerColor = (sCurseMenu->tab == CURSE_TAB_BOONS) ? sTextColors_BoonAccent : sTextColors_BaneAccent;

            FillWindowPixelRect(WIN_LIST, fill, 0, y - 1, WindowWidthPx(WIN_LIST) - 1, 11);
            AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 2, y, markerColor, TEXT_SKIP_DRAW, gText_SelectorArrow);
        }

        if (def != NULL)
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 14, y, colors, TEXT_SKIP_DRAW, def->name);
    }

    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void DrawDetails(void)
{
    u8 count = GetCurrentCount();

    FillWindowPixelBuffer(WIN_DETAIL, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_DETAIL, PIXEL_FILL(1), 0, 0, WindowWidthPx(WIN_DETAIL), CURSE_TOP_STRIP_HEIGHT);

    if (count == 0)
    {
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 4, 14, sTextColors_Secondary, TEXT_SKIP_DRAW, sText_DetailsUnavailable);
        CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
        return;
    }

    {
        const struct CurseDef *def = Curse_GetDef(GetCurrentCurseId());

        if (def != NULL)
        {
            ConvertIntToDecimalStringN(gStringVar1, 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            StringExpandPlaceholders(gStringVar4, sText_Level);

            if (sCurseMenu->tab == CURSE_TAB_BOONS)
                AddTextPrinterParameterized3(WIN_DETAIL, FONT_NORMAL, 4, 2, sTextColors_BoonAccent, TEXT_SKIP_DRAW, def->name);
            else
                AddTextPrinterParameterized3(WIN_DETAIL, FONT_NORMAL, 4, 2, sTextColors_BaneAccent, TEXT_SKIP_DRAW, def->name);

            AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 4, 18, sTextColors_Secondary, TEXT_SKIP_DRAW, gStringVar4);
            AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 4, 34, sTextColors_Primary, TEXT_SKIP_DRAW, def->description);
        }
    }

    CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
}

static void DrawFooter(void)
{
    s32 x;

    FillWindowPixelBuffer(WIN_FOOTER, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_FOOTER, PIXEL_FILL(1), 0, 0, WindowWidthPx(WIN_FOOTER), 1);
    x = GetStringCenterAlignXOffset(FONT_SMALL, sText_Footer, WindowWidthPx(WIN_FOOTER));
    AddTextPrinterParameterized3(WIN_FOOTER, FONT_SMALL, x, 6, sTextColors_Primary, TEXT_SKIP_DRAW, sText_Footer);
    CopyWindowToVram(WIN_FOOTER, COPYWIN_FULL);
}

static void DrawAll(void)
{
    DrawHeader();
    DrawTabs();
    DrawList();
    DrawDetails();
    DrawFooter();
}

static void BeginClose(void)
{
    PlaySE(SE_SELECT);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    sCurseMenu->closing = TRUE;
}

static void HandleInput(void)
{
    u8 count;

    if (JOY_NEW(B_BUTTON))
    {
        BeginClose();
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        return;
    }

    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT | L_BUTTON | R_BUTTON))
    {
        PlaySE(SE_SELECT);
        sCurseMenu->tab ^= 1;
        count = GetCurrentCount();
        if (count == 0)
            sCurseMenu->cursor = 0;
        else if (sCurseMenu->cursor >= count)
            sCurseMenu->cursor = count - 1;

        DrawTabs();
        DrawList();
        DrawDetails();
        return;
    }

    count = GetCurrentCount();
    if (count == 0)
        return;

    if (JOY_NEW(DPAD_UP))
    {
        PlaySE(SE_SELECT);
        if (sCurseMenu->cursor == 0)
            sCurseMenu->cursor = count - 1;
        else
            sCurseMenu->cursor--;

        DrawList();
        DrawDetails();
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        sCurseMenu->cursor++;
        if (sCurseMenu->cursor >= count)
            sCurseMenu->cursor = 0;

        DrawList();
        DrawDetails();
    }
}

static void CB2_CurseMenu(void)
{
    if (sCurseMenu->closing)
    {
        if (!gPaletteFade.active)
        {
            MainCallback callback = sCurseMenu->callback;

            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sCurseMenu);
            SetMainCallback2(callback);
            return;
        }
    }
    else if (!gPaletteFade.active)
    {
        HandleInput();
        if (sCurseMenu->tabMarkerVisible != IsTabMarkerVisible())
        {
            sCurseMenu->tabMarkerVisible = IsTabMarkerVisible();
            DrawTabs();
        }
    }

    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void InitWindowFrames(void)
{
    u32 i;

    for (i = 0; i < WIN_COUNT; i++)
    {
        PutWindowTilemap(i);
        if (i >= WIN_LIST)
            DrawStdWindowFrame(i, FALSE);
    }
}

static void CB2_InitCurseMenu(void)
{
    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);

        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);

        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);
        CopyBgTilemapBufferToVram(1);

        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();

        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ResetTasks();
        ResetSpriteData();
        FreeAllSpritePalettes();
        ScanlineEffect_Stop();
        LoadMessageBoxAndBorderGfx();
        InitWindowFrames();
        BuildEntries();
        sCurseMenu->tab = CURSE_TAB_BOONS;
        sCurseMenu->cursor = 0;
        sCurseMenu->tabMarkerVisible = IsTabMarkerVisible();
        DrawAll();
        gMain.state++;
        break;
    case 3:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_CurseMenu);
        SetMainCallback2(CB2_CurseMenu);
        break;
    }
}

void ShowCurseMenu(void (*callback)(void))
{
    sCurseMenu = AllocZeroed(sizeof(*sCurseMenu));
    if (sCurseMenu == NULL)
    {
        SetMainCallback2(callback);
        return;
    }

    sCurseMenu->callback = callback;
    sCurseMenu->tab = CURSE_TAB_BOONS;
    sCurseMenu->cursor = 0;
    sCurseMenu->tabMarkerVisible = IsTabMarkerVisible();
    sCurseMenu->closing = FALSE;
    gMain.state = 0;
    SetMainCallback2(CB2_InitCurseMenu);
}
