/*********************************************************************
 * iRogue - Rogue adapted for the PalmPilot.                         *
 * Copyright (C) 1999 Bridget Spitznagel                             *
 * Note: This program is derived from rogue 5.3-clone for Linux.     *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License       *
 * as published by the Free Software Foundation; either version 2    *
 * of the License, or (at your option) any later version.            *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License for more details.                      *
 *                                                                   *
 * You should have received a copy of the GNU General Public License *
 * along with this program; if not, write to                         *
 * The Free Software Foundation, Inc.,                               *
 * 59 Temple Place - Suite 330,                                      *
 * Boston, MA  02111-1307, USA.                                      *
 *********************************************************************/

#include "iRogue.h"

#ifndef I_AM_COLOR
#include <SystemMgr.rh> // for user name
#else /* I_AM_COLOR */
#include <SystemMgr.h> // needed? or not?
#endif /* I_AM_COLOR */
#include <DLServer.h> // for user name
#include <SoundMgr.h> // for feeping
#include <Preferences.h> // for feeping

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


/***************************************************************************
 *  written for form_prefs, generalized for form_call:
 ***************************************************************************/

/* written late at night, needs sanity check */
void usr_form_init(FormPtr frm, Word field_id, Char *source, Short max_size)
{
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_id));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (!vh) {
    vh = MemHandleNew((max_size + 2) * sizeof(Char));
  }
  FldSetTextHandle(fld, (Handle) 0);
  p = MemHandleLock(vh);
  //  StrNCopy(p, sotu->username, 30);
  if (source)
    StrNCopy(p, source, max_size);
  else
    p[0] = '\0';
  MemHandleUnlock(vh);
  FldSetTextHandle(fld, (Handle) vh);
}

/***************************************************************************
 *  written for form_prefs, generalized for form_call:
 ***************************************************************************/

/* written late at night, needs sanity check */
void usr_form_update(FormPtr frm, Word field_id, Char *target, Short max_size)
{
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_id));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (vh) {
    p = MemHandleLock(vh);
    //    StrNCopy(sotu->username, p, 30);
    if (target)
      StrNCopy(target, p, max_size);
    MemHandleUnlock(vh);
  }
}


