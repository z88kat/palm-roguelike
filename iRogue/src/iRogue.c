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
#include "Graffiti.h" /* just for GrfGetState */
#include <CharAttr.h> /* to keep 3.1 happy about ChrIsHardKey */
#ifndef I_AM_COLOR
#include <System/Globals.h> // for font munging
#else /* I_AM_COLOR */
// hm, do I need something?
#endif /* I_AM_COLOR */
//extern DmOpenRef       RogueDB;
DmOpenRef RogueDB = NULL;
DmOpenRef RogueTileDB = NULL;

extern Short ScrWidth; // display.c
extern Short ScrHeight;
extern Short WalkMagic;


// declarations
Boolean buttonsHandleEvent(EventPtr e); // now in main.c
//static void    start_of_Play_Level_loop(); // now in main.c
//static Boolean main_help_handle_cmd(Char c); // now in main.c
Boolean        Main_Form_HandleEvent(EventPtr e); // now in main.c
//static void    init_direction_select_list(FormPtr frm, Boolean zap); //main.c
//static void    init_direction_select(FormPtr frm, Short task); //main.c
Boolean        Direction_Form_HandleEvent(EventPtr e); // now in main.c
Boolean        MsgLog_Form_HandleEvent(EventPtr e);
Boolean        InvSelect_Form_HandleEvent(EventPtr e); // now inventory.c
Boolean        Call_Form_HandleEvent(EventPtr e); // it's around somewhere
Boolean        Wiz_Form_HandleEvent(EventPtr e);
Boolean        Prefs_Form_HandleEvent(EventPtr e);
Boolean        About_Form_HandleEvent(EventPtr e);
Boolean        Map_Form_HandleEvent(EventPtr e);
Boolean        HwButtons_Form_HandleEvent(EventPtr e);
Boolean        TopTen_Form_HandleEvent(EventPtr e);
Boolean Revivify_Form_HandleEvent(EventPtr e);
Boolean Snapshot_Form_HandleEvent(EventPtr e);
#ifdef I_AM_COLOR
Boolean Color_Form_HandleEvent(EventPtr e);
Boolean Thing_Form_HandleEvent(EventPtr e);
#endif
static Boolean OpenDatabase(void);
static Boolean ApplicationHandleEvent(EventPtr e);
static void    readRoguePrefs();
//static void    writeRoguePrefs();
static Word    StartApplication(void);
static void    free_everything();
static void    StopApplication(void);
static void    EventLoop(void);
DWord          PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags);
// end

Short x, y;
extern Short inventory_item; // main.c; used here lots

RoguePreferenceType my_prefs = {
  true, // run_on
  true, // formerly save_on_exit, now autosave - in 0.42
  4, // run_walk_border
  1, // walk_search_border
  false, // dave_mode
  false, // scroll_mode
  true, // sound_on
  false, // use hardware
  {0, 0, 0, 0, // the hardware buttons
   0, 0, 0, 0},
  false, // stay centered
  false, // use large (normal) font - new in 0.42
  false, // use rogue-relative movement - new in 0.42
  //
  0, // color on - new in 0.45 - changed to UChar in 0.47
  false, // crosshairs on - ditto
  false  // sort pack - new in 0.46
};
/*
Boolean run_on = true;
Boolean save_on_exit = true;
Short run_walk_border = 4;
Short walk_search_border = 1;
Boolean dave_mode = false;
Boolean scroll_mode = false;
*/


Boolean inventory_not_messages = true;

Boolean do_make_new_level = false;
Boolean save_me = true;
Boolean i_am_dead = false;


/* define INV_foo MOVED to rogue_defines.h */







/*****************************************************************************
 *                                                                           *
 *   The following stufff actually belongs in this file.                     *
 *                                                                           *
 *****************************************************************************/
/*****************************************************************************
 *                                                                           *
 *                      OpenDatabase                                         *
 *                                                                           *
 *****************************************************************************/
