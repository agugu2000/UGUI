/* -------------------------------------------------------------------------------- */
/* -- µGUI Simulator Application Layer (Platform Independent)                     -- */
/* -------------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>

#include "ugui_sim.h"

/* -------------------------------------------------------------------------------- */
/* -- Simulator configuration                                                     -- */
/* -------------------------------------------------------------------------------- */
#define WIDTH               800
#define HEIGHT              600
#define SCREEN_MULTIPLIER   2
#define SCREEN_MARGIN       15
#define WINDOW_BACK_COLOR   0x00C0C0C0

#define MAX_OBJS_PAGE1      20
#define MAX_OBJS_PAGE2      30
#define MAX_OBJS_PAGE3      15

/* -------------------------------------------------------------------------------- */
/* -- UI text constants                                                           -- */
/* -------------------------------------------------------------------------------- */

/* Page titles (FONT_8X8, English only) */
#define TXT_TITLE_P1            "Page 1: Control"
#define TXT_TITLE_P2            "Page 2: Styles"
#define TXT_TITLE_P3            "Page 3: Draw"

/* Page switch buttons */
#define TXT_BTN_P1              "第1页"
#define TXT_BTN_P2              "第2页"
#define TXT_BTN_P3              "第3页"

/* Page 1 */
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
    "记录中: %s\n"              \
    "自动刷新: %s\n"            \
    "显示详情: %s\n"            \
    "进度: %d%%\n"              \
    "速度: %d%%\n"              \
    "等级: %d%%"
#define TXT_ON                  "On"
#define TXT_OFF                 "Off"

/* Page 2 */
#define TXT_STYLE_3D            "3D"
#define TXT_STYLE_2D            "2D"
#define TXT_STYLE_2D_TOGGLE     "2D+Toggle"
#define TXT_STYLE_3D_ALT        "3D+Alt"
#define TXT_STYLE_NO_BORDER     "NoBorder"
#define TXT_STYLE_NO_FILL       "NoFill"

#define TXT_ALIGN_TL            "左上"
#define TXT_ALIGN_TC            "中上"
#define TXT_ALIGN_TR            "右上"
#define TXT_ALIGN_C             "居中"
#define TXT_ALIGN_BL            "左下"
#define TXT_ALIGN_BR            "右下"

/* Page 3 */
#define TXT_PUTSTR              "Hello 世界"
#define TXT_TRANSPARENT_OFF     "透明=0"
#define TXT_TRANSPARENT_ON      "透明=1"
#define TXT_HSPACE              "H S P A C E"
#define TXT_VSPACE              "行1\n行2\n行3"
#define TXT_FONT_ASCII          "ASCII 0123"
#define TXT_FONT_CHINESE        "中文测试"
#define TXT_CONSOLE_LINE1       "控制台输出：\n"
#define TXT_CONSOLE_LINE2       "Hello 世界\n"
#define TXT_CONSOLE_LINE3       "12345\n"

/* -------------------------------------------------------------------------------- */
/* -- BMP test pattern colors                                                     -- */
/* -------------------------------------------------------------------------------- */
#define RGB565(r, g, b) \
    ( (UG_U16)( (((r) & 0xF8) << 8) | \
                (((g) & 0xFC) << 3) | \
                (((b) & 0xF8) >> 3) ) )

#define BMP_COLOR_Q1            RGB565(0xFF, 0x00, 0x00)
#define BMP_COLOR_Q2            RGB565(0x00, 0xFF, 0x00)
#define BMP_COLOR_Q3            RGB565(0x00, 0x00, 0xFF)
#define BMP_COLOR_Q4            RGB565(0xFF, 0xFF, 0x00)

/* -------------------------------------------------------------------------------- */
/* -- Global vars                                                                 -- */
/* -------------------------------------------------------------------------------- */
static simcfg_t *simCfg = NULL;

static UG_GUI    ugui;
static UG_WINDOW wnd1, wnd2, wnd3;

