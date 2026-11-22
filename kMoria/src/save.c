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

static void create_save_structs() SEC_5;
static Boolean create_py_rec() SEC_5;
static Boolean read_py_rec() SEC_5;
static void read_save_structs() SEC_5;

// 16 Jun 2001 - trying to figure out what I was up to last month.
// looks like I was implementing both of:
// add a boolean to py_rec
// add a new "boolean" record
// I will go with the latter (simpler, lamer) and see if I can make it work.
// Commenting out the former stuff.
// The crash record is essentially like a lock file (e.g. netscape).

// This function is called as soon as we have found the save database.
// This function will return "true" if a crash record exists, that is,
// if the last game session crashed.  It will return false, and create
// a crash record, otherwise.
// ...If I were smart I would have save files carry version info too.
Boolean recover_crash()
{
  ULong uniqueID, mydate;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  // This should return true IFF there exists a "crash" record.
  uniqueID = REC_CRASHED;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    return true;
  // If a crash record does NOT exist, it should CREATE it and return false.
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, sizeof(ULong));
  p = MemHandleLock(vh);
  mydate = TimGetSeconds(); // (for no particular reason.  it's not used.)
  if (p) {
    DmWrite(p, 0, &mydate, sizeof(ULong));
  }
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  return false;
}

// This function is called when we exit the game "gracefully".
// It will delete the crash record.  We expect that this is not called
// when the game crashes, thus, if the crash record exists at game start,
// then the previous session crashed.
void didnt_crash()
{
  ULong uniqueID;
  UInt index;
  // Delete the "crash" record to show that we didn't crash.
  uniqueID = REC_CRASHED;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    // this is the expected case.  there's a bug if we don't find it.
    DmRemoveRecord(MoriaSaveDB, index);
}

void rest_on(Short rest_num);
void maybe_checkpoint()
{
  // Call this every thousand turns or so, and maybe when you ascend/descend
  // or gain a level/spell or things like that.
  Short old_command_count = command_count;
  Short old_pack_heavy = pack_heavy;
  Short old_rest = pyflags.rest;
  Boolean old_find_flag = find_flag;
  Boolean was_searching = ((pyflags.status & PY_SEARCH) != 0);

  save_char();
  // Undo the things that save_char did (hm does order matter?)
  command_count = old_command_count;
  pack_heavy = old_pack_heavy; change_speed(pack_heavy);
  find_flag = old_find_flag;
  if (old_rest) rest_on(old_rest);
  if (was_searching) search_on(); // "searching" message will flash; alas.
}

Boolean save_char()
{
  if (MoriaSaveDB == NULL) return false;

  disturb(true, false);		/* Turn off resting and searching. */
  change_speed(-pack_heavy);	/* Fix the speed */
  pack_heavy = 0;

  create_save_structs();
  // maybe need to write bytes:  major version, minor version, patchlevel
  return create_py_rec(); // yay.
}

Boolean load_char()
{
  Short sversion = get_savefile_version();
  Boolean found_me = false; // saved char does not exist..

  if (read_py_rec()) {
    read_save_structs();
    found_me = true;
  }

  // Check the save file version.
  if (sversion < MoriaSaveDBVersion)
    perform_conversion(sversion, found_me);

  return found_me;
}


// Dude.  I get a fatal error when the record does not exist.  WHY.
void antisave_char()
{
  ULong uniqueID;
  UInt index;

  uniqueID = REC_PY;
  if (MoriaSaveDB != NULL)
    if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
      DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
}

typedef struct my_save_struct
{
  ULong spell_learned;
  ULong spell_worked;
  ULong spell_forgotten;
  ULong town_seed; // skip randes_seed
  ULong current_time; // use TimGetWhatever
  ULong birth_date; // in create.c
  Long turn;
  Long total_points; // call total_points() to calc this
  Short missile_ctr;
  Short inven_ctr;
  Short inven_weight;
  Short equip_ctr;
  Short tcptr;
  Short mfptr;
  Boolean total_winner;
  Boolean death;
  // Skipping: panic_save, noscore.
  Char died_from[25];
  // Also save the previous-message-foo!
} my_save;
my_save my_save_info;
typedef struct floor_save_struct
{
  Short dun_level;
  Short char_row;
  Short char_col;
  Short mon_tot_mult;
  Short cur_height;
  Short cur_width;
  Short max_panel_rows;
  Short max_panel_cols;
} floor_save;
floor_save floor_save_info;


