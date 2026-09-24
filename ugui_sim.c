/* -------------------------------------------------------------------------------- */
/* -- µGUI Simulator Application Layer (Platform Independent)                     -- */
/* -------------------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include "ugui_sim.h"

#include "ugui_button.h"
#include "ugui_checkbox.h"
#include "ugui_textbox.h"
#include "ugui_progress.h"
#include "ugui_fonts.h"

/* -------------------------------------------------------------------------------- */
/* -- Simulator configuration                                                     -- */
/* -------------------------------------------------------------------------------- */
#define WIDTH               640
#define HEIGHT              480
#define SCREEN_MULTIPLIER   2
#define SCREEN_MARGIN       15
#define WINDOW_BACK_COLOR   0x00C0C0C0
#define MAX_OBJS            15

/* -------------------------------------------------------------------------------- */
/* -- Global vars                                                                 -- */
/* -------------------------------------------------------------------------------- */
static simcfg_t *simCfg = NULL;

static UG_GUI    ugui;
static UG_WINDOW wnd;

static UG_BUTTON   btn0, btn1, btn2, btn3;
static UG_CHECKBOX chb0, chb1, chb2, chb3;
static UG_TEXTBOX  txt0, txt1, txt2, txt3;
static UG_PROGRESS pgb0, pgb1;
static UG_OBJECT   objs[MAX_OBJS];

/* -------------------------------------------------------------------------------- */
/* -- Forward decls                                                               -- */
/* -------------------------------------------------------------------------------- */
static void windowHandler(UG_MESSAGE *msg);

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
/* -- Layout helpers                                                              -- */
/* -------------------------------------------------------------------------------- */
#define INITIAL_MARGIN  3
#define BTN_WIDTH       100
#define BTN_HEIGHT      30
#define CHB_WIDTH       100
#define CHB_HEIGHT      14
#define OBJ_Y(i)        (BTN_HEIGHT * (i) + (INITIAL_MARGIN * ((i) + 1)))

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
    UG_WindowSetTitleTextFont(&wnd, FONT_8X8);
    UG_WindowSetTitleText(&wnd, "testone");

    /* ---- Buttons ---- */
    UG_ButtonCreate(&wnd, &btn0, BTN_ID_0,
                    UGUI_POS(INITIAL_MARGIN, OBJ_Y(0), BTN_WIDTH, BTN_HEIGHT));
    UG_ButtonSetFont(&wnd, BTN_ID_0, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd, BTN_ID_0, "中文测试1");
    UG_ButtonSetStyle(&wnd, BTN_ID_0, BTN_STYLE_3D);

    UG_ButtonCreate(&wnd, &btn1, BTN_ID_1,
                    UGUI_POS(INITIAL_MARGIN, OBJ_Y(1), BTN_WIDTH, BTN_HEIGHT));
    UG_ButtonSetFont(&wnd, BTN_ID_1, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd, BTN_ID_1, "中文测试2");
    UG_ButtonSetStyle(&wnd, BTN_ID_1, BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS);

    UG_ButtonCreate(&wnd, &btn2, BTN_ID_2,
                    UGUI_POS(INITIAL_MARGIN, OBJ_Y(2), BTN_WIDTH, BTN_HEIGHT));
    UG_ButtonSetFont(&wnd, BTN_ID_2, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd, BTN_ID_2, "中文测试3");
    UG_ButtonSetStyle(&wnd, BTN_ID_2, BTN_STYLE_3D | BTN_STYLE_USE_ALTERNATE_COLORS);
    UG_ButtonSetAlternateForeColor(&wnd, BTN_ID_2, C_BLACK);
    UG_ButtonSetAlternateBackColor(&wnd, BTN_ID_2, C_WHITE);

    UG_ButtonCreate(&wnd, &btn3, BTN_ID_3,
                    UGUI_POS(INITIAL_MARGIN, OBJ_Y(3), BTN_WIDTH, BTN_HEIGHT));
    UG_ButtonSetFont(&wnd, BTN_ID_3, FONT_SIMSUN2_13X13);
    UG_ButtonSetText(&wnd, BTN_ID_3, "中文测试4");
    UG_ButtonSetStyle(&wnd, BTN_ID_3, BTN_STYLE_NO_BORDERS | BTN_STYLE_TOGGLE_COLORS);

    /* ---- Checkboxes ---- */
    UG_CheckboxCreate(&wnd, &chb0, CHB_ID_0,
                      UGUI_POS(INITIAL_MARGIN * 2 + BTN_WIDTH, OBJ_Y(0) + 7, CHB_WIDTH, CHB_HEIGHT));
    UG_CheckboxSetFont(&wnd, CHB_ID_0, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd, CHB_ID_0, "测试中文A");
    UG_CheckboxSetStyle(&wnd, CHB_ID_0, CHB_STYLE_3D);
    UG_CheckboxSetAlignment(&wnd, CHB_ID_0, ALIGN_TOP_LEFT);
