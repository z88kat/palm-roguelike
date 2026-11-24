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

#ifdef I_AM_COLOR
#include "kMoria.h"

// things i use
extern Boolean found_color;
extern IndexedColorType *mon_colors; 

// things i define
IndexedColorType *mon_colors = NULL; // this is private
IndexedColorType black, white;
Boolean found_color = false;


// x,y are dungeon cell coordinates
// from this, get monster (if any)
IndexedColorType get_color(Char c, Short x, Short y)
{
  // Ok.  We need: monster type
  // 
  Short i, c_ptr = y * MAX_WIDTH + x;
  UChar critter;
  IndexedColorType mycol = black;
  critter = cave_cptr[c_ptr];
  if ((critter > 1) && (m_list[critter].ml)) {
    // monster exists & is lighted.  lookup by the monster type.
    i = m_list[critter].mptr; // its index in the monster colors
    mycol = mon_colors[i];
  } else if (c > ' ' && c <= '~') {
    // not a monster, but is valid ascii; lookup by the ascii value.
    i = c - ' ';
    mycol = mon_colors[i+MAX_CREATURES];
    // Special case: '$' copper silver gold. (also mithril)
    // This is hardwired... not generally a good idea...
    if (c == '$') {
      Short mptr = 0;
      UChar subval, thing = cave_tptr[c_ptr];
      if ((thing != 0) && (t_list[thing].tval == TV_GOLD)) {
	subval = t_list[thing].subval;
	if (subval >= 1 && subval <= 3) // Cu
	  mptr = 52;
	else if (subval >= 4 && subval <= 6) // Ag
	  mptr = 76;
	else if ((subval == 14) || (subval >= 9 && subval <= 11)) // Au
	  mptr = 110;
	if (mptr != 0)
	  // see if the creeping <foo> coins has a color.  if so, use it.
	  // even if it's just black or white.
	  mycol = mon_colors[mptr];
      } // else - probably a bug, to have '$' and not a TV_GOLD.
    } // else - not '$' so we skip the special-case
  }

  // be helpful...
  // Indexes 230-255 of the 8-bit system palette are all black; treat any
  // of them as black, whichever one WinRGBToIndex picked.
  if (mycol >= 230)
    mycol = black;
  if (mycol == black && my_prefs.black_bg)
    mycol = white;
  else if (mycol == white && !my_prefs.black_bg)
    mycol = black;

  return mycol;
  /*
  if ((c == '$') ||
      (c == ',') ||
      ('A' <= c && c <= 'Z') || 
      ('a' <= c && c <= 'z'))
  */
}





/**********************************************************************
 *                                                                    *
 *         monster color definition                                   *
 *                                                                    *
 **********************************************************************/
static void init_color()
{
  RGBColorType rgbcolor;
  rgbcolor.r = rgbcolor.g = rgbcolor.b = 255; // white
  white = WinRGBToIndex(&rgbcolor);
  rgbcolor.r = rgbcolor.g = rgbcolor.b = 0; // black
  black = WinRGBToIndex(&rgbcolor);
}
void colors_record_lock(Boolean unlocked) SEC_7; // called in lock.c
DmOpenRef MoriaColorDB = NULL;
void colors_record_lock(Boolean unlocked)
{
  LocalID dbID;
  // ULong uniqueID
  UInt index = REC_CLIST_COLORS;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    found_color = false;
    // Find the color DB, if it exists
    if (0 == (dbID = DmFindDatabase(0, MoriaColorDBName))) return;
    if (0 == (MoriaColorDB = DmOpenDatabase(0,dbID,dmModeReadOnly))) return;
    // lock record... if it exists...
    //    if (0!=DmFindRecordByID(MoriaColorDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaColorDB, index);
    p = MemHandleLock(vh);
    mon_colors = (IndexedColorType *) p;
    found_color = true;
    init_color();
  } else {
    // unlock record
    if (mon_colors != NULL) {
      mon_colors = NULL;
      MemHandleUnlock(vh);
    }
    // close database
    if (MoriaColorDB != NULL) {
      DmCloseDatabase(MoriaColorDB);
      MoriaColorDB = NULL;
    }
  }
}



#endif // I_AM_COLOR
