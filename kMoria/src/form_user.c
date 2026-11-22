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

// Changing users might not work in OS2.  The POSE is so angry about
// it that I can't even hit debug to get to gdb to backtrace the problem,
// meanwhile, the old xcopilot doesn't see anything wrong.

static void what_users(FormPtr frm) SEC_7;
//static void toggle_hotsync(FormPtr frm, Short i) SEC_7;
static void change_user(Short selected_user) SEC_7;
//static Boolean create_user_dbname(Char *dbname) SEC_7;
static void create_user() SEC_7;
static void delete_user(Short i) SEC_7;
Boolean userform_first;

// I think LocalID is a DWord.
#define MAX_USERS 6
LocalID found_dbids[MAX_USERS];
Short num_found_db;
Char **db_display;

Boolean User_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  Short s;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    what_users(frm);
    //    FrmDrawForm(frm); // that's taken care of in what_users now...
    if (userform_first) {
      ControlPtr btn;
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_user_cancel));
      CtlHideControl(btn);
    }
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    s = e->data.lstSelect.selection;
    if (s != -1)
      show_user_info(frm, s);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case check_user_1:
      frm = FrmGetActiveForm();
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_user));
      s = LstGetSelection(lst);
      if (s != -1) {
	ControlPtr checkbox;
	checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_user_1));
	toggle_hotsync(found_dbids[s], CtlGetValue(checkbox));
      }
      handled = true;
      break;
    case btn_user_ok:
      frm = FrmGetActiveForm();
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_user));
      s = LstGetSelection(lst);
      LeaveForm();
      if (s != -1)
	change_user(s);
      handled = true;
      break;
    case btn_user_new:
      LeaveForm();
      create_user();
      handled = true;
      break;
    case btn_user_cancel:
      if (!userform_first)
	LeaveForm();
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}




// Call this when you enter the form to discover what databases are available.
static void what_users(FormPtr frm)
{
  // do like FindDatabaseByTypeCreator or something I guess.
  DmSearchStateType state;
  UInt card;
  LocalID currentDB = 0;
  Err err;
  Short i = 0;
  Char buf[32];
  ListPtr lst;
  ULong modified_date, mru_date = 0;
  Short default_selection = -1;
  ControlPtr checkbox;

  // start with the checkbox hidden... (it should be already anyway)
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_user_1));
  CtlHideControl(checkbox);

  for (i = 0 ; i < MAX_USERS ; i++)
    found_dbids[i] = 0;
  db_display = (Char **) md_malloc(sizeof(Char *) * MAX_USERS);
  num_found_db = 0;
  err = DmGetNextDatabaseByTypeCreator(true, &state,
				       MoriaSaveDBType, MoriaAppID,
				       false, &card, &currentDB);
  while (!err && currentDB && (num_found_db < MAX_USERS)) {
    if (card == 0) { // (I hope this rules out freakishness) take note of db.
      found_dbids[num_found_db] = currentDB;
      DmDatabaseInfo(card, currentDB, buf, NULL, NULL, NULL, &modified_date,
		     NULL, NULL, NULL, NULL, NULL, NULL);
      buf[32-1] = '\0';
      i = StrLen(buf) + 1;
      db_display[num_found_db] = (Char *) md_malloc(sizeof(Char *) * i);
      StrCopy(db_display[num_found_db], buf);
      if (my_prefs.determine_recent_user & RESTORE_RECENT) {
	if (modified_date > mru_date) {
	  mru_date = modified_date;
	  default_selection = num_found_db;
	}
      } else if (!StrCompare(buf, MoriaSaveDBName)) {
	default_selection = num_found_db;
      }
      num_found_db++;
    }
    // get the next DB
    err = DmGetNextDatabaseByTypeCreator(false, &state, 
					 MoriaSaveDBType, MoriaAppID,
					 false, &card, &currentDB);
  }
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_user));
  LstSetListChoices(lst, db_display, num_found_db);

  FrmDrawForm(frm);

  LstSetSelection(lst, default_selection);
  if (default_selection != -1)
    show_user_info(frm, default_selection);

  if (num_found_db >= MAX_USERS) {
    ControlPtr btn;
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_user_new));
    CtlHideControl(btn);
  }
  
}

