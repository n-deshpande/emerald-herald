#include "global.h"
#include "bg.h"
#include "relic.h"
#include "relic_menu.h"
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
    WIN_HEADER,
    WIN_LIST,
    WIN_DETAIL,
    WIN_FOOTER,
    WIN_COUNT,
};

#define RELIC_LIST_ROW_HEIGHT 14
#define RELIC_LIST_ROW_Y_START 2
#define RELIC_TOP_STRIP_HEIGHT 10

struct RelicMenuData
{
    MainCallback callback;
    u8 cursor;
    bool8 closing;
    u8 count;
    u8 entryId[RELIC_SLOTS];
    u8 entryTier[RELIC_SLOTS];
};

static EWRAM_DATA struct RelicMenuData *sRelicMenu = NULL;

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
    [WIN_LIST] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 3,
        .width = 12,
        .height = 13,
        .paletteNum = 15,
        .baseBlock = 61,
    },
    [WIN_DETAIL] = {
        .bg = 0,
        .tilemapLeft = 12,
        .tilemapTop = 3,
        .width = 18,
        .height = 13,
        .paletteNum = 15,
        .baseBlock = 217,
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

static const u8 sText_Relics[] = _("Relics");
static const u8 sText_NoActiveRelics[] = _("No Active Relics.");
static const u8 sText_DetailsUnavailable[] = _("No Relic Details Available.");
static const u8 sText_Tier[] = _("Tier {STR_VAR_1}");
static const u8 sText_Footer[] = _("D-Pad: Navigate   B: Exit");

static const u8 sTextColors_Primary[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
static const u8 sTextColors_Secondary[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_WHITE};
static const u8 sTextColors_Accent[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_GREEN, TEXT_COLOR_DARK_GRAY};

static void VBlankCB_RelicMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void BuildEntries(void)
{
    u32 i;

    sRelicMenu->count = 0;

    for (i = 0; i < RELIC_SLOTS; i++)
    {
        u8 relicId = Relic_GetSlotId(i);

        if (Relic_GetDef(relicId) != NULL)
        {
            sRelicMenu->entryId[sRelicMenu->count] = relicId;
            sRelicMenu->entryTier[sRelicMenu->count] = Relic_GetSlotTier(i);
            sRelicMenu->count++;
        }
    }
}

static u8 GetCurrentRelicId(void)
{
    return sRelicMenu->entryId[sRelicMenu->cursor];
}

static u8 GetCurrentTier(void)
{
    return sRelicMenu->entryTier[sRelicMenu->cursor];
}

static void DrawHeader(void)
{
    s32 x;

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_HEADER, PIXEL_FILL(1), 0, 0, WindowWidthPx(WIN_HEADER), 1);
    FillWindowPixelRect(WIN_HEADER, PIXEL_FILL(1), 0, 15, WindowWidthPx(WIN_HEADER), 1);
    x = GetStringCenterAlignXOffset(FONT_NORMAL, sText_Relics, 30 * 8);
    AddTextPrinterParameterized3(WIN_HEADER, FONT_NORMAL, x, 1, sTextColors_Primary, TEXT_SKIP_DRAW, sText_Relics);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawList(void)
{
    u8 count = sRelicMenu->count;
    u8 i;

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_LIST, PIXEL_FILL(1), 0, 0, WindowWidthPx(WIN_LIST), RELIC_TOP_STRIP_HEIGHT);
    FillWindowPixelRect(WIN_LIST, PIXEL_FILL(2), WindowWidthPx(WIN_LIST) - 1, 0, 1, sWindowTemplates[WIN_LIST].height * 8);

    if (count == 0)
    {
        AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 4, 14, sTextColors_Secondary, TEXT_SKIP_DRAW, sText_NoActiveRelics);
        CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
        return;
    }

    for (i = 0; i < count; i++)
    {
        const struct RelicDef *def = Relic_GetDef(sRelicMenu->entryId[i]);
        const u8 *colors = (i == sRelicMenu->cursor) ? sTextColors_Primary : sTextColors_Secondary;
        u8 y = i * RELIC_LIST_ROW_HEIGHT + RELIC_LIST_ROW_Y_START;

        if (i == sRelicMenu->cursor)
        {
            FillWindowPixelRect(WIN_LIST, PIXEL_FILL(7), 0, y - 1, WindowWidthPx(WIN_LIST) - 1, 11);
            AddTextPrinterParameterized3(WIN_LIST, FONT_NORMAL, 2, y, sTextColors_Accent, TEXT_SKIP_DRAW, gText_SelectorArrow);
        }

        if (def != NULL)
            AddTextPrinterParameterized3(WIN_LIST, FONT_SMALL, 14, y, colors, TEXT_SKIP_DRAW, def->name);
    }

    CopyWindowToVram(WIN_LIST, COPYWIN_FULL);
}

