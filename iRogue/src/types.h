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
#ifndef __TYPES_H__
#define __TYPES_H__

#define MAX_TITLE_LENGTH 40
struct id {
	Short value;
	Char title[MAX_TITLE_LENGTH];
	Char real[40];
	UShort id_status;
};

/* The following #defines provide more meaningful names for some of the
 * struct object fields that are used for monsters.  This, since each monster
 * and object (scrolls, potions, etc) are represented by a struct object.
 * Ideally, this should be handled by some kind of union structure.
 */

#define m_damage damage
#define hp_to_kill quantity
#define m_char ichar
#define kill_exp launcher
#define first_level is_protected
#define last_level is_cursed
#define m_hit_chance class
#define stationary_damage identified
#define breath_type identified
#define carcase_type identified
/* nothing stationary breaths;  nothing edible should breath;
   nothing stationary does stationary-type damage if it's edible. */
#define drop_percent which_kind
#define trail_char d_enchant
#define slowed_toggle quiver
#define moves_confused hit_enchant
#define nap_length picked_up
#define disguise what_is
#define next_monster next_object
#define m_flags1 o_flags
#define m_flags2 in_use_flags
#define trow weight
#define tcol o_unused
/* unused is used for food (to indicate which string in an array of names)
   and might be used for wands (to indicate the zap character to use) */

struct obj {				/* comment is monster meaning */
	ULong o_flags;	        /* monster flags 1 */
	ULong in_use_flags;	/* monster flags 2 */
  /*	char *damage;	*/		/* damage it does */
  	Char damage[4];                 /* Using char as very-short int. */
	Short quantity;			/* hit points to kill */
	Short ichar;			/* 'A' is for aquatar */
	Short launcher;			/* exp for killing it */
	Short is_protected;		/* level starts */
	Short is_cursed;		/* level ends */
	Short class;			/* chance of hitting you */
	Short identified;		/* 'F' damage, 1,2,3... */
	UShort which_kind; /* item carry/drop % */
	Short o_row, o_col, o;	/* o is how many times stuck at o_row, o_col */
	Short row, col;			/* current row, col */
	Short d_enchant;		/* room char when detect_monster */
	Short quiver;			/* monster slowed toggle */
	Short weight, o_unused;	/* target row, col */
	Short hit_enchant;		/* how many moves is confused */
	UShort what_is;	/* imitator's charactor (?!%: */
	Short picked_up;		/* sleep from wand of sleep */
  /*	UShort in_use_flags; */
	struct obj *next_object;	/* next monster */
};
/* I think I'll use o_unused for fruit name... */

typedef struct obj object;

#define INIT_HP 12

struct fight {
	object *armor;
	object *weapon;
        object *rings[8]; /* 0-3 on left hand, 4-7 on right hand */
	Short hp_current;
	Short hp_max;
	Short str_current;
	Short str_max;
	object pack;
	Long gold;
	Short exp;
	Long exp_points;
	Short row, col;
	Short fchar;
	Short moves_left;
};

typedef struct fight fighter;

struct dr {
  Short oth_room;
  Short oth_row,
        oth_col;
  Short door_row,
        door_col;
};

typedef struct dr door;

struct rm {
  Char bottom_row, right_col, left_col, top_row;
  door doors[4];
  UShort is_room;
};

typedef struct rm room;

struct tr {
	Short trap_type;
	Short trap_row, trap_col;
};

typedef struct tr trap;

struct sotu_Before_Conducts {
  ULong birthdate;
  ULong timestamp;
  UChar score_status; /* !=0: you don't go in the topten 
			      0x1: you've been a wizard
			      0x2: you're undead 
			      0x4: you're CURRENTLY dead */
  Boolean wizard; /* true if currently a wizard */
  Char hunger_str[8];
  Char username[32];
/*   object *fight_monster; */
/*   Char hit_message[80]; */
/*   Boolean is_wood[WANDS]; */
/*   Char *wand_materials[WAND_MATERIALS]; */
/*   Char *gems[GEMS]; */
/*   Char *syllables[MAXSYLLABLES]; */
  Short cur_level;
  Short max_level;
  Short cur_room;
/*   Char *new_level_message; */
  Short party_room;
  Short r_de;
/*   Long level_points[MAX_EXP_LEVEL]; */
/*   Char random_rooms[MAX_ROOMS+1]; */
/*   Boolean msg_cleared; */
/*   Object level_monsters; */
  Boolean mon_disappeared;
/*   Char *m_names[MONSTERS]; */
/*   Object mon_tab[MONSTERS]; */
  Short m_moves;
  Boolean jump;
/*   Char *you_can_move_again; */
/*   object level_objects; */
/*   UShort dungeon[DROWS][DCOLS]; */
  Short foods;
  Short party_counter;
/*   object *free_list; */

/*   fighter rogue; */ /* ... will be at end. */
/*   struct id ... */
/*   Char *curse_message; */
  Boolean interrupted;
/*   Char *unknown_command; */
/*   Char *left_or_right; */
/*   Char *no_ring; */
  Short r_rings;
  Short e_rings;
  Short stealthy; /* this is cumulative */
  Short regeneration;
  Short auto_search;
  Short add_strength;
  Short ring_dex;
  Short ring_ac;
/*   room rooms[MAXROOMS]; */
/*   Boolean rooms_visited[MAXROOMS]; */
  Short less_hp;
/*   Char *flame_name; */
  Boolean being_held;
/*   trap traps[MAXTRAPS]; */
  Boolean trap_door;
  ULong ring_flags;
  Short bear_trap;
  Short halluc;
  Short blind;
  Short confused;
  Short levitate;
  Short haste_self;
  Boolean see_invisible;
  Short extra_hp;
  Boolean detect_monster;
/*   Char *strange_feeling; */
  /* POINTERS to other MALLOC'd things */
  fighter *roguep;
  UShort **dungeon;
  room * rooms;
  trap * traps;
  struct id * id_potions;
  struct id * id_scrolls;
  struct id * id_wands;
  struct id * id_rings;
  struct id * id_weapons;
  struct id * id_armors;
  Boolean * is_wood; /* goes with id_wands */
  object * level_objects;
  object * level_monsters;
  object * fight_monster;
  object * free_list;
  Long * level_points;