static Boolean OpenDatabase(void)
{
  do_make_new_level = false; // so that we will load the saved character
  /* Try to open the Rogue database.
     If it does not exist, create one. */

  RogueDB =  DmOpenDatabaseByTypeCreator(RogueDBType,
					 RogueAppID,
					 dmModeReadWrite);
  if (!RogueDB) {
    if (DmCreateDatabase(0, // card number 0
			 RogueDBName,
			 RogueAppID,
			 RogueDBType,
			 false))
      return 1;
    RogueDB = DmOpenDatabaseByTypeCreator(RogueDBType,
					   RogueAppID,
					   dmModeReadWrite);
    /* Success has crowned my efforts!  and I may consider myself
       engaged to Phyllis!  ok, maybe not. */
    do_make_new_level = true;    
    create_top_ten_record();
    create_filecount_record();
    return 0;
  }

  if (DmNumRecords(RogueDB) <= 11)
    do_make_new_level = true; /* the database doesn't really have stuff */    
  else if (get_saved_rogue_status(0) & STATUS_ISDEAD)
    do_make_new_level = true; /* the saved character is DEAD, ignore it */

  /* there was a Rogue DB and we opened it */
  return 0;
}


/*****************************************************************************
 *                      ApplicationHandleEvent                               *
 *****************************************************************************/
static Boolean ApplicationHandleEvent(EventPtr e)
{
    FormPtr frm;
    Word    formId;
    Boolean handled = false;

    if (e->eType == frmLoadEvent) {
	formId = e->data.frmLoad.formID;
	frm = FrmInitForm(formId);
	FrmSetActiveForm(frm);

	switch(formId) {
	case MainForm:
	    FrmSetEventHandler(frm, Main_Form_HandleEvent);
	    break;
	case DirectionForm:
	    FrmSetEventHandler(frm, Direction_Form_HandleEvent);
	    break;
	case MsgLogForm:
	    FrmSetEventHandler(frm, MsgLog_Form_HandleEvent);
	    break;
	case InvSelectForm:
	    FrmSetEventHandler(frm, InvSelect_Form_HandleEvent);
	    break;
	case CallForm:
	    FrmSetEventHandler(frm, Call_Form_HandleEvent);
	    break;
	case WizForm:
	    FrmSetEventHandler(frm, Wiz_Form_HandleEvent);
	    break;
	case PrefsForm:
	    FrmSetEventHandler(frm, Prefs_Form_HandleEvent);
	    break;
	case AboutForm:
	    FrmSetEventHandler(frm, About_Form_HandleEvent);
	    break;
	case MapForm:
	    FrmSetEventHandler(frm, Map_Form_HandleEvent);
	    break;
	case TopTenForm:
	    FrmSetEventHandler(frm, TopTen_Form_HandleEvent);
	    break;
	case RevivifyForm:
	    FrmSetEventHandler(frm, Revivify_Form_HandleEvent);
	    break;
	case SnapshotForm:
	    FrmSetEventHandler(frm, Snapshot_Form_HandleEvent);
	    break;
	case HwButtonsForm:
	    FrmSetEventHandler(frm, HwButtons_Form_HandleEvent);
	    break;
#ifdef I_AM_COLOR
	case ThingForm:
	    FrmSetEventHandler(frm, Thing_Form_HandleEvent);
	    break;
	case ColorForm:
	    FrmSetEventHandler(frm, Color_Form_HandleEvent);
	    break;
#endif
	}
	handled = true;
    }

    return handled;
}

// dude - the 3.5ez (Vx) rom hassled me about this (I think it was about this).
// ...the III rom did not, and the color rom did not, and the 3.3's did not.
// wacky.
static void readRoguePrefs() {
  Word prefsSize;
  SWord prefsVersion;
  //  Short i;
  //  RoguePreferenceType prefs;
  prefsSize = sizeof(RoguePreferenceType);
  prefsVersion = PrefGetAppPreferences(RogueAppID, RogueAppPrefID, &my_prefs,
				       &prefsSize, true);
  if (prefsVersion > noPreferenceFound) {
    // just to be sure.
    my_prefs.run_walk_border = max(my_prefs.run_walk_border, 1);
    my_prefs.walk_search_border = max(my_prefs.walk_search_border, 1);
    // also, check that it's not an old version; if it is,
    // fix the new stuff.
    if (prefsVersion < RogueAppPrefVersion) {
      my_prefs.tiles_on = false; // new in 1.1.1
    }
    if (prefsVersion < 0x03) {
      my_prefs.color_on = false; // new in 0.46
      my_prefs.crosshairs_on = false; // new in 0.46
    }
    if (prefsVersion < 0x02) {
      my_prefs.autosave_on = false; // new in 0.42 // nah, change to multiuser
      my_prefs.font_small = false; // new in 0.42
      my_prefs.rogue_relative = false; // new in 0.42
    }
    //    if (ancient) {
    //      my_prefs.sound_on = true;
    //      my_prefs.use_hardware = false;
    //      for (i = 0 ; i < 8 ; i++)
    //      my_prefs.hardware[i] = HWB_NOOP;
    //      my_prefs.stay_centered = false; // new in 0.40
    //    }
  }
}
void writeRoguePrefs()
{
  Word prefsSize;
  //  RoguePreferenceType prefs;
  prefsSize = sizeof(RoguePreferenceType);

  PrefSetAppPreferences(RogueAppID, RogueAppPrefID, RogueAppPrefVersion,
			&my_prefs, prefsSize, true);
}

