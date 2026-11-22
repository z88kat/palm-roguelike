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
/* These are the ones that the GAME will use and not the database-builder */
/* Keep this up to date with respect to dbrecords.h */

// THis might also want to contain all the MAXLEN_foo !!!

void newrec_messagelog(); /* use if no SaveDB exists */
void add_message_to_log(Char *msg, everything *env);
void print_message_log(everything *env);
void copy_player_title(Short pclass, Short plevel, Char *buf);
void generate_background(Char *buf, everything *env);
ULong get_player_exp(Short plevel);
void newrec_playerhp(); /* use if no SaveDB exists */
void write_player_hp(Short plevel, UShort hp);
void reset_player_hp(); /* use on character creation if SaveDB exists */
UShort get_player_hp(Short plevel);
UChar rgold_adj(Short owner_race, Short player_race);
void copy_spell_name(Short spell, Char *buf);
void newrec_spellorder(); /* use if no SaveDB exists */
void reset_spell_order(); /* use on character creation if SaveDB exists */
void write_spell_order(Short new_spell, everything *env);
Short get_spell_order(Short i);
/* NOTE - Need a routine to get the store owner record */
/* NOTE - Need a routine to get the store type record */
/* NOTE - Need a routine to get the store inventory records */
/* NOTE - Need a routine to get the treasure (object) type records */
void copy_spec_name(Short spec, Char *buf);
/* NOTE - Need a routine to get the creature (monster) type records */
/* NOTE - Need a routine to get the monster attacks record */
void copy_color(Short i, Char *buf);
void copy_mushroom(Short i, Char *buf);
void copy_wood(Short i, Char *buf);
void copy_metal(Short i, Char *buf);
void copy_rock(Short i, Char *buf);
void copy_amulet(Short i, Char *buf);
void copy_syllable(Short i, Char *buf);
UShort get_normal_tab(Short i);
