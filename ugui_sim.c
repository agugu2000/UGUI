/* -------------------------------------------------------------------------------- */
/* -- µGUI Simulator Application Layer (Platform Independent)                     -- */
/* -------------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>

#include "ugui_sim.h"

#include "ugui_button.h"
#include "ugui_checkbox.h"
#include "ugui_textbox.h"
#include "ugui_progress.h"
#include "ugui_image.h"
#include "ugui_fonts.h"

/* -------------------------------------------------------------------------------- */
/* -- Simulator configuration                                                     -- */
/* -------------------------------------------------------------------------------- */
#define WIDTH               800
#define HEIGHT              600
#define SCREEN_MULTIPLIER   1
#define SCREEN_MARGIN       15
#define WINDOW_BACK_COLOR   0x00C0C0C0
#define MAX_OBJS            15

/* -------------------------------------------------------------------------------- */
/* -- UI text constants (edit here to change all displayed strings)               -- */
/* -------------------------------------------------------------------------------- */
#define TXT_WINDOW_TITLE        "uGUI simulator: Test"

#define TXT_BTN_START           "开始"
#define TXT_BTN_STOP            "停止"
#define TXT_BTN_RESET           "复位"

#define TXT_CHB_LOG             "启用记录"
#define TXT_CHB_REFRESH         "自动刷新"
#define TXT_CHB_DETAILS         "显示详情"

#define TXT_STATUS_PREFIX       "状态："
#define TXT_STATUS_STOPPED      "已停止"
#define TXT_STATUS_RUNNING      "运行中"
#define TXT_STATUS_RESET        "复位"

#define TXT_BMP_LABEL           "16x16 RGB565 BMP"

#define TXT_INFO_FMT            \
    "记录中: %s\n"             \
    "自动刷新: %s\n"        \
    "显示详情: %s\n"        \
    "进度: %d%%\n"          \
    "速度: %d%%\n"             \
    "等级: %d%%"

#define TXT_ON                  "On"
#define TXT_OFF                 "Off"

/* -------------------------------------------------------------------------------- */
/* -- BMP test pattern colors (RGB565)                                            -- */
/* -------------------------------------------------------------------------------- */

/* RGB888 (0xRRGGBB) -> RGB565 (0xRRRRRGGGGGGBBBBB) */
#define RGB888_TO_RGB565(rgb) \
    ( (UG_U16)( (((rgb) >> 8) & 0xF800) | \
                (((rgb) >> 5) & 0x07E0) | \
                (((rgb) >> 3) & 0x001F) ) )

#define BMP_COLOR_Q1            RGB888_TO_RGB565(0xFFC000)
#define BMP_COLOR_Q2            RGB888_TO_RGB565(0xb6bd69)
#define BMP_COLOR_Q3            RGB888_TO_RGB565(0x2b6f64)
#define BMP_COLOR_Q4            RGB888_TO_RGB565(0x203642)

/* -------------------------------------------------------------------------------- */
/* -- Global vars                                                                 -- */
/* -------------------------------------------------------------------------------- */
static simcfg_t *simCfg = NULL;

static UG_GUI    ugui;
static UG_WINDOW wnd;

/* Objects */
static UG_PROGRESS pgb_status;
static UG_TEXTBOX  txt_status;
static UG_BUTTON   btn_start, btn_stop, btn_reset;
static UG_CHECKBOX chb_log, chb_refresh, chb_details;
static UG_PROGRESS pgb_speed, pgb_level;
static UG_TEXTBOX  txb_info;
static UG_IMAGE    img_test;
static UG_TEXTBOX  txb_img_label;

static UG_OBJECT   objs[MAX_OBJS];

/* Runtime state */
static UG_U8 g_running = 0;      /* 0=stopped, 1=running */
static UG_U8 g_progress = 0;     /* 0-100 */
static UG_U8 g_speed = 50;       /* 0-100 */
static UG_U8 g_level = 30;       /* 0-100 */

