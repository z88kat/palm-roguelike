/**********************************************************************
 * kMoria - Moria adapted for the PalmPilot.                          *
 * This file is                                                       *
 * copyright (C) 1985-2001 Robert A. Koeneke, Jimmey Wayne Todd,      *
 * James E. Wilson, Bridget Spitznagel. (see BELOW for full credits)  *
 *                                                                    *
 * This program is freeware.  You can redistribute it and/or modify   *
 * it (provided that this notice and the full credits are retained),  *
 * but it CANNOT be distributed for profit.                           *
 **********************************************************************/

// the following notice is not to be removed:
//
/*********************************************************************
 *  FULL CREDITS for Moria:                                          *
 *                                                                   *
 *       Designer and Programmer : Robert Alan Koeneke               *
 *                                 University of Oklahoma            *
 *                                                                   *
 *       Assistant Programmers   : Jimmey Wayne Todd                 *
 *                                 University of Oklahoma            *
 *                                                                   *
 *                                 Gary D. McAdoo                    *
 *                                 University of Oklahoma            *
 *                                                                   *
 *       UNIX Port               : James E. Wilson                   *
 *                                 UC Berkeley                       *
 *                                 wilson@kithrup.com                *
 *                                                                   *
 *       MSDOS Port              : Don Kneller                       *
 *                                 1349 - 10th ave                   *
 *                                 San Francisco, CA 94122           *
 *                                 kneller@cgl.ucsf.EDU              *
 *                                 ...ucbvax!ucsfcgl!kneller         *
 *                                 kneller@ucsf-cgl.BITNET           *
 *                                                                   *
 *       BRUCE Moria             : Christopher Stuart                *
 *                                 Monash University                 *
 *                                 Melbourne, Victoria, AUSTRALIA    *
 *                                 cjs@moncsbruce.oz                 *
 *                                                                   *
 *       Amiga Port              : Corey Gehman                      *
 *                                 Clemson University                *
 *                                 cg377170@eng.clemson.edu          *
 *                                                                   *
 *       Version 5.5             : David Grabiner                    *
 *                                 Harvard University                *
 *                                 grabiner@math.harvard.edu         *
 *                                                                   *
 *       PalmOS Port             : Bridget Spitznagel                *
 *                                 Carnegie Mellon University        *
 *                                 i0lanthe@yahoo.com                *
 *                                                                   *
 *   Moria may be copied and modified freely as long as the above    *
 *   credits are retained.  No one who-so-ever may sell or market    *
 *   this software in any form without the expressed written consent *
 *   of the author Robert Alan Koeneke.                              *
 *                                                                   *
 *********************************************************************/

#include "kMoria.h"

DmOpenRef MoriaDB = NULL;
DmOpenRef MoriaSaveDB = NULL;
DmOpenRef MoriaScoreDB = NULL;

Boolean Version_GE_OS35 = false; // (is OS version >= 3.5)

MoriaPreferenceType my_prefs = {
  6, // run_walk_border
  2, // walk_center_border
  // I may eliminate some of these.
  true,  // big font
  false, // rel move
  false, // circles
  true,  // sound
  true,  // run
  true,  // cut
  true,  // examine
  true, // sector
  false, // doors
  false, // prompt
  false, // weights
  false, // minerals
  false, // stats at top
  // menu thingies
  false, // inv is compact
  false, // inv shows individual weights
  false, // store is compact
  true, // store is haggling
  // Hardware buttons
  false,
  false,
  { 0,0,0,0,0,0,0,0 },
  // Default character name
  "my name",
  false, // invert on
  false, // color on
  0x0, // what commands to auto-repeat
  { 0,0,0,0,0,0,0,0,0,0 }, // what spells to bind on buttons
  false,
  false,
  RESTORE_DEFAULT
};

/*****************************************************************************
 *                                                                           *
 *                  thrilling event handlers                                 *
 *                                                                           *
 *****************************************************************************/
Boolean do_pickup = true;
Boolean found_my_character = false;
extern Char last_command;
extern Boolean just_throwing;
extern Short WalkMagic;

static void reset_state(previous_state *ps) SEC_7;
void bump_state() SEC_7; // form_invselect needs it for zap. sigh
static void end_turn_start_turn();
static Boolean count_handler(Char c);
static void un_more();
//static void un_showinvis();
static void handle_main_menu(EventPtr e);
static Boolean rest_btnsHandleEvent(Short btn_num);
static Boolean buttonsHandleEvent(EventPtr e);
static Boolean OpenDatabase(void);
static LocalID most_recent_savedb();
static Boolean ApplicationHandleEvent(EventPtr e);
static void readMoriaPrefs();
static Word StartApplication(void);
static void StopApplication(void);
static void EventLoop(void);

//Short command_count = 0;
/*
static void game_init()
{

  //#if (COST_ADJ != 100)
  //  price_adjust();
  //#endif

  / Grab a random seed from the clock		/
  //  init_seeds(seed);

  / Init monster and treasure levels for allocate /
  //  init_m_level();
  //  init_t_level();

  / Init the store inventories			/
  // No!  Only if you are a new character!
  //  store_init();

  // do this if the old character dies, too.
  last_command = -1;
}
*/
previous_state prev_state;
previous_state curr_state;
static void reset_state(previous_state *ps)
{
  ps->mode = MODE_DEFAULT;
  ps->cmd = -1;
  ps->dir_cmd = DIR_NONE;
  ps->dir = 5;
  ps->item = -1;
  ps->count = 0;
  ps->spell = -1;
}
void bump_state()
{
  previous_state *ps = &curr_state;
  ps->mode = MODE_AGAIN;
  if (curr_state.cmd != -1)
    prev_state = curr_state;
  // reset_state:
  ps->mode = MODE_DEFAULT;
  ps->cmd = -1;
  ps->dir_cmd = DIR_NONE;
  ps->dir = 5;
  ps->item = -1;
  ps->count = 0;
  ps->spell = -1;
}


