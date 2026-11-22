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

Boolean came_from_generation = true;

// Not yet done:
// Background
// Recording the selected name
// Access to the name-change form from the game
// Aborting character creation

// in create.c:
void reroll_char();
void post_reroll();
void other_stuff();

/*
 *  This is the form for Character Generation.
 *  I would like to keep the 'feel' similar to the original.
 *  At the bottom is a transcript of original generation.
 */


#define ROW_4_line (ROW_4-2)

static void print_dnd_stat(Char *buf, Short stat)
{
  if (stat < 19)
    StrPrintF(buf, "%d", stat);
  else {
    stat -= 18;
    if (stat > 100) stat = 0; // 18/00 == 18/100 in MY world.
    if (stat < 10)
      StrPrintF(buf, "18/0%d", stat);
    else 
      StrPrintF(buf, "18/%d", stat);
  }
}

void charrace_draw()
{
  Char buf[40];
  Short x, y;
  x = COL_1;
  y = ROW_1;
  StrPrintF(buf, "Name:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Race:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Sex:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Class:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  WinDrawLine(0, y, SCR_WIDTH, y);
  y += 2;  
  StrPrintF(buf, "Please select a race:");
  WinDrawChars(buf, StrLen(buf), x, y);
}

/* STEP 1: Select a race. */
Boolean CharRace_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr pushbtn;
  Short i, tentative_race;
  Char c;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) {
      // Forcibly UNrotate the screen, for now.
      VgaSetScreenMode(screenMode1To1, rotateModeNone);
      PrvMoveForm(false);
    }
#endif
    FrmDrawForm(frm);
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_race_0));
    CtlSetValue(pushbtn, true);
    charrace_draw();
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

    // Keyboard support.  woo hoo.
  case keyDownEvent:
    frm = FrmGetActiveForm();
    c = e->data.keyDown.chr;
    if (c == ' ') {
      tentative_race = 0;
      for (i = 0 ; i <= 7 ; i++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i+pbtn_race_0));
	if (CtlGetValue(pushbtn)) tentative_race = i;
      	CtlSetValue(pushbtn, false);
      }
      i = (tentative_race + 1) % 8;
      pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i+pbtn_race_0));
      CtlSetValue(pushbtn, true);
    } else if (c == '\n') {
      hit_button_if_usable(frm, btn_race_ok);
      handled = true;
    }
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_race_ok:
      frm = FrmGetActiveForm();
      tentative_race = -1;
      for (i = pbtn_race_0 ; i <= pbtn_race_7 ; i++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
	if (CtlGetValue(pushbtn))
	  tentative_race = i - pbtn_race_0;
      }
      if (tentative_race != -1) {
	pymisc.prace = tentative_race;
	FrmGotoForm(CharGenderForm);
      }
      // else beep or something.
      handled = true;
      break;
    case pbtn_race_0:
    case pbtn_race_1:
    case pbtn_race_2:
    case pbtn_race_3:
    case pbtn_race_4:
    case pbtn_race_5:
    case pbtn_race_6:
    case pbtn_race_7:
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent

  default:
    break;
  }

  return handled;
}

void chargender_draw()
{
  Char buf[40];
  Short x, y;
  x = COL_1;
  y = ROW_1;
  StrPrintF(buf, "Name:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Race: %s", race[pymisc.prace].trace);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Sex:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Class:");
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  WinDrawLine(0, y, SCR_WIDTH, y);
  y += 2;  
  StrPrintF(buf, "Please select a gender:");
  WinDrawChars(buf, StrLen(buf), x, y);
}


