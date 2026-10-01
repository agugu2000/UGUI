#include "ugui_progress.h"

/* -------------------------------------------------------------------------------- */
/* -- Progress Bar FUNCTIONS                                                          -- */
/* -------------------------------------------------------------------------------- */

/* Static functions */
static void _UG_ProgressUpdate(UG_WINDOW* wnd, UG_OBJECT* obj);

const UG_COLOR pal_progress[] = {
    C_PAL_PROGRESS
};

UG_RESULT UG_ProgressCreate( UG_WINDOW* wnd, UG_PROGRESS* pgb, UG_U8 id, UG_S16 xs, UG_S16 ys, UG_S16 xe, UG_S16 ye )
{
   UG_OBJECT* obj;

   obj = _UG_GetFreeObject( wnd );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   /* Initialize object-specific parameters */
   pgb->style = PGB_STYLE_3D;
   pgb->fc = wnd->fc;
   pgb->bc = wnd->bc;
   pgb->progress = 0;

   /* Initialize standard object parameters */
   obj->update = _UG_ProgressUpdate;
   obj->type = OBJ_TYPE_PROGRESS;
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
   obj->data = (void*)pgb;

   /* Update function: Do your thing! */
   obj->state &= ~OBJ_STATE_FREE;

   return UG_RESULT_OK;
}

UG_RESULT UG_ProgressDelete( UG_WINDOW* wnd, UG_U8 id )
{
   return _UG_DeleteObject( wnd, OBJ_TYPE_PROGRESS, id );
}

UG_RESULT UG_ProgressShow( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state |= OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ProgressHide( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   obj->state &= ~OBJ_STATE_VISIBLE;
   obj->state |= OBJ_STATE_UPDATE;

   return UG_RESULT_OK;
}

UG_RESULT UG_ProgressSetForeColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR fc )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   pgb = (UG_PROGRESS*)(obj->data);
   pgb->fc = fc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ProgressSetBackColor( UG_WINDOW* wnd, UG_U8 id, UG_COLOR bc )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   pgb = (UG_PROGRESS*)(obj->data);
   pgb->bc = bc;
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ProgressSetStyle( UG_WINDOW* wnd, UG_U8 id, UG_U8 style )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   pgb = (UG_PROGRESS*)(obj->data);

   pgb->style &= ~(PGB_STYLE_NO_BORDERS | PGB_STYLE_FORE_COLOR_MESH | PGB_STYLE_NO_FILL);
   if ( style & PGB_STYLE_NO_BORDERS )
   {
      pgb->style |= PGB_STYLE_NO_BORDERS;
   }
   if ( style & PGB_STYLE_FORE_COLOR_MESH )
   {
      pgb->style |= PGB_STYLE_FORE_COLOR_MESH;
   }
   if ( style & PGB_STYLE_NO_FILL )
   {
      pgb->style |= PGB_STYLE_NO_FILL;
   }

   /* 3D or 2D */
   if ( style & PGB_STYLE_3D )
   {
      pgb->style |= PGB_STYLE_3D;
   }
   else
   {
      pgb->style &= ~PGB_STYLE_3D;
   }
   obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;

   return UG_RESULT_OK;
}

UG_RESULT UG_ProgressSetProgress( UG_WINDOW* wnd, UG_U8 id, UG_U8 progress )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj == NULL ) return UG_RESULT_FAIL;

   progress = progress > 100 ? 100 : progress;

   pgb = (UG_PROGRESS*)(obj->data);

   // Only update if different
   if(progress != pgb->progress)
   {
      // Any change requires a full redraw of the progress area
      obj->state |= OBJ_STATE_UPDATE | OBJ_STATE_REDRAW;
      pgb->progress = progress;
   }

   return UG_RESULT_OK;
}

UG_COLOR UG_ProgressGetForeColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj != NULL )
   {
      pgb = (UG_PROGRESS*)(obj->data);
      c = pgb->fc;
   }
   return c;
}

UG_COLOR UG_ProgressGetBackColor( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;
   UG_COLOR c = C_BLACK;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj != NULL )
   {
      pgb = (UG_PROGRESS*)(obj->data);
      c = pgb->bc;
   }
   return c;
}