#if !defined(UGUI_USE_COLOR_BW)
    UG_CheckboxSetBackColor(&wnd, CHB_ID_0, C_PALE_TURQUOISE);
#endif

    UG_CheckboxCreate(&wnd, &chb1, CHB_ID_1,
                      UGUI_POS(INITIAL_MARGIN * 2 + BTN_WIDTH, OBJ_Y(1) + 7, CHB_WIDTH, CHB_HEIGHT));
    UG_CheckboxSetFont(&wnd, CHB_ID_1, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd, CHB_ID_1, "测试中文B");
    UG_CheckboxSetStyle(&wnd, CHB_ID_1, CHB_STYLE_2D | CHB_STYLE_TOGGLE_COLORS);
    UG_CheckboxSetAlignment(&wnd, CHB_ID_1, ALIGN_CENTER);
    UG_CheckboxShow(&wnd, CHB_ID_1);

    UG_CheckboxCreate(&wnd, &chb2, CHB_ID_2,
                      UGUI_POS(INITIAL_MARGIN * 2 + BTN_WIDTH, OBJ_Y(2) + 7, CHB_WIDTH, CHB_HEIGHT));
    UG_CheckboxSetFont(&wnd, CHB_ID_2, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd, CHB_ID_2, "测试中文C");
    UG_CheckboxSetStyle(&wnd, CHB_ID_2, CHB_STYLE_3D | CHB_STYLE_USE_ALTERNATE_COLORS);
    UG_CheckboxSetAlignment(&wnd, CHB_ID_2, ALIGN_BOTTOM_LEFT);
    UG_CheckboxShow(&wnd, CHB_ID_2);

    UG_CheckboxCreate(&wnd, &chb3, CHB_ID_3,
                      UGUI_POS(INITIAL_MARGIN * 2 + BTN_WIDTH, OBJ_Y(3) + 7, CHB_WIDTH, CHB_HEIGHT));
    UG_CheckboxSetFont(&wnd, CHB_ID_3, FONT_SIMSUN2_13X13);
    UG_CheckboxSetText(&wnd, CHB_ID_3, "测试中文D");
    UG_CheckboxSetStyle(&wnd, CHB_ID_3, CHB_STYLE_NO_BORDERS | CHB_STYLE_TOGGLE_COLORS);
    UG_CheckboxSetAlignment(&wnd, CHB_ID_3, ALIGN_BOTTOM_RIGHT);
    UG_CheckboxShow(&wnd, CHB_ID_3);

    /* ---- Textboxes ---- */
    UG_TextboxCreate(&wnd, &txt0, TXB_ID_0,
                     UGUI_POS(INITIAL_MARGIN * 3 + BTN_WIDTH + CHB_WIDTH, OBJ_Y(0), 100, 15));
    UG_TextboxSetFont(&wnd, TXB_ID_0, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd, TXB_ID_0, "测试中文E");
#if !defined(UGUI_USE_COLOR_BW)
    UG_TextboxSetBackColor(&wnd, TXB_ID_0, C_PALE_TURQUOISE);
#endif

    UG_TextboxCreate(&wnd, &txt1, TXB_ID_1,
                     UGUI_POS(INITIAL_MARGIN * 3 + BTN_WIDTH + CHB_WIDTH, OBJ_Y(1) - 15, 100, 30));
    UG_TextboxSetFont(&wnd, TXB_ID_1, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd, TXB_ID_1, "测试中文F");
#if !defined(UGUI_USE_COLOR_BW)
    UG_TextboxSetBackColor(&wnd, TXB_ID_1, C_PALE_TURQUOISE);
#endif
    UG_TextboxSetAlignment(&wnd, TXB_ID_1, ALIGN_TOP_RIGHT);

    UG_TextboxCreate(&wnd, &txt2, TXB_ID_2,
                     UGUI_POS(INITIAL_MARGIN * 3 + BTN_WIDTH + CHB_WIDTH, OBJ_Y(2) - 15, 100, 45));
    UG_TextboxSetFont(&wnd, TXB_ID_2, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd, TXB_ID_2, "测试中文G");