/***************************************************************
                   x
 IN:
 n = size to allocate
 OUT:
 pointer to the locked chunk
 PURPOSE:
 Allocate and lock a moveable chunk of memory.
****************************************************************/
Char * md_malloc(Int n) {
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
static void seconds_to_string(ULong seconds, Char *buf, Short start)
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

// (Time/voltage display...
// I wrote this for kMoria and then copied it over to here. -bridget)
//
#define EMDASH 151
// Caller must promise that buf has at least ?? chars (60 is enough for sure)
void show_time_and_volts(Char *buf)
{
  // dateStringLength = 9, timeStringLength = 9, voltage length = 14...
  //   Char buf[60];
  Short len;
  //
  UInt volts_times_100;
  UInt warn_at, critical_at, max_ticks;
  UChar battpercent = 0;
  SysBatteryKind battkind;
  Boolean plugged_in;

  
#ifndef I_AM_COLOR
  volts_times_100 = SysBatteryInfo(false, &warn_at, &critical_at, &max_ticks,
				   &battkind, &plugged_in);
#else
  volts_times_100 = SysBatteryInfo(false, &warn_at, &critical_at, &max_ticks,
				   &battkind, &plugged_in, &battpercent);
#endif

  seconds_to_string(TimGetSeconds(), buf, 0);
  len = StrLen(buf);
  buf[len++] = ' ';
  if (!plugged_in) {
    Short v = volts_times_100/100, vdec = volts_times_100 % 100;
    StrPrintF(buf+len, "  %c   %d%s%d V", EMDASH, v,
	      (vdec<10 ? ".0" : "."), vdec);
    if (battpercent) {
      len = StrLen(buf);
      StrPrintF(buf+len, " (%d%%)", battpercent);
    }
  } else {
    StrPrintF(buf+len, "  %c   [charging]", EMDASH);
  }
  //  message(buf);
}


/***************************************************************
                   IS_VOWEL
 IN:
 character
 OUT:
 true if character is a vowel
 PURPOSE:
 Use to determine whether to prefix a word with "a" or "an"
****************************************************************/
Boolean is_vowel(Short ch)
{
  return( (ch == 'a') ||
	  (ch == 'e') ||
	  (ch == 'i') ||
	  (ch == 'o') ||
	  (ch == 'u') );
}



/**********************************************************************
                       GET_USERNAME
 IN:
 sotu = various globals (username)
 OUT:
 nothing
 PURPOSE:
 Get a default username, truncating it if necessary.
 (originally from Reptoids, modified some.)
 **********************************************************************/
void get_username() {
  Char *tmp, *first_wspace;
  VoidHand h;
  tmp = md_malloc(sizeof(Char) * (dlkMaxUserNameLength + 1));
  DlkGetSyncInfo(NULL, NULL, NULL, tmp, NULL, NULL);
  /* if it's too long, use the first name only */
  if (StrLen(tmp) > 30) {
    first_wspace = StrChr(tmp, spaceChr);
    if (first_wspace)
      *(first_wspace) = '\0';
    else
      tmp[30] = '\0';
  }
  /* username is 32 chars allocated */
  if (StrLen(tmp))
    StrNCopy(sotu->username, tmp, 31);
  else
    StrPrintF(sotu->username, "rogue");

  h = MemPtrRecoverHandle(tmp);
  if (h) MemHandleFree(h);  
}

// moved do_feep to iRogue.c
/**********************************************************************
                       MAKE_IT_FIT
 IN:
 buf = the string that might be too long
 avail_space = the width (in .rcp terms) of the space it goes in
 maxbuflen = the space allocated to buf
 OUT:
 nothing (may modify buf)
 PURPOSE:
 If necessary, this truncates 'buf' and adds "..." to the end.
 **********************************************************************/
void make_it_fit(CharPtr buf, Short avail_space, Short maxbuflen)
{
  /* let's not overflow the space available */
  Boolean fits;
  Int length_msg, width_msg;
  length_msg = StrLen(buf);
  width_msg = avail_space;
  FntCharsInWidth(buf, &width_msg, &length_msg, &fits);
  if (!fits) {
    width_msg -= 14; /* make room for ducklings, er, "..." */
    FntCharsInWidth(buf, &width_msg, &length_msg, &fits);
    if (length_msg+5 > maxbuflen)
      length_msg -= 5; /* to be sure.  actually this will never happen? */
    buf[length_msg+1] = '.';
    buf[length_msg+2] = '.';
    buf[length_msg+3] = '.';
    buf[length_msg+4] = '\0'; /* check for FENCEPOST error */
  }
}


/**********************************************************************
                       DRAW_CIRCLE
 IN:
 x,y = center (pixels)
 radius = radius (pixels)
 b = whether to draw it in black (or erase it in "white")
 various globals
 OUT:
 nothing
 PURPOSE:
 Draw a circle on the screen...
 **********************************************************************/
void draw_circle(Short x, Short y, Short radius, Boolean b)
{
  RectangleType r;
  RctSetRectangle(&r, x-radius, y-radius, 2*radius, 2*radius);
  if (b)
    WinDrawRectangle(&r, radius);
  else 
    WinEraseRectangle(&r, radius);
}




/**********************************************************************
                       DO_FEEP
 IN:
 frq = frequency
 dur = duration
 OUT:
 nothing
 PURPOSE:
 This will produce a beep of the requested frequency and duration,
 if the pilot owner has "game sound" turned on in the Preferences app.
 (Try to keep it down to chirps...)
 **********************************************************************/
/* SoundLevelType: slOn, slOff
   gameSoundLevel found in SystemPreferencesType
 */
void do_feep(Long frq, UInt dur) {
  SystemPreferencesChoice allgamesound;

  if (!my_prefs.sound_on)
    return;

#ifdef I_AM_OS_2
  allgamesound = prefGameSoundLevel;
#else
  allgamesound = prefGameSoundLevelV20;
#endif

  if (PrefGetPreference(allgamesound) != slOff) {
    /* click: 200, 9
       confirmation: 500, 70  */
    SndCommandType sndCmd;
    sndCmd.cmd = sndCmdFreqDurationAmp; /* "play a sound" */
    sndCmd.param1 = frq; /* frequency in Hz */
    sndCmd.param2 = dur; /* duration in milliseconds */
    sndCmd.param3 = sndDefaultAmp; /* amplitude (0 to sndMaxAmp) */
    SndDoCmd(0, &sndCmd, true);
  }
}


/* how to exit a popup form that you can enter from more than one place */
void LeaveForm()
{
   FormPtr frm;
   frm = FrmGetActiveForm();
   FrmEraseForm (frm);
   FrmDeleteForm (frm);
   FrmSetActiveForm (FrmGetFirstForm ());
}

/***********************************************************************
 * swiped from HandEra example...
 *
 * FUNCTION:    PrvMoveObject
 *
 * DESCRIPTION: This routine moves an object vertically within a form.
 *
 * PARAMETERS:  
 *
 * RETURNED:    nothing
 *
 ***********************************************************************/
#ifndef I_AM_OS_2
void PrvMoveObject(FormPtr frmP, UInt objid, Coord y_diff, Boolean draw)
{
    RectangleType r;
    UInt objIndex = FrmGetObjectIndex(frmP, objid);
    if (y_diff == 0) return;    

    FrmGetObjectBounds(frmP, objIndex, &r);
    if (draw)
    {
        RctInsetRectangle(&r, -2);   //need to erase the frame as well
        WinEraseRectangle(&r, 0);
        RctInsetRectangle(&r, 2);
    }    
    r.topLeft.y += y_diff;
    FrmSetObjectBounds(frmP, objIndex, &r);
}
void PrvResizeList(FormPtr frm, UInt objid, UInt bot_objid, Coord y_diff)
{
  // Wonder if I need to erase the old space.
  Short lines, x, y, y_bot;
  ListPtr lst;
  UInt objIndex = FrmGetObjectIndex(frm, objid);
  lst = FrmGetObjectPtr(frm, objIndex);
  // Ok, the lists I'm calling this on all are positioned just below the
  // title bar, and end just above a single row of buttons, one of which
  // is supplied in bot_objid.  Make the list as big as possible!
  //
  // In 160x160 these (x, y)'s start out at (0, 15) and (5, 141).
  // We really only care about the y's.
  FrmGetObjectPosition(frm, objIndex, &x, &y);
  FrmGetObjectPosition(frm, FrmGetObjectIndex(frm, bot_objid), &x, &y_bot);
  // Ok.  y_bot - y is the absolute max space available to the list.
  // Figure out how many lines we can put into that space (yay int division).
  lines = (y_bot - y) / FntLineHeight();
  LstSetHeight(lst, lines);
  /*
    // This did not work:  ... maybe because I had objid instead of objIndex!
  FrmGetObjectBounds(frm, objIndex, &r);
  h = FntLineHeight();
  vis_items = r.extent.y / h; // calculate height INCLUDING blank slots thanks!
  vis_items = vis_items + y_diff / h; // add (or subtract) change
  // Hey - if subtracting - will int division work right, or fencepost?
  LstSetHeight(lst, vis_items);
  */
}
void PrvResizeField(FormPtr frm, UInt objid, Coord y_diff)
{
  // Wonder if I need to erase the old space.  Probably I DO.
  RectangleType r;
  Short h;
  FieldPtr fld;
  UInt objIndex = FrmGetObjectIndex(frm, objid);
  if (y_diff == 0) return;
  fld = FrmGetObjectPtr(frm, objIndex);
  FldGetBounds(fld, &r); // same as using FrmGetObjectBounds, I b'lieve
  h = FntLineHeight();
  r.extent.y = r.extent.y + h * (y_diff / h); // could just add y_diff...
  FldSetBounds(fld, &r);
  // Hmmm.  It says "don't change the WIDTH of the object while it's visible"
}
#endif
