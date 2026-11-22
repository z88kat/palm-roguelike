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
/*
 * object.c
 *
 * This source herein may be modified and/or distributed by anybody who
 * so desires, with the following restrictions:
 *    1.)  No portion of this notice shall be removed.
 *    2.)  Credit shall not be taken for the creation of this source.
 *    3.)  This code is not to be traded, sold, or used for personal
 *         gain or profit.
 *
 */

#include "iRogue.h"

static void make_party() SEC_1;
static void rand_place(object *obj) SEC_1;
static Short next_party() SEC_1;


object * level_objects;
UShort ** dungeon;

/* id_foo are now dynamically allocated. */

extern Char *error_file;


struct weap_data {
  // name?
  ULong w_flags; // (to be |= with o_flags)
  Char damage[4]; // hit damage   [0] d [1],   hurl damage  [2] d [3]
  Short launcher;
  Short weight;
  // IS_COLLECTION, IS_MISSILE, IS_METAL
};
typedef struct weap_data weapon_data;

// [0,1] is dice to roll for straight damage (e.g. '1' d '4')
// If an IS_MISSILE weapon is properly launched/thrown:
// [2,3] is a fractional multiplier, applied after all other bonuses.
// When it is applied, a 4/3 multiplier should be applied to hit_chance also!
// Note - [0] is also used as a modifier to-hit !!
static weapon_data weapon_tab[WEAPONS] = {
  { (0),                                  {1,1,1,1}, NO_LAUNCHER, 40}, // bow
  { (O_COLLECTION | O_MISSILE),           {1,1,1,3}, NO_LAUNCHER,  5},  // dart
  { (O_COLLECTION | O_MISSILE),           {1,1,1,6}, BOW,          5},  // arrow
  { (O_COLLECTION | O_MISSILE | O_METAL), {1,4,1,6}, NO_LAUNCHER, 10}, // dagger
  { (O_COLLECTION | O_MISSILE),		  {1,2,2,5}, NO_LAUNCHER,  4}, // shuriken
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER,100},  // mace
  { (O_METAL),				  {3,4,1,2}, NO_LAUNCHER, 60},  // long sword
  { (O_METAL),				  {3,6,1,2}, NO_LAUNCHER,250},  // two handed sword
  // these taken entirely from urogue:
  { (O_COLLECTION | O_MISSILE),		  {1,2,1,4}, SLING,        5}, // rock
  { (0),				  {0,1,0,1}, NO_LAUNCHER,  5}, // sling
  { (0),				  {1,1,1,1}, NO_LAUNCHER,100}, // crossbow
  { (O_COLLECTION | O_MISSILE),		 {1,2,1,12}, CROSSBOW,     7}, // crossbow bolt
  { (O_METAL | O_MISSILE),		  {1,6,1,8}, NO_LAUNCHER, 50}, // spear
  { (O_METAL),				  {3,4,1,4}, NO_LAUNCHER, 50}, // trident
  { (O_METAL),				  {2,6,1,3}, NO_LAUNCHER, 50}, // spetum
  { (O_METAL),				  {3,4,1,2}, NO_LAUNCHER,125}, // bardiche
  { (O_METAL),				 {1,12,1,8}, NO_LAUNCHER, 80}, // short pike
  { (O_METAL),				  {2,8,1,2}, NO_LAUNCHER,100}, // bastard sword
  { (O_METAL),				  {2,6,1,3}, NO_LAUNCHER,175}, // halberd
  { (O_METAL),				  {1,8,1,3}, NO_LAUNCHER, 80}, // battle axe
  // leaving out the silver arrow for now
  { (O_METAL),			          {1,6,1,2}, NO_LAUNCHER, 50}, // hand axe
  { (0),				  {1,4,1,2}, NO_LAUNCHER, 30}, // club
  { (O_METAL),				  {1,7,1,4}, NO_LAUNCHER,150}, // flail
  { (O_METAL),				 {1,10,1,3}, NO_LAUNCHER, 80}, // glaive
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER, 80}, // guisarme
  { (O_METAL | O_MISSILE),		  {1,3,1,5}, NO_LAUNCHER, 50}, // hammer
  { (O_MISSILE),			  {1,4,1,6}, NO_LAUNCHER, 10}, // javelin
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER,125}, // morning star
  { (O_METAL),				  {1,6,1,2}, NO_LAUNCHER, 80}, // partisan
  { (O_METAL),				  {2,4,1,2}, NO_LAUNCHER, 60}, // pick
  { (O_METAL),				  {2,8,2,6}, NO_LAUNCHER, 80}, // long pike
  { (O_METAL),				  {1,8,1,2}, NO_LAUNCHER, 40}, // scimitar
  { (O_COLLECTION|O_METAL|O_MISSILE),	  {1,1,1,8}, NO_LAUNCHER,  3}, // sling bullet
  { (0),				  {1,6,1,2}, NO_LAUNCHER, 50}, // quarterstaff
  { (O_METAL),				  {2,4,1,3}, NO_LAUNCHER, 75}, // broadsword
  { (O_METAL),				  {1,6,1,2}, NO_LAUNCHER, 35}, // short sword
  // leaving out the boomerang for now
  // leaving out the burning oil for now
  { (O_METAL),				  {3,7,1,2}, NO_LAUNCHER,300}, // claymore
  // leaving out the crysknife for now
  { (0),                                  {1,1,1,1}, NO_LAUNCHER, 90}, // footbow
  { (O_COLLECTION | O_MISSILE),		 {1,2,1,10}, NO_LAUNCHER,  5}, // footbow bolt
  { (O_METAL),				 {4,12,2,6}, NO_LAUNCHER,350}, // katana
  { (O_METAL),				  {1,6,1,5}, NO_LAUNCHER, 40}, // leuku
  { (0),				  {1,6,1,6}, NO_LAUNCHER, 45}, // tomahawk
  { (O_METAL),				  {2,5,1,3}, NO_LAUNCHER,160}, // pertuska
  { (O_METAL),				  {2,6,1,3}, NO_LAUNCHER,120}, // sabre
  { (O_METAL),				 {1,10,1,2}, NO_LAUNCHER, 55} // cutlass
};
// 45 weapons - not including silver-arrow, boomerang, burning-oil, crysknife.
// Is this too many??