/* Page 1 objects */
static UG_PROGRESS pgb_status;
static UG_TEXTBOX  txt_status;
static UG_BUTTON   btn_start, btn_stop, btn_reset;
static UG_CHECKBOX chb_log, chb_refresh, chb_details;
static UG_PROGRESS pgb_speed, pgb_level;
static UG_TEXTBOX  txb_info;
static UG_IMAGE    img_test;
static UG_TEXTBOX  txb_img_label;
static UG_BUTTON   btn_p1_1, btn_p1_2, btn_p1_3;

/* Page 2 objects */
static UG_BUTTON   btn_s1, btn_s2, btn_s3, btn_s4, btn_s5, btn_s6;
static UG_CHECKBOX chb_s1, chb_s2, chb_s3, chb_s4, chb_s5, chb_s6;
static UG_PROGRESS pgb_s1, pgb_s2, pgb_s3, pgb_s4, pgb_s5;
static UG_TEXTBOX  txb_a1, txb_a2, txb_a3, txb_a4, txb_a5, txb_a6;
static UG_BUTTON   btn_p2_1, btn_p2_2, btn_p2_3;

/* Page 3 objects */
static UG_TEXTBOX  txb_p3_1, txb_p3_2, txb_p3_3, txb_p3_4;
static UG_BUTTON   btn_p3_1, btn_p3_2, btn_p3_3;

static UG_OBJECT   objs1[MAX_OBJS_PAGE1];
static UG_OBJECT   objs2[MAX_OBJS_PAGE2];
static UG_OBJECT   objs3[MAX_OBJS_PAGE3];

/* Runtime state */
static UG_U8 g_running = 0;
static UG_U8 g_progress = 0;
static UG_U8 g_speed = 50;
static UG_U8 g_level = 30;
static UG_U8 page3_drawn = 0;

/* -------------------------------------------------------------------------------- */
/* -- BMP test pattern: 16x16 RGB565, 4 quadrants                                 -- */
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
    16,
    16,
    BMP_BPP_16,
    BMP_RGB565
};

/* -------------------------------------------------------------------------------- */
/* -- Forward decls                                                               -- */
/* -------------------------------------------------------------------------------- */
static void windowHandler(UG_MESSAGE *msg);
static void update_info_text(void);
static void update_status_text(const char *s);
static void draw_page3(void);

/* -------------------------------------------------------------------------------- */
/* -- decode_msg                                                                  -- */
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
/* -- Info text                                                                   -- */
/* -------------------------------------------------------------------------------- */
static char info_buf[256];

static void update_info_text(void)
{
    snprintf(info_buf, sizeof(info_buf),
             TXT_INFO_FMT,
             UG_CheckboxGetChecked(&wnd1, CHB_ID_0) ? TXT_ON : TXT_OFF,
             UG_CheckboxGetChecked(&wnd1, CHB_ID_1) ? TXT_ON : TXT_OFF,
             UG_CheckboxGetChecked(&wnd1, CHB_ID_2) ? TXT_ON : TXT_OFF,
             (int)g_progress,
             (int)g_speed,
             (int)g_level);

    UG_TextboxSetText(&wnd1, TXB_ID_1, info_buf);
}

/* -------------------------------------------------------------------------------- */
/* -- Status text                                                                 -- */
/* -------------------------------------------------------------------------------- */
static char status_buf[64];

static void update_status_text(const char *s)
{
    snprintf(status_buf, sizeof(status_buf), "%s%s", TXT_STATUS_PREFIX, s);
    UG_TextboxSetText(&wnd1, TXB_ID_0, status_buf);
}

