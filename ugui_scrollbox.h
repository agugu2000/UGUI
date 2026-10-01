#ifndef __UGUI_SCROLLBOX_H
#define __UGUI_SCROLLBOX_H

#include "ugui.h"

typedef struct
{
   char*    str;
   UG_FONT* font;
   UG_COLOR fc;
   UG_COLOR bc;
   UG_S16   h_space;
   UG_S16   v_space;

   /* Optional pre-decoded color runs. ... */
   UG_ColorRun* runs;
   UG_U16       run_count;

   /* Optional pre-decoded shadow runs. Same rules as runs above. */
   UG_ShadowRun* shadow_runs;
   UG_U16        shadow_run_count;

   /* Viewport (screen coords, closed interval). Derived from a_abs.
    * UG_AREA is uGUI-global and stays UG_S16. */
   UG_AREA  view;

   /* Content offset: ... */
   UG_S32   offset_x;
   UG_S32   offset_y;

   /* Scroll state (content coords, pixels). ... */
   UG_S32   scroll_x;
   UG_S32   scroll_y;

   /* Scrollbar appearance. */
   UG_U8    hbar_mode;
   UG_U8    vbar_mode;
   UG_S16   vbar_thickness;   /* vertical scrollbar width */
   UG_S16   hbar_thickness;   /* horizontal scrollbar height */
   UG_S16   bar_min_thumb;    /* minimum thumb length (both axes) */
   UG_COLOR bar_track_color;
   UG_COLOR bar_thumb_color;

   /* Measured content size. */
   UG_S32   content_w;
   UG_S32   content_h;
   UG_S32   line_h;
   UG_S32   line_count;

   /* Touch drag state (only used when UGUI_USE_TOUCH is enabled). */
   UG_S16   touch_last_x;
   UG_S16   touch_last_y;
   UG_U8    touch_active;

   /* Internal: ... */
   UG_U8    layout_dirty;
} UG_SCROLLBOX;

#define UG_SCROLLBAR_AUTO    0
#define UG_SCROLLBAR_ALWAYS  1
#define UG_SCROLLBAR_NEVER   2

#define OBJ_TYPE_SCROLLBOX                            6

#define SCB_ID_0                                      OBJ_ID_0
#define SCB_ID_1                                      OBJ_ID_1
#define SCB_ID_2                                      OBJ_ID_2
#define SCB_ID_3                                      OBJ_ID_3
#define SCB_ID_4                                      OBJ_ID_4
#define SCB_ID_5                                      OBJ_ID_5
#define SCB_ID_6                                      OBJ_ID_6
#define SCB_ID_7                                      OBJ_ID_7
#define SCB_ID_8                                      OBJ_ID_8
#define SCB_ID_9                                      OBJ_ID_9
#define SCB_ID_10                                     OBJ_ID_10
#define SCB_ID_11                                     OBJ_ID_11
#define SCB_ID_12                                     OBJ_ID_12
#define SCB_ID_13                                     OBJ_ID_13
#define SCB_ID_14                                     OBJ_ID_14
#define SCB_ID_15                                     OBJ_ID_15
#define SCB_ID_16                                     OBJ_ID_16
#define SCB_ID_17                                     OBJ_ID_17
#define SCB_ID_18                                     OBJ_ID_18
#define SCB_ID_19                                     OBJ_ID_19

UG_RESULT UG_ScrollBoxCreate( UG_WINDOW* wnd, UG_SCROLLBOX* scb, UG_U8 id,
                              UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye );
UG_RESULT UG_ScrollBoxDelete( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ScrollBoxShow( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ScrollBoxHide( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ScrollBoxSetText( UG_WINDOW* wnd, UG_U8 id, char* str );
UG_RESULT UG_ScrollBoxSetRuns( UG_WINDOW* wnd, UG_U8 id,
                               UG_ColorRun* runs, UG_U16 run_count );
UG_RESULT UG_ScrollBoxSetShadowRuns( UG_WINDOW* wnd, UG_U8 id,
                                     UG_ShadowRun* runs, UG_U16 run_count );
UG_RESULT UG_ScrollBoxSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font );
UG_RESULT UG_ScrollBoxSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc );
UG_RESULT UG_ScrollBoxSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc );
UG_RESULT UG_ScrollBoxSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S16 hs );
UG_RESULT UG_ScrollBoxSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S16 vs );
UG_RESULT UG_ScrollBoxSetViewport( UG_WINDOW* wnd, UG_U8 id,
                                   UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye );
UG_RESULT UG_ScrollBoxSetContentOffset( UG_WINDOW* wnd, UG_U8 id,
                                        UG_S32 ox, UG_S32 oy );
UG_RESULT UG_ScrollBoxSetBarMode( UG_WINDOW* wnd, UG_U8 id,
                                  UG_U8 hbar, UG_U8 vbar,
                                  UG_S16 vbar_thickness,
                                  UG_S16 hbar_thickness );
UG_RESULT UG_ScrollBoxSetBarColor( UG_WINDOW* wnd, UG_U8 id,
                                   UG_COLOR track, UG_COLOR thumb );
/* Returns non-zero if the scrollbox is currently being dragged by touch.
 * Applications can use this to ignore keyboard input while a touch drag
 * is in progress, so the two input paths do not fight over scroll_x/y. */
UG_U8 UG_ScrollBoxIsTouchActive( UG_WINDOW* wnd, UG_U8 id );
UG_RESULT UG_ScrollBoxSetScroll( UG_WINDOW* wnd, UG_U8 id,
                                 UG_S32 sx, UG_S32 sy );
UG_RESULT UG_ScrollBoxScrollBy( UG_WINDOW* wnd, UG_U8 id,
                                UG_S32 dx, UG_S32 dy );
UG_RESULT UG_ScrollBoxGetContentSize( UG_WINDOW* wnd, UG_U8 id,
                                      UG_S32* w, UG_S32* h );
UG_RESULT UG_ScrollBoxGetScroll( UG_WINDOW* wnd, UG_U8 id,
                                 UG_S32* sx, UG_S32* sy );
UG_S32    UG_ScrollBoxGetLineHeight( UG_WINDOW* wnd, UG_U8 id );

#endif