// Here are probabilities used by the various gr_foo.
//static Short whatis_prob[] = {
// order should be: armor, weapon, scroll, potion, *SKIP*, food, wand, ring
//}
static Short scroll_prob[SCROLLS] = {
  // out of 0-85 .. make that 87
  5, 11, 16, 21, 36, 44, 51, 56, 65, 74, 80, 85, 87
};
// All scrolls weigh SCROLL_WEIGHT
#define SCROLL_WEIGHT 4
static Short potion_prob[POTIONS] = {
  // out of 1-118 
  10, 20, 30, 40, 50, 55, 65, 75, 85, 95, 105, 110, 114, 118
};
// All scrolls weigh POTION_WEIGHT
#define POTION_WEIGHT 10
static Short armor_prob[ARMORS] = {
  // out of 1-100
  10, 20, 31, 43, 55, 67, 77, 87, 93, 97, 99, 100
};
static Short armor_class[ARMORS] = {
  2,  3,  3,  4,  4,  5,  6,  6,  7,  8, 9, 10
};
// All wands are equally likely.
// All rings are equally likely.
// All weapons are equally likely, but some are more equal than others.
// Wands/staffs weigh ...
#define WAND_WEIGHT 20
#define STAFF_WEIGHT 50
#define FOOD_WEIGHT 400
#define FRUIT_WEIGHT 300
#define RING_WEIGHT 3
// Rings weigh ... except for the heavy one

static Short armor_weight[ARMORS] = {
  100, 250, 200, 250, 150, 300, 350, 350, 400, 450, 100, 300
};
// in urogue, armor weighs 20% less per + (20% more per -)
// and weapons weigh 50% less if sum of to-hit + and dam + is > 0
// and other stuff weighs 20% less if 'blessed'

// carrying ability modifier = (str - 8) * 50
// add to normal encumberance.
// take 20% off if 'weak', 50% off if 'faint'.
// (so.. what if you're carrying too much to move?)


extern Char fruit_names[21][14]; // in use.c
extern Char carcase_names[4][14];


/**********************************************************************
                       PUT_OBJECTS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Sometimes, randomly create + place a "party" of monsters in a "party room."
 Randomly create and place objects throughout the dungeon.  Ditto gold.
 **********************************************************************/