// Called by MainForm event handler and by hw buttons event handler.
extern Boolean draw_directional_p;
extern Boolean undraw_directional_p;
static void end_turn_start_turn()
{
  // undraw the directional foo if it needs to be undrawn.
  if (undraw_directional_p) undraw_directional();


  if (!free_turn_flag) {
    bump_state();

    end_of_turn();   // this is what moves the creatures and makes new levels
    // (should I check for death here?  I wonder..)
    start_of_turn(); // this is what makes your own clock tick.
  }
  show_messages();
  refresh(false);
  // how about "if (death) FrmGotoForm..."?
  if (death) {
    // wait for event
    EventType queued_event;
    do {
      EvtGetEvent(&queued_event, 0); // soak up any queued winclose events
    } while (queued_event.eType != nilEvent);
    EvtGetEvent(&queued_event, evtWaitForever);
    do {
      EvtGetEvent(&queued_event, 0); // soak up any accompanying queued events
    } while (queued_event.eType != nilEvent);
    // kill them
    LeaveForm();
    FrmGotoForm(DeathForm);
  }

  if (curr_state.mode == MODE_DEFAULT && message_clear(false))
    curr_state.mode = MODE_MORE;

  if (draw_directional_p) draw_directional();
}

//Boolean stats_toggler = true;
//Short ctr; // this is for testing foo_list
//static Boolean do_command(Char c);
extern Boolean came_from_generation;
extern UInt need_print_stats;



/* For Count Command: 0-9, backspace (BS), delete (DEL) */
#define IS_NUMERIC(c) (('0' <= (c) && (c) <= '9') || 8 == (c) || 127 == (c))

Boolean Main_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false, valid = false;
  FormPtr frm;
  Short tmp_x, tmp_y, dir, dist, edge;
  Char c;
  //Char buf[40]; // for testing

  free_turn_flag = false;// let's try this shall we
  /*
  {
    Char buf[40];
    StrPrintF(buf, "%d %d  %d     ", prev_state.mode, curr_state.mode,
	      curr_state.count);
    WinDrawChars(buf, StrLen(buf), 10, 10);
  }
  */

  // frmOpenEvent is same for all modes
  switch (e->eType) {
  case frmOpenEvent:
    reset_state(&prev_state);
    reset_state(&curr_state);
    frm = FrmGetActiveForm();
    /** place any form-initialization stuff here **/
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);

    // Make new level if needed, display level, display stats, yadda.
    if (came_from_generation) { // was (!found_my_character)
      came_from_generation = false;
      generate_cave();
      enter_new_level();
    } else {
      Char buf[NAMELEN + 10];
      StrPrintF(buf, "Hello, %s", pymisc.name);
      message(buf);
    }
    start_of_turn();
    // just to be really sure:
    get_panel(char_row, char_col, true);
    recalc_and_refresh(); //    recalc_screen();   refresh(true);
    print_stats(STATS_ALL);
    show_messages(); // in case you can learn a spell.

    return true;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

#ifdef I_AM_COLOR
  case menuCmdBarOpenEvent:
    // this will only work if we are on an OS3.5 or greater device.
    if (Version_GE_OS35) {
      MenuCmdBarAddButton(4, bmp_cmd_map, menuCmdBarResultMenuItem,
			  menu_mainMap, NULL);
      MenuCmdBarAddButton(3, bmp_cmd_log, menuCmdBarResultMenuItem,
			  menu_mainMsgs, NULL);
      MenuCmdBarAddButton(2, bmp_cmd_you, menuCmdBarResultMenuItem,
			  menu_mainStats, NULL);
      MenuCmdBarAddButton(1, bmp_cmd_fnt, menuCmdBarResultMenuItem,
			  menu_mainFont, NULL);
    }
    return false;