/* -------------------------------------------------------------------------------- */
/* -- BMP test pattern: 16x16 RGB565, 4 quadrants (red/green/blue/yellow)         -- */
/* -------------------------------------------------------------------------------- */
static const UG_U16 bmp_test_data[16*16] = {
    /* row 0 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 1 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 2 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 3 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 4 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 5 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 6 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 7 */
    BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1,BMP_COLOR_Q1, BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,BMP_COLOR_Q2,
    /* row 8 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,
    /* row 9 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,
    /* row 10 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,
    /* row 11 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,
    /* row 12 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,
    /* row 13 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,
    /* row 14 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,
    /* row 15 */
    BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3,BMP_COLOR_Q3, BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4,BMP_COLOR_Q4
};

static const UG_BMP bmp_test = {
    (const void*)bmp_test_data,
    16,                     /* width  */
    16,                     /* height */
    BMP_BPP_16,             /* bpp    */
    BMP_RGB565              /* colors */
};

/* -------------------------------------------------------------------------------- */
/* -- Forward decls                                                               -- */
/* -------------------------------------------------------------------------------- */
static void windowHandler(UG_MESSAGE *msg);
static void update_info_text(void);
static void update_status_text(const char *s);

/* -------------------------------------------------------------------------------- */
/* -- decode_msg: application-level message print                                 -- */
/* -------------------------------------------------------------------------------- */
static const char *message_type[] = {
    "NONE",
    "WINDOW",
    "OBJECT"
};

static const char *event_type[] = {
    "NONE",
    "PRERENDER",
    "POSTRENDER",
    "PRESSED",
    "RELEASED"
};

static void decode_msg(UG_MESSAGE *msg)
{
    printf("%s %s for ID %d (SubId %d)\n",
           message_type[msg->type],
           event_type[msg->event],
           msg->id, msg->sub_id);
}

/* -------------------------------------------------------------------------------- */
/* -- Config                                                                      -- */
/* -------------------------------------------------------------------------------- */
simcfg_t* GUI_SimCfg(void)
{
    simCfg = (simcfg_t *)malloc(sizeof(simcfg_t));
    if (!simCfg) return NULL;

    simCfg->width            = WIDTH;
    simCfg->height           = HEIGHT;
    simCfg->screenMultiplier = SCREEN_MULTIPLIER;
    simCfg->screenMargin     = SCREEN_MARGIN;
    simCfg->windowBackColor  = WINDOW_BACK_COLOR;
    return simCfg;
}

/* -------------------------------------------------------------------------------- */
/* -- Info text: compose from checkbox states                                     -- */
/* -------------------------------------------------------------------------------- */
static char info_buf[256];

static void update_info_text(void)
{
    snprintf(info_buf, sizeof(info_buf),
             TXT_INFO_FMT,
             UG_CheckboxGetChecked(&wnd, CHB_ID_0) ? TXT_ON : TXT_OFF,
             UG_CheckboxGetChecked(&wnd, CHB_ID_1) ? TXT_ON : TXT_OFF,
             UG_CheckboxGetChecked(&wnd, CHB_ID_2) ? TXT_ON : TXT_OFF,
             (int)g_progress,
             (int)g_speed,
             (int)g_level);

    UG_TextboxSetText(&wnd, TXB_ID_1, info_buf);
}

/* -------------------------------------------------------------------------------- */
/* -- Status text                                                                 -- */
/* -------------------------------------------------------------------------------- */
static char status_buf[64];

static void update_status_text(const char *s)
{
    snprintf(status_buf, sizeof(status_buf), "%s%s", TXT_STATUS_PREFIX, s);
    UG_TextboxSetText(&wnd, TXB_ID_0, status_buf);
}

