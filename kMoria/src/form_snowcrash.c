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

extern Short snowcrash;
static void explain_error();

Boolean SnowCrash_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    /* this must be AFTER draw form */
    explain_error();
    handled = true;
    break;

  default:
    break;
  }

  return handled;
}

static void explain_error()
{
  Char buf[80];
  Short dy = LINE_HEIGHT, y = dy+1;
  switch(snowcrash) { // see main.c StartApplication
  case 1:    // compiled for OS2, running on OS3
  case 2:    // compiled for OS3, running on OS2
    StrPrintF(buf, "This PRC was compiled for OS %d.x", snowcrash+1);
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy;
    StrPrintF(buf, "It will not work on your %d.x handheld.", 4-snowcrash);
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy+3;
    StrPrintF(buf, "You need a different PRC:", snowcrash+1);
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy;
    // The OS 2 build is a CFLAGS switch in the Makefile; it is still
    // named kMoria.prc (there is no kMoriaOS2.prc target any more).
    StrPrintF(buf, "Build %s\"kMoria.prc\" from",
	      ((snowcrash==2)?"an OS 2 ":""));
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy;
    StrPrintF(buf, "github.com/z88kat/palm-roguelike");
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy;
    StrPrintF(buf, "and install it with \"kMoriaDB.pdb\".");
    WinDrawChars(buf, StrLen(buf), 0, y);
    break;
  case 3:  // missing database
    StrPrintF(buf, "kMoria needs its database.");
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy;
    StrPrintF(buf, "Please install \"kMoriaDB.pdb\"");
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy+3;
    StrPrintF(buf, "If you don't have this file, get it");
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy;
    StrPrintF(buf, "from kMoria/src at");
    WinDrawChars(buf, StrLen(buf), 0, y);
    y += dy;
    StrPrintF(buf, "github.com/z88kat/palm-roguelike");
    WinDrawChars(buf, StrLen(buf), 0, y);
    break;
  }

}
