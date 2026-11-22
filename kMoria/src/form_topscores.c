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


static void draw_topscores_header() SEC_6;

extern Boolean came_from_generation;
extern Long death_date;
Boolean Scores_Form_HandleEvent(EventPtr e)
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
    if (!death) {
      ControlPtr btn;
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_scores_ok));
      CtlSetLabel(btn, "- Done -");
      CtlShowControl(btn);
    }
    draw_topscores(true);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case keyDownEvent:
    // hardware button -- or else graffiti.
    switch(e->data.keyDown.chr) {
    case pageUpChr: case '<':
      draw_topscores(true);
      handled = true;
      break;
    case pageDownChr: case '>': case ' ':
      draw_topscores(false);
      handled = true;
      break;
    case '\n': // should be the same as if btn_scores_ok
      LeaveForm();
      if (death) FrmGotoForm(CharRaceForm);
      // else we're still alive and were viewing a popup on top of MainForm!
      handled = true;
      break;
    default:
      break;
    }
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_scores_ok:
      LeaveForm();
      if (death) FrmGotoForm(CharRaceForm);
      // else we're still alive and were viewing a popup on top of MainForm!
      handled = true;
      break;
    case btn_scores_next:
      draw_topscores(false);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}


// was:  Rank Points Name Sex Race Class Lvl Killed By
// now:  Rank Score [Sex] Race Class Level
//        [Name]
//         [Killed by]
extern high_scores score;


static void draw_topscores_header()
{
  Char buf[80];
  Short x, y;
  RectangleType r;
  FormPtr frm = FrmGetActiveForm();
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_scores_ok), &r);
  x = r.topLeft.y - 1; // 146
  RctSetRectangle(&r, 0, ROW_1, SCR_WIDTH, x-ROW_1); // 0, 15, 160, 146-15
  WinEraseRectangle(&r, 0);

  ///////////////////////////////////////////////////////////////
  //     v---right justified    v---right justified
  // 0  +17  +42  +10+42  +32  +15
  y = ROW_1; // "Rank"=20 "Points"=25 "S."=7 "Race"=19 "Class"=20 "Level"=22
  x = 0;
  StrPrintF(buf, "Rank"); // left-justify // width: 20
  WinDrawChars(buf, StrLen(buf), x, y); // 1234567 might be enough for points.
  x += FntCharsWidth("000 12345678 ", 13);// width of rank+points = 17 + 42.
  StrPrintF(buf, "Points "); // right-justify above points column
  WinDrawChars_rj(buf, StrLen(buf), x, y);
  StrPrintF(buf, " S."); // center over gender
  WinDrawChars(buf, StrLen(buf), x, y);//S looks better sorta centered
  x += FntCharsWidth("M ",2); // width of "M " = 10
  StrPrintF(buf, "Race"); // left-justify
  WinDrawChars(buf, StrLen(buf), x, y);
  x += FntCharsWidth("Half-Troll ", 11);//Half-Troll=42;
  StrPrintF(buf, "Class"); // left-justify
  WinDrawChars(buf, StrLen(buf), x, y);
  x = SCR_WIDTH;
  StrPrintF(buf, "Level"); // right-justify!
  WinDrawChars_rj(buf, StrLen(buf), x, y);
  WinDrawLine(0, y+LINE_HEIGHT-1, SCR_WIDTH, y+LINE_HEIGHT-1);
}