#endif
  default:
    break;
  } // end FIRST switch e->eType

  // Other modes will have different handlers
  switch(curr_state.mode) {

    /*
  case MODE_SHOWINVIS:
    // Can be entered only from spells.c for detect{invis,monsters,evil}.
    // Check to see if the mode gets cleared too soon by
    // end_turn_start_turn ...
    switch (e->eType) {
    case keyDownEvent: case penDownEvent:
      un_showinvis();
      //      return true;
    default:
      return handled;
    }
    break; // end MODE_SHOWINVIS
    */

  case MODE_MORE:
    // Go into this mode when message_clear(false) returns true.
    // right now, we can only enter it from end_turn_start_turn.. WILL IT WORK?
    switch (e->eType) {
    case keyDownEvent: case penDownEvent:
      un_more();
      return true;
    default:
      return handled;
    }
    break; // end MODE_MORE

  case MODE_SLASH:
    // Can be entered only from do_command: iff '/' was entered
    switch (e->eType) {
    case keyDownEvent:
      ident_monster(e->data.keyDown.chr);
      valid = true;
      break;
    case penDownEvent:
      where_in_dungeon(e->screenX, e->screenY, &tmp_x, &tmp_y);//cnvert to cell
      valid = do_xy_command(tmp_x, tmp_y);
      break;
    default:
      return handled;
    }
    if (valid) bump_state();
    else reset_state(&curr_state);
    show_messages(); // let's see if this fixes my little problem
    return true; // end MODE_SLASH

  case MODE_DIRECTIONAL:
    // Can be entered only from do_command: iff one of "f^cSod?-" was entered.
    /*      if (undraw_directional_p) undraw_directional(); */ // NO.
    switch (e->eType) {
    case keyDownEvent:
      c = e->data.keyDown.chr;
      dir = map_roguedir(&c); // dir==5 if NOOP or N/A
      if (dir == 5) {
	message("Aborted...");
	free_turn_flag = true;
      } else {
	curr_state.dir = dir;
	valid = do_dir_command(dir, 0);
      }
      break;
    case penDownEvent:
      tmp_x = e->screenX - SCR_WIDTH/2;
      tmp_y = e->screenY - SCR_HEIGHT/2;
      dir = convert_to_dir(tmp_x, tmp_y, WalkMagic);
      if (dir == 5 && curr_state.cmd != '?') {// 'aborted' unless 'look'
	message("Aborted...");
	free_turn_flag = true;
      } else {
	curr_state.dir = dir;
	valid = do_dir_command(dir, 0);
      }
      break;
    default:
      return handled;
    }
    //    if (valid) bump_state();
    //    else reset_state(&curr_state); // end_turn_start_turn should do it..
    // but wait, '?' doesn't take time, does it???  e_t_s_t will only
    // bump_state for things that don't get a free turn flag.  gar.
    if (valid) bump_state();
    else reset_state(&curr_state); // mumble paradigm mumble...
    handled = true;
    break; // end MODE_DIRECTIONAL

  case MODE_DEFAULT:
    // Mode gets set back to this every time we end a time-taking command.
    switch (e->eType) {
    case menuEvent:
      handle_main_menu(e);
      return true;

    case keyDownEvent:
      c = e->data.keyDown.chr;
      handled = do_command(c);
      if (!handled) {
	if (IS_NUMERIC(c)) {
	  handled = count_handler(c);
	  command_count = curr_state.count;
	} else {
	  return handled;
	}
      }
      break;

    case penDownEvent:
      tmp_x = e->screenX - SCR_WIDTH/2;
      tmp_y = e->screenY - SCR_HEIGHT/2;
      if (my_prefs.relative_move) {
	// translate tap coordinates so that rogue's screen location = 0,0.
	relativize_move(&tmp_x, &tmp_y);
      }
      dist = tmp_x * tmp_x + tmp_y * tmp_y;
      edge = my_prefs.walk_center_border*WalkMagic;
      edge *= edge;
      if (dist < edge) {
	free_turn_flag = true;
      } else {
	dir = convert_to_dir(tmp_x, tmp_y, WalkMagic);
	do_feep(200,9);	// make a tap sound
	curr_state.cmd = unmap_roguedir(dir);
	curr_state.dir = dir;
	edge = my_prefs.run_walk_border*WalkMagic;
	edge *= edge;
	if (!my_prefs.run || (dist < edge) || hit_dont_run(dir))
	  move_char(dir, do_pickup); // walk or attack
	else { // run
	  find_init(dir);
	  if (find_flag > 0) find_run();
	}
      }
      do_pickup = true;
      curr_state.count = command_count = 0;
      handled = true;
      break;
    
    default:
      //return true; // ah maybe this is the Shortcut Bug.
      return handled;
    } // end switch event type!
  } // end my_mode switch

  // for anyone who has not returned,
  // call something to move the monsters.

  end_turn_start_turn();
  return handled;
}
/*****************************************************************************/
static Boolean count_handler(Char c)
{
  Char buf[16];
  Short n = curr_state.count;
  free_turn_flag = true;
  if ('0' <= c && c <= '9') {
    Short incr = c - '0';
    if (n <= 0)        n = incr;
    else if (n <= (MAX_COUNT/10))  n = n * 10 + incr;
    else               n = MAX_COUNT; /* maxed-out */
    StrPrintF(buf, "count: %d", n);
    count_message(buf);
  } else if (8 == c || 127 == c) {   // backspace (BS), delete (DEL)
    n = n / 10; // yay integer division.
    if (n) {
      StrPrintF(buf, "count: %d", n);
      count_message(buf);
    } else {
      print_stats(STATS_WORD); // maybe this will erase it
      curr_state.mode = MODE_DEFAULT;
    }
  }
  curr_state.count = n;
  return true;
}
// Show next message and switch from more state to default state if no more.
static void un_more()
{
  show_messages();
  if (!message_clear(false))
    curr_state.mode = MODE_DEFAULT;
  // else there are still more messages, so stay in "more" mode.
}
/*****************************************************************************/
// xxx fix this, I haven't touched it yet.
static void handle_main_menu(EventPtr e)
{
  Short cmd;
  Boolean handled; // do I need this?
#ifdef I_AM_COLOR
  DWord version;
#endif
  MenuEraseStatus(NULL); /* clear menu status from display */
  free_turn_flag = true;
  switch(e->data.menu.itemID) {
  case menu_mainInstruct:        FrmHelp(InstructStr);     break;
  case menu_mainScores:     FrmPopupForm(ScoresForm);      break;
  case menu_mainMap:        FrmPopupForm(MapForm);         break;
  case menu_mainSettings:   FrmPopupForm(PrefsForm);       break;
  case menu_mainButtons:    FrmPopupForm(HwButtonsForm);   break;
  case menu_mainRepeaters:  FrmPopupForm(RepeatCmdsForm);  break;
  case menu_mainSpellBtns:  FrmPopupForm(SpellHwBtnsForm); break;
  case menu_mainMsgs:       FrmPopupForm(MsgLogForm);      break;
  case menu_mainAbout:      FrmPopupForm(AboutForm);       break;
    /*
  case menu_mainAbout:
    {
    // testing compact_{monster,object}
      Char buf[80];
      StrPrintF(buf, "mon: %d  obj: %d", mfptr, tcptr);
      message(buf);
    }
    break;
    */
    /*
  case menu_mainAbout:
    {
    // testing how long save_char takes
    Char buf[80];
    ULong t0, t1;
    t0 = TimGetTicks();
    save_char(); save_char(); save_char(); save_char(); save_char();
    save_char(); save_char(); save_char(); save_char(); save_char();
    t1 = TimGetTicks();
    StrPrintF(buf, "%ld - %ld = %ld", t1, t0, t1 - t0);
    WinDrawChars(buf, StrLen(buf), 10, 10);
    break;
    }
    */
  case menu_mainUser:
    save_char(); // maybe dangerous
    FrmPopupForm(UserForm);
    break;
  case menu_mainTime:
    //    pymisc.exp = MAX_EXP; // xxxx debugging, remove me!!!!
    //    pymisc.au = MAX_LONG; // xxxx debugging, remove me!!!!
    show_time_and_volts();
    break;
  case menu_mainStats:
    came_from_generation = false;
    FrmPopupForm(CharSummaryForm);
    break;
  case menu_cmd_quit:
    // quit is not accessible by 'Q'... this is a "feature": typo<-->'G'
    // to make it 'Q'able instead, move this case into dungeon.c's do_command.
    // I think I'll make it /Q-able though.
    if (0 == FrmAlert((!total_winner)?QuitP:RetireP)) {
      free_turn_flag = false; //?
      death = true;
      StrCopy(died_from, "Quitting"); // form_death will change for winners
      LeaveForm();
      FrmGotoForm(DeathForm);
      return;
    }
    break;
  case menu_mainFont:
    toggle_itsy();
    get_panel(char_row, char_col, true); // FORCE recalculation of panel
    recalc_and_refresh(); //    recalc_screen();   refresh(true);
    break;
  case menu_mainBeam: // XXX Make sure to test this, in the old xcopilot.
#ifdef I_AM_COLOR
    FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
    if (version < 0x03000000l)  message("can't beam in OS2");
    else  FrmPopupForm(BeamForm);
#else
    message("can't beam in OS2 version");
#endif
    break;
  default:
    free_turn_flag = false;
    // MAGIC & MORE MAGIC..
    cmd = e->data.menu.itemID - MAGIC_MENU_NUMBER;
    if (cmd > 0 && cmd < 128)
      handled = do_command((Char) cmd);
    //    if (handled && !free_turn_flag) command_count = 0; // XXX
    curr_state.count = command_count = 0;
    break;
  }
  end_turn_start_turn();
}
/*****************************************************************************/
// P.I.T.A. ...
void rest_on(Short rest_num);
Boolean Rest_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  //  FieldPtr fld;
  Short rest_num = 0;
    
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

  case keyDownEvent:
    {
      Word objid = -1;
      frm = FrmGetActiveForm();
      switch(e->data.keyDown.chr) {
      case '0': objid = btn_rest_oops; break;
      case '1': objid = btn_rest_10; break;
      case '2': objid = btn_rest_20; break;
      case '3': objid = btn_rest_max; break;
      }
      if (objid != -1)
	hit_button_if_usable(frm, objid);
    }
    break;

  case ctlSelectEvent:
    rest_num = 0;
    switch(e->data.ctlSelect.controlID) {
    case btn_rest_oops:
      LeaveForm();
      handled = true;
      break;
    case btn_rest_10:  rest_num = 10;         break;
    case btn_rest_20:  rest_num = 20;         break;
    case btn_rest_max: rest_num = -MAX_SHORT; break;
    }
    if (rest_num != 0) {
      free_turn_flag = false;
      LeaveForm();
      rest_on(rest_num);
      {
	// Here we are soaking up a winExit, winEnter, and penUp.  sigh.
	// Necessary now since I have changed user_interrupted().
	EventType queued_event;
	do {
	  EvtGetEvent(&queued_event, 0);
	} while (queued_event.eType != nilEvent);
      }
      end_turn_start_turn(); // gotta kick-start the resting...
      handled = true;
    }
    break;

  default:
    break;
  }

  return handled;
}
static Boolean rest_btnsHandleEvent(Short btn_num) // 0, 1, 2, 3.
{
  Short rest_num = 0;
  if (btn_num < 0 || btn_num > 3) return false; // out of bounds
  switch(btn_num) {
    // case 0: rest_num stays 0
  case 1:  rest_num = 10;         break;
  case 2:  rest_num = 20;         break;
  case 3:  rest_num = -MAX_SHORT; break;
  }
  LeaveForm();
  if (rest_num != 0) {
    free_turn_flag = false;
    rest_on(rest_num);
    {
      // Here we are soaking up a winExit and winEnter.  sigh.
      EventType queued_event;
      do {
	EvtGetEvent(&queued_event, 0);
      } while (queued_event.eType != nilEvent);
    }
    end_turn_start_turn(); // gotta kick-start the resting...
  }
  return true;
}
/*****************************************************************************/

