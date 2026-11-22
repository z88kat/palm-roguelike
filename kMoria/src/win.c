/*********************************************************************
 * kMoria - Moria adapted for the PalmPilot.                         *
 * This file is copyright (C) 2001 Bridget Spitznagel                *
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


void qWinEraseRectangle(RectanglePtr r, Word cornerDiam)
{
  if (my_prefs.black_bg)
    WinDrawRectangle(r, cornerDiam);
  else
    WinEraseRectangle(r, cornerDiam);
}
void qWinDrawLine(short x1, short y1, short x2, short y2)
{
  if (my_prefs.black_bg)
    WinEraseLine(x1, y1, x2, y2);
  else
    WinDrawLine(x1, y1, x2, y2);
}
void qWinEraseLine(short x1, short y1, short x2, short y2)
{
  if (my_prefs.black_bg)
    WinDrawLine(x1, y1, x2, y2);
  else
    WinEraseLine(x1, y1, x2, y2);
}
void qWinDrawChars(CharPtr chars, Word len, SWord x, SWord y)
{
  if (my_prefs.black_bg)
    WinDrawInvertedChars(chars, len, x, y);
  else
    WinDrawChars(chars, len, x, y);
}
void qWinDrawInvertedChars(CharPtr chars, Word len, SWord x, SWord y)
{
  if (my_prefs.black_bg)
    WinDrawChars(chars, len, x, y);
  else
    WinDrawInvertedChars(chars, len, x, y);
}


// Draw text centered around x_mid, at y.
void WinDrawChars_ctr(Char *buf, Short len, Short x_mid, Short y)
{
  Short x, width;
  width = FntCharsWidth(buf, len);
  x = x_mid - width / 2;
  if (x < 0) x = 0;
  WinDrawChars(buf, len, x, y);
}

// Draw text right-justified to x_right, at y.
void WinDrawChars_rj(Char *buf, Short len, Short x_right, Short y)
{
  Short x, width;
  width = FntCharsWidth(buf, len);
  x = x_right - width;
  if (x < 0) x = 0;
  WinDrawChars(buf, len, x, y);
}
