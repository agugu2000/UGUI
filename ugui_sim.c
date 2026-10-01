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
#define MAX_OBJS_PAGE4      15
#define MAX_OBJS_PAGE5      15
#define MAX_OBJS_PAGE6      20

/* -------------------------------------------------------------------------------- */
/* -- UI text constants                                                           -- */
/* -------------------------------------------------------------------------------- */

/* Page titles (FONT_8X8, English only) */
#define TXT_TITLE_P1            "Page 1: Control"
#define TXT_TITLE_P2            "Page 2: Styles"
#define TXT_TITLE_P3            "Page 3: Draw"
#define TXT_TITLE_P4            "Page 4: Color"
#define TXT_TITLE_P5            "Page 5: Scrollbox"
#define TXT_TITLE_P6            "Page 6: Clipping"

/* Page switch buttons */
#define TXT_BTN_P1              "第1页"
#define TXT_BTN_P2              "第2页"
#define TXT_BTN_P3              "第3页"
#define TXT_BTN_P4              "第4页"
#define TXT_BTN_P5              "第5页"
#define TXT_BTN_P6              "第6页"

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

/* Page 4 — color showcase */
#define TXT_C1  "Normal {#FF0000}Red {#00FF00}Green {#0000FF}Blue{#} Normal"
#define TXT_C2  "中文{#FF0000}红色{#00FF00}绿色{#}默认"
#define TXT_C3  "{#FF0000}Line1 red\nLine2 still red{#}\nLine3 default"
#define TXT_C4  "Esc {{ brace, {#FFAA00}orange{#} back"
#define TXT_C5  "Bad tag {#xyz} stays literal"
#define TXT_C6  "Button text {#FF0000}red{#00FF00}green{#}"

/* Page 4 — shadow showcase ('s' case-insensitive; default == {@s0} == off) */
#define TXT_S1  "阴影 {@s1}落影{@s0} {@s2}描边{@s0} 默认"
#define TXT_S2  "大小写{@S1}DROP{@s0} 混合{#FF0000}红{@s2}影{#}{@s0}"

/* Page 5 — scrollbox demo */
#define TXT_SCB_LABEL \
    "Scrollbox demo: arrows to scroll, L/R for hstep, PgUp/PgDn/Home/End"
#define TXT_SCB_LONG \
    "Line 01: The quick brown fox jumps over the lazy dog.\n" \
    "Line 02: {#FF0000}Red text{#} followed by {#00FF00}green{#} and {#0000FF}blue{#}.\n" \
    "Line 03: 中文测试，滚动盒子应该能正确显示。\n" \
    "Line 04: This is a fairly long line that will overflow horizontally so you can test the horizontal scrollbar.\n" \
    "Line 05: Line 05.\n" \
    "Line 06: Line 06.\n" \
    "Line 07: Line 07.\n" \
    "Line 08: Line 08.\n" \
    "Line 09: Line 09.\n" \
    "Line 10: Line 10.\n" \
    "Line 11: Line 11.\n" \
    "Line 12: Line 12.\n" \
    "Line 13: Line 13.\n" \
    "Line 14: Line 14.\n" \
    "Line 15: Line 15.\n" \
    "Line 16: Line 16.\n" \
    "Line 17: Line 17.\n" \
    "Line 18: Line 18.\n" \
    "Line 19: Line 19.\n" \
    "Line 20: Line 20.\n" \
    "Line 21: Line 21.\n" \
    "Line 22: Line 22.\n" \
    "Line 23: Line 23.\n" \
    "Line 24: Line 24.\n" \
    "Line 25: Line 25 — the end."

/* Page 6 — clipping test */
#define TXT_CLIP_LONG \
    "This is a very long single line that will not fit inside the " \
    "textbox and should be clipped on the right side, not dropped."