//static Char hwb_commands[HWB_last+1] = {  0 , 'k', 'j', 'l', 'h',
//					  'M', '?', 'i', 'm', 's',
//					  '#', 'r', 'd', 'f' };
/*static Char hwb_commands[HWB_last+2] = "\0kjlhM?ims#rdf"*/
const Char hwb_commands[HWB_last+2] = "\0kjlhM?ims#rdfocSF!xtaz/-g@";

// Need to revise this for the state/mode paradigm!!
static Boolean buttonsHandleEvent(EventPtr e)
{
  Boolean handled = false, valid = false;
  //  Boolean took_time = true;
  Short btn, dir, dispatch_type = 0;
  Word curfrm;
  //  Word which_form;
  Char command;
  Boolean capslock_on, numlock_on, autoshifted;
  Short tmp_shift;

  // I've decided that "if (!(ChrIsHardKey(e->data.keyDown.chr)) ..)" is 
  // not so good, since that includes hardPowerChr, hardCradleChr etc.!
  if ( ((e->data.keyDown.chr < hard1Chr) || (e->data.keyDown.chr > hard4Chr))
       && (e->data.keyDown.chr != calcChr)
       && (e->data.keyDown.chr != findChr)
       && (e->data.keyDown.chr != pageUpChr)
       && (e->data.keyDown.chr != pageDownChr))
    return false; // it's NOT a hardware button.. probably graffiti.

  curfrm = FrmGetActiveFormID();

  // Ok, we are dealing now ONLY with the 8 keys that we might rebind.
  // IF we are in main form, then rebind what we're rebinding.
  // ELSE if it is rebound in main form, just MASK it.

  // <incs>/UI/Chars.h is useful....   ChrIsHardKey(c)
  // hard[1-4]Chr, calcChr, findChr.
  switch (e->data.keyDown.chr) {
    // this just maps it from wacky-button-char to '0...7'.
  case hard1Chr:     btn = HW_hard1Chr   ;  break;      // datebook
  case hard2Chr:     btn = HW_hard2Chr   ;  break;      // address
  case hard3Chr:     btn = HW_hard3Chr   ;  break;      // todo
  case hard4Chr:     btn = HW_hard4Chr   ;  break;      // memos
  case pageUpChr:    btn = HW_pageUpChr  ;  break;
  case pageDownChr:  btn = HW_pageDownChr;  break;
  case calcChr:      btn = HW_calcChr    ;  break;
  case findChr:      btn = HW_findChr    ;  break;
  default:
    return false;
  }

  if (curfrm == SpellForm) {
    if (spell_btnsHandleEvent(btn)) return true;
  } else if (curfrm == BookForm) {
    if (book_btnsHandleEvent(btn)) return true;
  } else if (curfrm == RestForm) {
    if (rest_btnsHandleEvent(btn)) return true;
  } else if (curfrm == LookForm) {
    if (look_btnsHandleEvent(btn)) return true;
  } else if (curfrm == RecallForm) {
    if (btn >= HW_hard1Chr && btn <= HW_hard4Chr) {
      LeaveForm(); return true;
    }
  }

  dispatch_type = my_prefs.hardware[btn];

  // tapping the 'goto map' button a second time will EXIT the map.. rah.
  if ((curfrm == MapForm) && (dispatch_type == HWB_MAP)) {
    LeaveForm();
    return true;
  }

  if (curfrm != MainForm) {
    // IF the key is not bound, return false, else return TRUE to MASK it.
    // unless it is up/down, of course.
    return ( dispatch_type != HWB_NOOP &&
	     e->data.keyDown.chr != pageUpChr &&
	     e->data.keyDown.chr != pageDownChr );
  }

  // OK, we ARE in the main form.  

  if (dispatch_type < HWB_first || dispatch_type > HWB_last)
    return false;

  command = hwb_commands[dispatch_type];
  if (command == 'm' && class[pymisc.pclass].spell == PRIEST) command = 'p';

  if (dispatch_type >= 1 && dispatch_type <= 4) { // "kjlh"
    // This part is just like map_roguedir...
    GrfGetState(&capslock_on, &numlock_on, &tmp_shift, &autoshifted);
    if ((tmp_shift == grfTempShiftUpper) || capslock_on)
      command -= 'a' - 'A';
    // ... except we need to "turn off" the temp shift afterwards (if any).
    GrfSetState(capslock_on, numlock_on, false);
  }

  switch(curr_state.mode) {
    /*
  case MODE_SHOWINVIS:
    un_showinvis();
    //    return true;
    */
  case MODE_MORE:
    un_more();
    return true;
  case MODE_SLASH:
    return true; // do nothing
  case MODE_DIRECTIONAL:
    // Act like keyDownEvent in Main_Form_HandleEvent
    /*      if (undraw_directional_p) undraw_directional(); */ // NO.
    dir = map_roguedir(&command); // dir==5 if NOOP or N/A
    if (dir == 5) {
      message("Aborted...");
      free_turn_flag = true;
    } else {
      curr_state.dir = dir;
      valid = do_dir_command(dir, 0);
    }
    if (valid) bump_state();
    else reset_state(&curr_state); // mumble paradigm mumble...
    handled = true;
    break;
  case MODE_DEFAULT:
    if (command == 'F') {
      // just like "case menu_mainFont" in main form event handler
      toggle_itsy();
      get_panel(char_row, char_col, true); // FORCE recalculation of panel
      recalc_and_refresh(); //    recalc_screen();   refresh(true);
      free_turn_flag = true;
      handled = true;
    } else if (command == '@') {
      // act like we toggled the CapsLock
      GrfGetState(&capslock_on, &numlock_on, &tmp_shift, &autoshifted);
      GrfSetState(!capslock_on, numlock_on, false);
      return true;
    } else {
      // Act like keyDownEvent in Main_Form_HandleEvent
      handled = do_command(command);
    }
    break;
  }

  end_turn_start_turn();
  return handled;
  // Well, let's see if this works.
}