void put_objects()
{
  Short i, n;
  object *obj;

  if (sotu->cur_level < sotu->max_level) {
    return;
  }
  n = coin_toss() ? get_rand(2, 4) : get_rand(3, 5);
  while (rand_percent(33)) {
    n++;
  }

  /* I guess this is in the object file because a party room
     tends to have extra treasure */
  if (sotu->cur_level == sotu->party_counter) {
    make_party();
    if ( ((sotu->ring_flags & (RING_WARNING | RING_ESP))
	  && rand_percent(90))
	 || ((sotu->ring_flags & (RING_MOOD | RING_DELUSION))
	     && rand_percent(40)) )
      message("you hear something...");
    sotu->party_counter = next_party();
  } else if ((sotu->ring_flags & RING_DELUSION)
	     && rand_percent(20)) {
    // careful on the false positives, since there are more !party than party.
    message("you hear something...");
  }

  for (i = 0; i < n; i++) {
    obj = gr_object();
    rand_place(obj);
  }

  put_gold();
}


/***************************************************************
                   PLANT_GOLD
 IN:
 row,col = location to place the gold
 is_maze = whether it's a room or maze; rooms get more gold
 sotu = various globals (dungeon, level_objects, cur_level)
 OUT:
 nothing
 PURPOSE:
 Create and place a random quantity of gold at row,col.
****************************************************************/
static void plant_gold(Short row, Short col, Boolean is_maze) SEC_L;
static void plant_gold(Short row, Short col, Boolean is_maze) 
{
  object *obj;

  obj = alloc_object();
  obj->row = row; 
  obj->col = col;
  obj->what_is = GOLD;
  obj->quantity = get_rand((2 * sotu->cur_level), (16 * sotu->cur_level));
  if (is_maze) {
    obj->quantity += obj->quantity / 2;
  }
  sotu->dungeon[row][col] |= OBJECT;
  (void) add_to_pack(obj, sotu->level_objects, 0);
}

/***************************************************************
                   PUT_GOLD
 IN:
 sotu = various globals (rooms, dungeon)
 OUT:
 nothing
 PURPOSE:
 Called when a new level is created; randomly places gold in its
 rooms and mazes.
****************************************************************/
void put_gold()
{
  Short i, j;
  Short row,col;
  Boolean is_maze, is_room;
  room * rooms = sotu->rooms;
  UShort ** dungeon = sotu->dungeon;

  for (i = 0; i < MAXROOMS; i++) {
    is_maze = (rooms[i].is_room & R_MAZE) ? 1 : 0;
    is_room = (rooms[i].is_room & R_ROOM) ? 1 : 0;

    if (!(is_room || is_maze)) {
      continue;
    }
    /* All mazes and some rooms get gold... */
    if (is_maze || rand_percent(GOLD_PERCENT)) {
      for (j = 0; j < 50; j++) {
	row = get_rand(rooms[i].top_row+1,
		       rooms[i].bottom_row-1);
	col = get_rand(rooms[i].left_col+1,
		       rooms[i].right_col-1);
	if ((dungeon[row][col] == FLOOR) ||
	    (dungeon[row][col] == TUNNEL)) {
	  plant_gold(row, col, is_maze);
	  break;
	}
      }
    }
  }
}


/***************************************************************
                   PLACE_AT
 IN:
 obj = object to place
 row,col = location to put it at
 sotu = various globals (dungeon, level_objects)
 OUT:
 nothing
 PURPOSE:
 Put the object at the given location and add it to level_objects.
****************************************************************/
void place_at(object * obj, Short row, Short col) 
{
  obj->row = row;
  obj->col = col;
  sotu->dungeon[row][col] |= OBJECT;
  (void) add_to_pack(obj, sotu->level_objects, 0);
}

/***************************************************************
                   OBJECT_AT
 IN:
 pack = list of things to check in (level_obj. or level_mons.)
 row,col = location to check
 OUT:
 thing found, if any
 PURPOSE:
 Find the object (or monster?) at the given location, if any.
****************************************************************/
object * object_at(object *pack, Short row, Short col)
{
  object *obj;

  obj = pack->next_object;

  while (obj && ((obj->row != row) || (obj->col != col))) {
    obj = obj->next_object;
  }
  return(obj);
}

/***************************************************************
                   FREE_STUFF
 IN:
 objlist = list of objects to be freed
 sotu = various globals (not used)
 OUT:
 nothing
 PURPOSE:
 Frees the objects pointed to by objlist (but not objlist itself)
****************************************************************/
void free_stuff(object *objlist)
{
  object *obj;

  while (objlist->next_object) {
    obj = objlist->next_object;
    objlist->next_object =
      objlist->next_object->next_object;
    free_object(obj);
  }
}