#if !defined(UGUI_USE_COLOR_BW)
    UG_TextboxSetBackColor(&wnd, TXB_ID_2, C_PALE_TURQUOISE);
#endif

    UG_TextboxCreate(&wnd, &txt3, TXB_ID_3,
                     UGUI_POS(INITIAL_MARGIN * 3 + BTN_WIDTH + CHB_WIDTH, OBJ_Y(3), 100, 53));
    UG_TextboxSetFont(&wnd, TXB_ID_3, FONT_SIMSUN2_13X13);
    UG_TextboxSetText(&wnd, TXB_ID_3, "测试中文H");
#if !defined(UGUI_USE_COLOR_BW)
    UG_TextboxSetBackColor(&wnd, TXB_ID_3, C_PALE_TURQUOISE);
#endif

    /* ---- Progress bars ---- */
    UG_ProgressCreate(&wnd, &pgb0, PGB_ID_0,
                      UGUI_POS(INITIAL_MARGIN, OBJ_Y(4) + 20, 157, 20));
    UG_ProgressSetProgress(&wnd, PGB_ID_0, 35);

    UG_ProgressCreate(&wnd, &pgb1, PGB_ID_1,
                      UGUI_POS(159 + INITIAL_MARGIN * 2, OBJ_Y(4) + 25, 156, 10));
    UG_ProgressSetStyle(&wnd, PGB_ID_1, PGB_STYLE_2D | PGB_STYLE_FORE_COLOR_MESH);
    UG_ProgressSetProgress(&wnd, PGB_ID_1, 75);

    UG_WindowShow(&wnd);
}

/* -------------------------------------------------------------------------------- */
/* -- Process                                                                     -- */
/* -------------------------------------------------------------------------------- */
void GUI_Process(void)
{
    UG_Update();
}

/* -------------------------------------------------------------------------------- */
/* -- Window message handler                                                      -- */
/* -------------------------------------------------------------------------------- */
static void windowHandler(UG_MESSAGE *msg)
{
    decode_msg(msg);

#if defined(UGUI_USE_TOUCH)
    if (msg->type == MSG_TYPE_OBJECT)
    {
        UG_OBJECT* obj = msg->src;
        if (obj)
        {
            if (obj->touch_state & OBJ_TOUCH_STATE_CHANGED)                 printf("|CHANGED");
            if (obj->touch_state & OBJ_TOUCH_STATE_PRESSED_ON_OBJECT)       printf("|PRESSED_ON_OBJECT");
            if (obj->touch_state & OBJ_TOUCH_STATE_PRESSED_OUTSIDE_OBJECT)  printf("|PRESSED_OUTSIDE_OBJECT");
            if (obj->touch_state & OBJ_TOUCH_STATE_RELEASED_ON_OBJECT)      printf("|RELEASED_ON_OBJECT");
            if (obj->touch_state & OBJ_TOUCH_STATE_RELEASED_OUTSIDE_OBJECT) printf("|RELEASED_OUTSIDE_OBJECT");
            if (obj->touch_state & OBJ_TOUCH_STATE_IS_PRESSED_ON_OBJECT)    printf("|IS_PRESSED_ON_OBJECT");
            if (obj->touch_state & OBJ_TOUCH_STATE_IS_PRESSED)              printf("|IS_PRESSED");
            if (obj->touch_state & OBJ_TOUCH_STATE_INIT)                    printf("|INIT");
            printf("\n");
        }

        /* Advance progress bars on click (same behavior as original) */
        if (UG_ProgressGetProgress(&wnd, PGB_ID_0) == 100)
            UG_ProgressSetProgress(&wnd, PGB_ID_0, 0);
        else
            UG_ProgressSetProgress(&wnd, PGB_ID_0, UG_ProgressGetProgress(&wnd, PGB_ID_0) + 1);

        if (UG_ProgressGetProgress(&wnd, PGB_ID_1) == 100)
            UG_ProgressSetProgress(&wnd, PGB_ID_1, 0);
        else
            UG_ProgressSetProgress(&wnd, PGB_ID_1, UG_ProgressGetProgress(&wnd, PGB_ID_1) + 1);
    }
#endif
}