/*****************************************************************************
 *                                                                           *
 *                      OpenDatabase                                         *
 *                                                                           *
 *****************************************************************************/
static Boolean OpenDatabase(void)
{
  // The '0' arguments are all 'card number'.
  // Note I have two databases of the same type now, so
  //  DmOpenDatabaseByTypeCreator is not going to work.
  LocalID dbID;
  Boolean created = false;

  // We cannot run if we cannot find the MoriaDB, so return error.
  // hm, shouldn't it be read-only?
  if (0 == (dbID = DmFindDatabase(0, MoriaDBName))) return 1;
  if (0 == (MoriaDB = DmOpenDatabase(0, dbID, dmModeReadWrite))) return 1;

  // We can run if we cannot find the MoriaScoreDB.  Just create it.
  if (0 == (dbID = DmFindDatabase(0, MoriaScoreDBName))) {
    if (DmCreateDatabase(0, MoriaScoreDBName, MoriaAppID, MoriaScoreDBType,
			 false))
      return 1;
    if (0 == (dbID = DmFindDatabase(0, MoriaScoreDBName))) return 1;
    toggle_hotsync(dbID, true);
  }
  if (0 == (MoriaScoreDB = DmOpenDatabase(0, dbID, dmModeReadWrite))) return 1;


  // We can run if we cannot find the MoriaSaveDB.  Just create it.
  if (my_prefs.determine_recent_user & RESTORE_RECENT)
    dbID = most_recent_savedb();
  else
    dbID = DmFindDatabase(0, MoriaSaveDBName);
  if (0 == dbID) {
    if (DmCreateDatabase(0, MoriaSaveDBName, MoriaAppID, MoriaSaveDBType,
			 false))
      return 1;
    created = true;
    if (0 == (dbID = DmFindDatabase(0, MoriaSaveDBName))) return 1;
    toggle_hotsync(dbID, true);
  }
  if (0 == (MoriaSaveDB = DmOpenDatabase(0, dbID, dmModeReadWrite))) return 1;

  if (DmNumRecords(MoriaSaveDB) < 1) {
    data_record_create();
  }

  return 0;
}