/***************************************************************
                   FREE_OLD_LEVEL
 IN:
 sotu = various globals (level_objects, level_monsters)
 OUT:
 nothing
 PURPOSE:
 Free space that belongs to the old level: objects that the
 rogue isn't carrying; monsters that weren't killed.
 Called when the rogue goes up/down a level...
****************************************************************/
/* FREE EVERYTHING ALLOCED BY make_new_level (main.c) !!! */
/* FREE EVERYTHING ALLOCED BY anything after that !!! */
void free_old_level()
{
  /* clear_level - nothing */
  /* make_level - calls some stuff.. */
  /* put_objects - make_party. gr_object. allocates mons and objs */
  /* put_stairs - nothing */
  /* add_traps - nothing */
  /* put_mons - gr_monster allocates a monster. */
  /* put_player - nothing, I think */
  /* print_stats - nothing, I think */

  /* party_monster. wanderer. create_monster. put_mons. */

  free_stuff(sotu->level_objects);
  free_stuff(sotu->level_monsters);
}

/***************************************************************
                   GET_NTH_OBJECT  replacement for get_letter_object
 IN:
 n = index
 pack = list
 OUT:
 nth item in the list, or 0
 PURPOSE:
 Walk down the list to the nth object and return it.
 (Or walk off the list and return 0.)
****************************************************************/
object * get_nth_object(Short n, object *pack)
{
  object *obj;
  Short i;

  obj = pack->next_object;
  i = 0;
  while (obj && i < n) {
    i++;
    obj = obj->next_object;    
  }

  return(obj);
}


/***************************************************************
                   NAME_OF
 IN:
 obj = object to get the type of
 sotu = various globals (fruit, is_wood, id_weapons)
 OUT:
 string representing a general description of the object
 PURPOSE:
 Given an object, get a string describing it e.g. "scrolls",
 "shuriken", "slime-mold"
****************************************************************/
const Char * name_of(object *obj) 
{
  const Char *retstring;

  switch(obj->what_is) {
  case SCROLL:
    retstring = obj->quantity > 1 ? "scrolls " : "scroll ";
    break;
  case POTION:
    retstring = obj->quantity > 1 ? "potions " : "potion ";
    break;
  case FOOD:
    {
      const Char *get_food_name(object *obj);
      retstring = get_food_name(obj);
      break;
    }
  case WAND:
    retstring = sotu->is_wood[obj->which_kind] ? "staff " : "wand ";
    break;
  case WEAPON:
    retstring = sotu->id_weapons[obj->which_kind].title;
    if ((obj->o_flags & O_COLLECTION) && (obj->quantity <= 1)) {
      // XXXXX make it singular!!!
      // retstring = sotu->id_weapons[obj->which_kind].title;      
      switch(obj->which_kind) {
      case DART:
	retstring = "dart ";
	break;
      case ARROW:
	retstring = "arrow ";
	break;
      case DAGGER:
	retstring = "dagger ";
	break;
      case SHURIKEN:
	retstring = "shuriken ";
	break;
      case ROCK:
	retstring = "rock ";
	break;
      case CROSSBOW_BOLT:
	retstring = "crossbow bolt ";
	break;
      case SLING_BULLET:
	retstring = "sling bullet ";
	break;
      case FOOTBOW_BOLT:
	retstring = "footbow bolt ";
	break;
	// don't need a default
      }
    }
    break;
  case ARMOR:
    retstring = "armor ";
    break;
  case RING:
    retstring = "ring ";
    break;
  case AMULET:
    retstring = "amulet ";
    break;
  default:
    retstring = "unknown ";
    break;
  }
  return(retstring);
}

/***************************************************************
                   COPY_NAME_OF
 IN:
 obj = object to get name_of
 foo = space supplied by caller to put name_of result in
 sotu = various globals
 OUT:
 nothing (foo)
 PURPOSE:
 well, this is one of those funky porting hack things.
****************************************************************/
void copy_name_of(object *obj, Char *foo)
{
  StrCopy(foo, name_of(obj));
}
// do I still need copy_name_of?

/************************************************
                item generation
************************************************/