/* -------------------------------------------------------------------------------- */
/* -- Setup                                                                       -- */
/* -------------------------------------------------------------------------------- */
void GUI_Setup(UG_DEVICE *device)
{
    setlocale(LC_ALL, "");

    UG_Init(&ugui, device);
    UG_FillScreen(C_BLACK);

    /* Window */
    UG_WindowCreate(&wnd, objs, MAX_OBJS, windowHandler);
    UG_WindowSetTitleHeight(&wnd, 0);
    UG_WindowSetTitleTextFont(&wnd, FONT_8X8);
    UG_WindowSetTitleText(&wnd, TXT_WINDOW_TITLE);

    /* ---- Status area ---- */
    UG_ProgressCreate(&wnd, &pgb_status, PGB_ID_0,
                      UGUI_POS(10, 5, 770, 30));
    UG_ProgressSetProgress(&wnd, PGB_ID_0, g_progress);

    UG_TextboxCreate(&wnd, &txt_status, TXB_ID_0,
                     UGUI_POS(10, 45, 770, 30));
    UG_TextboxSetFont(&wnd, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd, TXB_ID_0, TXT_STATUS_PREFIX TXT_STATUS_STOPPED);
    UG_TextboxSetAlignment(&wnd, TXB_ID_0, ALIGN_CENTER_LEFT);

    /* ---- Control area ---- */
    UG_ButtonCreate(&wnd, &btn_start, BTN_ID_0,
                    UGUI_POS(10, 85, 180, 40));
    UG_ButtonSetFont(&wnd, BTN_ID_0, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd, BTN_ID_0, TXT_BTN_START);
    UG_ButtonSetStyle(&wnd, BTN_ID_0, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd, &btn_stop, BTN_ID_1,
                    UGUI_POS(200, 85, 180, 40));
    UG_ButtonSetFont(&wnd, BTN_ID_1, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd, BTN_ID_1, TXT_BTN_STOP);
    UG_ButtonSetStyle(&wnd, BTN_ID_1, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd, &btn_reset, BTN_ID_2,
                    UGUI_POS(390, 85, 180, 40));
    UG_ButtonSetFont(&wnd, BTN_ID_2, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd, BTN_ID_2, TXT_BTN_RESET);
    UG_ButtonSetStyle(&wnd, BTN_ID_2, BTN_STYLE_3D);

    /* ---- Option area ---- */
    UG_CheckboxCreate(&wnd, &chb_log, CHB_ID_0,
                      UGUI_POS(10, 135, 300, 22));
    UG_CheckboxSetFont(&wnd, CHB_ID_0, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd, CHB_ID_0, TXT_CHB_LOG);
    UG_CheckboxSetStyle(&wnd, CHB_ID_0, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd, CHB_ID_0, ALIGN_CENTER_LEFT);
    UG_CheckboxSetChecked(&wnd, CHB_ID_0, 1);

    UG_CheckboxCreate(&wnd, &chb_refresh, CHB_ID_1,
                      UGUI_POS(10, 162, 300, 22));
    UG_CheckboxSetFont(&wnd, CHB_ID_1, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd, CHB_ID_1, TXT_CHB_REFRESH);
    UG_CheckboxSetStyle(&wnd, CHB_ID_1, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd, CHB_ID_1, ALIGN_CENTER_LEFT);
    UG_CheckboxSetChecked(&wnd, CHB_ID_1, 0);

    UG_CheckboxCreate(&wnd, &chb_details, CHB_ID_2,
                      UGUI_POS(10, 189, 300, 22));
    UG_CheckboxSetFont(&wnd, CHB_ID_2, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd, CHB_ID_2, TXT_CHB_DETAILS);
    UG_CheckboxSetStyle(&wnd, CHB_ID_2, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd, CHB_ID_2, ALIGN_CENTER_LEFT);
    UG_CheckboxSetChecked(&wnd, CHB_ID_2, 1);

    /* ---- Parameter area ---- */
    UG_ProgressCreate(&wnd, &pgb_speed, PGB_ID_1,
                      UGUI_POS(10, 220, 770, 30));
    UG_ProgressSetProgress(&wnd, PGB_ID_1, g_speed);

    UG_ProgressCreate(&wnd, &pgb_level, PGB_ID_2,
                      UGUI_POS(10, 260, 770, 30));
    UG_ProgressSetProgress(&wnd, PGB_ID_2, g_level);

    /* ---- Info area ---- */
    UG_TextboxCreate(&wnd, &txb_info, TXB_ID_1,
                     UGUI_POS(10, 300, 770, 130));
    UG_TextboxSetFont(&wnd, TXB_ID_1, FONT_SIMSUN2_13X13);
    UG_TextboxSetAlignment(&wnd, TXB_ID_1, ALIGN_TOP_LEFT);
    UG_TextboxSetText(&wnd, TXB_ID_1, "");

    /* ---- Icon area ---- */
    UG_ImageCreate(&wnd, &img_test, IMG_ID_0,
                   UGUI_POS(10, 440, 16, 16));
    UG_ImageSetBMP(&wnd, IMG_ID_0, &bmp_test);

    UG_TextboxCreate(&wnd, &txb_img_label, TXB_ID_2,
                     UGUI_POS(40, 440, 300, 20));
    UG_TextboxSetFont(&wnd, TXB_ID_2, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd, TXB_ID_2, TXT_BMP_LABEL);
    UG_TextboxSetAlignment(&wnd, TXB_ID_2, ALIGN_CENTER_LEFT);

    /* Initial info text */
    update_info_text();

    UG_WindowShow(&wnd);
}