static LocalID most_recent_savedb()
{
  DmSearchStateType state;
  UInt card;
  LocalID currentDB = 0, most_recent = 0;
  Err err;
  ULong modified_date, mru_date = 0;

  err = DmGetNextDatabaseByTypeCreator(true, &state,
				       MoriaSaveDBType, MoriaAppID,
				       false, &card, &currentDB);
  while (!err && currentDB) {
    if (card == 0) { // (I hope this rules out freakishness) take note of db.
      DmDatabaseInfo(card, currentDB, NULL, NULL, NULL, NULL, &modified_date,
		     NULL, NULL, NULL, NULL, NULL, NULL);
      if (modified_date > mru_date) {
	mru_date = modified_date;
	most_recent = currentDB;
      }
    }
    // get the next DB
    err = DmGetNextDatabaseByTypeCreator(false, &state, 
					 MoriaSaveDBType, MoriaAppID,
					 false, &card, &currentDB);
  }
  return most_recent;
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
	case SnowCrashForm:
	    FrmSetEventHandler(frm, SnowCrash_Form_HandleEvent);
	    break;
	case AboutForm:
	    FrmSetEventHandler(frm, About_Form_HandleEvent);
	    break;
	case IntroForm:
	    FrmSetEventHandler(frm, Intro_Form_HandleEvent);
	    break;
	case MapForm:
	    FrmSetEventHandler(frm, Map_Form_HandleEvent);
	    break;
	case PrefsForm:
	    FrmSetEventHandler(frm, Prefs_Form_HandleEvent);
	    break;
	case HwButtonsForm:
	    FrmSetEventHandler(frm, HwButtons_Form_HandleEvent);
	    break;
 	case RepeatCmdsForm:
 	    FrmSetEventHandler(frm, RepeatCmds_Form_HandleEvent);
 	    break;
	case SpellHwBtnsForm:
	    FrmSetEventHandler(frm, SpellHwBtns_Form_HandleEvent);
	    break;
	case MsgLogForm:
	    FrmSetEventHandler(frm, MsgLog_Form_HandleEvent);
	    break;
	case RestForm:
	    FrmSetEventHandler(frm, Rest_Form_HandleEvent);
	    break;
#ifdef I_AM_COLOR
	case BeamForm:
	    FrmSetEventHandler(frm, Beam_Form_HandleEvent);
	    break;
	case BeamCashForm:
	    FrmSetEventHandler(frm, BeamCash_Form_HandleEvent);
	    break;
#endif
	case UserForm:
	    FrmSetEventHandler(frm, User_Form_HandleEvent);
	    break;
	case InvForm:
	    FrmSetEventHandler(frm, Inv_Form_HandleEvent);
	    break;
	case EquForm:
	    FrmSetEventHandler(frm, Equ_Form_HandleEvent);
	    break;
	case InvSelectForm:
	    FrmSetEventHandler(frm, InvS_Form_HandleEvent);
	    break;
	case ScribeForm:
	    FrmSetEventHandler(frm, Scribe_Form_HandleEvent);
	    break;
	case RecallForm:
	    FrmSetEventHandler(frm, Recall_Form_HandleEvent);
	    break;
	case LookForm:
	    FrmSetEventHandler(frm, Look_Form_HandleEvent);
	    break;
	case StoreForm:
	    FrmSetEventHandler(frm, Store_Form_HandleEvent);
	    break;
	case HaggleForm:
	    FrmSetEventHandler(frm, Haggle_Form_HandleEvent);
	    break;
	case BookForm:
	    FrmSetEventHandler(frm, Book_Form_HandleEvent);
	    break;
	case SpellForm:
	    FrmSetEventHandler(frm, Spell_Form_HandleEvent);
	    break;
	case DalekForm:
	    FrmSetEventHandler(frm, Dalek_Form_HandleEvent);
	    break;
	case GainSpForm:
	    FrmSetEventHandler(frm, GainSp_Form_HandleEvent);
	    break;
	case DeathForm:
	    FrmSetEventHandler(frm, Death_Form_HandleEvent);
	    break;
	case CrownForm:
	    FrmSetEventHandler(frm, Crown_Form_HandleEvent);
	    break;
	case ScoresForm:
	    FrmSetEventHandler(frm, Scores_Form_HandleEvent);
	    break;
       // Generation stuff:
	case CharRaceForm:
	    FrmSetEventHandler(frm, CharRace_Form_HandleEvent);
	    break;
	case CharGenderForm:
	    FrmSetEventHandler(frm, CharGender_Form_HandleEvent);
	    break;
	case CharClassForm:
	    FrmSetEventHandler(frm, CharClass_Form_HandleEvent);
	    break;
	case CharSummaryForm:
	    FrmSetEventHandler(frm, CharSummary_Form_HandleEvent);
	    break;
	}
	handled = true;
    }

    return handled;
}