/***************************************************************
                   GR_WHAT_IS
 IN:
 nothing
 OUT:
 a random item-category
 PURPOSE:
 Randomly select from the 7 possible types of items.
****************************************************************/
static UShort gr_what_is() SEC_L;
static UShort gr_what_is()
{
  Short percent;
  UShort what_is;

  percent = get_rand(1, 91);
  // I can improve this - make it like armor_prob
  // 1-30 = scroll
  // 31-60 = potion
  // 61-74 = weapon
  // 75-83 = armor
  // 84-88 = food
  // 89-91 = ring
  // order should be: armor, weapon, scroll, potion, *SKIP*, food, wand, ring
  // These puppies are flags, so they will need a "shift<<i".
  if (percent <= 30) {
    what_is = SCROLL;
  } else if (percent <= 60) {
    what_is = POTION;
  } else if (percent <= 64) {
    what_is = WAND;
  } else if (percent <= 74) {
    what_is = WEAPON;
  } else if (percent <= 83) {
    what_is = ARMOR;
  } else if (percent <= 88) {
    what_is = FOOD;
  } else {
    what_is = RING;
  }
  return(what_is);
}

/***************************************************************
                   GR_SCROLL
 IN:
 obj = object destined to be a random scroll
 OUT:
 nothing (scroll)
 PURPOSE:
 Assign a type of scroll to the object.
****************************************************************/
static void gr_scroll(object *obj) SEC_L;
static void gr_scroll(object *obj)
{
  Short percent, kind = 0;

  obj->what_is = SCROLL;
  percent = get_rand(0, 87); // was 85
  while ( !(percent <= scroll_prob[kind]) && (kind < SCROLLS-1) )
    kind++; // the kind<SCROLLS-1 is "just in case".
  obj->which_kind = kind;
  obj->weight = SCROLL_WEIGHT;
  // PROTECT_ARMOR     6   5
  // HOLD_MONSTER      6  11
  // ENCH_WEAPON       9  16
  // ENCH_ARMOR       15  21
  // IDENTIFY          8  36
  // TELEPORT          7  44
  // SLEEP             5  51
  // SCARE_MONSTER     9  56
  // REMOVE_CURSE      5  65
  // CREATE_MONSTER    5  74
  // AGGRAVATE_MONSTER 6  80
  // MAGIC_MAPPING     5  85
  // create_obj        2  87
  // If there seem to be too many of them, reduce it to (0,86)
}

/***************************************************************
                   GR_POTION
 IN:
 obj = object destined to be a random potion
 OUT:
 nothing (potion)
 PURPOSE:
 Assign a type of potion to the object.
****************************************************************/
static void gr_potion(object *obj) SEC_L;
static void gr_potion(object *obj)
{
  Short percent, kind = 0;

  obj->what_is = POTION;
  percent = get_rand(1, 118);
  while ( !(percent <= potion_prob[kind]) && (kind < POTIONS) )
    kind++; // the kind<SCROLLS is "just in case".
  obj->which_kind = kind;
  obj->weight = POTION_WEIGHT;
  // INCREASE_STRENGTH 10  10
  // RESTORE_STRENGTH  10  20
  // HEALING           10  30
  // EXTRA_HEALING     10  40
  // POISON            10  50
  // RAISE_LEVEL        5  55
  // BLINDNESS         10  65
  // HALLUCINATION     10  75
  // DETECT_MONSTER    10  85
  // DETECT_OBJECTS    10  95
  // CONFUSION         10  105
  // LEVITATION         5  110
  // HASTE_SELF         4  114
  // SEE_INVISIBLE      4  118
}

/***************************************************************
                   GR_WEAPON
 IN:
 obj = object destined to be a random weapon
 assign_wk = if true, randomly assign a type of weapon
 OUT:
 nothing (weapon)
 PURPOSE:
 Initialize a new weapon object with appropriate random values
 (optionally, randomly choose its type.)
****************************************************************/
static void gr_weapon(object *obj, Boolean assign_wk, Char wkind) SEC_L;
static void gr_weapon(object *obj, Boolean assign_wk, Char wkind)
{
  Short percent;
  Short i;
  Short blessing, increment;

  obj->what_is = WEAPON;
  if (assign_wk) {
    // 50% chance of just "common" weapons
    obj->which_kind = get_rand(0, (coin_toss() ? 11 : (WEAPONS - 1)) );
  } else {
    obj->which_kind = min(max(wkind,0),WEAPONS-1);
  }
  // copy the weapon-table information
  obj->o_flags |= weapon_tab[obj->which_kind].w_flags;
  obj->weight = weapon_tab[obj->which_kind].weight;

  obj->launcher = weapon_tab[obj->which_kind].launcher;
  for (i = 0 ; i < 4 ; i++)
    obj->damage[i] = weapon_tab[obj->which_kind].damage[i];

  // this is basically the same as it was before
  if (obj->o_flags & O_COLLECTION) {
    obj->quantity = get_rand(3, 15);
    obj->quiver = get_rand(0, 126);
  } else {
    obj->quantity = 1;
  }
  obj->hit_enchant = obj->d_enchant = 0;

  percent = get_rand(1, 96);
  blessing = get_rand(1, 3);

  if (percent <= 16) {
    increment = 1;
  } else if (percent <= 32) {
    increment = -1;
    obj->is_cursed = 1;
  }
  if (percent <= 32) {
    for (i = 0; i < blessing; i++) {
      if (coin_toss()) {
	obj->hit_enchant += increment;
      } else {
	obj->d_enchant += increment;
      }
    }
  }
}