/* -------------------------------------------------------------------------------- */
/* -- Page 3 drawing                                                              -- */
/* -------------------------------------------------------------------------------- */
static void draw_page3(void)
{
    /* UG_DrawLine */
    UG_DrawLine(10, 25, 200, 25, C_RED);
    UG_DrawLine(10, 35, 200, 75, C_GREEN);
    UG_DrawLine(10, 85, 200, 85, C_BLUE);

    /* UG_DrawFrame */
    UG_DrawFrame(220, 25, 350, 95, C_BLACK);

    /* UG_DrawRoundFrame */
    UG_DrawRoundFrame(370, 25, 500, 95, 10, C_BLACK);

    /* UG_DrawCircle */
    UG_DrawCircle(50, 165, 30, C_RED);

    /* UG_FillCircle */
    UG_FillCircle(150, 165, 30, C_GREEN);

    /* UG_DrawArc */
    UG_DrawArc(250, 165, 30, 0x0F, C_BLUE);

    /* UG_DrawTriangle */
    UG_DrawTriangle(350, 135, 400, 195, 300, 195, C_BLACK);

    /* UG_FillTriangle */
    UG_FillTriangle(450, 135, 500, 195, 400, 195, C_YELLOW);

    /* UG_DrawMesh */
    UG_DrawMesh(550, 135, 700, 195, 10, C_GRAY);

    /* UG_FillRoundFrame */
    UG_FillRoundFrame(10, 235, 200, 315, 15, C_PALE_TURQUOISE);

    /* UG_PutString */
    UG_SetForecolor(C_BLACK);
    UG_SetBackcolor(C_WHITE);
    UG_FontSelect(FONT_SIMSUN2_13X13);
    UG_PutString(220, 235, TXT_PUTSTR);

    /* UG_ConsolePutString */
    UG_ConsoleSetArea(220, 265, 780, 315);
    UG_ConsoleSetForecolor(C_BLACK);
    UG_ConsoleSetBackcolor(C_WHITE);
    UG_ConsolePutString(TXT_CONSOLE_LINE1);
    UG_ConsolePutString(TXT_CONSOLE_LINE2);
    UG_ConsolePutString(TXT_CONSOLE_LINE3);
}