#define TXT_CLIP_SCB \
    "Scrollbox clipping test.\n" \
    "Line 2.\n" \
    "Line 3.\n" \
    "Line 4.\n" \
    "Line 5.\n" \
    "Line 6.\n" \
    "Line 7.\n" \
    "Line 8.\n" \
    "Line 9.\n" \
    "Line 10.\n"

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
static UG_WINDOW wnd1, wnd2, wnd3, wnd4, wnd5, wnd6;

/* Page 1 objects */
static UG_PROGRESS pgb_status;
static UG_TEXTBOX  txt_status;
static UG_BUTTON   btn_start, btn_stop, btn_reset;
static UG_CHECKBOX chb_log, chb_refresh, chb_details;
static UG_PROGRESS pgb_speed, pgb_level;
static UG_TEXTBOX  txb_info;
static UG_IMAGE    img_test;
static UG_TEXTBOX  txb_img_label;
static UG_BUTTON   btn_p1_1, btn_p1_2, btn_p1_3, btn_p1_4, btn_p1_5, btn_p1_6;

/* Page 2 objects */
static UG_BUTTON   btn_s1, btn_s2, btn_s3, btn_s4, btn_s5, btn_s6;
static UG_CHECKBOX chb_s1, chb_s2, chb_s3, chb_s4, chb_s5, chb_s6;
static UG_PROGRESS pgb_s1, pgb_s2, pgb_s3, pgb_s4, pgb_s5;
static UG_TEXTBOX  txb_a1, txb_a2, txb_a3, txb_a4, txb_a5, txb_a6;
static UG_BUTTON   btn_p2_1, btn_p2_2, btn_p2_3, btn_p2_4, btn_p2_5, btn_p2_6;

/* Page 3 objects */
static UG_TEXTBOX  txb_p3_1, txb_p3_2, txb_p3_3, txb_p3_4;
static UG_BUTTON   btn_p3_1, btn_p3_2, btn_p3_3, btn_p3_4, btn_p3_5, btn_p3_6;

/* Page 4 objects */
static UG_TEXTBOX  txb_c1, txb_c2, txb_c3, txb_c4, txb_c5, txb_c6;
static UG_TEXTBOX  txb_s1, txb_s2;
static UG_BUTTON   btn_p4_1, btn_p4_2, btn_p4_3, btn_p4_4, btn_p4_5, btn_p4_6;

/* Page 5 objects */
static UG_SCROLLBOX scb_main;
static UG_TEXTBOX   txb_scb_label;
static UG_BUTTON    btn_p5_1, btn_p5_2, btn_p5_3, btn_p5_4, btn_p5_5, btn_p5_6;

/* Page 6 objects */
static UG_TEXTBOX   txb_clip_a;
static UG_BUTTON    btn_clip_b;
static UG_SCROLLBOX scb_clip;
static UG_IMAGE     img_clip_f;
static UG_TEXTBOX   txb_clip_g;
static UG_BUTTON    btn_clip_i;
static UG_BUTTON    btn_p6_1, btn_p6_2, btn_p6_3, btn_p6_4, btn_p6_5, btn_p6_6;

