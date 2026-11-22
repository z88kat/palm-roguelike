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

static void winnify_scores() SEC_5;
static void windraw_xcentered(Char *buf, Short, Short, Short *) SEC_5;
// [6][40]
static DmOpenRef NPOpenDB(Char *name) SEC_5;
static void prtstat(Short i, Char *stat) SEC_5;
static Char *likert(Short x, Short y) SEC_5;
static void snarf_history_block(Char *history_block) SEC_5;
static void death_record_memo() SEC_5;

static void init_highscore() SEC_6;
void insert_score(high_scores *score) SEC_6; // also used in form_beam.c

extern Boolean came_from_generation;
extern Char *player_title; // from lock.c
Long death_date;
extern FontID MyFont;

// does this reeeally need a whole form of its own.

Boolean Crown_Form_HandleEvent(EventPtr e)
{
  FormPtr frm;
  Boolean handled=false;
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    crown_draw();
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case penDownEvent:
    LeaveForm();
    handled = true;
    break;
  default:
    break;
  }
  return handled;
}
/*          #
          #####
            #
      ,,,  $$$  ,,,
  ,,=$   "$$$$$"   $=,,
,$$        $$$        $$,
*>         <*>         <*
$$         $$$         $$
"$$        $$$        $$"
 "$$       $$$       $$"
  *#########*#########*
  *#########*#########*    */
void crown_draw()
{
  /* Let the player know that he did good.	 */
  Short y = visible_char_h_itsy, dy = visible_char_h_itsy + 1, maxw = 0;
  Short cw = SCR_WIDTH-4; // 156
  // I need a tiny, fixed-width font...
#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  // XXXXX Even if we have handera we still want FIXED-WIDTH font.....
  FntSetFont(MyFont);
#endif
  windraw_xcentered("#", cw, y, &maxw);
  y += dy;
  windraw_xcentered("#####", cw, y, &maxw);
  y += dy;
  windraw_xcentered("#", cw, y, &maxw);
  y += dy;
  windraw_xcentered(",,,\t\t$$$\t\t,,,", cw, y, &maxw);
  y += dy;
  windraw_xcentered(",,=$\t\t\t\"$$$$$\"\t\t\t$=,,", cw, y, &maxw);
  y += dy;
  windraw_xcentered(",$$\t\t\t\t\t\t\t\t$$$\t\t\t\t\t\t\t\t$$,", cw, y, &maxw);
  y += dy;
  windraw_xcentered("*>\t\t\t\t\t\t\t\t\t<*>\t\t\t\t\t\t\t\t\t<*", cw, y, &maxw);
  y += dy;
  windraw_xcentered("$$\t\t\t\t\t\t\t\t\t$$$\t\t\t\t\t\t\t\t\t$$", cw, y, &maxw);
  y += dy;
  windraw_xcentered("\"$$\t\t\t\t\t\t\t\t$$$\t\t\t\t\t\t\t\t$$\"", cw, y, &maxw);
  y += dy;
  windraw_xcentered("\"$$\t\t\t\t\t\t\t$$$\t\t\t\t\t\t\t$$\"", cw, y, &maxw);
  y += dy;
  windraw_xcentered("*#########*#########*", cw, y, &maxw);
  y += dy;
  windraw_xcentered("*#########*#########*", cw, y, &maxw);
#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(BigFont);
#endif
  y += dy;
  dy = visible_char_h + 1;
  y += dy;
  windraw_xcentered("Veni, Vidi, Vici!", cw, y, &maxw);
  y += dy;
  windraw_xcentered("I came, I saw, I conquered!", cw, y, &maxw);
  y += dy;
  if (pymisc.male)
    windraw_xcentered("All Hail the Mighty King!", cw, y, &maxw);
  else
    windraw_xcentered("All Hail the Mighty Queen!", cw, y, &maxw);
  y += 2*dy;
  windraw_xcentered("(tap anywhere)", cw, y, &maxw);

}

static void winnify_scores()
{
  /* Change the character attributes.		 */
  dun_level = 0;
  StrCopy(died_from, "Ripe Old Age");
  restore_level(); // restores any drained experience
  pymisc.lev += MAX_PLAYER_LEVEL;
  pymisc.au += 250000L;
  pymisc.max_exp += 5000000L;
  pymisc.exp = pymisc.max_exp;
}

///////////////////////////////////////////////////////////////////