/***************************************************************
                   GR_ARMOR
 IN:
 obj = object destined to be a random armor
 OUT:
 nothing (armor)
 PURPOSE:
 Initialize a new armor object with randomly chosen attributes.
****************************************************************/

static void gr_armor(object *obj, Boolean assign_ak, Char akind) SEC_L;
static void gr_armor(object *obj, Boolean assign_ak, Char akind)
{
  Short percent;
  Short blessing;
  Short kind = 0;

  obj->what_is = ARMOR;
  if (assign_ak) {
    percent = get_rand(1,100); // should this be 99? but then !crystalline...
    while ( !(percent <= armor_prob[kind]) && (kind < ARMORS) )
      kind++; // the kind<ARMORS is "just in case".
  } else
    kind = min(max(akind,0), ARMORS-1);
  obj->which_kind = kind; // = get_rand(0, (ARMORS - 1));

  obj->class = armor_class[kind];
  obj->weight = armor_weight[kind] + get_rand(0,20) - 10; // add some individual variation
  obj->is_protected = 0;
  obj->d_enchant = 0;

  percent = get_rand(1, 100);
  blessing = get_rand(1, 3);
  /*
  if (percent <= 16) {
    obj->is_cursed = 1;
    obj->d_enchant -= blessing;
  } else if (percent <= 33) {
    obj->d_enchant += blessing;
  }
  */
  if ((percent < 20) && (kind != MITHRIL)) {
    obj->is_cursed = 1;
    obj->d_enchant -= blessing;
  } else if ((percent < 28) || (kind == MITHRIL)) {
    obj->d_enchant += blessing;
  }
  if (kind == MITHRIL)
    obj->is_protected = 1;
  // if it's "metal", it can rust.  heh.
  if (kind != LEATHER && kind != PADDED_ARMOR
      && kind != CRYSTAL_ARMOR && kind != MITHRIL)
    obj->o_flags |= O_METAL;
}

/***************************************************************
                   GR_WAND
 IN:
 obj = object destined to be a random wand
 OUT:
 nothing (wand)
 PURPOSE:
 Assign a type of wand and number of shots to the object.
****************************************************************/
static void gr_wand(object *obj) SEC_L;
static void gr_wand(object *obj)
{
  obj->what_is = WAND;
  obj->which_kind = get_rand(0, (WANDS - 1));
  obj->weight = (sotu->is_wood[obj->which_kind]) ? STAFF_WEIGHT : WAND_WEIGHT;
  if (obj->which_kind == MAGIC_MISSILE) {
    obj->class = get_rand(6, 12);
  } else if (obj->which_kind == CANCELLATION) {
    obj->class = get_rand(5, 9);
  } else {
    obj->class = get_rand(3, 6);
  }
}

/***************************************************************
                   GR_FOOD
 IN:
 obj = object destined to be a random food
 force_ration = chance of ration is 100% instead of 80%
 OUT:
 nothing (food)
 PURPOSE:
 Initialize a new food object (choosing ration/fruit.)
****************************************************************/
void get_food(object *obj, Boolean force_ration)
{
  obj->what_is = FOOD;

  if (force_ration || rand_percent(80)) {
    obj->which_kind = RATION;
    obj->weight = FOOD_WEIGHT;
  } else {
    obj->which_kind = FRUIT;
    obj->weight = FRUIT_WEIGHT;
    obj->o_unused = get_rand(0,20); // pick a random fruit name
  }
}