/* -------------------------------------------------------------------------------- */
/* -- Page 1 setup                                                                -- */
/* -------------------------------------------------------------------------------- */
static void setup_page1(void)
{
    UG_WindowCreate(&wnd1, objs1, MAX_OBJS_PAGE1, windowHandler);
    UG_WindowSetTitleHeight(&wnd1, 0);
    UG_WindowSetTitleTextFont(&wnd1, FONT_8X8);
    UG_WindowSetTitleText(&wnd1, TXT_TITLE_P1);

    /* Status area */
    UG_ProgressCreate(&wnd1, &pgb_status, PGB_ID_0,
                      UGUI_POS(10, 5, 770, 30));
    UG_ProgressSetProgress(&wnd1, PGB_ID_0, g_progress);

    UG_TextboxCreate(&wnd1, &txt_status, TXB_ID_0,
                     UGUI_POS(10, 45, 770, 30));
    UG_TextboxSetFont(&wnd1, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd1, TXB_ID_0, TXT_STATUS_PREFIX TXT_STATUS_STOPPED);
    UG_TextboxSetAlignment(&wnd1, TXB_ID_0, ALIGN_CENTER_LEFT);

    /* Control area */
    UG_ButtonCreate(&wnd1, &btn_start, BTN_ID_0,
                    UGUI_POS(10, 85, 180, 40));
    UG_ButtonSetFont(&wnd1, BTN_ID_0, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_0, TXT_BTN_START);
    UG_ButtonSetStyle(&wnd1, BTN_ID_0, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_stop, BTN_ID_1,
                    UGUI_POS(200, 85, 180, 40));
    UG_ButtonSetFont(&wnd1, BTN_ID_1, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_1, TXT_BTN_STOP);
    UG_ButtonSetStyle(&wnd1, BTN_ID_1, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_reset, BTN_ID_2,
                    UGUI_POS(390, 85, 180, 40));
    UG_ButtonSetFont(&wnd1, BTN_ID_2, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_2, TXT_BTN_RESET);
    UG_ButtonSetStyle(&wnd1, BTN_ID_2, BTN_STYLE_3D);

    /* Option area */
    UG_CheckboxCreate(&wnd1, &chb_log, CHB_ID_0,
                      UGUI_POS(10, 135, 300, 22));
    UG_CheckboxSetFont(&wnd1, CHB_ID_0, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd1, CHB_ID_0, TXT_CHB_LOG);
    UG_CheckboxSetStyle(&wnd1, CHB_ID_0, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd1, CHB_ID_0, ALIGN_CENTER_LEFT);
    UG_CheckboxSetChecked(&wnd1, CHB_ID_0, 1);

    UG_CheckboxCreate(&wnd1, &chb_refresh, CHB_ID_1,
                      UGUI_POS(10, 162, 300, 22));
    UG_CheckboxSetFont(&wnd1, CHB_ID_1, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd1, CHB_ID_1, TXT_CHB_REFRESH);
    UG_CheckboxSetStyle(&wnd1, CHB_ID_1, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd1, CHB_ID_1, ALIGN_CENTER_LEFT);
    UG_CheckboxSetChecked(&wnd1, CHB_ID_1, 0);

    UG_CheckboxCreate(&wnd1, &chb_details, CHB_ID_2,
                      UGUI_POS(10, 189, 300, 22));
    UG_CheckboxSetFont(&wnd1, CHB_ID_2, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd1, CHB_ID_2, TXT_CHB_DETAILS);
    UG_CheckboxSetStyle(&wnd1, CHB_ID_2, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd1, CHB_ID_2, ALIGN_CENTER_LEFT);
    UG_CheckboxSetChecked(&wnd1, CHB_ID_2, 1);

    /* Parameter area */
    UG_ProgressCreate(&wnd1, &pgb_speed, PGB_ID_1,
                      UGUI_POS(10, 220, 770, 30));
    UG_ProgressSetProgress(&wnd1, PGB_ID_1, g_speed);

    UG_ProgressCreate(&wnd1, &pgb_level, PGB_ID_2,
                      UGUI_POS(10, 260, 770, 30));
    UG_ProgressSetProgress(&wnd1, PGB_ID_2, g_level);

    /* Info area */
    UG_TextboxCreate(&wnd1, &txb_info, TXB_ID_1,
                     UGUI_POS(10, 300, 770, 130));
    UG_TextboxSetFont(&wnd1, TXB_ID_1, FONT_SIMSUN2_13X13);
    UG_TextboxSetAlignment(&wnd1, TXB_ID_1, ALIGN_TOP_LEFT);
    UG_TextboxSetText(&wnd1, TXB_ID_1, "");

    /* Icon area */
    UG_ImageCreate(&wnd1, &img_test, IMG_ID_0,
                   UGUI_POS(10, 440, 16, 16));
    UG_ImageSetBMP(&wnd1, IMG_ID_0, &bmp_test);

    UG_TextboxCreate(&wnd1, &txb_img_label, TXB_ID_2,
                     UGUI_POS(40, 440, 300, 20));
    UG_TextboxSetFont(&wnd1, TXB_ID_2, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd1, TXB_ID_2, TXT_BMP_LABEL);
    UG_TextboxSetAlignment(&wnd1, TXB_ID_2, ALIGN_CENTER_LEFT);

    /* Page switch buttons */
    UG_ButtonCreate(&wnd1, &btn_p1_1, BTN_ID_17, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_17, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd1, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_p1_2, BTN_ID_18, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_18, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd1, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_p1_3, BTN_ID_19, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_19, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd1, BTN_ID_19, BTN_STYLE_3D);
}