/*****************************************************************************
 *                      StartApplication                                     *
 *****************************************************************************/
Boolean IsVGA = false; // like IsColor.  tells you if you're on a Handera 330.
#ifndef I_AM_OS_2
FontID SmallFont;
FontID BigFont;
#endif

/* moved to the head of the class */
#ifdef I_AM_OS_2
FontPtr oldFontSix = 0;
#endif
Boolean evil = false;
static Word StartApplication(void)
{
  Word error;
#ifdef I_AM_OS_2
  DWord version;
  void *font128 = 0;
#else
  DWord version;
  VoidHand fontHandle;
  FontType *fontPtr;
#endif

  error = OpenDatabase();
  if (error) { evil = true; return error; }
/*   error = checkAppInfo(); */
/*   if (error) return error; */

  /* DO NOT START if there is no rogue library loaded.  so there. */
  //  if (! DmFindDatabase(0, RogueLibraryName))
  //    return 1;

  /* load game and/or do all the ONE-TIME initialization */
  init_game();

  readRoguePrefs();

#ifdef I_AM_OS_2
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  // evil font kludges!
  font128 = MemHandleLock(DmGetResource('NFNT', ItsyFont));
  if (version < 0x03000000L) {
    oldFontSix = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6];
    ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = font128;
  }
#else
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version >= 0x03000000L) {
    UInt32 version2;
    UInt32 defaultFont;
    IsVGA = _TRGVGAFeaturePresent(&version2); // Are we on a Handera 330.
    FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont);
    if (IsVGA) {
      if (VgaIsVgaFont(defaultFont)) { // vga fonts are LARGER
	SmallFont = VgaVgaToBaseFont(defaultFont);
	BigFont = defaultFont;
      } else {
	SmallFont = defaultFont;
	BigFont = VgaBaseToVgaFont(defaultFont);
      }
      FntSetFont(SmallFont);// So the forms OTHER than Main have non-VGA font!!
    } else {
      BigFont = defaultFont;
      fontHandle = DmGetResource('NFNT',ItsyFont);
      fontPtr = MemHandleLock(fontHandle);
      // "user defined fonts start from 129"
      FntDefineFont(129, fontPtr);
      SmallFont = 129;
      // the custom font is uninstalled automatically when we leave iRogue;
      // however, fontHandle MUST remain locked until then.
    }
  }
  // set HasColor boolean to false
#ifdef I_AM_COLOR
  if (version >= 0x03503000L) {
    if (my_prefs.color_on & PREF_COLOR) {
      look_for_memo(); // find and grok 'iRogue-RGB' memo
    }
    if (my_prefs.tiles_on) {
      look_for_tiles(); // look for a database of 9x10 color icons.
    }
  }
#endif // COLOR
#endif // OS 2

  if (my_prefs.font_small) // user likes the small font.
    toggle_itsy(); // might return false, if OS is wrong version; ignore.

#ifndef I_AM_OS_2
  if (IsVGA) {// If Handera, go to hi res mode
    VgaSetScreenMode(screenMode1To1, rotateModeNone);
    init_display_size();
  }
#endif

  FrmGotoForm(MainForm);
  return 0;
}

/*****************************************************************************
 *                      StopApplication                                      *
 *****************************************************************************/
/* Save preferences, close forms, close app database */
extern Char * hit_message;
extern Boolean *rooms_visited;
#define Free_me(a)  h = MemPtrRecoverHandle((a)); if (h) MemHandleFree(h);
static void free_everything() {
  VoidHand h;
  Short i;

  free_old_level();
  Free_me(sotu->level_objects);
  Free_me(sotu->level_monsters);

  Free_me(hit_message);
  Free_me(rooms_visited);

  //  Free_me(sotu->fruit);
  Free_me(sotu->level_points);
  //  for (i = 0 ; i < TRAPS*2 ; i++) {
  //    Free_me(sotu->trap_strings[i]);
  //  }
  for (i = 0 ; i < DROWS ; i++) {
    Free_me(sotu->dungeon[i]);
  }
  Free_me(sotu->dungeon);
  Free_me(sotu->rooms);
  Free_me(sotu->traps);
  Free_me(sotu->id_potions);
  Free_me(sotu->id_scrolls);
  Free_me(sotu->id_weapons);
  Free_me(sotu->id_armors);
  Free_me(sotu->id_rings);
  Free_me(sotu->id_wands);
  Free_me(sotu->is_wood);

  Free_me(sotu->roguep);
  Free_me(sotu);

}