/* STEP 2: Select a gender. */
Boolean CharGender_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr pushbtn;
  Short i, tentative_gender;
  Boolean is_m, toggle = false;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_gender_0));
    CtlSetValue(pushbtn, true);
    chargender_draw();
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

    // Keyboard support.  woo hoo.
  case keyDownEvent:
    frm = FrmGetActiveForm();
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_gender_0));
    is_m = CtlGetValue(pushbtn);
    switch(e->data.keyDown.chr) {
    case ' ':           toggle =  true; handled = true; break;
    case 'm': case 'M': toggle = !is_m; handled = true; break;
    case 'f': case 'F': toggle =  is_m; handled = true; break;
    case '\n': // same as 'ok' button
      hit_button_if_usable(frm, btn_gender_ok);
      handled = true;
      break;
    case 'Q': // same as 'no' button
      FrmGotoForm(CharRaceForm);
      handled = true;
      break;
    }
    if (toggle) {
      pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_gender_0));
      CtlSetValue(pushbtn, !is_m);
      pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_gender_1));
      CtlSetValue(pushbtn, is_m);
    }
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_gender_no:
      FrmGotoForm(CharRaceForm);
      handled = true;
      break;
    case btn_gender_ok:
      frm = FrmGetActiveForm();
      tentative_gender = -1;
      for (i = pbtn_gender_0 ; i <= pbtn_gender_1 ; i++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
	if (CtlGetValue(pushbtn))
	  tentative_gender = i - pbtn_gender_0;
      }
      if (tentative_gender != -1) {
	pymisc.male = (tentative_gender==0);
	FrmGotoForm(CharClassForm);
      }
      // else beep or something.
      handled = true;
      break;
    case pbtn_gender_0:
    case pbtn_gender_1:
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent

  default:
    break;
  }

  return handled;
}

void charclass_draw(Boolean all, Boolean squeeze)
{
  Char buf[40];
  Short x, y, i, five = FntCharWidth('0'); // five == width of a numeral
  RectangleType r;
  x = COL_1;
  y = ROW_1;
  if (all) {
    if (!squeeze) {
      StrPrintF(buf, "Name:");
      WinDrawChars(buf, StrLen(buf), x, y);
      y += LINE_HEIGHT;
    }
    StrPrintF(buf, "Race: %s", race[pymisc.prace].trace);
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Sex: %s", pymisc.male?"Male":"Female");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Class:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
  } else {
    y += (squeeze ? 3 : 4) * LINE_HEIGHT;
    RctSetRectangle(&r, x, y, COL_4-x, 3*LINE_HEIGHT);
    WinEraseRectangle(&r, 0);
  }
  StrPrintF(buf, "Social Class: %d", pymisc.sc);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "+ To Hit: %d", pymisc.dis_th);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "+ To AC: %d", pymisc.dis_tac);
  WinDrawChars(buf, StrLen(buf), x, y);
  // end of first column
  x = COL_2;
  y = squeeze ? ROW_3 - LINE_HEIGHT : ROW_3;
  StrPrintF(buf, "To Damage: %d", pymisc.dis_td);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Total AC: %d", pymisc.dis_ac);
  WinDrawChars(buf, StrLen(buf), x, y);
  // end of second column
  if (all) {
    x = COL_3;
    y = squeeze ? ROW_1 : ROW_2;
    StrPrintF(buf, "Age:", pymisc.age);
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Height:", pymisc.ht);
    WinDrawChars(buf, StrLen(buf), x, y); // Hey!  I should have a METRIC pref!
    y += LINE_HEIGHT;
    StrPrintF(buf, "Weight:", pymisc.wt);
    WinDrawChars(buf, StrLen(buf), x, y);
  }
  x = COL_3a;
  y = squeeze ? ROW_1 : ROW_2;
  if (!all) {
    RctSetRectangle(&r, x, y, COL_4-x, 3*LINE_HEIGHT);
    WinEraseRectangle(&r, 0);
  }
  StrPrintF(buf, "%d", pymisc.age);
  WinDrawChars(buf, StrLen(buf), pymisc.age<100?x+five:x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "%d", pymisc.ht); // Hey!  I should have a UNITS preference!
  WinDrawChars(buf, StrLen(buf), pymisc.ht<100?x+five:x, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "%d", pymisc.wt);
  WinDrawChars(buf, StrLen(buf), pymisc.wt<100?x+five:x, y);

  // end of third column
  if (all) {
    x = COL_4;
    y = squeeze ? ROW_1 : ROW_2;
    // Hey, I should make this smarter for !came_from_generation,
    // so that it reverse-videos this string iff you need a restore-stat
    // potion... hmmm...  see below.  (real moria prints out your max
    // stat and your cur stat, if different, but we've no ROOM for 18/xx)
    StrPrintF(buf, "Str:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Int:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Wis:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Dex:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Con:");
    WinDrawChars(buf, StrLen(buf), x, y);
    y += LINE_HEIGHT;
    StrPrintF(buf, "Chr:");
    WinDrawChars(buf, StrLen(buf), x, y);
  }
  //
  x = COL_5;
  y = squeeze ? ROW_1 : ROW_2;
  if (!all) {
    RctSetRectangle(&r, x, y, SCR_WIDTH-x, 6*LINE_HEIGHT - 1);
    WinEraseRectangle(&r, 0);
  }
  for (i = 0 ; i < 6 ; i++) {
    // hey what about 18/00
    print_dnd_stat(buf, pystats.max_stat[i]);
    WinDrawChars(buf, StrLen(buf), pystats.max_stat[i]<10?x+five:x, y);
    // ok here is the inverting smarts:
    // invert the rectangle from COL_4 to COL_5
    if ((pystats.max_stat[i] - pystats.cur_stat[i] > 0)
	&& !came_from_generation) {
      RctSetRectangle(&r, COL_4-1, y, COL_5-COL_4, LINE_HEIGHT);
      WinInvertRectangle(&r, 0);
    }
    y += LINE_HEIGHT;
  }
  //  WinDrawLine(0, y, 160, y);
}