/* -------------------------------------------------------------------------------- */
/* -- Page 2 setup                                                                -- */
/* -------------------------------------------------------------------------------- */
static void setup_page2(void)
{
    UG_WindowCreate(&wnd2, objs2, MAX_OBJS_PAGE2, windowHandler);
    UG_WindowSetTitleHeight(&wnd2, 0);
    UG_WindowSetTitleTextFont(&wnd2, FONT_8X8);
    UG_WindowSetTitleText(&wnd2, TXT_TITLE_P2);

    /* Buttons row 1 */
    UG_ButtonCreate(&wnd2, &btn_s1, BTN_ID_0, UGUI_POS(10, 10, 150, 40));
    UG_ButtonSetFont(&wnd2, BTN_ID_0, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_0, TXT_STYLE_3D);
    UG_ButtonSetStyle(&wnd2, BTN_ID_0, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_s2, BTN_ID_1, UGUI_POS(170, 10, 150, 40));
    UG_ButtonSetFont(&wnd2, BTN_ID_1, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_1, TXT_STYLE_2D);
    UG_ButtonSetStyle(&wnd2, BTN_ID_1, BTN_STYLE_2D);

    UG_ButtonCreate(&wnd2, &btn_s3, BTN_ID_2, UGUI_POS(330, 10, 150, 40));
    UG_ButtonSetFont(&wnd2, BTN_ID_2, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_2, TXT_STYLE_2D_TOGGLE);
    UG_ButtonSetStyle(&wnd2, BTN_ID_2, BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS);

    /* Buttons row 2 */
    UG_ButtonCreate(&wnd2, &btn_s4, BTN_ID_3, UGUI_POS(10, 60, 150, 40));
    UG_ButtonSetFont(&wnd2, BTN_ID_3, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_3, TXT_STYLE_3D_ALT);
    UG_ButtonSetStyle(&wnd2, BTN_ID_3, BTN_STYLE_3D | BTN_STYLE_USE_ALTERNATE_COLORS);
    UG_ButtonSetAlternateForeColor(&wnd2, BTN_ID_3, C_BLACK);
    UG_ButtonSetAlternateBackColor(&wnd2, BTN_ID_3, C_WHITE);

    UG_ButtonCreate(&wnd2, &btn_s5, BTN_ID_4, UGUI_POS(170, 60, 150, 40));
    UG_ButtonSetFont(&wnd2, BTN_ID_4, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_4, TXT_STYLE_NO_BORDER);
    UG_ButtonSetStyle(&wnd2, BTN_ID_4, BTN_STYLE_NO_BORDERS | BTN_STYLE_TOGGLE_COLORS);

    UG_ButtonCreate(&wnd2, &btn_s6, BTN_ID_5, UGUI_POS(330, 60, 150, 40));
    UG_ButtonSetFont(&wnd2, BTN_ID_5, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_5, TXT_STYLE_NO_FILL);
    UG_ButtonSetStyle(&wnd2, BTN_ID_5, BTN_STYLE_NO_FILL | BTN_STYLE_TOGGLE_COLORS);

    /* Checkboxes row 1 */
    UG_CheckboxCreate(&wnd2, &chb_s1, CHB_ID_0, UGUI_POS(10, 110, 150, 22));
    UG_CheckboxSetFont(&wnd2, CHB_ID_0, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd2, CHB_ID_0, TXT_STYLE_3D);
    UG_CheckboxSetStyle(&wnd2, CHB_ID_0, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd2, CHB_ID_0, ALIGN_CENTER_LEFT);

    UG_CheckboxCreate(&wnd2, &chb_s2, CHB_ID_1, UGUI_POS(170, 110, 150, 22));
    UG_CheckboxSetFont(&wnd2, CHB_ID_1, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd2, CHB_ID_1, TXT_STYLE_2D);
    UG_CheckboxSetStyle(&wnd2, CHB_ID_1, CHB_STYLE_2D);
    UG_CheckboxSetAlignment(&wnd2, CHB_ID_1, ALIGN_CENTER_LEFT);

    UG_CheckboxCreate(&wnd2, &chb_s3, CHB_ID_2, UGUI_POS(330, 110, 150, 22));
    UG_CheckboxSetFont(&wnd2, CHB_ID_2, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd2, CHB_ID_2, TXT_STYLE_2D_TOGGLE);
    UG_CheckboxSetStyle(&wnd2, CHB_ID_2, CHB_STYLE_2D | CHB_STYLE_TOGGLE_COLORS);
    UG_CheckboxSetAlignment(&wnd2, CHB_ID_2, ALIGN_CENTER_LEFT);

    /* Checkboxes row 2 */
    UG_CheckboxCreate(&wnd2, &chb_s4, CHB_ID_3, UGUI_POS(10, 140, 150, 22));
    UG_CheckboxSetFont(&wnd2, CHB_ID_3, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd2, CHB_ID_3, TXT_STYLE_3D_ALT);
    UG_CheckboxSetStyle(&wnd2, CHB_ID_3, CHB_STYLE_3D | CHB_STYLE_USE_ALTERNATE_COLORS);
    UG_CheckboxSetAlignment(&wnd2, CHB_ID_3, ALIGN_CENTER_LEFT);

    UG_CheckboxCreate(&wnd2, &chb_s5, CHB_ID_4, UGUI_POS(170, 140, 150, 22));
    UG_CheckboxSetFont(&wnd2, CHB_ID_4, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd2, CHB_ID_4, TXT_STYLE_NO_BORDER);
    UG_CheckboxSetStyle(&wnd2, CHB_ID_4, CHB_STYLE_NO_BORDERS | CHB_STYLE_TOGGLE_COLORS);
    UG_CheckboxSetAlignment(&wnd2, CHB_ID_4, ALIGN_CENTER_LEFT);

    UG_CheckboxCreate(&wnd2, &chb_s6, CHB_ID_5, UGUI_POS(330, 140, 150, 22));
    UG_CheckboxSetFont(&wnd2, CHB_ID_5, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd2, CHB_ID_5, TXT_STYLE_NO_FILL);
    UG_CheckboxSetStyle(&wnd2, CHB_ID_5, CHB_STYLE_NO_FILL | CHB_STYLE_TOGGLE_COLORS);
    UG_CheckboxSetAlignment(&wnd2, CHB_ID_5, ALIGN_CENTER_LEFT);

    /* Progress bars */
    UG_ProgressCreate(&wnd2, &pgb_s1, PGB_ID_0, UGUI_POS(10, 170, 770, 22));
    UG_ProgressSetStyle(&wnd2, PGB_ID_0, PGB_STYLE_3D);
    UG_ProgressSetProgress(&wnd2, PGB_ID_0, 60);

    UG_ProgressCreate(&wnd2, &pgb_s2, PGB_ID_1, UGUI_POS(10, 198, 770, 22));
    UG_ProgressSetStyle(&wnd2, PGB_ID_1, PGB_STYLE_2D);
    UG_ProgressSetProgress(&wnd2, PGB_ID_1, 55);

    UG_ProgressCreate(&wnd2, &pgb_s3, PGB_ID_2, UGUI_POS(10, 226, 770, 22));
    UG_ProgressSetStyle(&wnd2, PGB_ID_2, PGB_STYLE_2D | PGB_STYLE_FORE_COLOR_MESH);
    UG_ProgressSetProgress(&wnd2, PGB_ID_2, 50);

    UG_ProgressCreate(&wnd2, &pgb_s4, PGB_ID_3, UGUI_POS(10, 254, 770, 22));
    UG_ProgressSetStyle(&wnd2, PGB_ID_3, PGB_STYLE_3D | PGB_STYLE_NO_BORDERS);
    UG_ProgressSetProgress(&wnd2, PGB_ID_3, 45);

    UG_ProgressCreate(&wnd2, &pgb_s5, PGB_ID_4, UGUI_POS(10, 282, 770, 22));
    UG_ProgressSetStyle(&wnd2, PGB_ID_4, PGB_STYLE_3D | PGB_STYLE_NO_FILL);
    UG_ProgressSetProgress(&wnd2, PGB_ID_4, 40);

    /* Textbox alignment row 1 */
    UG_TextboxCreate(&wnd2, &txb_a1, TXB_ID_0, UGUI_POS(10, 320, 150, 40));
    UG_TextboxSetFont(&wnd2, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd2, TXB_ID_0, TXT_ALIGN_TL);
    UG_TextboxSetAlignment(&wnd2, TXB_ID_0, ALIGN_TOP_LEFT);

    UG_TextboxCreate(&wnd2, &txb_a2, TXB_ID_1, UGUI_POS(170, 320, 150, 40));
    UG_TextboxSetFont(&wnd2, TXB_ID_1, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd2, TXB_ID_1, TXT_ALIGN_TC);
    UG_TextboxSetAlignment(&wnd2, TXB_ID_1, ALIGN_TOP_CENTER);

    UG_TextboxCreate(&wnd2, &txb_a3, TXB_ID_2, UGUI_POS(330, 320, 150, 40));
    UG_TextboxSetFont(&wnd2, TXB_ID_2, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd2, TXB_ID_2, TXT_ALIGN_TR);
    UG_TextboxSetAlignment(&wnd2, TXB_ID_2, ALIGN_TOP_RIGHT);

    /* Textbox alignment row 2 */
    UG_TextboxCreate(&wnd2, &txb_a4, TXB_ID_3, UGUI_POS(10, 370, 150, 40));
    UG_TextboxSetFont(&wnd2, TXB_ID_3, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd2, TXB_ID_3, TXT_ALIGN_C);
    UG_TextboxSetAlignment(&wnd2, TXB_ID_3, ALIGN_CENTER);

    UG_TextboxCreate(&wnd2, &txb_a5, TXB_ID_4, UGUI_POS(170, 370, 150, 40));
    UG_TextboxSetFont(&wnd2, TXB_ID_4, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd2, TXB_ID_4, TXT_ALIGN_BL);
    UG_TextboxSetAlignment(&wnd2, TXB_ID_4, ALIGN_BOTTOM_LEFT);

    UG_TextboxCreate(&wnd2, &txb_a6, TXB_ID_5, UGUI_POS(330, 370, 150, 40));
    UG_TextboxSetFont(&wnd2, TXB_ID_5, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd2, TXB_ID_5, TXT_ALIGN_BR);
    UG_TextboxSetAlignment(&wnd2, TXB_ID_5, ALIGN_BOTTOM_RIGHT);

    /* Page switch buttons */
    UG_ButtonCreate(&wnd2, &btn_p2_1, BTN_ID_17, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_17, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd2, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_p2_2, BTN_ID_18, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_18, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd2, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_p2_3, BTN_ID_19, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_19, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd2, BTN_ID_19, BTN_STYLE_3D);
}