/***************************************************************
                   GR_RING
 IN:
 obj = object destined to be a random ring
 assign_wk = whether to randomly assign its type
 OUT:
 nothing (ring)
 PURPOSE:
 Initialize a new ring object with random attributes.
****************************************************************/
static void gr_ring(object *ring, Boolean assign_wk) SEC_L;
static void gr_ring(object *ring, Boolean assign_wk)
{
  ring->what_is = RING;
  if (assign_wk) {
    ring->which_kind = get_rand(0, (RINGS - 1));
  }
  ring->class = 0;
  ring->weight = RING_WEIGHT;

  switch(ring->which_kind) {
    /*
      case STEALTH:
      break;
      case SLOW_DIGEST:
      break;
      case REGENERATION:
      break;
      case R_SEE_INVISIBLE:
      break;
      case SUSTAIN_STRENGTH:
      break;
      case R_MAINTAIN_ARMOR:
      break;
      case SEARCHING:
      break;
      */
  case R_BURDEN:
    ring->weight = 300;
    // no break
  case R_TELEPORT:
  case R_AGGRAVATE:
  case R_DELUSION:
    ring->is_cursed = 1;
    break;
  case ADD_STRENGTH:
  case DEXTERITY:
  case R_PROTECTION:
    while ((ring->class = (get_rand(0, 4) - 2)) == 0) 
      ;
    ring->is_cursed = (ring->class < 0);
    break;
  case ADORNMENT:
  case R_MOOD:
    //  case R_LEVITATE:
    ring->is_cursed = coin_toss();
    break;
  }
}

/***************************************************************
                   GR_OBJECT
 IN:
 sotu = various globals (foods, cur_level)
 OUT:
 a randomly generated object!
 PURPOSE:
 Randomly generate a new object - allocate, determine category
 and specific type, and assign all attributes.
****************************************************************/
object * gr_object()
{
  object *obj;

  obj = alloc_object(); // everything should be 0 initially

  if (sotu->foods < (sotu->cur_level/2 )) {
    obj->what_is = FOOD;
    sotu->foods++;
  } else {
    obj->what_is = gr_what_is();
  }
  switch(obj->what_is) {
  case SCROLL:
    gr_scroll(obj);
    break;
  case POTION:
    gr_potion(obj);
    break;
  case WEAPON:
    gr_weapon(obj, true, 0);
    break;
  case ARMOR:
    gr_armor(obj, true, 0);
    break;
  case WAND:
    gr_wand(obj);
    break;
  case FOOD:
    get_food(obj, 0);
    break;
  case RING:
    gr_ring(obj, 1);
    break;
  }
  return(obj);
}


/***************************************************************
                   PUT_STAIRS
 IN:
 sotu = various globals (dungeon)
 OUT:
 nothing
 PURPOSE:
 When a new level is created, one stairway is randomly placed.
****************************************************************/
void put_stairs()
{
  Short row, col;

  gr_row_col(&row, &col, (FLOOR | TUNNEL));
  sotu->dungeon[row][col] |= STAIRS;
}

/***************************************************************
                   GET_ARMOR_CLASS
 IN:
 obj = armor, I hope.
 OUT:
 number representing an armor class (higher is better)
 PURPOSE:
 Use to figure out how hard something will be to hit...
****************************************************************/
//Short get_armor_class(object *obj) {
Short get_armor_class()
{
  Short ac = sotu->ring_ac;
  object *obj = sotu->roguep->armor;
  if (obj)
    ac += obj->class + obj->d_enchant;
  return ac;
}


/***************************************************************
                   ALLOC_OBJECT
 IN:
 sotu = various globals... (unused)
 OUT:
 a newly allocated and initialized object/monster
 PURPOSE:
 Allocate space for a new object or monster.
 Maybe this should check whether md_malloc failed, or is
 md_malloc doing that, I forget.
****************************************************************/
object * alloc_object()
{
  object *obj;

  obj = (object *) md_malloc(sizeof(object)); // everything's initially 'zero'

  obj->quantity = 1;
  obj->ichar = 'Z';
  obj->picked_up = obj->is_cursed = 0;
  obj->in_use_flags = NOT_USED;
  obj->identified = UNIDENTIFIED;
  obj->damage[0] = 1; //  obj->damage = "1d1";
  obj->damage[1] = 1;
  obj->damage[2] = 1; // thrown damage = 1d1
  obj->damage[3] = 1;
  return(obj);
}