static void charclass_draw_bg(Boolean bg_only)
{
  // [Calculate and print] the background information
  Short x, y, btn_top;
  RectangleType r;
  FormPtr frm = FrmGetActiveForm();
  if (bg_only) {
    x = COL_1;
    y = ROW_4-3-LINE_HEIGHT;
  } else {
    x = COL_1;
    y = ROW_1;
  }
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_class_ok), &r);
  btn_top = r.topLeft.y - 1; // 146
  RctSetRectangle(&r, x, y, SCR_WIDTH-x, btn_top - y);
  WinEraseRectangle(&r, 0);
}

/* STEP 3: Select a class. */
Boolean chose_class;
Boolean CharClass_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr pushbtn, btn;
  Short i, tentative_class;
  Boolean foo=false;
  Char c;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    // Also, print their race, gender, and rolled stats:
    reroll_char();
    charclass_draw(true, true);
    chose_class = false;
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

    // Keyboard support.  woo hoo.  this one is ugly, sorry.
  case keyDownEvent:
    c = e->data.keyDown.chr;
    frm = FrmGetActiveForm();
    if (c == ' ' && chose_class) {
      // play with pushbuttons
      tentative_class = 0;
      for (i = 0 ; i <= 5 ; i++) {
	pushbtn=FrmGetObjectPtr(frm,FrmGetObjectIndex(frm,i+pbtn_class_0));
	if (CtlGetValue(pushbtn)) tentative_class = i;
	CtlSetValue(pushbtn, false);
      }
      i = tentative_class;
      do { i = (i + 1) % 6;
      } while (!(1<<i & race[pymisc.prace].rtclass));
      pushbtn = FrmGetObjectPtr(frm,FrmGetObjectIndex(frm,i+pbtn_class_0));
      CtlSetValue(pushbtn, true);
      handled = true;
      break;
    } else {
      i = 0;
      switch(c) {
      case ' ':
	if (!chose_class && !i) i = btn_class_reroll;
	// FALL THROUGH
      case 'Q':
	if (chose_class && !i) i = btn_class_no;
	// FALL THROUGH
      case '\n': // accept
	if (!i) i = (!chose_class ? btn_class_ok : btn_class_ok2);
	hit_button_if_usable(frm, i);
	handled = true;
	break;
      } 
    }
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_class_no:
      FrmGotoForm(CharRaceForm);
      handled = true;
      break;
    case btn_class_reroll:
      // call again whatever routine we called to roll & print stats:
      charclass_draw_bg(true);
      reroll_char();
      charclass_draw(false, true);
      handled = true;
      break;
    case btn_class_ok:
      chose_class = true;
      frm = FrmGetActiveForm();
      charclass_draw_bg(false);
      charclass_draw(true, false);
      WinDrawLine(0, ROW_4_line, SCR_WIDTH, ROW_4_line);
      // enable class buttons based on character race
      for (i = 0 ; i < MAX_CLASS ; i++) {
	if (1<<i & race[pymisc.prace].rtclass) {
	  pushbtn = FrmGetObjectPtr(frm,
				    FrmGetObjectIndex(frm, pbtn_class_0+i));
	  CtlShowControl(pushbtn);
	  if (!foo) CtlSetValue(pushbtn, true);
	  foo = true;
	}
      }
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_ok));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_reroll));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_ok2));
      CtlShowControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_class_no));
      CtlShowControl(btn);
      FrmCopyTitle(frm, "Now pick a class");
      handled = true;
      break;
    case btn_class_ok2:
      frm = FrmGetActiveForm();
      tentative_class = -1;
      for (i = pbtn_class_0 ; i <= pbtn_class_5 ; i++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
	if (CtlGetValue(pushbtn))
	  tentative_class = i - pbtn_class_0;
      }
      if (tentative_class != -1) {
	pymisc.pclass = tentative_class;
	post_reroll();
	FrmGotoForm(CharSummaryForm);
      }
      // else beep or something.
      handled = true;
      break;
    case pbtn_class_0:
    case pbtn_class_1:
    case pbtn_class_2:
    case pbtn_class_3:
    case pbtn_class_4:
    case pbtn_class_5:
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent

  default:
    break;
  }

  return handled;
}