  Char *new_level_message;
  Char *fruit;
  Char *old_messages[SAVED_MSGS];
  Short last_old_message_shown;
  Short fruit_number;

  Char warning_level;
  Char warning_moves;
};


struct state_of_the_union {
  ULong birthdate;
  ULong timestamp;
  UInt conduct; /*  0x0001: you are a wizard RIGHT NOW
                    0x0100: (256) you've ever been a wizard
                    0x0200: (512) you're undead
                    0x0400: you're CURRENTLY dead
                       If (conduct & 0x0700) YOU DON'T GET IN THE TOPTEN. */
  //  UChar score_status; /* !=0: you don't go in the topten 
  //			      0x1: you've been a wizard
  //			      0x2: you're undead 
  //			      0x4: you're CURRENTLY dead */
  //  Boolean wizard; /* true if currently a wizard */
  Char hunger_str[8];
  Char username[32];
/*   object *fight_monster; */
/*   Char hit_message[80]; */
/*   Boolean is_wood[WANDS]; */
/*   Char *wand_materials[WAND_MATERIALS]; */
/*   Char *gems[GEMS]; */
/*   Char *syllables[MAXSYLLABLES]; */
  Short cur_level;
  Short max_level;
  Short cur_room;
/*   Char *new_level_message; */
  Short party_room;
  Short r_de;
/*   Long level_points[MAX_EXP_LEVEL]; */
/*   Char random_rooms[MAX_ROOMS+1]; */
/*   Boolean msg_cleared; */
/*   Object level_monsters; */
  Boolean mon_disappeared;
/*   Char *m_names[MONSTERS]; */
/*   Object mon_tab[MONSTERS]; */
  Short m_moves;
  Boolean jump;
/*   Char *you_can_move_again; */
/*   object level_objects; */
/*   UShort dungeon[DROWS][DCOLS]; */
  Short foods;
  Short party_counter;
/*   object *free_list; */

/*   fighter rogue; */ /* ... will be at end. */
/*   struct id ... */
/*   Char *curse_message; */
  Boolean interrupted;
/*   Char *unknown_command; */
/*   Char *left_or_right; */
/*   Char *no_ring; */
  Short r_rings;
  Short e_rings;
  Short stealthy; /* this is cumulative */
  Short regeneration;
  Short auto_search;
  Short add_strength;
  Short ring_dex;
  Short ring_ac;
/*   room rooms[MAXROOMS]; */
/*   Boolean rooms_visited[MAXROOMS]; */
  Short less_hp;
/*   Char *flame_name; */
  Boolean being_held;
/*   trap traps[MAXTRAPS]; */
  Boolean trap_door;
  ULong ring_flags;
  Short bear_trap;
  Short halluc;
  Short blind;
  Short confused;
  Short levitate;
  Short haste_self;
  Boolean see_invisible;
  Short extra_hp;
  Boolean detect_monster;
/*   Char *strange_feeling; */
  /* POINTERS to other MALLOC'd things */
  fighter *roguep;
  UShort **dungeon;
  room * rooms;
  trap * traps;
  struct id * id_potions;
  struct id * id_scrolls;
  struct id * id_wands;
  struct id * id_rings;
  struct id * id_weapons;
  struct id * id_armors;
  Boolean * is_wood; /* goes with id_wands */
  object * level_objects;
  object * level_monsters;
  object * fight_monster;
  object * free_list;
  Long * level_points;

  Char *new_level_message;
  Char *fruit;
  Char *old_messages[SAVED_MSGS];
  Short last_old_message_shown;
  Short fruit_number;

  Char warning_level;
  Char warning_moves;
};


#define PREF_COLOR 0x1
#define PREF_BLACKBG 0x2
/* whats AbtShowAbout ? */
struct RoguePreferenceType_s {
  /* ... */
  Boolean run_on;
  Boolean autosave_on; // actually I think I should replace with "multiuser"
  //Boolean save_on_exit; // changed this to autosave in 0.42 !!!
  Short run_walk_border;
  Short walk_search_border;
  Boolean dave_mode;
  Boolean scroll_mode;
  Boolean sound_on;
  Boolean use_hardware;
  Short hardware[8];
  // RogueAppPrefVersion = 0x01 (?)
  Boolean stay_centered; // new in 0.40
  // RogueAppPrefVersion = 0x02 (?)
  Boolean font_small; // new in 0.42
  Boolean rogue_relative; // new in 0.42
  // RogueAppPrefVersion = 0x03  ----  These are added in iRogue Version 0.45:
  Boolean color_on;
  Boolean crosshairs_on;
  // RogueAppPrefVersion = 0x03  ----  These are added in iRogue Version 0.46:
  Boolean sort_pack;
  Boolean black_bg;
  // RogueAppPrefVersion = 0x04  ----  These are added in iRogue 1.1.1
  Boolean tiles_on;
};
typedef struct RoguePreferenceType_s RoguePreferenceType;


#endif