static UG_OBJECT   objs1[MAX_OBJS_PAGE1];
static UG_OBJECT   objs2[MAX_OBJS_PAGE2];
static UG_OBJECT   objs3[MAX_OBJS_PAGE3];
static UG_OBJECT   objs4[MAX_OBJS_PAGE4];
static UG_OBJECT   objs5[MAX_OBJS_PAGE5];
static UG_OBJECT   objs6[MAX_OBJS_PAGE6];

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
    UG_ConsoleReset();
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
    UG_ButtonCreate(&wnd1, &btn_p1_1, BTN_ID_15, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_15, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_15, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd1, BTN_ID_15, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_p1_2, BTN_ID_16, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_16, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_16, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd1, BTN_ID_16, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_p1_3, BTN_ID_17, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_17, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd1, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_p1_4, BTN_ID_18, UGUI_POS(400, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_18, TXT_BTN_P4);
    UG_ButtonSetStyle(&wnd1, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_p1_5, BTN_ID_19, UGUI_POS(530, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_19, TXT_BTN_P5);
    UG_ButtonSetStyle(&wnd1, BTN_ID_19, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd1, &btn_p1_6, BTN_ID_14, UGUI_POS(660, 550, 120, 35));
    UG_ButtonSetFont(&wnd1, BTN_ID_14, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd1, BTN_ID_14, TXT_BTN_P6);
    UG_ButtonSetStyle(&wnd1, BTN_ID_14, BTN_STYLE_3D);

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
    UG_ButtonCreate(&wnd2, &btn_p2_1, BTN_ID_15, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_15, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_15, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd2, BTN_ID_15, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_p2_2, BTN_ID_16, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_16, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_16, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd2, BTN_ID_16, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_p2_3, BTN_ID_17, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_17, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd2, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_p2_4, BTN_ID_18, UGUI_POS(400, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_18, TXT_BTN_P4);
    UG_ButtonSetStyle(&wnd2, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_p2_5, BTN_ID_19, UGUI_POS(530, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_19, TXT_BTN_P5);
    UG_ButtonSetStyle(&wnd2, BTN_ID_19, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd2, &btn_p2_6, BTN_ID_14, UGUI_POS(660, 550, 120, 35));
    UG_ButtonSetFont(&wnd2, BTN_ID_14, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd2, BTN_ID_14, TXT_BTN_P6);
    UG_ButtonSetStyle(&wnd2, BTN_ID_14, BTN_STYLE_3D);
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
    UG_ButtonCreate(&wnd3, &btn_p3_1, BTN_ID_15, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_15, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_15, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd3, BTN_ID_15, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd3, &btn_p3_2, BTN_ID_16, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_16, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_16, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd3, BTN_ID_16, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd3, &btn_p3_3, BTN_ID_17, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_17, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd3, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd3, &btn_p3_4, BTN_ID_18, UGUI_POS(400, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_18, TXT_BTN_P4);
    UG_ButtonSetStyle(&wnd3, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd3, &btn_p3_5, BTN_ID_19, UGUI_POS(530, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_19, TXT_BTN_P5);
    UG_ButtonSetStyle(&wnd3, BTN_ID_19, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd3, &btn_p3_6, BTN_ID_14, UGUI_POS(660, 550, 120, 35));
    UG_ButtonSetFont(&wnd3, BTN_ID_14, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd3, BTN_ID_14, TXT_BTN_P6);
    UG_ButtonSetStyle(&wnd3, BTN_ID_14, BTN_STYLE_3D);

}

/* -------------------------------------------------------------------------------- */
/* -- Page 4 setup                                                                -- */
/* -------------------------------------------------------------------------------- */
static void setup_page4(void)
{
    UG_WindowCreate(&wnd4, objs4, MAX_OBJS_PAGE4, windowHandler);
    UG_WindowSetTitleHeight(&wnd4, 0);
    UG_WindowSetTitleTextFont(&wnd4, FONT_8X8);
    UG_WindowSetTitleText(&wnd4, TXT_TITLE_P4);

    UG_TextboxCreate(&wnd4, &txb_c1, TXB_ID_0, UGUI_POS(10, 10, 770, 40));
    UG_TextboxSetFont(&wnd4, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_0, TXT_C1);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_0, ALIGN_CENTER);

    UG_TextboxCreate(&wnd4, &txb_c2, TXB_ID_1, UGUI_POS(10, 60, 770, 40));
    UG_TextboxSetFont(&wnd4, TXB_ID_1, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_1, TXT_C2);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_1, ALIGN_CENTER);

    UG_TextboxCreate(&wnd4, &txb_c3, TXB_ID_2, UGUI_POS(10, 110, 770, 70));
    UG_TextboxSetFont(&wnd4, TXB_ID_2, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_2, TXT_C3);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_2, ALIGN_CENTER);

    UG_TextboxCreate(&wnd4, &txb_c4, TXB_ID_3, UGUI_POS(10, 190, 770, 40));
    UG_TextboxSetFont(&wnd4, TXB_ID_3, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_3, TXT_C4);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_3, ALIGN_CENTER);

    UG_TextboxCreate(&wnd4, &txb_c5, TXB_ID_4, UGUI_POS(10, 240, 770, 40));
    UG_TextboxSetFont(&wnd4, TXB_ID_4, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_4, TXT_C5);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_4, ALIGN_CENTER);

    UG_TextboxCreate(&wnd4, &txb_c6, TXB_ID_5, UGUI_POS(10, 290, 770, 40));
    UG_TextboxSetFont(&wnd4, TXB_ID_5, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_5, TXT_C6);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_5, ALIGN_CENTER);

    /* Shadow tags demo */
    UG_TextboxCreate(&wnd4, &txb_s1, TXB_ID_6, UGUI_POS(10, 345, 770, 40));
    UG_TextboxSetFont(&wnd4, TXB_ID_6, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_6, TXT_S1);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_6, ALIGN_CENTER);

    UG_TextboxCreate(&wnd4, &txb_s2, TXB_ID_7, UGUI_POS(10, 395, 770, 40));
    UG_TextboxSetFont(&wnd4, TXB_ID_7, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd4, TXB_ID_7, TXT_S2);
    UG_TextboxSetAlignment(&wnd4, TXB_ID_7, ALIGN_CENTER);

    /* Page switch buttons */
    UG_ButtonCreate(&wnd4, &btn_p4_1, BTN_ID_15, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd4, BTN_ID_15, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd4, BTN_ID_15, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd4, BTN_ID_15, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd4, &btn_p4_2, BTN_ID_16, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd4, BTN_ID_16, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd4, BTN_ID_16, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd4, BTN_ID_16, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd4, &btn_p4_3, BTN_ID_17, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd4, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd4, BTN_ID_17, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd4, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd4, &btn_p4_4, BTN_ID_18, UGUI_POS(400, 550, 120, 35));
    UG_ButtonSetFont(&wnd4, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd4, BTN_ID_18, TXT_BTN_P4);
    UG_ButtonSetStyle(&wnd4, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd4, &btn_p4_5, BTN_ID_19, UGUI_POS(530, 550, 120, 35));
    UG_ButtonSetFont(&wnd4, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd4, BTN_ID_19, TXT_BTN_P5);
    UG_ButtonSetStyle(&wnd4, BTN_ID_19, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd4, &btn_p4_6, BTN_ID_14, UGUI_POS(660, 550, 120, 35));
    UG_ButtonSetFont(&wnd4, BTN_ID_14, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd4, BTN_ID_14, TXT_BTN_P6);
    UG_ButtonSetStyle(&wnd4, BTN_ID_14, BTN_STYLE_3D);
}