// Note: I assume a maximum length of 7 digits (decimal) for experience,gold.
// These two magic numbers happen to be the same:
#define FNORD 47 /* xxx in 160 world */
#define QUUX 47 /* xxx in 160 world */
static void charsummary_draw_a()
{
  Char buf[40];
  Short x, y, five = FntCharWidth('0'); // five == width of a numeral (0-9)

  x = COL_1;
  y = ROW_4;

  StrPrintF(buf, "Level:");
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.lev);
  WinDrawChars(buf, StrLen(buf), x+FNORD+five*(7-StrLen(buf)), y);//R-justify!
  y += LINE_HEIGHT;
  StrPrintF(buf, "Experience:");
  WinDrawChars(buf, StrLen(buf), x, y);
  //  pymisc.exp = 1234567L;
  StrPrintF(buf, "%ld", pymisc.exp);
  WinDrawChars(buf, StrLen(buf), x+FNORD+five*(7-StrLen(buf)), y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Max Exp:");
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%ld", pymisc.max_exp);
  WinDrawChars(buf, StrLen(buf), x+FNORD+five*(7-StrLen(buf)), y);
  y += LINE_HEIGHT;
  if (pymisc.lev < MAX_PLAYER_LEVEL && pymisc.lev > 0) {
    StrPrintF(buf, "Exp to Adv.:");
    WinDrawChars(buf, StrLen(buf), x, y);
    // xxx this might be bogus, check for division truncation problems
    StrPrintF(buf, "%ld", (Long) (player_exp[pymisc.lev-1]
				  * pymisc.expfact / 100));
    WinDrawChars(buf, StrLen(buf), x+FNORD+five*(7-StrLen(buf)), y);
  }
  // end of first column
  x = COL_31;
  y = ROW_4;
  StrPrintF(buf, "Max HP:", pymisc.mhp);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.mhp);
  WinDrawChars(buf, StrLen(buf), x+QUUX+five*(4-StrLen(buf)), y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Cur HP:", pymisc.chp);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.chp);
  WinDrawChars(buf, StrLen(buf), x+QUUX+five*(4-StrLen(buf)), y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Max Mana:", pymisc.mana);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.mana);
  WinDrawChars(buf, StrLen(buf), x+QUUX+five*(4-StrLen(buf)), y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Cur Mana:", pymisc.cmana);
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%d", pymisc.cmana);
  WinDrawChars(buf, StrLen(buf), x+QUUX+five*(4-StrLen(buf)), y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Gold:");
  WinDrawChars(buf, StrLen(buf), x, y);
  StrPrintF(buf, "%ld", pymisc.au);
  WinDrawChars(buf, StrLen(buf), x+(QUUX-3*five)+five*(7-StrLen(buf)), y);
}
#undef FNORD
#undef QUUX
// was in misc3
static Char *likert(Short x, Short y)
{
  // I hope this still DTRT
  switch((x/y)) {
  case -3: case -2: case -1: return("v.bad");
  case 0: case 1:            return("bad");
  case 2:                    return("poor");
  case 3: case 4:            return("fair");
  case  5:                   return("good");
  case 6:                    return("v.good");
  case 7: case 8:            return("excell.");
  default:                   return("superb");
  }
}

static void charsummary_draw_b()
{
  Char buf[40], *val;
  Short x, y, x2;
  Short xbth, xbthb, xfos, xsrh, xstl, xdis, xsave, xdev;

  // Calculate the miscellaneous abilities;
  // this replaces calc_misc_abilities()    in misc3.c
  xbth  = pymisc.bth + pymisc.ptohit*BTH_PLUS_ADJ
    + (class_level_adj[pymisc.pclass][CLA_BTH] * pymisc.lev);
  xbthb = pymisc.bthb + pymisc.ptohit*BTH_PLUS_ADJ
    + (class_level_adj[pymisc.pclass][CLA_BTHB] * pymisc.lev);
  /* this results in a range from 0 to 29 */
  xfos  = 40 - pymisc.fos;
  if (xfos < 0)  xfos = 0;
  xsrh  = pymisc.srh;
  /* this results in a range from 0 to 9 */
  xstl  = pymisc.stl + 1;
  xdis  = pymisc.disarm + 2*todis_adj() + stat_adj(A_INT)
    + (class_level_adj[pymisc.pclass][CLA_DISARM] * pymisc.lev / 3);
  xsave = pymisc.save + stat_adj(A_WIS)
    + (class_level_adj[pymisc.pclass][CLA_SAVE] * pymisc.lev / 3);
  xdev  = pymisc.save + stat_adj(A_INT)
    + (class_level_adj[pymisc.pclass][CLA_DEVICE] * pymisc.lev / 3);
  // done calculating, now print stuff

  x = COL_1;
  x2 = x + FntCharsWidth("Saving Throw ", 13) - 1;//56 for 160. need -1 to fit.
  y = ROW_4;
  StrPrintF(buf, "Fighting");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xbth, 12); // fighting
  WinDrawChars(val, StrLen(val), x2, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Bows/Throw");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xbthb,12); // bows/throw
  WinDrawChars(val, StrLen(val), x2, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Magic Device");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xdev, 6); // magic device
  WinDrawChars(val, StrLen(val), x2, y);
  y += LINE_HEIGHT;
  StrPrintF(buf, "Saving Throw");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xsave, 6); // saving throw
  WinDrawChars(val, StrLen(val), x2, y);
  x = COL_31;
  x2 = x + FntCharsWidth("Perception ", 11); // 45 for 160. need -1 to fit.
  y = ROW_4;
  StrPrintF(buf, "Disarming");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xdis, 8); // disarming
  WinDrawChars(val, StrLen(val), x2, y);
  y += LINE_HEIGHT;  
  StrPrintF(buf, "Stealth");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xstl, 1); // stealth
  WinDrawChars(val, StrLen(val), x2, y);
  y += LINE_HEIGHT;  
  StrPrintF(buf, "Searching");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xsrh, 6); // searching
  WinDrawChars(val, StrLen(val), x2, y);
  y += LINE_HEIGHT;  
  StrPrintF(buf, "Perception");
  WinDrawChars(buf, StrLen(buf), x, y);
  val = likert(xfos, 3); // perception
  WinDrawChars(val, StrLen(val), x2, y);
  y += LINE_HEIGHT;  
  StrPrintF(buf, "Infravision");
  WinDrawChars(buf, StrLen(buf), x, y);
  // xxx Hey, when I have a metric preference option, don't forget infravision
  StrPrintF(buf, "%d'", pyflags.see_infra*10); // infravision
  WinDrawChars(buf, StrLen(buf), x2, y);
}
Boolean charsum_is_a;
void charsummary_draw()
{
  Char buf[40];
  Short x, y;

  charclass_draw(true, false);

  x = COL_1;
  y = ROW_1 + 3*LINE_HEIGHT;
  StrPrintF(buf, "Class: %s", class[pymisc.pclass].title);
  WinDrawChars(buf, StrLen(buf), x, y);
  charsummary_draw_a();
  charsum_is_a = true;

  WinDrawLine(0, ROW_4_line, SCR_WIDTH, ROW_4_line);
}
static void charsummary_draw_swapbot()
{
  FormPtr frm = FrmGetActiveForm();
  ControlPtr btn;
  RectangleType r;
  Short x, y, btn_top;
  x = 0; y = ROW_4;

  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_charsum_ok), &r);
  btn_top = r.topLeft.y - 1; // 146
  RctSetRectangle(&r, x, y, COL_31-x, btn_top - y);
  WinEraseRectangle(&r, 0);
  x = COL_31;
  RctSetRectangle(&r, x, y, SCR_WIDTH-x, SCR_HEIGHT-y);
  WinEraseRectangle(&r, 0);
  if (charsum_is_a)
    charsummary_draw_b();
  else 
    charsummary_draw_a();
  charsum_is_a = !charsum_is_a;
  if (came_from_generation) {
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_charsum_no));
    CtlShowControl(btn);
  }
}

