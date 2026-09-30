#include "ugui_scrollbox.h"
#include <string.h>

/* -------------------------------------------------------------------------------- */
/* -- SCROLLBOX FUNCTIONS                                                       -- */
/* -------------------------------------------------------------------------------- */

static void _UG_ScrollBoxUpdate(UG_WINDOW* wnd, UG_OBJECT* obj);

/* Clamp v to [lo, hi]. Requires lo <= hi. */
static UG_S32 _clamp32(UG_S32 v, UG_S32 lo, UG_S32 hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Track length in pixels, inclusive endpoints. */
static UG_S32 _track_len(UG_S32 start, UG_S32 end)
{
    UG_S32 len = end - start + 1;
    return (len > 0) ? len : 0;
}

/* thumb = clamp(track * view / content, min_thumb, track).
 * Division protected: content <= 0 or content <= view -> full track. */
static UG_S32 _thumb_len(UG_S32 track, UG_S32 view, UG_S32 content, UG_S32 min_thumb)
{
    UG_S32 t;
    if (track <= 0) return 0;
    if (content <= 0 || content <= view) return track;
    t = (track * view) / content;
    if (t < min_thumb) t = min_thumb;
    if (t > track) t = track;
    if (t < 1) t = 1;
    return t;
}

/* pos = (track - thumb) * scroll / scroll_max. Division protected. */
static UG_S32 _thumb_pos(UG_S32 track, UG_S32 thumb, UG_S32 scroll, UG_S32 scroll_max)
{
    UG_S32 span;
    if (scroll_max <= 0) return 0;
    span = track - thumb;
    if (span <= 0) return 0;
    return (span * scroll) / scroll_max;
}

/*
 * Decide which scrollbars are visible and compute effective viewport size.
 * Two-pass: assume no bars, then subtract bars that pass 1 said are needed.
 * Convergence guaranteed in at most two passes.
 */
static void _resolve_bars(const UG_SCROLLBOX* scb,
                          UG_S32* out_hbar_visible,
                          UG_S32* out_vbar_visible,
                          UG_S32* out_view_w_eff,
                          UG_S32* out_view_h_eff)
{
    UG_S32 view_w = _track_len(scb->view.xs, scb->view.xe);
    UG_S32 view_h = _track_len(scb->view.ys, scb->view.ye);
    UG_S32 bar_w = scb->vbar_thickness;   /* vertical bar width */
    UG_S32 bar_h = scb->hbar_thickness;   /* horizontal bar height */
    UG_S32 content_w_ext = scb->content_w + scb->offset_x;
    UG_S32 content_h_ext = scb->content_h + scb->offset_y;
    UG_S32 need_h = 0, need_v = 0;
    UG_S32 vw, vh;

    /* Pass 1: assume no bars, decide which are needed. */
    vw = view_w;
    vh = view_h;
    if (scb->hbar_mode == UG_SCROLLBAR_ALWAYS) need_h = 1;
    else if (scb->hbar_mode == UG_SCROLLBAR_AUTO && content_w_ext > vw) need_h = 1;
    if (scb->vbar_mode == UG_SCROLLBAR_ALWAYS) need_v = 1;
    else if (scb->vbar_mode == UG_SCROLLBAR_AUTO && content_h_ext > vh) need_v = 1;

    /* Pass 2: subtract the bars decided in pass 1, then re-decide.
     * Reset need_h / need_v to 0 before re-deciding, otherwise a "needed"
     * verdict from pass 1 could never be revoked. */
    vw = view_w - (need_v ? bar_w : 0);
    vh = view_h - (need_h ? bar_h : 0);
    if (vw < 0) vw = 0;
    if (vh < 0) vh = 0;

    need_h = 0;
    if (scb->hbar_mode == UG_SCROLLBAR_ALWAYS) need_h = 1;
    else if (scb->hbar_mode == UG_SCROLLBAR_AUTO && content_w_ext > vw) need_h = 1;

    need_v = 0;
    if (scb->vbar_mode == UG_SCROLLBAR_ALWAYS) need_v = 1;
    else if (scb->vbar_mode == UG_SCROLLBAR_AUTO && content_h_ext > vh) need_v = 1;

    /* Final effective sizes. */
    vw = view_w - (need_v ? bar_w : 0);
    vh = view_h - (need_h ? bar_h : 0);
    if (vw < 0) vw = 0;
    if (vh < 0) vh = 0;

    *out_hbar_visible = need_h;
    *out_vbar_visible = need_v;
    *out_view_w_eff = vw;
    *out_view_h_eff = vh;
}

/*
 * Re-measure content size.
 *   content_h = line_count * line_h
 *   content_w = max over lines of _UG_MeasureTextLine
 *   line_h    = ascender - descender + v_space
 * _UG_MeasureTextLine consumes the trailing '\n', so *c points past it.
 */
static void _UG_ScrollBoxLayout(UG_SCROLLBOX* scb)
{
    UG_FONT* font = scb->font;
    UG_S32 line_h;
    UG_S32 max_w = 0;
    UG_S32 lines = 0;
    char* s;

    if (!font || !scb->str) {
        scb->content_w = 0;
        scb->content_h = 0;
        scb->line_h = 0;
        scb->line_count = 0;
        scb->layout_dirty = 0;
        return;
    }

    /* Industry standard: line height = ascender - descender + v_space */
    line_h = (UG_S32)UG_GetFontAscender(font)
           - (UG_S32)UG_GetFontDescender(font)
           + (UG_S32)scb->v_space;
    if (line_h < 1) line_h = 1;
    scb->line_h = line_h;

    s = scb->str;
    lines = 1;
    while (1) {
        char* c = s;
        UG_S32 wl = _UG_MeasureTextLine(&c, font, scb->h_space, scb->runs);
        if (wl > max_w) max_w = wl;
        if (*c == '\0') break;
        s = c;
        lines++;
    }

    scb->content_w = max_w;
    scb->content_h = line_h * lines;
    scb->line_count = lines;
    scb->layout_dirty = 0;
}

static void _clamp_scroll(UG_SCROLLBOX* scb, UG_S32 view_w_eff, UG_S32 view_h_eff)
{
    UG_S32 max_x = scb->content_w + scb->offset_x - view_w_eff;
    UG_S32 max_y = scb->content_h + scb->offset_y - view_h_eff;
    if (max_x < 0) max_x = 0;
    if (max_y < 0) max_y = 0;
    scb->scroll_x = _clamp32(scb->scroll_x, 0, max_x);
    scb->scroll_y = _clamp32(scb->scroll_y, 0, max_y);
}

static void _draw_vbar(UG_SCROLLBOX* scb, UG_S32 view_h_eff, UG_AREA* clip)
{
    UG_S32 bar_w = scb->vbar_thickness;
    UG_S32 track_xs = scb->view.xe - bar_w + 1;
    UG_S32 track_xe = scb->view.xe;
    UG_S32 track_ys = scb->view.ys;
    UG_S32 track_ye = scb->view.ys + view_h_eff - 1;
    UG_S32 track_len = _track_len(track_ys, track_ye);
    UG_S32 content_ext = scb->content_h + scb->offset_y;
    UG_S32 scroll_max = (content_ext > view_h_eff) ? (content_ext - view_h_eff) : 0;
    UG_S32 thumb = _thumb_len(track_len, view_h_eff, content_ext, scb->bar_min_thumb);
    UG_S32 pos = _thumb_pos(track_len, thumb, scb->scroll_y, scroll_max);
    UG_S32 thumb_ys = track_ys + pos;
    UG_S32 thumb_ye = thumb_ys + thumb - 1;

    _UG_FillFrameClipped(track_xs, track_ys,
                         track_xe, track_ye, clip, scb->bar_track_color);
    _UG_FillFrameClipped(track_xs, thumb_ys,
                         track_xe, thumb_ye, clip, scb->bar_thumb_color);
}

static void _draw_hbar(UG_SCROLLBOX* scb, UG_S32 view_w_eff, UG_AREA* clip)
{
    UG_S32 bar_h = scb->hbar_thickness;
    UG_S32 track_ys = scb->view.ye - bar_h + 1;
    UG_S32 track_ye = scb->view.ye;
    UG_S32 track_xs = scb->view.xs;
    UG_S32 track_xe = scb->view.xs + view_w_eff - 1;
    UG_S32 track_len = _track_len(track_xs, track_xe);
    UG_S32 content_ext = scb->content_w + scb->offset_x;
    UG_S32 scroll_max = (content_ext > view_w_eff) ? (content_ext - view_w_eff) : 0;
    UG_S32 thumb = _thumb_len(track_len, view_w_eff, content_ext, scb->bar_min_thumb);
    UG_S32 pos = _thumb_pos(track_len, thumb, scb->scroll_x, scroll_max);
    UG_S32 thumb_xs = track_xs + pos;
    UG_S32 thumb_xe = thumb_xs + thumb - 1;
    UG_S32 thumb_ye = track_ye;

    _UG_FillFrameClipped(track_xs, track_ys,
                         track_xe, track_ye, clip, scb->bar_track_color);
    _UG_FillFrameClipped(thumb_xs, track_ys,
                         thumb_xe, thumb_ye, clip, scb->bar_thumb_color);
}

static void _UG_ScrollBoxUpdate(UG_WINDOW* wnd, UG_OBJECT* obj)
{
    UG_SCROLLBOX* scb;
    UG_AREA a;
    UG_AREA vis;
    UG_AREA clip;
    UG_S32 hbar_vis, vbar_vis;
    UG_S32 view_w_eff, view_h_eff;
    UG_S32 yp;
    char* s;
    UG_U16 char_index = 0;
    UG_COLOR cur_fc;

    /* Get object-specific data */
    scb = (UG_SCROLLBOX*)(obj->data);

#ifdef UGUI_USE_TOUCH
    /* Touch drag: process before the state checks, because a drag may
     * be the only thing that sets OBJ_STATE_UPDATE | OBJ_STATE_REDRAW. */
    if (obj->touch_state & OBJ_TOUCH_STATE_CHANGED) {
        if (obj->touch_state & OBJ_TOUCH_STATE_PRESSED_ON_OBJECT) {
            scb->touch_last_x = UG_GetGUI()->touch.xp;
            scb->touch_last_y = UG_GetGUI()->touch.yp;
            scb->touch_active = 1;
        }
        if (obj->touch_state & (OBJ_TOUCH_STATE_RELEASED_ON_OBJECT |
                                OBJ_TOUCH_STATE_RELEASED_OUTSIDE_OBJECT)) {
            scb->touch_active = 0;
        }
        obj->touch_state &= ~OBJ_TOUCH_STATE_CHANGED;
    }
    if (scb->touch_active &&
        (obj->touch_state & OBJ_TOUCH_STATE_IS_PRESSED_ON_OBJECT)) {
        UG_S16 dx = UG_GetGUI()->touch.xp - scb->touch_last_x;
        UG_S16 dy = UG_GetGUI()->touch.yp - scb->touch_last_y;
        if (dx != 0) {
            scb->scroll_x -= dx;
            scb->touch_last_x = UG_GetGUI()->touch.xp;
            obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
        }
        if (dy != 0) {
            scb->scroll_y -= dy;
            scb->touch_last_y = UG_GetGUI()->touch.yp;
            obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
        }
    }
#endif

    if ( !(obj->state & OBJ_STATE_UPDATE) ) return;
    if ( !(obj->state & OBJ_STATE_VISIBLE) ) {
        UG_FillFrame(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, wnd->bc);
        obj->state &= ~OBJ_STATE_UPDATE;
        return;
    }

    if ( !(obj->state & OBJ_STATE_REDRAW) ) {
        obj->state &= ~OBJ_STATE_UPDATE;
        return;
    }

    UG_WindowGetArea(wnd,&a);
    obj->a_abs.xs = obj->a_rel.xs + a.xs;
    obj->a_abs.ys = obj->a_rel.ys + a.ys;
    obj->a_abs.xe = obj->a_rel.xe + a.xs;
    obj->a_abs.ye = obj->a_rel.ye + a.ys;

    /* Visible rectangle = object rectangle ∩ window rectangle */
    vis.xs = (obj->a_abs.xs > wnd->xs) ? obj->a_abs.xs : wnd->xs;
    vis.ys = (obj->a_abs.ys > wnd->ys) ? obj->a_abs.ys : wnd->ys;
    vis.xe = (obj->a_abs.xe < wnd->xe) ? obj->a_abs.xe : wnd->xe;
    vis.ye = (obj->a_abs.ye < wnd->ye) ? obj->a_abs.ye : wnd->ye;
    if (vis.xs > vis.xe || vis.ys > vis.ye) {
        obj->state &= ~OBJ_STATE_UPDATE;
        return;
    }

#ifdef UGUI_USE_PRERENDER_EVENT
    _UG_SendObjectPrerenderEvent(wnd, obj);
#endif

    scb->view.xs = obj->a_abs.xs;
    scb->view.ys = obj->a_abs.ys;
    scb->view.xe = obj->a_abs.xe;
    scb->view.ye = obj->a_abs.ye;

    if (scb->layout_dirty) _UG_ScrollBoxLayout(scb);

    _resolve_bars(scb, &hbar_vis, &vbar_vis, &view_w_eff, &view_h_eff);
    _clamp_scroll(scb, view_w_eff, view_h_eff);

    /* Content clip = content area (view minus bars), intersected with
     * the window-visible rectangle. Base the content area on scb->view
     * (the full object rectangle), not on vis, so that partially
     * off-window scrollboxes still get a correct content clip. */
    clip.xs = scb->view.xs;
    clip.ys = scb->view.ys;
    clip.xe = (UG_S16)(scb->view.xs + view_w_eff - 1);
    clip.ye = (UG_S16)(scb->view.ys + view_h_eff - 1);
    if (clip.xs < vis.xs) clip.xs = vis.xs;
    if (clip.ys < vis.ys) clip.ys = vis.ys;
    if (clip.xe > vis.xe) clip.xe = vis.xe;
    if (clip.ye > vis.ye) clip.ye = vis.ye;
    if (clip.xs > clip.xe || clip.ys > clip.ye) {
        obj->state &= ~OBJ_STATE_UPDATE;
        return;
    }

    /* Clear viewport */
    _UG_FillFrameClipped(scb->view.xs, scb->view.ys,
                         scb->view.xe, scb->view.ye, &vis, scb->bc);

    /* Draw content. No culling: glyphs outside clip are skipped by
     * _UG_PutGlyph -> _UG_ClipGlyph. */
    if (scb->font && scb->str) {
        /* For new-format fonts, baseline = line top + ascender.
         * For old-format fonts, ascender == line height, so ascender works
         * for both. Fall back to line_h if the font reports nothing. */
        UG_S32 asc = (UG_S32)UG_GetFontAscender(scb->font);
        if (asc <= 0) asc = scb->line_h;

        yp = scb->view.ys
           + scb->offset_y
           - scb->scroll_y
           + asc;

        s = scb->str;
        cur_fc = scb->fc;
        while (1) {
            UG_S32 xp = scb->view.xs
                      + scb->offset_x
                      - scb->scroll_x;

            /* Bottom cull: if this line's top edge is already below the
             * clip bottom, every subsequent line is below too. Stop the
             * loop instead of iterating over the rest of the content.
             * This is standard viewport culling. */
            if (yp - asc > clip.ye) break;

            _UG_DrawTextLine(&s, xp, yp, &cur_fc, scb->fc, scb->bc,
                             scb->font, scb->runs, scb->run_count,
                             &char_index, &clip, scb->h_space);

            if (*s == '\0') break;
            yp += scb->line_h;
        }
    }

    if (vbar_vis) _draw_vbar(scb, view_h_eff, &vis);
    if (hbar_vis) _draw_hbar(scb, view_w_eff, &vis);

    obj->state &= ~OBJ_STATE_REDRAW;
#ifdef UGUI_USE_POSTRENDER_EVENT
    _UG_SendObjectPostrenderEvent(wnd, obj);
#endif
    obj->state &= ~OBJ_STATE_UPDATE;
}

/* -------------------------------------------------------------------------------- */
/* -- Public API                                                                -- */
/* -------------------------------------------------------------------------------- */

UG_RESULT UG_ScrollBoxCreate( UG_WINDOW* wnd, UG_SCROLLBOX* scb, UG_U8 id,
                              UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
    UG_OBJECT* obj;

    obj = _UG_GetFreeObject( wnd );
    if ( obj == NULL ) return UG_RESULT_FAIL;

    memset(scb, 0, sizeof(*scb));
    scb->font = UG_GetGUI() != NULL ? (UG_GetGUI()->font) : NULL;
    scb->fc = wnd->fc;
    scb->bc = wnd->bc;
    scb->h_space = 0;
    scb->v_space = 0;
    scb->runs = NULL;
    scb->run_count = 0;
    scb->hbar_mode = UG_SCROLLBAR_AUTO;
    scb->vbar_mode = UG_SCROLLBAR_AUTO;
    /* Scrollbar thickness: 1.5% of the corresponding viewport dimension,
     * clamped to [3, 12]. The vertical bar's width scales with view_h,
     * the horizontal bar's height scales with view_w, so both stay
     * proportional on any aspect ratio. */
    {
        UG_S16 w = xe - xs + 1;
        UG_S16 h = ye - ys + 1;
        UG_S16 tv = (UG_S16)((UG_S32)h * 15 / 1000);
        UG_S16 th = (UG_S16)((UG_S32)w * 15 / 1000);
        if (tv < 3) tv = 3;
        if (tv > 12) tv = 12;
        if (th < 3) th = 3;
        if (th > 12) th = 12;
        scb->vbar_thickness = tv;
        scb->hbar_thickness = th;
    }
    scb->bar_min_thumb = 8;
    scb->bar_track_color = C_WHITE_94;
    scb->bar_thumb_color = C_WHITE_39;
    scb->touch_last_x = 0;
    scb->touch_last_y = 0;
    scb->touch_active = 0;
    scb->layout_dirty = 1;

    obj->update = _UG_ScrollBoxUpdate;
    #ifdef UGUI_USE_TOUCH
    obj->touch_state = OBJ_TOUCH_STATE_INIT;
    #endif
    obj->type = OBJ_TYPE_SCROLLBOX;
    obj->event = OBJ_EVENT_NONE;
    obj->a_rel.xs = xs;
    obj->a_rel.ys = ys;
    obj->a_rel.xe = xe;
    obj->a_rel.ye = ye;
    obj->a_abs.xs = -1;
    obj->a_abs.ys = -1;
    obj->a_abs.xe = -1;
    obj->a_abs.ye = -1;
    obj->id = id;
    obj->state |= OBJ_STATE_VISIBLE | OBJ_STATE_REDRAW | OBJ_STATE_VALID;
    #ifdef UGUI_USE_TOUCH
    obj->state |= OBJ_STATE_TOUCH_ENABLE;
    #endif
    obj->data = (void*)scb;
    obj->state &= ~OBJ_STATE_FREE;

    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxDelete( UG_WINDOW* wnd, UG_U8 id )
{
    return _UG_DeleteObject( wnd, OBJ_TYPE_SCROLLBOX, id );
}

UG_RESULT UG_ScrollBoxShow( UG_WINDOW* wnd, UG_U8 id )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    if ( obj == NULL ) return UG_RESULT_FAIL;
    obj->state |= OBJ_STATE_VISIBLE;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxHide( UG_WINDOW* wnd, UG_U8 id )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    if ( obj == NULL ) return UG_RESULT_FAIL;
    obj->state &= ~OBJ_STATE_VISIBLE;
    obj->state |= OBJ_STATE_UPDATE;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetText( UG_WINDOW* wnd, UG_U8 id, char* str )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->str = str;
    /* Setting new text invalidates any previously-set runs. Caller must
     * call UG_ScrollBoxSetRuns again if runs are still wanted. */
    scb->runs = NULL;
    scb->run_count = 0;
    scb->layout_dirty = 1;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetRuns( UG_WINDOW* wnd, UG_U8 id,
                               UG_ColorRun* runs, UG_U16 run_count )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->runs = runs;
    scb->run_count = run_count;
    /* Runs do not change layout (they are pure color info), so no
     * layout_dirty. But the string is now interpreted as plain text,
     * so we must redraw. */
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetFont( UG_WINDOW* wnd, UG_U8 id, UG_FONT* font )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->font = font;
    scb->layout_dirty = 1;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->fc = fc;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->bc = bc;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetHSpace( UG_WINDOW* wnd, UG_U8 id, UG_S16 hs )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->h_space = hs;
    scb->layout_dirty = 1;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetVSpace( UG_WINDOW* wnd, UG_U8 id, UG_S16 vs )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->v_space = vs;
    scb->layout_dirty = 1;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetViewport( UG_WINDOW* wnd, UG_U8 id,
                                   UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    if ( obj == NULL ) return UG_RESULT_FAIL;
    obj->a_rel.xs = xs;
    obj->a_rel.ys = ys;
    obj->a_rel.xe = xe;
    obj->a_rel.ye = ye;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetContentOffset( UG_WINDOW* wnd, UG_U8 id,
                                        UG_S32 ox, UG_S32 oy )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->offset_x = ox;
    scb->offset_y = oy;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetBarMode( UG_WINDOW* wnd, UG_U8 id,
                                  UG_U8 hbar, UG_U8 vbar,
                                  UG_S16 vbar_thickness,
                                  UG_S16 hbar_thickness )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->hbar_mode = hbar;
    scb->vbar_mode = vbar;
    if (vbar_thickness > 0) scb->vbar_thickness = vbar_thickness;
    if (hbar_thickness > 0) scb->hbar_thickness = hbar_thickness;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxSetBarColor( UG_WINDOW* wnd, UG_U8 id,
                                   UG_COLOR track, UG_COLOR thumb )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->bar_track_color = track;
    scb->bar_thumb_color = thumb;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_U8 UG_ScrollBoxIsTouchActive( UG_WINDOW* wnd, UG_U8 id )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return 0;
    scb = (UG_SCROLLBOX*)(obj->data);
    return scb->touch_active;
}

UG_RESULT UG_ScrollBoxSetScroll( UG_WINDOW* wnd, UG_U8 id,
                                 UG_S32 sx, UG_S32 sy )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->scroll_x = sx;
    scb->scroll_y = sy;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxScrollBy( UG_WINDOW* wnd, UG_U8 id,
                                UG_S32 dx, UG_S32 dy )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    scb->scroll_x += dx;
    scb->scroll_y += dy;
    obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxGetContentSize( UG_WINDOW* wnd, UG_U8 id,
                                      UG_S32* w, UG_S32* h )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    if (scb->layout_dirty) _UG_ScrollBoxLayout(scb);
    if (w) *w = scb->content_w;
    if (h) *h = scb->content_h;
    return UG_RESULT_OK;
}

UG_RESULT UG_ScrollBoxGetScroll( UG_WINDOW* wnd, UG_U8 id,
                                 UG_S32* sx, UG_S32* sy )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return UG_RESULT_FAIL;
    scb = (UG_SCROLLBOX*)(obj->data);
    if (sx) *sx = scb->scroll_x;
    if (sy) *sy = scb->scroll_y;
    return UG_RESULT_OK;
}

UG_S32 UG_ScrollBoxGetLineHeight( UG_WINDOW* wnd, UG_U8 id )
{
    UG_OBJECT* obj = _UG_SearchObject( wnd, OBJ_TYPE_SCROLLBOX, id );
    UG_SCROLLBOX* scb;
    if ( obj == NULL ) return 0;
    scb = (UG_SCROLLBOX*)(obj->data);
    if (scb->layout_dirty) _UG_ScrollBoxLayout(scb);
    return scb->line_h;
}