/***************************************************************
                   FREE_OBJECT
 IN:
 obj = object to free
 sotu = various globals... (unused)
 OUT:
 nothing
 PURPOSE:
 please don't crash
****************************************************************/
/* I hope the free_list is reasonably bounded... */
void free_object(object *obj)
{
  VoidHand h = MemPtrRecoverHandle(obj);
  if (h) 
    MemHandleFree(h);
  else
    FrmGotoForm(WitsEndForm);
}



/**********************************************************************
                       MAKE_PARTY
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Call other stuff to make a party room....
 **********************************************************************/
static void make_party()
{
  Short n;

  sotu->party_room = gr_room(sotu->rooms);

  n = rand_percent(99) ? party_objects(sotu->party_room) : 11;
  if (rand_percent(99)) {
    party_monsters(sotu->party_room, n);
  }

  /* party_monsters is in monster.c.  oddly, party_objects is in room.c */

}


/************************************************
  show_objects MOVED to libRogue/use.c
************************************************/

/**********************************************************************
                       PUT_AMULET
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Place the amulet of yendor!
 would this be better inline? nah
 **********************************************************************/
void put_amulet()
{
  object *obj;

  obj = alloc_object();
  obj->what_is = AMULET;
  rand_place(obj);
}

/**********************************************************************
                       RAND_PLACE
 IN:
 object = the thing to put somewhere
 various globals
 OUT:
 nothing
 PURPOSE:
 Place the object at a random dungeon location...
 **********************************************************************/
static void rand_place(object *obj)
{
  Short row, col;

  gr_row_col(&row, &col, (FLOOR | TUNNEL));
  place_at(obj, row, col);
}

/***************************************************************
                   NEW_OBJECT_FOR_WIZARD
 IN:
 chtype = the category of object to create
 whatkind = the kind of <chtype> to create
 sotu = various globals (rogue)
 OUT:
 nothing
 PURPOSE:
 In wizard mode, create the specified object.  (Useful for
 testing/debugging purposes.)
****************************************************************/
/* Create a new object in wizard-mode. */
void new_object_for_wizard(Short chtype, Short whatkind)
{
  Short new_weight, max = 0;
  object *obj;
  Char buf[80];

  // I'll allow the wizard to create things even when the pack is
  // too heavy, because it's too much trouble to check :-)
  if (pack_count((object *) 0, sotu->roguep, &new_weight) >= MAX_PACK_COUNT) {
    message("pack full");
    return;
  }
  if (new_weight > max_weight()) {
    message("pack too heavy");
    return;
  }
  if (chtype < 0 || chtype > 7)
    return;

  obj = alloc_object();

  switch(chtype) {
    /* 	LIST "!" "?" "," ":" ")" "]" "/" "=" */
    /* ':' is now replaced by FOODCHAR */
  case 0:
    obj->what_is = POTION;
    max = POTIONS - 1;
    break;
  case 1:
    obj->what_is = SCROLL;
    max = SCROLLS - 1;
    break;
  case 2:
    gr_weapon(obj, false, whatkind); // whatkind should NOT be used below!
    max = WEAPONS - 1;
    break;
  case 3:
    gr_armor(obj, false, whatkind); // whatkind should NOT be used below!
    max = ARMORS - 1;
    break;
  case 4:
    gr_wand(obj);
    max = WANDS - 1;
    break;
  case 5:
    max = RINGS - 1;
    obj->what_is = RING;
    break;
  case 6:
    get_food(obj, 0);
    break;
  case 7:
    obj->what_is = AMULET;
    break;
  default:
    return;
  }
  if ((chtype != 2) && (chtype != 3)) {
    if ((whatkind >= 0) && (whatkind <= max)) {
      obj->which_kind = (UShort) whatkind;
      if (obj->what_is == RING) {
	gr_ring(obj, 0);
      }
    } else {
      message("oops"); // note: this should no longer happen
      free_object(obj);
      return;
    }
  }
  /* whee */
  get_desc(obj, buf);
  message(buf);

  (void) add_to_pack(obj, &sotu->roguep->pack, 1);
}



/**********************************************************************
                       NEXT_PARTY
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Calculate the next time to have a party room...
 **********************************************************************/
static Short next_party()
{
  int n;

  n = sotu->cur_level;
  while (n % PARTY_TIME) {
    n++;
  }
  return (get_rand((n + 1), (n + PARTY_TIME)));
}