/* -------------------------------------------------------------------------------- */
/* -- Page 3 setup                                                                -- */
/* -------------------------------------------------------------------------------- */
static void setup_page3(void)
{
    UG_WindowCreate(&wnd3, objs3, MAX_OBJS_PAGE3, windowHandler);
    UG_WindowSetTitleHeight(&wnd3, 0);
    UG_WindowSetTitleTextFont(&wnd3, FONT_8X8);
    UG_WindowSetTitleText(&wnd3, TXT_TITLE_P3);

    /* Text boxes for transparent / hspace / vspace / font demos */
    UG_TextboxCreate(&wnd3, &txb_p3_1, TXB_ID_0, UGUI_POS(10, 310, 200, 40));
    UG_TextboxSetFont(&wnd3, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd3, TXB_ID_0, TXT_TRANSPARENT_OFF);
    UG_TextboxSetAlignment(&wnd3, TXB_ID_0, ALIGN_CENTER);

    UG_TextboxCreate(&wnd3, &txb_p3_2, TXB_ID_1, UGUI_POS(220, 310, 200, 40));
    UG_TextboxSetFont(&wnd3, TXB_ID_1, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd3, TXB_ID_1, TXT_HSPACE);
    UG_TextboxSetAlignment(&wnd3, TXB_ID_1, ALIGN_CENTER);

    UG_TextboxCreate(&wnd3, &txb_p3_3, TXB_ID_2, UGUI_POS(430, 310, 200, 60));
    UG_TextboxSetFont(&wnd3, TXB_ID_2, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd3, TXB_ID_2, TXT_VSPACE);
    UG_TextboxSetAlignment(&wnd3, TXB_ID_2, ALIGN_TOP_LEFT);

    UG_TextboxCreate(&wnd3, &txb_p3_4, TXB_ID_3, UGUI_POS(10, 360, 200, 40));
    UG_TextboxSetFont(&wnd3, TXB_ID_3, FONT_8X8);
    UG_TextboxSetText(&wnd3, TXB_ID_3, TXT_FONT_ASCII);
    UG_TextboxSetAlignment(&wnd3, TXB_ID_3, ALIGN_CENTER);

    /* Page switch buttons */
    UG_ButtonCreate(&wnd3, &btn_p3_1, BTN_ID_17, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_17, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd3, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd3, &btn_p3_2, BTN_ID_18, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_18, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd3, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd3, &btn_p3_3, BTN_ID_19, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_19, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd3, BTN_ID_19, BTN_STYLE_3D);
}