/* -------------------------------------------------------------------------------- */
/* -- Page 5 setup                                                                -- */
/* -------------------------------------------------------------------------------- */
static void setup_page5(void)
{
    UG_WindowCreate(&wnd5, objs5, MAX_OBJS_PAGE5, windowHandler);
    UG_WindowSetTitleHeight(&wnd5, 0);
    UG_WindowSetTitleTextFont(&wnd5, FONT_8X8);
    UG_WindowSetTitleText(&wnd5, TXT_TITLE_P5);

    /* Label */
    UG_TextboxCreate(&wnd5, &txb_scb_label, TXB_ID_0, UGUI_POS(10, 5, 770, 20));
    UG_TextboxSetFont(&wnd5, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd5, TXB_ID_0, TXT_SCB_LABEL);
    UG_TextboxSetAlignment(&wnd5, TXB_ID_0, ALIGN_CENTER_LEFT);

    /* Scrollbox */
    /* Outer frame: a black-filled textbox acting as a "frame" around the
     * scrollbox. The scrollbox sits on top with a small inner margin. */
    /* Scrollbox inside the frame */
    UG_ScrollBoxCreate(&wnd5, &scb_main, SCB_ID_0, UGUI_POS(209, 28, 386, 286));
    UG_ScrollBoxSetFont(&wnd5, SCB_ID_0, FONT_SIMSUN2_13X13);
    UG_ScrollBoxSetText(&wnd5, SCB_ID_0, TXT_SCB_LONG);
    UG_ScrollBoxSetForeColor(&wnd5, SCB_ID_0, C_BLACK);
    UG_ScrollBoxSetBackColor(&wnd5, SCB_ID_0, C_WHITE);
    UG_ScrollBoxSetBarMode(&wnd5, SCB_ID_0, UG_SCROLLBAR_AUTO, UG_SCROLLBAR_AUTO, 0, 0);

    /* Page switch buttons */
    UG_ButtonCreate(&wnd5, &btn_p5_1, BTN_ID_15, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd5, BTN_ID_15, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd5, BTN_ID_15, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd5, BTN_ID_15, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd5, &btn_p5_2, BTN_ID_16, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd5, BTN_ID_16, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd5, BTN_ID_16, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd5, BTN_ID_16, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd5, &btn_p5_3, BTN_ID_17, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd5, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd5, BTN_ID_17, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd5, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd5, &btn_p5_4, BTN_ID_18, UGUI_POS(400, 550, 120, 35));
    UG_ButtonSetFont(&wnd5, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd5, BTN_ID_18, TXT_BTN_P4);
    UG_ButtonSetStyle(&wnd5, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd5, &btn_p5_5, BTN_ID_19, UGUI_POS(530, 550, 120, 35));
    UG_ButtonSetFont(&wnd5, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd5, BTN_ID_19, TXT_BTN_P5);
    UG_ButtonSetStyle(&wnd5, BTN_ID_19, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd5, &btn_p5_6, BTN_ID_14, UGUI_POS(660, 550, 120, 35));
    UG_ButtonSetFont(&wnd5, BTN_ID_14, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd5, BTN_ID_14, TXT_BTN_P6);
    UG_ButtonSetStyle(&wnd5, BTN_ID_14, BTN_STYLE_3D);
}

