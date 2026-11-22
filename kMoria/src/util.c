/*********************************************************************
 * kMoria - Moria adapted for the PalmPilot.                         *
 * This file is copyright (C) 2000 Bridget Spitznagel                *
 *                                                                   *
 * Other portions of this program are derived from Moria.            *
 * Moria is copyright 1985-94 James E. Wilson, Robert A. Koeneke,    *
 * Jimmey Wayne Todd, et al. (see main.c for full credits)           *
 *                                                                   *
 * This program is freeware.  You can redistribute it and/or modify  *
 * it (provided that this notice and the full credits are retained), *
 * but it CANNOT be distributed for profit.                          *
 *********************************************************************/
/* #include "palm.h" That's all this file REALLY needs... */
#include "kMoria.h"

/**********************************************************************
  Tell the Palm to act like the user tapped the given button...
  Return true if we did (a very few callers would like to know)
**********************************************************************/
Boolean hit_button_if_usable(FormPtr frm, Word btn_index)
{
  ControlPtr btn;
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_index));
  if (btn->attr.usable)
    CtlHitControl(btn);
  return (btn->attr.usable);
}


/**********************************************************************
                       UPDATE_FIELD_SCROLLERS
 IN:
 frm, fld, up_scroller, down_scroller = various UI doodads
 OUT:
 nothing
 PURPOSE:
 Update the given scroller widgets (for the given field 
 (in the given form)), according to whether the field is scrollable
 in the "up" and "down" directions.
 **********************************************************************/
void update_field_scrollers(FormPtr frm, FieldPtr fld,
			    Word up_scroller, Word down_scroller) 
{
  Boolean u, d;
  u = FldScrollable(fld, winUp);
  d = FldScrollable(fld, winDown);
  FrmUpdateScrollers(frm, 
		     FrmGetObjectIndex(frm, up_scroller),
		     FrmGetObjectIndex(frm, down_scroller),
		     u, d);
  return;
}


/**********************************************************************
                       PAGE_SCROLL_FIELD
 IN:
 frm, fld = various UI doodads
 dir = whether to scroll 'fld' up or down
 OUT:
 PURPOSE:
 Call this to scroll the field 'fld' up/down by one "page".
 (The caller should call update_field_scrollers immediately afterward.)
 **********************************************************************/
#ifndef I_AM_COLOR
void page_scroll_field(FormPtr frm, FieldPtr fld, DirectionType dir)
#else /* I_AM_COLOR */
void page_scroll_field(FormPtr frm, FieldPtr fld, WinDirectionType dir)
#endif /* I_AM_COLOR */
{
  Word linesToScroll;

  /* how many lines can we scroll? */
  if (FldScrollable(fld, dir)) {
    linesToScroll = FldGetVisibleLines(fld) - 1;
    FldScrollField(fld, linesToScroll, dir);
  }

  return;
}


/* how to exit a popup form that you can enter from more than one place */
void LeaveForm()
{
   FormPtr frm;
   frm = FrmGetActiveForm();
   FrmEraseForm (frm);
   FrmDeleteForm (frm);
   FrmSetActiveForm (FrmGetFirstForm ());
#ifndef I_AM_OS_2
   if (IsVGA) {
     Word curfrm =  FrmGetActiveFormID();
     if (curfrm == MainForm)
       move_gsi(FrmGetActiveForm(), true);
   }
#endif
}


/***************************************************************
                   md_malloc
 IN:
 n = size to allocate
 OUT:
 pointer to the locked chunk
 PURPOSE:
 Allocate and lock a moveable chunk of memory.
****************************************************************/
Char * md_malloc(Int n)
{
  VoidHand h;
  VoidPtr p;

  h = MemHandleNew((ULong) n); /* will this cast work??  apparently. */
  if (!h) {
    /* the caller might want to check this and die. */
    return NULL;
  }

  p = MemHandleLock(h);
  MemSet(p, n, 0); /* just to make really sure the memory is zeroed */
  return p;
}



// Caller must promise that buf+len has at least 20 chars.
void seconds_to_string(ULong seconds, Char *buf, Short start)
{
  DateTimeType datetime;
  Short len;
  len = start;
  if (len)
    buf[len++] = ' ';
  TimSecondsToDateTime(seconds, &datetime); // see DateTime.h
  DateToAscii(datetime.month, datetime.day, datetime.year,
	      PrefGetPreference(prefDateFormat), buf+len);
  len = StrLen(buf);
  buf[len++] = ' ';
  TimeToAscii(datetime.hour, datetime.minute,
	      PrefGetPreference(prefTimeFormat), buf+len);
}