/* STEP 4: Pick a name. */
/* megaNOTE: This form must be re-visitable! somehow! */
static void charsum_init_name(FormPtr frm)
{
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_charsum));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (!vh) {
    vh = MemHandleNew(PLAYER_NAME_SIZE * sizeof(Char));
  }
  FldSetTextHandle(fld, (Handle) 0);
  p = MemHandleLock(vh);
  StrNCopy(p, pymisc.name, PLAYER_NAME_SIZE);
  p[PLAYER_NAME_SIZE-1] = '\0';
  MemHandleUnlock(vh);
  FldSetTextHandle(fld, (Handle) vh);
}
static void charsum_commit_name(FormPtr frm)
{
  FieldPtr fld;
  CharPtr p;
  VoidHand vh;
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_charsum));
  vh = (VoidHand) FldGetTextHandle(fld);
  if (vh) {
    p = MemHandleLock(vh);
    StrNCopy(pymisc.name, p, PLAYER_NAME_SIZE);
    MemHandleUnlock(vh);
    pymisc.name[PLAYER_NAME_SIZE-1] = '\0';
  }
  if (StrLen(pymisc.name) == 0)
    get_default_username();
}

void clear_message_log();
Boolean CharSummary_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
  FieldAttrType fieldattr;
  ControlPtr btn;
  // see Trans_Form_HandleEvent in ilarn/src/main.c
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    charsum_init_name(frm);
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    // Also, print all kinds of stuff!
    charsummary_draw();
    if (came_from_generation) {
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_charsum_no));
      CtlShowControl(btn);
    }
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

    // Keyboard support.  woo hoo.
  case keyDownEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_charsum));
    FldGetAttributes(fld, &fieldattr);
    if (fieldattr.hasFocus) {
      switch(e->data.keyDown.chr) {
      case '\n': // give up focus!
	FldReleaseFocus(fld);
	handled = true;
	break;
      }
    } else {
      switch(e->data.keyDown.chr) {
      case ' ': // page the bottom info
	charsummary_draw_swapbot();
	handled = true;
	break;
      case '\n': // same as 'ok' button
	hit_button_if_usable(frm, btn_charsum_ok);
	handled = true;
	break;
      case 'Q': // same as 'no' button
	if (came_from_generation)
	  FrmGotoForm(CharRaceForm);
	handled = true;
	break;
      }
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_charsum_no:
      FrmGotoForm(CharRaceForm);
      handled = true;
      break;
    case btn_charsum_ok:
      // Hey!  do something with field_charsum, which is the character name.
      charsum_commit_name(frm);
      if (came_from_generation) {
	clear_message_log();
	other_stuff();
	store_init(); // this used to be done for reloaded characters too.
	magic_init(); // ditto.  I think it was a "save the rnd seed" thing.
	death = false;
	FrmGotoForm(MainForm);
	//	show_messages();
      } else
	LeaveForm(); // reckon we are a popup..
      // came_from_generation = false; // popup caller should also have set it!
      handled = true;
      break;
    }
    break; // end of ctlSelectEvent


  case penDownEvent:
    frm = FrmGetActiveForm();
    if (e->screenY > ROW_1) {
      fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_charsum));
      FldReleaseFocus(fld); // for the sake of the keyboard ui
    }
    if (e->screenY > ROW_4) {
      RectangleType r;
      FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_charsum_ok), &r);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_charsum_no));
      if (e->screenY < r.topLeft.y ||
	  (e->screenX > COL_2 && (e->screenX < COL_4 || !btn->attr.usable))) {
	charsummary_draw_swapbot();
	handled = true;
	break;
      }
    }
    break;

  default:
    break;
  }

  return handled;
}