/* -------------------------------------------------------------------------------- */
/* -- Page 6 setup: clipping test                                                -- */
/* -------------------------------------------------------------------------------- */
static void setup_page6(void)
{
    UG_WindowCreate(&wnd6, objs6, MAX_OBJS_PAGE6, windowHandler);
    UG_WindowSetTitleHeight(&wnd6, 0);
    UG_WindowSetTitleTextFont(&wnd6, FONT_8X8);
    UG_WindowSetTitleText(&wnd6, TXT_TITLE_P6);

    /* --- A: textbox with bottom-right corner outside the window --- */
    UG_TextboxCreate(&wnd6, &txb_clip_a, TXB_ID_0, UGUI_POS(700, 500, 200, 150));
    UG_TextboxSetFont(&wnd6, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd6, TXB_ID_0, TXT_CLIP_LONG);
    UG_TextboxSetAlignment(&wnd6, TXB_ID_0, ALIGN_TOP_LEFT);

    /* --- B: button with left edge outside the window --- */
    UG_ButtonCreate(&wnd6, &btn_clip_b, BTN_ID_0, UGUI_POS(-50, 50, 150, 100));
    UG_ButtonSetFont(&wnd6, BTN_ID_0, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_0, "Cut L");
    UG_ButtonSetStyle(&wnd6, BTN_ID_0, BTN_STYLE_3D);

    /* --- Scrollbox (also the source of OBJ_EVENT_POSTRENDER) --- */
    UG_ScrollBoxCreate(&wnd6, &scb_clip, SCB_ID_0, UGUI_POS(10, 200, 200, 100));
    UG_ScrollBoxSetFont(&wnd6, SCB_ID_0, FONT_SIMSUN2_13X13);
    UG_ScrollBoxSetText(&wnd6, SCB_ID_0, TXT_CLIP_SCB);
    UG_ScrollBoxSetForeColor(&wnd6, SCB_ID_0, C_BLACK);
    UG_ScrollBoxSetBackColor(&wnd6, SCB_ID_0, C_WHITE);
    UG_ScrollBoxSetBarMode(&wnd6, SCB_ID_0, UG_SCROLLBAR_AUTO, UG_SCROLLBAR_AUTO, 0, 0);

    /* --- F: BMP with right edge outside the window --- */
    UG_ImageCreate(&wnd6, &img_clip_f, IMG_ID_0, UGUI_POS(790, 250, 16, 16));
    UG_ImageSetBMP(&wnd6, IMG_ID_0, &bmp_test);

    /* --- G: textbox with an overlong line, must be clipped --- */
    UG_TextboxCreate(&wnd6, &txb_clip_g, TXB_ID_1, UGUI_POS(10, 320, 380, 180));
    UG_TextboxSetFont(&wnd6, TXB_ID_1, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd6, TXB_ID_1, TXT_CLIP_LONG);
    UG_TextboxSetAlignment(&wnd6, TXB_ID_1, ALIGN_TOP_LEFT);

    /* --- I: button whose text is wider than the button --- */
    UG_ButtonCreate(&wnd6, &btn_clip_i, BTN_ID_1, UGUI_POS(410, 320, 120, 40));
    UG_ButtonSetFont(&wnd6, BTN_ID_1, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_1, "Text much wider than button");
    UG_ButtonSetStyle(&wnd6, BTN_ID_1, BTN_STYLE_3D);

    /* Page switch buttons */
    UG_ButtonCreate(&wnd6, &btn_p6_1, BTN_ID_15, UGUI_POS(10, 550, 120, 35));
    UG_ButtonSetFont(&wnd6, BTN_ID_15, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_15, TXT_BTN_P1);
    UG_ButtonSetStyle(&wnd6, BTN_ID_15, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd6, &btn_p6_2, BTN_ID_16, UGUI_POS(140, 550, 120, 35));
    UG_ButtonSetFont(&wnd6, BTN_ID_16, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_16, TXT_BTN_P2);
    UG_ButtonSetStyle(&wnd6, BTN_ID_16, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd6, &btn_p6_3, BTN_ID_17, UGUI_POS(270, 550, 120, 35));
    UG_ButtonSetFont(&wnd6, BTN_ID_17, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_17, TXT_BTN_P3);
    UG_ButtonSetStyle(&wnd6, BTN_ID_17, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd6, &btn_p6_4, BTN_ID_18, UGUI_POS(400, 550, 120, 35));
    UG_ButtonSetFont(&wnd6, BTN_ID_18, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_18, TXT_BTN_P4);
    UG_ButtonSetStyle(&wnd6, BTN_ID_18, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd6, &btn_p6_5, BTN_ID_19, UGUI_POS(530, 550, 120, 35));
    UG_ButtonSetFont(&wnd6, BTN_ID_19, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_19, TXT_BTN_P5);
    UG_ButtonSetStyle(&wnd6, BTN_ID_19, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd6, &btn_p6_6, BTN_ID_14, UGUI_POS(660, 550, 120, 35));
    UG_ButtonSetFont(&wnd6, BTN_ID_14, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd6, BTN_ID_14, TXT_BTN_P6);
    UG_ButtonSetStyle(&wnd6, BTN_ID_14, BTN_STYLE_3D);
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
    setup_page4();
    setup_page5();
    setup_page6();

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
    UG_FontSetShadow(target == &wnd1 ? 0 :
                     target == &wnd2 ? 2 :
                     target == &wnd3 ? 1 : 0);

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
/* -- Keyboard handling (platform-independent)                                    -- */
/* -------------------------------------------------------------------------------- */
void GUI_HandleKey(int key)
{
    if (ugui.active_window != &wnd5) return;

    /* If the scrollbox is being dragged by touch, ignore keyboard input
     * so the two input paths do not fight over scroll_x/y. */
    if (UG_ScrollBoxIsTouchActive(&wnd5, SCB_ID_0)) return;

#ifdef UGUI_USE_TOUCH
    /* Same-frame guard: if the mouse went down in this frame, the
     * scrollbox's touch_active flag has not been updated yet (that
     * happens in _UG_ScrollBoxUpdate, after GUI_HandleKey runs). */
    if (ugui.touch.state) return;
#endif

    switch (key)
    {
    case GUI_KEY_UP:
        UG_ScrollBoxScrollBy(&wnd5, SCB_ID_0, 0, -UG_ScrollBoxGetLineHeight(&wnd5, SCB_ID_0));
        break;
    case GUI_KEY_DOWN:
        UG_ScrollBoxScrollBy(&wnd5, SCB_ID_0, 0, UG_ScrollBoxGetLineHeight(&wnd5, SCB_ID_0));
        break;
    case GUI_KEY_LEFT:
        UG_ScrollBoxScrollBy(&wnd5, SCB_ID_0, -16, 0);
        break;
    case GUI_KEY_RIGHT:
        UG_ScrollBoxScrollBy(&wnd5, SCB_ID_0, 16, 0);
        break;
    case GUI_KEY_PAGEUP:
        UG_ScrollBoxScrollBy(&wnd5, SCB_ID_0, 0, -100);
        break;
    case GUI_KEY_PAGEDOWN:
        UG_ScrollBoxScrollBy(&wnd5, SCB_ID_0, 0, 100);
        break;
    case GUI_KEY_HOME:
        UG_ScrollBoxSetScroll(&wnd5, SCB_ID_0, 0, 0);
        break;
    case GUI_KEY_END:
        UG_ScrollBoxSetScroll(&wnd5, SCB_ID_0, 0, 100000);
        break;
    default:
        break;
    }
}

/* -------------------------------------------------------------------------------- */
/* -- Window message handler                                                      -- */
/* -------------------------------------------------------------------------------- */
static void windowHandler(UG_MESSAGE *msg)
{
    decode_msg(msg);

    if (msg->type != MSG_TYPE_OBJECT) return;
    if (msg->event == OBJ_EVENT_POSTRENDER &&
        msg->id == OBJ_TYPE_SCROLLBOX &&
        msg->sub_id == SCB_ID_0 &&
        ugui.active_window == &wnd5)
    {
        UG_DrawFrame(210, 40, 600, 330, C_BLACK);
        return;
    }

    /* Page 6: out-of-range drawing primitives. Redrawn every time the
     * scrollbox finishes rendering, so they survive window redraws. */
    if (msg->event == OBJ_EVENT_POSTRENDER &&
        msg->id == OBJ_TYPE_SCROLLBOX &&
        msg->sub_id == SCB_ID_0 &&
        ugui.active_window == &wnd6)
    {
        /* Line crossing the whole screen, both ends far outside */
        UG_DrawLine(-100, -100, 900, 700, C_RED);
        /* Frame far outside, only the middle visible */
        UG_DrawFrame(-50, -50, 850, 650, C_BLUE);
        /* Circle mostly outside (top-left) */
        UG_DrawCircle(-50, -50, 100, C_GREEN);
        /* Arc mostly outside (bottom-right) */
        UG_DrawArc(850, 650, 100, 0xFF, C_MAGENTA);
        /* Filled frame partially outside */
        UG_FillFrame(700, 400, 850, 550, C_YELLOW);
        return;
    }

    if (msg->event != OBJ_EVENT_RELEASED) return;

    switch (msg->id)
    {
    case OBJ_TYPE_BUTTON:
        switch (msg->sub_id)
        {
        /* Page switch buttons (same IDs in all windows) */
        case BTN_ID_15:
            UG_WindowShow(&wnd1);
            return;
        case BTN_ID_16:
            UG_WindowShow(&wnd2);
            return;
        case BTN_ID_17:
            if (ugui.active_window != &wnd3) {
                UG_WindowShow(&wnd3);
            }
            page3_drawn = 0;
            return;
        case BTN_ID_18:
            UG_WindowShow(&wnd4);
            return;
        case BTN_ID_19:
            UG_WindowShow(&wnd5);
            return;
        case BTN_ID_14:
            UG_WindowShow(&wnd6);
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