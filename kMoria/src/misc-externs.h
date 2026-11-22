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
// form_recall.c
extern Short recall_this_monster;

// moria1.c - this could be a database record.. but it's only 60 bytes..
extern Short class_level_adj[MAX_CLASS][MAX_LEV_ADJ];

// spells-a.c - this could be a database record.. but it's only 160 bytes..
extern ULong player_exp[MAX_PLAYER_LEVEL];
extern class_type class[MAX_CLASS]; // ditto 186 bytes or so

// create.c - this is how big? like 240 bytes I think.
extern race_type race[MAX_RACES];
extern Short player_hp[MAX_PLAYER_LEVEL];// another 80 bytes

// magic.c
// these should probably be database records.  they are read only.
extern spell_type magic_spell[MAX_CLASS-1][31];
extern Char spell_names[62][24]; // could also be const.

// misc5.c
extern Short needed_direction_for;
extern Short needed_direction_item;

// store1.c
extern owner_type owners[MAX_OWNERS];

// main.c
extern MoriaPreferenceType my_prefs;


// display.c, main.c ...
extern Boolean IsVGA;
#ifndef I_AM_OS_2
extern FontID BigFont;
extern FontID SmallFont;
extern FontID NanoFont;
#endif