Boolean Death_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    death_date = TimGetSeconds(); // record when character is "pronounced dead"
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    // hm... if (total_winner), then first we should do kingly() . . .
    if (total_winner) winnify_scores();
    tombstone_draw();
    if (total_winner) FrmPopupForm(CrownForm);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_death_wok:
      death_record_memo();      
      // fall through
    case btn_death_ok:
      // note: at THIS point I should write the new scorefile record,
      // then I can have a global boolean checked in ScoresForm to say
      // how to behave on exit, and it will be like totally viewable in-game!
      {
	high_scores score;
	init_highscore(&score);
	insert_score(&score);
      }
      antisave_char(); // delete the save file.
      re_init();
      came_from_generation = true;
      LeaveForm();
      FrmGotoForm(ScoresForm);
      /* then scoresform will: FrmGotoForm(CharRaceForm); 
	 ...'death' will not be set false until character generation */
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}

void tombstone_draw()
{
  Char buf[80];
  Boolean total_winner = false;
  Short x, x2, dx, y, dy = visible_char_h + 1, maxw = 0, topw, midw;
  Short foo17, sw = SCR_WIDTH - 4; // 156

  y = LINE_HEIGHT+1 + dy;
  StrPrintF(buf, "%s", pymisc.name);
  windraw_xcentered(buf, sw, y, &maxw);
  topw = maxw; // remember width at top of tombstone
  maxw += LINE_HEIGHT+5;//16; // to make sure it's not just a boring rectangle

  y += dy;
  StrPrintF(buf, "the");
  windraw_xcentered(buf, sw, y, &maxw);

  y += dy;
  if (!total_winner) // from player.c
    //    StrPrintF(buf, "player_title[%d][%d]", pymisc.pclass, pymisc.lev-1);
    StrPrintF(buf, "%s", 
	      &player_title[( (pymisc.pclass*MAX_PLAYER_LEVEL)+(pymisc.lev-1) )
			   * MAXLEN_PTITLE]);
  else
    StrPrintF(buf, "Magnificent");
  windraw_xcentered(buf, sw, y, &maxw);
  midw = maxw; // remember width at ~middle of tombstone

  y += dy;
  y += dy;
  if (!total_winner)
    StrPrintF(buf, "%s, Level %d", class[pymisc.pclass].title, pymisc.lev);
  else
    StrPrintF(buf, "%s, Level %d", ((pymisc.male)?"King":"Queen"), pymisc.lev);
  windraw_xcentered(buf, sw, y, &maxw);

  y += dy;
  StrPrintF(buf, "%ld Exp %ld Au", pymisc.exp, pymisc.au);
  windraw_xcentered(buf, sw, y, &maxw);

  y += dy;
  StrPrintF(buf, "Died on Level: %d", dun_level);
  windraw_xcentered(buf, sw, y, &maxw);

  y += dy;
  StrPrintF(buf, "killed by");
  windraw_xcentered(buf, sw, y, &maxw);

  y += dy;
  StrPrintF(buf, "%s.", died_from); // "an Ancient Multi-Hued Dragon" = widest
  windraw_xcentered(buf, sw, y, &maxw);

  y += dy;
  {
    DateTimeType datetime;
    Char datestr[dateStringLength];
    TimSecondsToDateTime(death_date, &datetime); // see DateTime.h
    DateToAscii(datetime.month, datetime.day, datetime.year,
		PrefGetPreference(prefDateFormat), datestr);
    StrPrintF(buf, "%s", datestr);
  }
  windraw_xcentered(buf, sw, y, &maxw);
  // maxw is the max width of the tombstone

  // Now!  Draw a tombstone sort of shape around it.  If possible.
  // Draw a line from (bottom, sw/2 +- maxw/2) to (mid, same x).
  y += dy;
  x = sw/2 - topw/2;
  foo17 = (3*LINE_HEIGHT+1)/2;
  WinDrawLine(x, foo17, x+topw, foo17);// top horizontal of tombstone.

  dx = (maxw/2 + topw/2); // find the mean
  if (midw > dx) dx = midw;
  // left upper diagonal
  x = sw/2 - topw/2;
  x2 = sw/2 - dx/2;
  if (x2 < 0) x2 = 0;
  WinDrawLine(x, foo17, x2, foo17+LINE_HEIGHT);
  // left middle diagonal
  x = x2;
  x2 = sw/2 - (maxw/2 + 3);
  if (x2 < 0) x2 = 0;
  WinDrawLine(x, foo17+LINE_HEIGHT, x2, y-6*LINE_HEIGHT);
  // left lower vertical
  x = x2;
  WinDrawLine(x, y, x, y-6*LINE_HEIGHT);
  // now the right side...
  // right upper diagonal
  x = sw/2 + topw/2;
  x2 = sw/2 + dx/2;
  if (x2 > sw) x2 = sw;
  WinDrawLine(x, foo17, x2, foo17+LINE_HEIGHT);
  // right middle diagonal
  x = x2;
  x2 = sw/2 + (maxw/2 + 3);
  if (x2 > sw) x2 = sw;
  WinDrawLine(x, foo17+LINE_HEIGHT, x2, y-6*LINE_HEIGHT);
  // right lower vertical
  x = x2;
  WinDrawLine(x, y, x, y-6*LINE_HEIGHT);

}