static void StopApplication(void)
{
#ifdef I_AM_OS_2
  DWord version;
  //void *font128 = 0;
#else
  DWord version;
  VoidHand fontHandle;
#endif
  if (evil) return;

#ifdef I_AM_OS_2
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version < 0x03000000L) {
    if (oldFontSix) {
      ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = oldFontSix;
      oldFontSix = 0;
      //      if ((font128 = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6]))
      //	MemHandleUnlock(font128);
    }
  }
#else
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version >= 0x03000000L) {
    if (!IsVGA) {
      fontHandle = DmGetResource('NFNT',ItsyFont);
      MemHandleUnlock(fontHandle);
    }
  }  
#endif // OS 2

  //  if (save_me && my_prefs.save_on_exit)
  if (save_me) { // XXX DEBUGGING
    check_message(); // force clear
    sotu->last_old_message_shown = SAVED_MSGS-1; // no "--more--"
    message("Saving...");
    save_character(0); /* save if not DEAD. */
  }
  
  FrmSaveAllForms();
  FrmCloseAllForms();

  /* it might be nice to CLOSE the DATABASE.  not necessary? */
  if (RogueDB!=NULL) {
    DmCloseDatabase(RogueDB);
    RogueDB = NULL;
  }
  if (RogueTileDB!=NULL) {
    DmCloseDatabase(RogueTileDB);
    RogueTileDB = NULL;
  }
  free_everything();
  /* it might be nice to free some crap, too */
}


/*****************************************************************************
 *                      EventLoop                                            *
 *****************************************************************************/

/* The main event loop */


static void EventLoop(void)
{
    Word err;
    EventType e;
     
    do {
	EvtGetEvent(&e, evtWaitForever);
	// first see if it's a hardware button thing!!!
	// don't ask me what the poweredOnKeyMask is, though; cargo cult.
	// Do special hardware button things only if:
	// it's a hardware button event, [you're alive,] and in the main form.
	if ( (e.eType != keyDownEvent)
	     || !my_prefs.use_hardware 
	     || (e.data.keyDown.modifiers & poweredOnKeyMask)
	     /*	     || (FrmGetActiveFormID() != MainForm) */
	     || !buttonsHandleEvent(&e) )
	  // now proceed with usual handling
	  if (! SysHandleEvent (&e))
	    if (! MenuHandleEvent (NULL, &e, &err))
	      if (! ApplicationHandleEvent (&e))
		FrmDispatchEvent (&e);
    } while (e.eType != appStopEvent);
}

/* Main entry point; it is unlikely you will need to change this except to
   handle other launch command codes */
/*****************************************************************************
 *                      PilotMain                                            *
 *****************************************************************************/
//#define ERROR_CHECK_LEVEL ERROR_CHECK_FULL
DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    Word err;

    /*
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
    Word crc = Crc16CalcBlock(NULL, 256, 0);
#endif
*/
/* #if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL */
/*     DWord startFreeDynamicMemory, endFreeDynamicMemory, junk; */
/*     MemHeapFreeBytes(0, &startFreeDynamicMemory, &junk); */
/* #endif */

    if (cmd == sysAppLaunchCmdNormalLaunch) {

      err = StartApplication();
      if (err) return err;

      EventLoop();
      StopApplication();

    } else {
      return sysErrParamErr;
    }
    /*
#if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL
    ErrNonFatalDisplayIf (crc != Crc16CalcBlock(NULL, 256, 0), 
			  "low Memory Trashed)");
#endif
*/
/* #if ERROR_CHECK_LEVEL == ERROR_CHECK_FULL */
/*     MemHeapFreeBytes(0, &endFreeDynamicMemory, &junk); */
/*     ErrNonFatalDisplayIf (startFreeDynamicMemory != endFreeDynamicMemory, */
/* 			  "Memory Leak Occurred"); */
/* #endif */

    return 0;
}

/*******************************************************************
                    add new stuff below
*******************************************************************/

 