/* -------------------------------------------------------------------------------- */
/* -- Process                                                                     -- */
/* -------------------------------------------------------------------------------- */
void GUI_Process(void)
{
    if (g_running)
    {
        /* Advance progress bar */
        g_progress = (g_progress >= 100) ? 0 : (UG_U8)(g_progress + 1);

        /* Wobble speed / level slightly to show "running" */
        g_speed = (UG_U8)(50 + ((g_progress * 3) % 50));
        g_level = (UG_U8)(30 + ((g_progress * 7) % 70));

        UG_ProgressSetProgress(&wnd, PGB_ID_0, g_progress);
        UG_ProgressSetProgress(&wnd, PGB_ID_1, g_speed);
        UG_ProgressSetProgress(&wnd, PGB_ID_2, g_level);

        update_info_text();
    }

    UG_Update();
}

/* -------------------------------------------------------------------------------- */
/* -- Window message handler                                                      -- */
/* -------------------------------------------------------------------------------- */
static void windowHandler(UG_MESSAGE *msg)
{
    decode_msg(msg);

    if (msg->type != MSG_TYPE_OBJECT) return;
    if (msg->event != OBJ_EVENT_RELEASED) return;

    switch (msg->id)
    {
    case OBJ_TYPE_BUTTON:
        switch (msg->sub_id)
        {
        case BTN_ID_0:  /* Start */
            g_running = 1;
            update_status_text(TXT_STATUS_RUNNING);
            break;

        case BTN_ID_1:  /* Stop */
            g_running = 0;
            update_status_text(TXT_STATUS_STOPPED);
            break;

        case BTN_ID_2:  /* Reset */
            g_running = 0;
            g_progress = 0;
            g_speed = 50;
            g_level = 30;
            UG_ProgressSetProgress(&wnd, PGB_ID_0, 0);
            UG_ProgressSetProgress(&wnd, PGB_ID_1, 50);
            UG_ProgressSetProgress(&wnd, PGB_ID_2, 30);
            UG_CheckboxSetChecked(&wnd, CHB_ID_0, 1);
            UG_CheckboxSetChecked(&wnd, CHB_ID_1, 0);
            UG_CheckboxSetChecked(&wnd, CHB_ID_2, 1);
            update_status_text(TXT_STATUS_RESET);
            update_info_text();
            break;
        }
        break;

    case OBJ_TYPE_CHECKBOX:
        update_info_text();
        break;
    }
}