// buf must be null-terminated string.  s_width is 160 or less pixels.
// y is the y location to draw it at.  w is for remembering the max width.
void windraw_xcentered(Char *buf, Short s_width, Short y, Short *w)
{
  Short x, width, len;
  len = StrLen(buf);
  width = FntCharsWidth(buf, len);
  x = (s_width - width) / 2;
  if (x < 0) x = 0;
  WinDrawChars(buf, len, x, y);
  if (w && (width > *w)) *w = width;
}


/**************************************************************************
 **************************************************************************
 * The rest of this file is dedicated to printing the standard 80-column
 * dead Character Record to a MemoPad record.  It does not look too hot
 * when viewed in memopad but should be spiff IF YOU SYNC IT TO A COMPUTER.
 * Dedicated to the people who post 'em on rec.games.roguelike.moria .
 **************************************************************************
 **************************************************************************/

/* Let's also have something that can record to a memo... 
   I wonder how one does that.
   Make the title of the memo be "kMoria <date> <time>"     */

/*
 * Thanks to Matthias Jordan for YANoff (took a look at the Export To Memo
 * feature that shows it really is easy).
 */

//
// Opens database by name. If it doesn't exist: error
//
// returns 0 in case of error
// returns DmOpenRef > 0 in case of success
//
static DmOpenRef NPOpenDB(Char *name)
{
  LocalID lid;
  //  Err err;
  DmOpenRef db;

  lid = DmFindDatabase(0, name);
  if (lid == 0) {
    // not found - so create it
    //    err = DmCreateDatabase(0, name, CREATOR, TYPE, 0);
    //    if (err) return 0;
    //    lid = DmFindDatabase(0, name);
    return 0;
  }
  db = DmOpenDatabase(0, lid, dmModeReadWrite);
  return db;
} /* NPOpenDB */


// similar fn in display.c
static void prtstat(Short i, Char *stat)
{
  Short tmp = pystats.use_stat[i];
  if (tmp > 18) {
    tmp -= 18;
    if (tmp > 99) tmp = 0;
    StrPrintF(stat, (tmp > 9 ? "18/%d" : "18/0%d"), tmp);
  } else {
    StrPrintF(stat, "%d", tmp);
  }
}
// similar fn in form_chargen.c
static Char *likert(Short x, Short y)
{
  // I hope this still DTRT
  switch((x/y)) {
  case -3: case -2: case -1: return("Very Bad");
  case 0: case 1:            return("Bad");
  case 2:                    return("Poor");
  case 3: case 4:            return("Fair");
  case  5:                   return("Good");
  case 6:                    return("Very Good");
  case 7: case 8:            return("Excellent");
  default:                   return("Superb");
  }
}

static void snarf_history_block(Char *history_block)
{
  ULong uniqueID = REC_PHISTORY;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  // lock, copy, unlock record
  if (0!=DmFindRecordByID(MoriaSaveDB, uniqueID, &index)) return;
  vh = DmQueryRecord(MoriaSaveDB, index);
  p = MemHandleLock(vh);
  StrNCopy(history_block, (Char *) p, 239);
  MemHandleUnlock(vh);
  history_block[239] = '\0';//I forget if StrNCopy always terminates it
}

