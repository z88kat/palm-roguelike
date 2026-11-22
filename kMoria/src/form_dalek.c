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

/***********************************************************************
 * This form allows the player to select a letter to genocide...
 *   abcdefghijklmnopqrstuvwxyz   no u, v, x.
 *   ....................  . ..
 *   ABCDEFGHIJKLMNOPQRSTUVWXYZ,$   no I, N, Z.
 *   ........ .... ........... ..
 *
 * abcdefghijklmnopqrstwyz,$ABCDEFGHJKLMOPQRSTUVWXY
 *
 ***********************************************************************/
static Char dalek_c[48] = "abcdefghijklmnopqrstwyz,ABCDEFGHJKLMOPQRSTUVWXY$";
#define MCOLS 8
#define MROWS 6
Boolean Dalek_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short x, y, k;
  Short dx = SCR_WIDTH/MCOLS, dy = SCR_HEIGHT/MCOLS; // 20, for 160.    

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    dalek_draw();
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case penDownEvent:
    x = e->screenX / dx;
    y = (e->screenY - ROW_1) / dy;
    LeaveForm();
    if (x >= 0 && y >= 0 && x < MCOLS && y < MROWS) {
      // Translate the cell into dalek_c index.
      k = y * MCOLS + x;
      // call genocide with appropriate character.
      genocide(dalek_c[k]);
    }
    take_a_turn(true);
    handled = true;
    break;

  default:
    break;
  }
  return handled;
}

void dalek_draw()
{
  Short k, x, y, dx = SCR_WIDTH/MCOLS, dy = SCR_HEIGHT/MCOLS;//both 20, for 160

  for (y = ROW_1, k = 0 ; y < ROW_1 + MROWS*dy ; y += dy) {
    WinDrawLine(0, y, SCR_WIDTH, y);
    for (x = 0 ; x < SCR_WIDTH ; x += dx, k++)
      WinDrawChars_ctr(dalek_c+k, 1, x+dx/2, // x+10-4 for 160
		       y+(dy-visible_char_h)/2); // y+10-5 for 160.
  }
  for (x = dx ; x < SCR_WIDTH ; x += dx)
    WinDrawLine(x, ROW_1, x, ROW_1+MROWS*dy);

  WinDrawLine(0, ROW_1+MROWS*dy, SCR_WIDTH, ROW_1+MROWS*dy);
  WinDrawChars_ctr("None", 4, 
		   SCR_WIDTH/2, // 69 in 160
		   ROW_1+MROWS*dy+LINE_HEIGHT-1);//15+120+10 in 160

}

/*

abcdefgh
ijklmnop
qrstwyz,
ABCDEFGH
JKLMOPQR
STUVWXY$
 -None-
*/