static void DrawDetails(void)
{
    u8 count = sRelicMenu->count;

    FillWindowPixelBuffer(WIN_DETAIL, PIXEL_FILL(3));
    FillWindowPixelRect(WIN_DETAIL, PIXEL_FILL(1), 0, 0, WindowWidthPx(WIN_DETAIL), RELIC_TOP_STRIP_HEIGHT);

    if (count == 0)
    {
        AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 4, 14, sTextColors_Secondary, TEXT_SKIP_DRAW, sText_DetailsUnavailable);
        CopyWindowToVram(WIN_DETAIL, COPYWIN_FULL);
        return;
    }

    {
        const struct RelicDef *def = Relic_GetDef(GetCurrentRelicId());
        u8 tier = GetCurrentTier();

        if (def != NULL)
        {
            if (tier > RELIC_MAX_TIER)
                tier = RELIC_MAX_TIER;

            ConvertIntToDecimalStringN(gStringVar1, tier + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            StringExpandPlaceholders(gStringVar4, sText_Tier);

            AddTextPrinterParameterized3(WIN_DETAIL, FONT_NORMAL, 4, 2, sTextColors_Accent, TEXT_SKIP_DRAW, def->name);
            AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 4, 18, sTextColors_Secondary, TEXT_SKIP_DRAW, gStringVar4);
            AddTextPrinterParameterized3(WIN_DETAIL, FONT_SMALL, 4, 34, sTextColors_Primary, TEXT_SKIP_DRAW, def->descriptions[tier]);
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
    DrawList();
    DrawDetails();
    DrawFooter();
}

static void BeginClose(void)
{
    PlaySE(SE_SELECT);
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    sRelicMenu->closing = TRUE;
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

    count = sRelicMenu->count;
    if (count == 0)
        return;

    if (JOY_NEW(DPAD_UP))
    {
        PlaySE(SE_SELECT);
        if (sRelicMenu->cursor == 0)
            sRelicMenu->cursor = count - 1;
        else
            sRelicMenu->cursor--;

        DrawList();
        DrawDetails();
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        PlaySE(SE_SELECT);
        sRelicMenu->cursor++;
        if (sRelicMenu->cursor >= count)
            sRelicMenu->cursor = 0;

        DrawList();
        DrawDetails();
    }
}

static void CB2_RelicMenu(void)
{
    if (sRelicMenu->closing)
    {
        if (!gPaletteFade.active)
        {
            MainCallback callback = sRelicMenu->callback;

            FreeAllWindowBuffers();
            FREE_AND_SET_NULL(sRelicMenu);
            SetMainCallback2(callback);
            return;
        }
    }
    else if (!gPaletteFade.active)
    {
        HandleInput();
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

static void CB2_InitRelicMenu(void)
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
        sRelicMenu->cursor = 0;
        DrawAll();
        gMain.state++;
        break;
    case 3:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_RelicMenu);
        SetMainCallback2(CB2_RelicMenu);
        break;
    }
}

void ShowRelicMenu(void (*callback)(void))
{
    sRelicMenu = AllocZeroed(sizeof(*sRelicMenu));
    if (sRelicMenu == NULL)
    {
        SetMainCallback2(callback);
        return;
    }

    sRelicMenu->callback = callback;
    sRelicMenu->cursor = 0;
    sRelicMenu->closing = FALSE;
    gMain.state = 0;
    SetMainCallback2(CB2_InitRelicMenu);
}