/* -------------------------------------------------------------------------------- */
/* -- Setup                                                                       -- */
/* -------------------------------------------------------------------------------- */
void GUI_Setup(UG_DEVICE *device)
{
    setlocale(LC_ALL, "");

    UG_Init(&ugui, device);
    UG_FillScreen(C_BLACK);

    setup_page1();
    setup_page2();
    setup_page3();

    update_info_text();

    UG_WindowShow(&wnd1);
}

/* -------------------------------------------------------------------------------- */
/* -- Process                                                                     -- */
/* -------------------------------------------------------------------------------- */
void GUI_Process(void)
{
    /* Shadow only on Page 3. Use next_window if pending. */
    UG_WINDOW *target = ugui.next_window ? ugui.next_window : ugui.active_window;
    UG_FontSetShadow(target == &wnd3 ? 1 : 0);

    if (g_running)
    {
        g_progress = (g_progress >= 100) ? 0 : (UG_U8)(g_progress + 1);
        g_speed = (UG_U8)(50 + ((g_progress * 3) % 50));
        g_level = (UG_U8)(30 + ((g_progress * 7) % 70));

        UG_ProgressSetProgress(&wnd1, PGB_ID_0, g_progress);
        UG_ProgressSetProgress(&wnd1, PGB_ID_1, g_speed);
        UG_ProgressSetProgress(&wnd1, PGB_ID_2, g_level);

        update_info_text();
    }

    UG_Update();

    if (ugui.active_window == &wnd3 && !page3_drawn)
    {
        draw_page3();
        page3_drawn = 1;
        if (ugui.device->flush) ugui.device->flush();
    }
    if (ugui.active_window != &wnd3)
    {
        page3_drawn = 0;
    }
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
        /* Page switch buttons (same IDs in all windows) */
        case BTN_ID_17:
            UG_WindowShow(&wnd1);
            return;
        case BTN_ID_18:
            UG_WindowShow(&wnd2);
            return;
        case BTN_ID_19:
            UG_WindowShow(&wnd3);
            return;

        /* Page 1 buttons */
        case BTN_ID_0:
            g_running = 1;
            update_status_text(TXT_STATUS_RUNNING);
            break;

        case BTN_ID_1:
            g_running = 0;
            update_status_text(TXT_STATUS_STOPPED);
            break;

        case BTN_ID_2:
            g_running = 0;
            g_progress = 0;
            g_speed = 50;
            g_level = 30;
            UG_ProgressSetProgress(&wnd1, PGB_ID_0, 0);
            UG_ProgressSetProgress(&wnd1, PGB_ID_1, 50);
            UG_ProgressSetProgress(&wnd1, PGB_ID_2, 30);
            UG_CheckboxSetChecked(&wnd1, CHB_ID_0, 1);
            UG_CheckboxSetChecked(&wnd1, CHB_ID_1, 0);
            UG_CheckboxSetChecked(&wnd1, CHB_ID_2, 1);
            update_status_text(TXT_STATUS_RESET);
            update_info_text();
            break;

        default:
            break;
        }
        break;

    case OBJ_TYPE_CHECKBOX:
        if (ugui.active_window == &wnd1)
        {
            update_info_text();
        }
        break;

    default:
        break;
    }
}