// call this when player toggles the "Hotsync?" checkbox.
void toggle_hotsync(LocalID dbid, Boolean backup)
{
  UInt attribs;
  if (!dbid) return;
  DmDatabaseInfo(0, dbid, NULL, &attribs, NULL, NULL, NULL,
		 NULL, NULL, NULL, NULL, NULL, NULL);
  if (backup)
    attribs |= dmHdrAttrBackup;
  else
    attribs &= ~dmHdrAttrBackup;
  DmSetDatabaseInfo(0, dbid, NULL, &attribs, NULL, NULL, NULL,
		    NULL, NULL, NULL, NULL, NULL, NULL);
}

// Ha!  It works nicely.  at least as far as I have tested.
// Call this when you select a list item, and it will print details.
extern Char *player_title; // from lock.c
void show_user_info(FormPtr frm, Short i)
{
  DmOpenRef tmpDBRef;
  UInt attribs;
  ULong created, modified;
  Char buf[80]; // dateStringLength = 9, timeStringLength = 9
  Short len, y;
  ULong uniqueID;
  UInt index;
  Boolean found = false;
  RectangleType r;
  ControlPtr checkbox;
  if (i < 0 || i >= num_found_db || found_dbids[i] == 0)
    return;
  // card, id, name, attribs, version, ...
  DmDatabaseInfo(0, found_dbids[i], NULL, &attribs, NULL, &created, &modified,
		 NULL, NULL, NULL, NULL, NULL, NULL);

  // show the checkbox
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_user_1));
  CtlShowControl(checkbox);
  CtlSetValue(checkbox, ((attribs & dmHdrAttrBackup) ? 1 : 0));

  y = ROW_1 + 5*LINE_HEIGHT + LINE_HEIGHT/2; // was 15+55+5 = 75 (out of 160)
   // really that wants to be "bottom of some UI widget + a small margin"
  RctSetRectangle(&r, 0, y, SCR_WIDTH, 5 * LINE_HEIGHT);
  WinEraseRectangle(&r, 0);
  // I've taken out the "created" line. 
  StrPrintF(buf, "modified:");//  StrPrintF(buf, "modified:");
  len = StrLen(buf);
  seconds_to_string(modified, buf, len);
  WinDrawChars(buf, StrLen(buf), 0, y);
  y += LINE_HEIGHT;
  // Not much else I can grab without reading the database.
  // I should do basic top-scores stuff: character name, class, race, whatever.
  if (0 == (tmpDBRef = DmOpenDatabase(0, found_dbids[i], dmModeReadOnly)))
    return;
  uniqueID = REC_PY;
  if (0 == DmFindRecordByID(tmpDBRef, uniqueID, &index)) {
    VoidHand vh;
    VoidPtr p;
    vh = DmQueryRecord(tmpDBRef, index);
    p = MemHandleLock(vh);
    if (p) {
      struct misc *tmp_pymisc;
      tmp_pymisc = (struct misc *) p;
      StrPrintF(buf, "\"%s\"", tmp_pymisc->name);
      len = StrLen(buf);
      // btw, the widest title is "Common Rogue". <32 pixels.
      StrPrintF(buf+len, " the %s",
		&player_title[( (tmp_pymisc->pclass*MAX_PLAYER_LEVEL)+
				(tmp_pymisc->lev-1) )
			     * MAXLEN_PTITLE]);
      if (FntCharsWidth(buf, StrLen(buf)) < SCR_WIDTH)
	WinDrawChars(buf, StrLen(buf), 0, y);
      else { // cheap line-wrap
	buf[len] = '\0';
	WinDrawChars(buf, StrLen(buf), 0, y);
	y += LINE_HEIGHT;
	WinDrawChars(buf+len+1, StrLen(buf+len+1), 10, y); // xxx 10 out of 160
      }
      y += LINE_HEIGHT;
      StrPrintF(buf, "level %d %s %s",
		tmp_pymisc->lev, race[tmp_pymisc->prace].trace,
		class[tmp_pymisc->pclass].title);
      /*
	// I could add this if I dig all the way to floor_save_info->dun_level
      len = StrLen(buf);
      if (dun_level > 0) StrPrintF(buf+len, "at %d'", dun_level * 50);
      else               StrPrintF(buf+len, "in Town");
      */
      WinDrawChars(buf, StrLen(buf), 3*FntCharWidth(' '), y);
      y += LINE_HEIGHT;
      if (tmp_pymisc->max_dlv > 0) {
	StrPrintF(buf, "intrepid explorer of %d feet", tmp_pymisc->max_dlv * 50);
	WinDrawChars(buf, StrLen(buf), 3*FntCharWidth(' '), y);
	y += LINE_HEIGHT;
      }

      found = true;
    }
    MemHandleUnlock(vh);
  }
  if (!found) {
    StrPrintF(buf, "[empty]");
    WinDrawChars(buf, StrLen(buf), 2, y); // xxx 2 out of 160
  }
  DmCloseDatabase(tmpDBRef);
}