// A ridiculously long function.  It prints character record to memopad memo.
static void death_record_memo()
{
  DmOpenRef mdb;
  Char buf[200], stat[16]; // 160 + 24 < 200 so ok
  UInt index, memosize = 4000 * sizeof(Char); // xxxx MAKE THE SIZE BETTER
  VoidHand vh;
  Char *p;
  Short i, offset = 0;

  if (0 != (mdb = NPOpenDB("MemoDB"))) {
    if (0 != (vh = DmNewRecord(mdb, &index, memosize))) {
      if (0 != (p = MemHandleLock(vh))) {
	DmSet(p, 0, memosize, ' '); // will this work?
	StrPrintF(buf, "kMoria");
	seconds_to_string(death_date, buf, StrLen(buf));
	StrCat(buf, "\n\n");
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 26 chars

	////////////////////////////////// Section 1

	StrPrintF(buf,  " Name        : %s",                pymisc.name);
	for (i = StrLen(buf) ; i < 39 ; i++) buf[i] = ' ';
	StrPrintF(buf+39, "Age          :       ");
	StrPrintF(stat,   "%d",                             pymisc.age);
	StrPrintF(buf+60-StrLen(stat), "%s   STR :       ", stat);
	prtstat(0, stat);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	StrPrintF(buf,  " Race        : %s",        race[pymisc.prace].trace);
	for (i = StrLen(buf) ; i < 39 ; i++) buf[i] = ' ';
	StrPrintF(buf+39, "Height       :       ");
	StrPrintF(stat,   "%d",                             pymisc.ht);
	StrPrintF(buf+60-StrLen(stat), "%s   INT :       ", stat);
	prtstat(1, stat);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	StrPrintF(buf,  " Sex         : %s",    pymisc.male?"Male":"Female");
	for (i = StrLen(buf) ; i < 39 ; i++) buf[i] = ' ';
	StrPrintF(buf+39, "Weight       :       ");
	StrPrintF(stat,   "%d",                             pymisc.wt);
	StrPrintF(buf+60-StrLen(stat), "%s   WIS :       ", stat);
	prtstat(2, stat);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	StrPrintF(buf,  " Class       : %s",    class[pymisc.pclass].title);
	for (i = StrLen(buf) ; i < 39 ; i++) buf[i] = ' ';
	StrPrintF(buf+39, "Social Class :       ");
	StrPrintF(stat,   "%d",                             pymisc.sc);
	StrPrintF(buf+60-StrLen(stat), "%s   DEX :       ", stat);
	prtstat(3, stat);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	StrPrintF(buf,  " Title       : %s", ((total_winner) ? "Magnificent" :
	      (&player_title[((pymisc.pclass*MAX_PLAYER_LEVEL)+(pymisc.lev-1))
			    * MAXLEN_PTITLE]) ));
	for (i = StrLen(buf) ; i < 63 ; i++) buf[i] = ' ';
	StrPrintF(buf+63, "CON :       ", stat);
	prtstat(4, stat);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	for (i = 0 ; i < 63 ; i++) buf[i] = ' ';
	StrPrintF(buf+63, "CHR :       ", stat);
	prtstat(5, stat);
	StrPrintF(buf+75-StrLen(stat), "%s\n\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 77 chars

	////////////////////////////////// Section 2

	StrPrintF(buf, " + To Hit    :       ");
	StrPrintF(stat,   "%d",                             pymisc.dis_th);
	StrPrintF(buf+21-StrLen(stat), "%s       Level      :        ", stat);
	StrPrintF(stat,   "%d",                             pymisc.lev);
	StrPrintF(buf+48-StrLen(stat), "%s    Max Hit Points :      ", stat);
	StrPrintF(stat,   "%d",                             pymisc.mhp);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	StrPrintF(buf, " + To Damage :       ");
	StrPrintF(stat,   "%d",                             pymisc.dis_td);
	StrPrintF(buf+21-StrLen(stat), "%s       Experience :        ", stat);
	StrPrintF(stat,   "%d",                             pymisc.exp);
	StrPrintF(buf+48-StrLen(stat), "%s    Cur Hit Points :      ", stat);
	StrPrintF(stat,   "%d",                             pymisc.chp);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	StrPrintF(buf, " + To AC     :       ");
	StrPrintF(stat,   "%d",                             pymisc.dis_tac);
	StrPrintF(buf+21-StrLen(stat), "%s       Max Exp    :        ", stat);
	StrPrintF(stat,   "%d",                             pymisc.max_exp);
	StrPrintF(buf+48-StrLen(stat), "%s    Max Mana       :      ", stat);
	StrPrintF(stat,   "%d",                             pymisc.mana);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	StrPrintF(buf, "   Total AC  :       ");
	StrPrintF(stat,   "%d",                             pymisc.dis_ac);
	StrPrintF(buf+21-StrLen(stat), "%s       Exp to Adv :        ", stat);
	// xxx this might be bogus, check for division truncation problems
	StrPrintF(stat, "%ld", (Long) (player_exp[pymisc.lev-1]
				       * pymisc.expfact / 100));//form_chargen.
	StrPrintF(buf+48-StrLen(stat), "%s    Cur Mana       :      ", stat);
	StrPrintF(stat,   "%d",                             pymisc.cmana);
	StrPrintF(buf+75-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 76 chars

	for (i = 0; i < 28 ; i++) buf[i] = ' ';
	StrPrintF(buf+28, "Gold       :        ");
	StrPrintF(stat,   "%ld",                             pymisc.au);
	StrPrintF(buf+48-StrLen(stat), "%s\n", stat);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 50 chars

	////////////////////////////////// Section 3
	StrPrintF(buf, "\n(Miscellaneous Abilities)\n\n");
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 27 chars ... total of 864 chars so far.

	{
	  // From form_chargen.c - would it save space as a separate fn? maybe
	  // Calculate the miscellaneous abilities;
	  Short xbth, xbthb, xfos, xsrh, xstl, xdis, xsave, xdev;

	  xbth  = pymisc.bth + pymisc.ptohit*BTH_PLUS_ADJ
	    + (class_level_adj[pymisc.pclass][CLA_BTH] * pymisc.lev);
	  /* this results in a range from 0 to 9 */
	  xstl  = pymisc.stl + 1;
	  xfos  = 40 - pymisc.fos;
	  if (xfos < 0)  xfos = 0;

	  StrPrintF(buf, " Fighting    : %s", likert(xbth, 12));
	  for (i = StrLen(buf) ; i < 28 ; i++) buf[i] = ' ';
	  StrPrintF(buf+28, "Stealth     : %s", likert(xstl, 1));
	  for (i = StrLen(buf) ; i < 55; i++) buf[i] = ' ';
	  StrPrintF(buf+55, "Perception  : %s\n", likert(xfos, 3));
	  DmStrCopy(p, offset, buf);
	  offset += StrLen(buf);
	  // 79 chars
	
	  xbthb = pymisc.bthb + pymisc.ptohit*BTH_PLUS_ADJ
	    + (class_level_adj[pymisc.pclass][CLA_BTHB] * pymisc.lev);
	  xdis  = pymisc.disarm + 2*todis_adj() + stat_adj(A_INT)
	    + (class_level_adj[pymisc.pclass][CLA_DISARM] * pymisc.lev / 3);
	  /* this results in a range from 0 to 29 */
	  xsrh  = pymisc.srh;

	  StrPrintF(buf, " Bows/Throw  : %s", likert(xbthb,12));
	  for (i = StrLen(buf) ; i < 28 ; i++) buf[i] = ' ';
	  StrPrintF(buf+28, "Disarming   : %s", likert(xdis, 8));
	  for (i = StrLen(buf) ; i < 55; i++) buf[i] = ' ';
	  StrPrintF(buf+55, "Searching   : %s\n", likert(xsrh, 6));
	  DmStrCopy(p, offset, buf);
	  offset += StrLen(buf);
	  // 79 chars
  
	  xsave = pymisc.save + stat_adj(A_WIS)
	    + (class_level_adj[pymisc.pclass][CLA_SAVE] * pymisc.lev / 3);
	  xdev  = pymisc.save + stat_adj(A_INT)
	    + (class_level_adj[pymisc.pclass][CLA_DEVICE] * pymisc.lev / 3);

	  StrPrintF(buf, " Saving Throw: %s", likert(xsave, 6));
	  for (i = StrLen(buf) ; i < 28 ; i++) buf[i] = ' ';
	  StrPrintF(buf+28, "Magic Device: %s", likert(xdev, 6));
	  for (i = StrLen(buf) ; i < 55; i++) buf[i] = ' ';
	  StrPrintF(buf+55, "Infra-Vision: %d feet\n\n", pyflags.see_infra*10);
	  DmStrCopy(p, offset, buf);
	  offset += StrLen(buf);
	  // 79 chars (if infra-vision is a max of 3 digits)
	}
	
	////////////////////////////////// Section 4
	StrPrintF(buf, "\nCharacter Background\n");
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 22 chars.  total of 1123 chars so far.  now it gets variable..

	// copy the REC_PHISTORY string!
	{
	  Char history_block[240];
	  Short j, k, len;

	  snarf_history_block(history_block);
	  // word-wrap the history block!
	  len = StrLen(history_block);
	  j = k = 0;
	  // this (60) is the line length used in moria create.c get_history().
#define LINELEN 60
	  while (j < len) {
	    //Step left from the end of the (desired) line to find a whitespace
	    for (j = k + LINELEN ; j > k ; j--)
	      if (history_block[j] == ' ')
		break;
	    if (j == k) // this case (really long word) should never happen:
	      j = k + LINELEN;
	    history_block[j] = '\n';
	    k = j;
	  }
	  history_block[239] = '\0'; // bride of paranoia
#undef LINELEN
	  // ok now copy the word-wrapped string to the memopad record
	  DmStrCopy(p, offset, history_block);
	  offset += StrLen(history_block);
	}		
	// 240 chars (at most).  total of 1363 (at most) so far.

	////////////////////////////////// Section 5
	StrPrintF(buf, "\n\n\n  [Character's Equipment List]\n\n");
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 35 chars.  total of 1398 (at most) so far.

	//StrPrintF(buf, "buffalo\n"); // XXX
	if (equip_ctr == 0) {
	  StrPrintF(buf, "  Character has no equipment in use.\n\n");
	  DmStrCopy(p, offset, buf);
	  offset += StrLen(buf);
	  // (38 chars.)
	} else {
	  inven_type *i_ptr;
	  Short i, j=0, k, len;
	  for (i = INVEN_WIELD; i < INVEN_ARRAY_SIZE; i++) {  // i = [22,34).
	    i_ptr = &inventory[i];
	    // THERE IS A BUG SOMEWHERE AFTER HERE.
	    // (a somewhat old comment).  is there still?  I'm dubious.
	    if (i_ptr->tval != TV_NOTHING) {
	      StrPrintF(buf, "  %c) ", (Char) (j+'a'));
	      j++;
	      switch (i) {
	      case INVEN_WIELD:	StrPrintF(buf+5, "You are wielding");	break;
	      case INVEN_HEAD:	StrPrintF(buf+5, "Worn on head");	break;
	      case INVEN_NECK:	StrPrintF(buf+5, "Worn around neck");	break;
	      case INVEN_BODY:	StrPrintF(buf+5, "Worn on body");	break;
	      case INVEN_ARM:	StrPrintF(buf+5, "Worn on shield arm"); break;
	      case INVEN_HANDS:	StrPrintF(buf+5, "Worn on hands");	break;
	      case INVEN_RIGHT:	StrPrintF(buf+5, "Right ring finger");  break;
	      case INVEN_LEFT:	StrPrintF(buf+5, "Left  ring finger");  break;
	      case INVEN_FEET:	StrPrintF(buf+5, "Worn on feet");	break;
	      case INVEN_OUTER:	StrPrintF(buf+5, "Worn about body");	break;
	      case INVEN_LIGHT:	StrPrintF(buf+5, "Light source is");	break;
	      case INVEN_AUX:	StrPrintF(buf+5, "Secondary weapon");	break;
	      default: StrPrintF(buf+5, "*Unknown value*");     break;
	      }
	      for (k = StrLen(buf) ; k < 26 ; k++) buf[k] = ' ';
	      buf[24] = ':'; // (and 25 is already a space)
	      objdes(buf+26, &inventory[i], true); // print object descrip
	      len = StrLen(buf); // tack on the newline...
	      buf[len] = '\n';
	      buf[len+1] = '\0'; // done...
	      DmStrCopy(p, offset, buf);
	      offset += StrLen(buf);
	    }
	  }
	}
	// There can be at most 12 lines of equipment, each w/same max length.
	// 26 chars + objdes() + "\n".  I haven't calculated the max length
	// possible for objdes(); it's between 80 and 160.
	// 324 chars + 12*objdes.  (Even if 160, we're at most 3642 here.)

	////////////////////////////////// Section 6
	StrPrintF(buf, "\n  [General Inventory List]\n\n");
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	// 29 chars.

	if (inven_ctr == 0) {
	  StrPrintF(buf, "  Character has no objects in inventory.\n\n");
	  DmStrCopy(p, offset, buf);
	  offset += StrLen(buf);
	  // (42 chars.)
	} else {
	  Short i, len;
	  for (i = 0; i < inven_ctr; i++) {
	    StrPrintF(buf, "%c) ", (Char) (i+'a'));
	    objdes(buf+3, &inventory[i], true); // print object description
	    len = StrLen(buf); // tack on the newline...
	    buf[len] = '\n';
	    buf[len+1] = '\0'; // done...
	    // If you have enscribed everything, it's possible to get item
	    // names so long that we'll run out of space even with 4000 chars.
	    // So, check for this, and stop writing if we're out of space.
	    // (I've calculated that we won't run out any sooner than here.)
	    if ((len + offset) > memosize)
	      break; // Hey, we're out of space.
	    DmStrCopy(p, offset, buf);
	    offset += StrLen(buf);
	  }
	  // there can be at most 22 inventory items.  I think.
	  // 3 chars + objdes() + "\n".
	  // 88 chars + 22*objdes.
	  //
	  // Total of: 1398 + 324 + 88 + (22+12)*objdes = 1810 + 34*objdes.
	  // Unless the user is perverse, this will be less than 4000 chars,
	  // (but we check above anyway and truncate during inventory output).
	}
	// all done!!!!!

	// clean up now
	MemPtrUnlock(p);
	DmReleaseRecord(mdb, index, true);
	DmCloseDatabase(mdb);
	return;
      }
    }
    DmCloseDatabase(mdb);    
  }
  // Report the failure... make the button go away and do a popup alert?

}



/************************************************************************
 ************************************************************************
 *******************   Record the highscore thing   *********************
 ************************************************************************
 ************************************************************************/

static void init_highscore(high_scores *score)
{
  Char *p;
  UChar c;  // c = 0rrrCCCm

  c = (((pymisc.prace << 3) | pymisc.pclass) << 1) | (pymisc.male ? 0x1 : 0x0);
  score->points = total_points();
  score->birth_date = birth_date;
  //  score->mhp = pymisc.mhp;
  //  score->chp = pymisc.chp;
  //  score->dun_level = dun_level;
  //  score->max_dlv = pymisc.max_dlv;
  score->lev = pymisc.lev;
  score->race_class_sex = c;
  score->source = 0;  // change this for multiplayer platform!
  StrCopy(score->name, pymisc.name);
  p = died_from;
  // skip "an " or "a "
  if (*p == 'a') {
    p++;
    if (*p == 'n') p++;
    while (*p == ' ') p++;
  }
  StrCopy(score->died_from, p);
}


// was in death.c ...
/* Calculates the total number of points earned		-JWT-	 */
Long total_points()
{
  Long total;
  Short i;

  total = pymisc.max_exp + (100 * pymisc.max_dlv);
  total += pymisc.au / 100;
  for (i = 0; i < INVEN_ARRAY_SIZE; i++)
    total += item_value(&inventory[i]);
  total += dun_level * 50;

  /* Don't ever let the score decrease from one save to the next.  */
  if (max_score > total)
    return max_score; // max_score is set/saved as my_save_info.total_points.

  return total;
}



/*****************************************
 * Ok, here is my new outline of what to do.

 Write player info to a struct on the stack.
 Figure out what index it goes at, and whether to insert or replace.
 Create a new record.  Delete old record, where applicable.
 Release new record.

 *************************************/
// This is supremely untested.
void insert_score(high_scores *score)
{
  VoidHand vh;
  VoidPtr p;
  high_scores *scores_old = 0;
  Short num = 1, i = 0;
  ULong size;
  //  Long birth_date;
  Boolean is_me;

  num = DmNumRecords(MoriaScoreDB);
  i = 0;
  // Skip records until we find one that we are better than (or
  // have same birthdate as).  If we have same birthday, delete that
  // record before breaking, so that we replace it.  or something.
  while (i < num) {
    is_me = false;
    vh = DmQueryRecord(MoriaScoreDB, i);
    p = MemHandleLock(vh);
    scores_old = (high_scores *) p;
    if (score->points >= scores_old->points) {
      MemHandleUnlock(vh);
      break; // found our rank
    }
    if ((score->birth_date == scores_old->birth_date) &&
	(score->source == scores_old->source))
      is_me = true; // we only allow one score per "character"
    MemHandleUnlock(vh);
    if (is_me) {
      DmRemoveRecord(MoriaScoreDB, i); // hope this does what I think it does
      num--;
    } else
      i++;
  }
  // ok we have found the index where we belong
  size = sizeof(high_scores);
  vh = DmNewRecord(MoriaScoreDB, &i, size);
  p = MemHandleLock(vh);
  DmWrite(p, 0, score, sizeof(high_scores));
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaScoreDB, i, true);
  // ok now clean up the rest of the records, if any.
  num = DmNumRecords(MoriaScoreDB);
  i++; // ?
  while (i < num) {
    is_me = false;
    vh = DmQueryRecord(MoriaScoreDB, i);
    p = MemHandleLock(vh);
    scores_old = (high_scores *) p;
    if (score->birth_date == scores_old->birth_date) is_me = true;
    MemHandleUnlock(vh);
    if (is_me) {
      DmRemoveRecord(MoriaScoreDB, i); // hope this does what I think it does
      num--;
    } else
      i++;
  }
  // finally, make sure there are not Too Many
  while (DmNumRecords(MoriaScoreDB) > SCOREFILE_SIZE)
    DmRemoveRecord(MoriaScoreDB, DmNumRecords(MoriaScoreDB)-1);
}

// well, in theory this should work.
// it will be called at most once, by perform_conversion.
void convert_topscores()
{
  ULong uniqueID;
  UInt index_old;
  VoidHand vh, vh_old = 0;
  VoidPtr p, p_old;
  high_scores score;
  high_scores_old *scores_old = 0;
  Short num, i = 0, j = 0; // i counts old entries, j the new.
  Short *nump;
  ULong size;
  Boolean exists;

  // Is the "new" top scores database empty.  If not, we have probably
  // already done this (and someone is reloading a version 1 save file.
  // tsk tsk tsk.)
  if (DmNumRecords(MoriaScoreDB) > 0)
    return;
  // Does the top scores list exist.
  uniqueID = REC_TOPSCORES;
  exists = (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index_old));
  if (!exists)
    return;
  // it's a Short (n), followed by n records.
  vh_old = DmQueryRecord(MoriaSaveDB, index_old);
  p_old = MemHandleLock(vh_old);
  nump = (Short *) p_old;
  num = nump[0];
  if (num <= 0) {
    MemHandleUnlock(vh_old);
    DmRemoveRecord(MoriaSaveDB, index_old);      // EX TERM IN ATE !
    return;
  }
  scores_old = (high_scores_old *) &(nump[1]);
  if (num > SCOREFILE_SIZE) num = SCOREFILE_SIZE; // put a cap on it..
  // ok, now, iterate through this record, creating records in the new db
  size = sizeof(high_scores);
  for (i = 0 ; i < num ; i++) {
    // read scores_old[i]; create new record i; write to it.
    score.points = scores_old[i].points;
    score.birth_date = scores_old[i].birth_date;
    score.lev = scores_old[i].lev;
    score.race_class_sex = scores_old[i].race_class_sex;
    score.source = 0;
    for (j = 0 ; j < PLAYER_NAME_SIZE ; j++)
      score.name[j] = '\0';
    for (j = 0 ; j < 25 ; j++)
      score.died_from[j] = '\0';
    StrNCopy(score.name, scores_old[i].name, PLAYER_NAME_SIZE);
    StrNCopy(score.died_from, scores_old[i].died_from, 25);
    vh = DmNewRecord(MoriaScoreDB, &i, size);
    p = MemHandleLock(vh);
    DmWrite(p, 0, &score, sizeof(high_scores));
    MemPtrUnlock(p);
    DmReleaseRecord(MoriaScoreDB, i, true);
  }
  DmRemoveRecord(MoriaSaveDB, index_old);
}

