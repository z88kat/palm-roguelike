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
#include "kMoria.h"


Boolean About_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_about_ok:
      LeaveForm();
      handled = true;
      break;
    case btn_about_more:
      //      FrmHelp(AboutStr);
      FrmPopupForm(IntroForm); // let's try that.
      handled = true;
      break;
    case btn_about_credits:
      FrmHelp(CreditStr);
      handled = true;
      break;
    }
    break;

  case keyDownEvent:
    switch(e->data.keyDown.chr) {
    case '\n': case 'd':
      LeaveForm();
      handled = true;
      break;
    case 'v': case 'c': case 'i':      
      FrmHelp(AboutStr);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}



static void print_intro_stuff() SEC_3;
Boolean Intro_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    print_intro_stuff();
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case penDownEvent: case keyDownEvent:
    LeaveForm();
    handled = true;
    break;

  default:
    break;
  }

  return handled;
}




static void print_intro_stuff()
{
  Char buf[80];
  Short x=2, y=0, dy=LINE_HEIGHT, maxx=SCR_WIDTH-4, len;
  StrPrintF(buf, "kMoria ©2000-02, Moria ©1985-94");
  len = StrLen(buf);
  WinDrawChars_ctr(buf, len, maxx/2, y);
  y += dy;
#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  FntSetFont(SmallFont);
#endif
  x = 2;
  dy = visible_char_h_itsy + 1;
  y += dy/2;

  StrPrintF(buf, "Designer and programmer of Moria:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "Robert Alan Koeneke");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  y += dy/2;
  StrPrintF(buf, "Assistant programmers: ");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "Jimmey Wayne Todd, Gary D. McAdoo");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  y += dy/2;
  StrPrintF(buf, "UNIX Port: James E. Wilson");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  y += dy/2;
  StrPrintF(buf, "MSDOS Port: Don Kneller");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "BRUCE Moria: Christopher Stuart");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "Amiga Port: Corey Gehman");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  y += dy/2;
  StrPrintF(buf, "Version 5.5: David J. Grabiner");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "PalmOS Port: Bridget Spitznagel");
  WinDrawChars(buf, StrLen(buf), x, y);

  y = SCR_HEIGHT - (dy*7 + 2);
  StrPrintF(buf, "Moria may be copied and modified");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "freely as long as the above");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "credits are retained.  No one");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "who-so-ever may sell or market");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "this software without the express");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "written consent of the author");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += dy;
  StrPrintF(buf, "Robert Alan Koeneke.");
  WinDrawChars(buf, StrLen(buf), x, y);
  
#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(BigFont);
#endif

}

// Fabulous.  This would all fit on an expanded screen in DEFAULT font
// for HandEra 330.  Need to tweak the line wrapping for the last part tho'.