UG_U8 UG_ProgressGetStyle( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;
   UG_U8 style = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj != NULL )
   {
      pgb = (UG_PROGRESS*)(obj->data);
      style = pgb->style;
   }
   return style;
}

UG_U8 UG_ProgressGetProgress( UG_WINDOW* wnd, UG_U8 id )
{
   UG_OBJECT* obj=NULL;
   UG_PROGRESS* pgb=NULL;
   UG_U8 c = 0;

   obj = _UG_SearchObject( wnd, OBJ_TYPE_PROGRESS, id );
   if ( obj != NULL )
   {
      pgb = (UG_PROGRESS*)(obj->data);
      c = pgb->progress;
   }
   return c;
}

static void _UG_ProgressUpdate(UG_WINDOW* wnd, UG_OBJECT* obj)
{
   UG_PROGRESS* pgb;
   UG_AREA a;
   UG_AREA vis;
   UG_U8 d=0;
   UG_S16 w, wps, wpe;

   /* Get object-specific data */
   pgb = (UG_PROGRESS*)(obj->data);

   /* -------------------------------------------------- */
   /* Object update section                              */
   /* -------------------------------------------------- */
   if ( obj->state & OBJ_STATE_UPDATE )
   {
      if ( obj->state & OBJ_STATE_VISIBLE )
      {
         /* Full redraw necessary? */
         if ( obj->state & OBJ_STATE_REDRAW )
         {
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
            if (vis.xs > vis.xe || vis.ys > vis.ye) return;

#ifdef UGUI_USE_PRERENDER_EVENT
            _UG_SendObjectPrerenderEvent(wnd, obj);
#endif

            d = 1;
            /* 3D or 2D style? */
            if ( !(pgb->style & PGB_STYLE_NO_BORDERS) )
            {
               if ( pgb->style & PGB_STYLE_3D )
               {  /* 3D */
                  _UG_DrawObjectFrameClipped(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, &vis, (UG_COLOR*)pal_progress);
                  d += 3;
               }
               else
               {  /* 2D */
                  _UG_DrawFrameClipped(obj->a_abs.xs, obj->a_abs.ys, obj->a_abs.xe, obj->a_abs.ye, &vis, pgb->fc);
                  d += 1;
               }
            }

            w   = ((obj->a_abs.xe-d)-(obj->a_abs.xs+d));
            wps = (w > 0) ? (w * pgb->progress / 100) : 0;
            wpe = w - wps;

            /* Draw the remaining (background) frame first, unless NO_FILL. */
            if ( !(pgb->style & PGB_STYLE_NO_FILL) )
            {
               if(wpe > 0)
               {
                  UG_S16 xs = obj->a_abs.xs + d + wps;
                  UG_S16 xe = obj->a_abs.xe - d;

                  if ( pgb->style & PGB_STYLE_FORE_COLOR_MESH )
                  {
                     // FIXME: Need fix, if start at 0, it is shifted 1 pixel right.
                     // Needed to match mesh pattern, otherwise it would "scroll right"
                     if((((obj->a_abs.xs+d) & 1) && (wps & 1)) || (!((obj->a_abs.xs+d) & 1) && !(wps & 1)))
                        xs++;
                     _UG_DrawMeshClipped(xs, obj->a_abs.ys+d, xe, obj->a_abs.ye-d, 2, &vis, pgb->fc);
                  }
                  else
                  {
                     _UG_FillFrameClipped(xs, obj->a_abs.ys+d, xe, obj->a_abs.ye-d, &vis, pgb->bc);
                  }
               }
            }

            // Draw elapsed frame
            if(pgb->progress > 0)
            {
               _UG_FillFrameClipped(obj->a_abs.xs+d, obj->a_abs.ys+d, obj->a_abs.xs+d+wps, obj->a_abs.ye-d, &vis, pgb->fc);
            }
#ifdef UGUI_USE_POSTRENDER_EVENT
            _UG_SendObjectPostrenderEvent(wnd, obj);
#endif
         }
      }
      else
      {
         if ( !(pgb->style & PGB_STYLE_NO_FILL) )
            UG_FillFrame(obj->a_abs.xs+d, obj->a_abs.ys+d, obj->a_abs.xe-d, obj->a_abs.ye-d, wnd->bc);
      }
      obj->state &= ~OBJ_STATE_UPDATE;
   }
}