/*****************************************************************************
 *                      Preferences                                          *
 *****************************************************************************/

static void readMoriaPrefs()
{
  Word prefsSize;
  SWord prefsVersion;

  prefsSize = sizeof(MoriaPreferenceType);
  prefsVersion = PrefGetAppPreferences(MoriaAppID, MoriaAppPrefID, &my_prefs,
				       &prefsSize, true);
  if (!my_prefs.big_font)
    toggle_itsy();

  if (prefsVersion < MoriaAppPrefVersion) {
    // hmm... remember to zero out the new space ...
    if (prefsVersion < 0x03) {
      my_prefs.no_animations = my_prefs.unused = false;
      my_prefs.determine_recent_user = RESTORE_DEFAULT;
    }
    writeMoriaPrefs();
  }
}
void useMoriaPrefs()
{
  find_cut = my_prefs.run_cut_corners;
  find_examine = my_prefs.run_examine_corners;
  find_prself = true;
  find_bound = my_prefs.run_stop_sector;
  find_ignore_doors = my_prefs.run_thru_doors;
  prompt_carry_flag = my_prefs.prompt_pickup; // might wire this to false
  //  rogue_like_commands = true; // this is not used and also only quasitrue
  //  show_weight_flag = my_prefs.show_weights; // this is not used I guess
  highlight_seams = my_prefs.highlight_minerals;
  //  sound_beep_flag = false;
  //  display_counts = false;
  set_statsbar_loc(my_prefs.stats_at_top);
}
void writeMoriaPrefs()
{
  Word prefsSize;

  prefsSize = sizeof(MoriaPreferenceType);
  PrefSetAppPreferences(MoriaAppID, MoriaAppPrefID, MoriaAppPrefVersion,
			&my_prefs, prefsSize, true);
}




/*****************************************************************************
 *                      StartApplication                                     *
 *****************************************************************************/
Boolean IsVGA = false; // like IsColor.  tells you if you're on a Handera 330.
#ifdef I_AM_OS_2
FontPtr oldFontSix = 0;
#else
FontID SmallFont, MyFont;
FontID BigFont;
FontType *fontPtr; // was in StartApplication
#endif
Boolean evil = false;
Short snowcrash = 0;
extern Boolean userform_first;
void StartApp_helper()
{
  userform_first = false;
  found_my_character = load_char();

  came_from_generation = !found_my_character;
  if (recover_crash()) // 'true' if we need to recover from a crash
    if (found_my_character) {
      if (0 == FrmAlert(CrashP)) {
	// "Just Quit"
	death = true;
	StrCopy(died_from, "software bug");
	FrmGotoForm(DeathForm);
	return;
      } else {
	// "Recover Me"
	came_from_generation = true; // Generate the level all over again! ha!
	dun_level = 0; // Send you to the town!  It's a safe place...
	// DANGER DANGER
	// There may still be inconsistent counters, e.g. inventory, equipment!
	// But you may consider this resuscitation worth the risk.
      }
    } // else there was no save file, no need for recovery

  if (found_my_character) {
    FrmGotoForm(MainForm);
  } else {
    re_init(); // we could be called by form_user.
    FrmGotoForm(CharRaceForm);
    FrmPopupForm(IntroForm); // let's try this puppy out
  }
  return;
}

static Word StartApplication(void)
{
  Word error = 0;
  DWord version;
#ifdef I_AM_OS_2
  void *custom_font = 0;
#else
  VoidHand fontHandle;
  //  FontType *fontPtr; // Moved out to a global
#endif

  Version_GE_OS35 = false;

  // Deal with the font.
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  // evil font kludges!
  custom_font = MemHandleLock(DmGetResource('NFNT', ItsyFont));
  if (version < 0x03000000L) {
    // Note: POSE will warn you that this line is a bad idea:
    oldFontSix = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6];
    ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = custom_font;
  } else {
    snowcrash = 1;
    evil = true;
  }