/*****************************************
 * Ok, here is my outline of what to do.

 Write player info to a struct on the stack.
 Create a new record.
 _If_ there is already a REC_TOPSCORES record,
      figure out what index the player info will appear at (or replace),
      copy up to that index from old record to new record.
 Write the player info from the struct to the new record.
 _If_ there is already a REC_TOPSCORES record,
      copy the remainder (skip a spot if replacing index) to the new record.
      delete the REC_TOPSCORES record.
 Release the new record.
 Set its uid to REC_TOPSCORES.
 Congratulate self on doing things the easy way this time (sorta).

 *************************************/

/* Add the player's data to the top-score list, perhaps replacing an entry */
/*
static void frob_topscores_old()
{
  ULong uniqueID;
  UInt index_old, index;
  VoidHand vh, vh_old = 0;
  VoidPtr p, p_old;
  high_scores score, *scores_old = 0;
  Short num = 1, offset = 0, i = 0, j = 0; // i counts old entries, j the new.
  Short *nump;
  Boolean exists;
  ULong size;
  Char buf[40];

  init_highscore(&score);

  // Does the top scores list exist.
  uniqueID = REC_TOPSCORES;
  exists = (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index_old));
  if (exists) {
    // it's a Short (n), followed by n records.
    vh_old = DmQueryRecord(MoriaSaveDB, index_old);
    p_old = MemHandleLock(vh_old);
    nump = (Short *) p_old;
    num += nump[0];
    scores_old = (high_scores *) &(nump[1]);
  }

  if (num > SCOREFILE_SIZE) num = SCOREFILE_SIZE;
  // we will always replace the last entry even if we are at max size.

  // Open a new record.
  index = DmNumRecords(MoriaSaveDB);
  size = sizeof(Short) + num * sizeof(high_scores);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p) {
    DmWrite(p, offset, &num, sizeof(Short)); // write the # of items in set
    offset += sizeof(Short);
    if (exists && scores_old) { // testing
      // Write records until we find one that we are better than (or
      // have the same birthday as).  We will pick up here (if we have
      // the same birthday, ++ the counter so that we'll skip it) after.
      // Oh, stop if we run out of records or space.
      while ((i < num-1) && (j < SCOREFILE_SIZE-1)) {
	if (score.points >= scores_old[i].points) break; // found our rank
	if (score.birth_date != scores_old[i].birth_date) {
	  DmWrite(p, offset, &scores_old[i], sizeof(high_scores));
	  offset += sizeof(high_scores);
	  j++;
	} // else we are a clone of i; we'll delete i cause we're newer
	i++;
      }
    }
    DmWrite(p, offset, &score, sizeof(high_scores));
    offset += sizeof(high_scores);
    j++;
    if (exists && scores_old) {
      // Write the remaining records.  Stop after num-1.
      while ((i < num-1) && (j < SCOREFILE_SIZE)) {
	if (score.birth_date != scores_old[i].birth_date) {
	  DmWrite(p, offset, &scores_old[i], sizeof(high_scores));
	  offset += sizeof(high_scores);
	  j++;
	} // else we are a clone of i; we'll delete i cause we're newer
	i++;
      }
      MemHandleUnlock(vh_old);
    }
  }

  exists = (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index_old));

  // in with the new
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  if (exists)
    DmRemoveRecord(MoriaSaveDB, index_old);      // EX TERM IN ATE !

}
*/