static void create_save_structs()
{
  my_save_info.spell_learned = spell_learned;     
  my_save_info.spell_worked = spell_worked;
  my_save_info.spell_forgotten = spell_forgotten;
  my_save_info.town_seed = town_seed;
  my_save_info.current_time = TimGetSeconds();
  my_save_info.birth_date = birth_date;
  my_save_info.turn =  turn;
  my_save_info.total_points = total_points();
  my_save_info.missile_ctr = missile_ctr;
  my_save_info.inven_ctr = inven_ctr;
  my_save_info.inven_weight = inven_weight;
  my_save_info.equip_ctr = equip_ctr;
  my_save_info.tcptr = tcptr;
  my_save_info.mfptr = mfptr;
  my_save_info.total_winner = total_winner;
  my_save_info.death = death;  // Skipping: panic_save, noscore.
  StrNCopy(my_save_info.died_from, died_from, 25);
  floor_save_info.dun_level = dun_level;
  floor_save_info.char_row = char_row;
  floor_save_info.char_col = char_col;
  floor_save_info.mon_tot_mult = mon_tot_mult;
  floor_save_info.cur_height = cur_height;
  floor_save_info.cur_width = cur_width;
  floor_save_info.max_panel_rows = max_panel_rows;
  floor_save_info.max_panel_cols = max_panel_cols;
}
static void read_save_structs()
{
  spell_learned   = my_save_info.spell_learned;     
  spell_worked    = my_save_info.spell_worked;
  spell_forgotten = my_save_info.spell_forgotten;
  town_seed       = my_save_info.town_seed;
  //  current_time    = my_save_info.TimGetSeconds();
  birth_date      = my_save_info.birth_date;
  turn            = my_save_info.turn;
  max_score       = my_save_info.total_points;
  missile_ctr     = my_save_info.missile_ctr;
  inven_ctr       = my_save_info.inven_ctr;
  inven_weight    = my_save_info.inven_weight;
  equip_ctr       = my_save_info.equip_ctr;
  tcptr       = my_save_info.tcptr;
  mfptr       = my_save_info.mfptr;
  total_winner    = my_save_info.total_winner;
  death           = my_save_info.death;
  StrNCopy(died_from, my_save_info.died_from, 25);
  dun_level      = floor_save_info.dun_level;
  char_row       = floor_save_info.char_row;
  char_col       = floor_save_info.char_col;
  mon_tot_mult   = floor_save_info.mon_tot_mult;
  cur_height     = floor_save_info.cur_height;
  cur_width      = floor_save_info.cur_width;
  max_panel_rows = floor_save_info.max_panel_rows;
  max_panel_cols = floor_save_info.max_panel_cols;
}
// Create the py storage record (will delete it if it already exists)
static Boolean create_py_rec()
{
  Boolean ok = false;/*, crashed = false; */
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong size = sizeof(struct flags) + sizeof(struct stats);
  size += sizeof(struct misc);
  size += sizeof(UChar) * (MAX_SPELLS+1);
  size += sizeof(Short) * MAX_PLAYER_LEVEL;
  size += sizeof(my_save) + sizeof(floor_save);
  /*    size += sizeof(Boolean); */

  // Delete existing py record
  uniqueID = REC_PY;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p) {
    Short offset = 0;
    // Write things.
    DmWrite(p, offset, &pymisc, sizeof(struct misc));
    offset += sizeof(struct misc);
    DmWrite(p, offset, &pystats, sizeof(struct stats));
    offset += sizeof(struct stats);
    DmWrite(p, offset, &pyflags, sizeof(struct flags));
    offset += sizeof(struct flags);
    //  player_hp;
    DmWrite(p, offset, player_hp, sizeof(Short) * MAX_PLAYER_LEVEL);
    offset += sizeof(Short) * MAX_PLAYER_LEVEL;
    //  spell_order;
    DmWrite(p, offset, spell_order, sizeof(UChar) * (MAX_SPELLS+1));
    offset += sizeof(UChar) * (MAX_SPELLS+1);
    //  my_save;
    DmWrite(p, offset, &my_save_info, sizeof(my_save));
    offset += sizeof(my_save);
    //  floor_save;
    DmWrite(p, offset, &floor_save_info, sizeof(floor_save));
    offset += sizeof(floor_save);
    /*      DmWrite(p, offset, &crashed, sizeof(Boolean));
            offset += sizeof(Boolean); */
    ok = true;
  }
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  return ok;
}
static Boolean read_py_rec()
{
  UInt recindex;
  VoidHand vh;
  VoidPtr p; /*, p0; */
  Err err;
  ULong uniqueID;
  /*    Boolean b=true, crashed; */
  //
  uniqueID = REC_PY;
  err = DmFindRecordByID(MoriaSaveDB, uniqueID, &recindex);
  if (err) return false; // didn't find it
  vh = DmQueryRecord(MoriaSaveDB, recindex); /* DmGetRecord(..) */
  p = MemHandleLock(vh); /* p0 = p; */
  if (!p) return false;   // we're in some kind of trouble

  // Ok now read a whole bunch of stuff.
  err = MemMove(&pymisc, p, sizeof(struct misc));
  if (err) { MemHandleUnlock(vh); return false; }
  p += sizeof(struct misc);
  err = MemMove(&pystats, p, sizeof(struct stats));
  if (err) { MemHandleUnlock(vh); return false; }
  p += sizeof(struct stats);
  err = MemMove(&pyflags, p, sizeof(struct flags));
  if (err) { MemHandleUnlock(vh); return false; }
  p += sizeof(struct flags);
  err = MemMove(player_hp, p, sizeof(Short) * MAX_PLAYER_LEVEL);
  if (err) { MemHandleUnlock(vh); return false; }
  p += sizeof(Short) * MAX_PLAYER_LEVEL;
  err = MemMove(spell_order, p, sizeof(UChar) * (MAX_SPELLS+1));
  if (err) { MemHandleUnlock(vh); return false; }
  p += sizeof(UChar) * (MAX_SPELLS+1);
  err = MemMove(&my_save_info, p, sizeof(my_save));
  if (err) { MemHandleUnlock(vh); return false; }
  p += sizeof(my_save);
  err = MemMove(&floor_save_info, p, sizeof(floor_save));
  if (err) { MemHandleUnlock(vh); return false; }

  /*    p += sizeof(floor_save);
        crashed = ((Boolean *) p)[0];
        DmWrite(p0, p - p0, &b, sizeof(Boolean));     // will it work? */

  MemHandleUnlock(vh);
  /*    DmReleaseRecord(MoriaSaveDB, recindex, true);
	DmSetRecordInfo(MoriaSaveDB, recindex, NULL, &uniqueID); // set uid!
	return (!crashed); */
  return true; 
}
/*
// Create the miscsave storage record (will delete it if it already exists)
static Boolean create_miscsave_rec()
{
  Boolean ok = false;
  ULong uniqueID;
  UInt index;
  VoidHand vh;
  VoidPtr p;
  ULong 
  // Delete existing miscsave record
  uniqueID = REC_MISCSAVE;
  if (0 == DmFindRecordByID(MoriaSaveDB, uniqueID, &index))
    DmRemoveRecord(MoriaSaveDB, index);      // EX TERM IN ATE !
  index = DmNumRecords(MoriaSaveDB);
  vh = DmNewRecord(MoriaSaveDB, &index, size);
  p = MemHandleLock(vh);
  if (p) {
    Short offset = 0;
    // Write things.
    ok = true;
  }
  MemPtrUnlock(p);
  DmReleaseRecord(MoriaSaveDB, index, true);
  DmSetRecordInfo(MoriaSaveDB, index, NULL, &uniqueID); // set its uid!
  return ok;
}

*/