#else
  if (version >= 0x03503000L) {
    Version_GE_OS35 = true;
  }
  if (version >= 0x03000000L) {
    UInt32 version2;
    UInt32 defaultFont;
    // Figure out whether we are on a Handera.
    IsVGA = _TRGVGAFeaturePresent(&version2);
    // Get a pointer to what is the "default" font.
    FtrGet(sysFtrCreator, sysFtrDefaultFont, &defaultFont);
    // Load up our custom font. "user defined fonts start from 129"
    fontHandle = DmGetResource('NFNT',ItsyFont);
    fontPtr = MemHandleLock(fontHandle);
    FntDefineFont(USERFONT, fontPtr);
    MyFont = USERFONT;
    // the custom font is uninstalled automatically when we leave the app;
    // however, fontHandle must remain locked until then.
    if (IsVGA) {
      if (VgaIsVgaFont(defaultFont)) { // vga fonts are 50% LARGER
	SmallFont = VgaVgaToBaseFont(defaultFont);
	BigFont = defaultFont;
      } else {
	SmallFont = defaultFont;
	BigFont = VgaBaseToVgaFont(defaultFont);
      }
      //FntSetFont(SmallFont);// So the forms OTHER than Main have non-VGA font
    } else {
      BigFont = defaultFont;
      SmallFont = USERFONT;
    }
  } else {
    snowcrash = 2;
    evil = true;
  }
#endif

  readMoriaPrefs();
  // Return error if we don't have MoriaDB.  Open or create MoriaSaveDB.
  error = OpenDatabase();
  if (error && !evil) {
    snowcrash = 3;
    evil = true;
  }
  if (evil) {
    FrmGotoForm(SnowCrashForm);
    return 0;
  }

#ifndef I_AM_OS_2
  if (IsVGA) {// If Handera, go to hi res mode
    VgaSetScreenMode(screenMode1To1, rotateModeNone);
    init_display_size(); // xxx it's not setting the global arrays right.
  }
  //  init_display_size(); // xxx JUST FOR TESTING!!!
  // xxx Move that call back into the ifIsVGA body as soon as I get
  // indication that the globals are no longer scrod.
#endif

  alloc_message_log();
  useMoriaPrefs();

  data_record_lock(true);
  userform_first = false;
  if (my_prefs.determine_recent_user & RESTORE_PROMPT) {
    userform_first = true;
    FrmGotoForm(UserForm);
  } else {
    // Lock the various database records; also take care of random seed.
    StartApp_helper();
  }
  
  return 0;
}


/*****************************************************************************
 *                      StopApplication                                      *
 *****************************************************************************/
#define free_me(a)  h = MemPtrRecoverHandle((a)); if (h) MemHandleFree(h);

static void StopApplication(void)
{
  //  VoidHand h;
  DWord version;
#ifdef I_AM_OS_2
  //void *custom_font = 0;
#else
  VoidHand fontHandle;
#endif

  if (evil) return;

  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  if (version < 0x03000000L) {
    if (oldFontSix) {
      ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6] = oldFontSix;
      oldFontSix = 0;
      //      if ((custom_font = ((UIGlobalsPtr)GUIGlobalsP)->uiFontTable[6]))
      //	MemHandleUnlock(custom_font);
    }
  }
#else
  if (version >= 0x03000000L) {
    //    if (!IsVGA) {
    fontHandle = DmGetResource('NFNT',ItsyFont);
    MemHandleUnlock(fontHandle);
    //    }
  }  
#endif

  // try to save!  woohoo.  but I should only do this if you're not dead...
  if (!userform_first) {
    if (!death) save_char(); // returns false on failure; ignored for now.
    else        antisave_char(); // delete old save file, if any

    didnt_crash();
  }

  data_record_lock(false);
  // possibly I should free some things too.

  /*
    TO DO:
    Call function to delete a You Crashed record
    because we have exited gracefully
  */

  FrmSaveAllForms();
  FrmCloseAllForms();

  if (MoriaDB!=NULL) {
    DmCloseDatabase(MoriaDB);
    MoriaDB = NULL;
  }
  if (MoriaSaveDB!=NULL) {
    DmCloseDatabase(MoriaSaveDB);
    MoriaSaveDB = NULL;
  }
  // free stuff

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
	// Do special hardware button things if it's a hardware button event.
	// don't ask me what the poweredOnKeyMask is, though; cargo cult.
	if ( (e.eType != keyDownEvent)
	     || !my_prefs.use_hardware 
	     || (e.data.keyDown.modifiers & poweredOnKeyMask)
	     // || (FrmGetActiveFormID() != MainForm)
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
#ifdef I_AM_COLOR
Err beam_receive(ExgSocketPtr socketPtr);
#endif
DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    Word err;

    switch(cmd) {
    case sysAppLaunchCmdNormalLaunch:

      err = StartApplication();
      if (err) return err;

      EventLoop();
      StopApplication();

      break;

#ifdef I_AM_COLOR
    case sysAppLaunchCmdSyncNotify: // register to receive beamed mime types
      {
	DWord romVersion;
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion >= 0x03000000) {
	  ExgRegisterData(MoriaAppID, exgRegTypeID, MY_MIMETYPE);
	}
      }
      break;
    case sysAppLaunchCmdExgReceiveData:
      // I suppose we only get this if we're the right rom version anyway?
      if ((launchFlags & sysAppLaunchFlagSubCall) != 0)
	err = beam_receive((ExgSocketPtr) cmdPBP);
      else
	err = exgErrTargetMissing; // user was not in kMoria; refuse delivery.
      if (err) return err;
      break;
    case sysAppLaunchCmdExgAskUser:
      // Check whether we were "running" kMoria already.
      // If so, we have access to globals (and will accept beams without
      // asking the user!)  If not... we are not going to accept anything!
      if ((launchFlags & sysAppLaunchFlagSubCall) != 0)
	((ExgAskParamPtr)cmdPBP)->result = exgAskOk;
      else
	((ExgAskParamPtr)cmdPBP)->result = exgAskCancel;
      break;
#endif

      default:
	return sysErrParamErr;
      }

    return 0;
}