// Revised for multi-record score database
// Returns index of the next one to draw..
Short draw_topscores(Boolean reset)
{
  static Short i = 0;
  VoidHand vh;
  VoidPtr p = NULL;
  Short scores_ctr, x_0 = 3*FntCharWidth(' '); // 6
  high_scores *score;
  //
  Char buf[80];
  UChar prace, pclass;
  Short x, y, y_max, diedlen = FntCharsWidth(" died: ",7);
  Boolean terse;
  //
  RectangleType r;
  FormPtr frm = FrmGetActiveForm();
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_scores_ok), &r);
  y_max = r.topLeft.y; // 147
  //
  scores_ctr = DmNumRecords(MoriaScoreDB);
  // no need for a version info record, I think I can put it in db header.
  if (scores_ctr <= 0) {
    WinDrawChars("None", 4, ROW_1+LINE_HEIGHT/2, ROW_1+LINE_HEIGHT/2);
    return -1;
  }
  draw_topscores_header();
  StrPrintF(buf, "n/%d  ", scores_ctr);
  WinDrawChars_rj(buf, StrLen(buf), SCR_WIDTH, SCR_HEIGHT-(LINE_HEIGHT+1));
  y = ROW_1 + LINE_HEIGHT+1;
  x = 0;
  if (reset || (i >= scores_ctr)) i = 0;
  for ( ; i < scores_ctr ; i++) {
    vh = DmQueryRecord(MoriaScoreDB, i);
    p = MemHandleLock(vh);
    score = (high_scores *) p;
    terse = false;
    if (x_0 + diedlen
	+ FntCharsWidth(score->died_from, StrLen(score->died_from))
	+ FntCharsWidth(score->name, StrLen(score->name)) < SCR_WIDTH)
      terse = true; // add "died: " line 2, or "killed by: " line3.
    // Check whether we will have enough room.
    // We will take 2 lines if terse, 3 lines if !terse.  Button at y_max.
    if ((y + (terse ? 2 : 3)*LINE_HEIGHT) > y_max) {
      MemHandleUnlock(vh);
      break; // when we go to the next page, start with *this* value of i..
    }
    // Ok, we have enough space to print this record.
    // Line 1: Numbers
    // Skip to line 2: Name
    y += LINE_HEIGHT-1;
    x = x_0;
    WinDrawChars(score->name, StrLen(score->name), x, y);
    if (terse) {
      StrPrintF(buf, "died: %s", score->died_from);
      WinDrawChars_rj(buf, StrLen(buf), SCR_WIDTH, y);
    }
    y -= LINE_HEIGHT-1;
    // Skip back to line 1
    x = 0;
    StrPrintF(buf, "%d", i+1); // Rank
    if (score->source == MAX_UCHAR) // we got this from a beam!
      WinDrawInvertedChars(buf, StrLen(buf), x, y);
    else
      WinDrawChars(buf, StrLen(buf), x, y);
    x += FntCharsWidth("000 12345678 ", 13);
    StrPrintF(buf, "%ld ", score->points); // Score
    WinDrawChars_rj(buf, StrLen(buf), x, y);
    buf[0] = ((score->race_class_sex & 0x1) ? 'M' : 'F'); // Gender
    WinDrawChars(buf, 1, x, y);
    x += FntCharsWidth("M ",2); // 8+2 = 10
    prace = (score->race_class_sex >> 4) & 0x7;;
    StrPrintF(buf, "%s", race[prace].trace); // Race
    WinDrawChars(buf, StrLen(buf), x, y);
    x += FntCharsWidth("Half-Troll ", 11);//Half-Troll=42;
    pclass = (score->race_class_sex >> 1) & 0x7;
    StrPrintF(buf, "%s", class[pclass].title); // Class  
    WinDrawChars(buf, StrLen(buf), x, y);
    x = SCR_WIDTH; // for right-justified column.  at right edge of screen.
    StrPrintF(buf, "%d", score->lev); // Level (experience, not dungeon)
    WinDrawChars_rj(buf, StrLen(buf), x, y);
    // Skip over line 2:
    y += LINE_HEIGHT-1;
    // Line 3: COD
    if (!terse) {
      y += LINE_HEIGHT;
      x = x_0 * 2;
      StrPrintF(buf, "killed by: %s", score->died_from);
      WinDrawChars(buf, StrLen(buf), x, y);
    }
    WinDrawGrayLine(0, y+LINE_HEIGHT, SCR_WIDTH, y+LINE_HEIGHT);  
    y += LINE_HEIGHT+1;
    MemHandleUnlock(vh);
  }
  return i;
}