// ah crud, I need a couple more buttons:  Create and Delete.
void StartApp_helper(); // this is in main.c
// Call this to change to a different user's database.
static void change_user(Short selected_user)
{
  DmOpenRef tmpDBRef;
  // First, make sure that selected_user will lead us to a valid SaveDB.
  if (selected_user < 0 || selected_user >= num_found_db ||
      found_dbids[selected_user] == 0) {
    // I should print an error msg in UserForm...
    message("no database selected");
    return;
  }

  // Save the current player and close the current database.
  if (MoriaSaveDB != NULL) {
    if (!userform_first) {
      save_char();
      didnt_crash();
    }
    data_record_lock(false);
    DmCloseDatabase(MoriaSaveDB);
  }

  if (0 == (tmpDBRef = DmOpenDatabase(0, found_dbids[selected_user],
					 dmModeReadWrite))) {
    message("Can't open selected database.  Panic!");
    // If this happens you're in serious trouble.
    return;
  }

  if (!userform_first)
    LeaveForm(); // get rid of the old MainForm.

  // Switch to the new database!  And do whatever we normally do at startup.
  MoriaSaveDB = tmpDBRef;
  if (DmNumRecords(MoriaSaveDB) < 1) {
    data_record_create();
  }
  data_record_lock(true);
  StartApp_helper();

  // We should now be in Char Generation or in MainForm.  Hurrah!  I hope.
  return;
}

static void create_user()
{
  // First, figure out whether we have too many users already
  // If so, print an error message and stay here.
  Short i, j;
  LocalID dbID;
  Char newdb_name[40], buf[32];
  Boolean is_used = true;
  if (num_found_db >= MAX_USERS) {
    message("too many users");
    return; // too many...
  }
  // Also, I need to generate a name for the new DB.  or let the user generate.
  //if (!create_user_dbname(newdb_name)) return;
  for (i = 0 ; is_used && (i < MAX_USERS+1) ; i++) {
    StrPrintF(newdb_name, "kMoriaSave%dDB", i);
    is_used = false;
    for (j = 0 ; !is_used && (j < MAX_USERS) ; j++) {
      if (found_dbids[j]) {
	DmDatabaseInfo(0, found_dbids[j], buf,
		       NULL, NULL, NULL, NULL, NULL,
		       NULL, NULL, NULL, NULL, NULL);
	if (0 == StrNCompare(newdb_name, buf, 32))
	  is_used = true;
      }
    }
  }
  message(newdb_name);
  if (is_used) {
    message("namegen failed");
    return;
  }
  message(newdb_name);
  if (0 != (dbID = DmFindDatabase(0, newdb_name))) {
    // database with this name already exists!
    message("can't find a good db name");
    return;
  }
  if (DmCreateDatabase(0, newdb_name, MoriaAppID, MoriaSaveDBType, false)) {
    // unable to create db!
    message("can't create db");
    return;
  }
  if (0 == (dbID = DmFindDatabase(0, newdb_name))) {
    // unable to find newly-created db!
    message("can't find db");
    return;
  }
  toggle_hotsync(dbID, true);
  // Ok, the rest will be the same as change_user.  let's just use that.
  i = num_found_db;
  found_dbids[i] = dbID;
  num_found_db++;
  change_user(i);
}

/*
static Boolean create_user_dbname(Char *dbname)
{
  Char buf[32];
  Short i, j;
  Boolean is_used;
  for (i = 0 ; i < MAX_USERS+1 ; i++) {
    StrPrintF(dbname, "kMoriaSave%dDB", i);
    is_used = false;
    for (j = 0 ; j < MAX_USERS ; j++) {
      if (found_dbids[j]) {
	DmDatabaseInfo(0, found_dbids[j], buf,
		       NULL, NULL, NULL, NULL, NULL,
		       NULL, NULL, NULL, NULL, NULL);
	if (0 != StrNCompare(dbname, buf, 32)) {
	  is_used = true;
	  break;
	}
      }
    }
    if (!is_used) return true;
  }
  // btw, if we got this far, we are in trouble...
  return false;
}
*/

// i is an index into found_dbids
static void delete_user(Short i)
{
  // should ask if you are really, really sure.
  // also, should refuse to delete kMoriaSaveDB, which hypothetically
  // belongs strictly to the owner of the device.
}
