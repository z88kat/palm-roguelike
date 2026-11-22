/**********************************************************************
 * kMoria - Moria adapted for the PalmPilot.                          *
 * This file is                                                       *
 * copyright (C) 1985-2001 Robert A. Koeneke, Jimmey Wayne Todd,      *
 * James E. Wilson, Bridget Spitznagel. (see main.c for full credits) *
 *                                                                    *
 * This program is freeware.  You can redistribute it and/or modify   *
 * it (provided that this notice and the full credits are retained),  *
 * but it CANNOT be distributed for profit.                           *
 **********************************************************************/
#include "kMoria.h"

// Needs:
// get_dir
// copy_spell_name
// add_inscribe

/* Wands for the aiming. */
// Warning - assumes item_val IS a wand
void aim(Short item_val, Short dir)
{
  ULong i;
  Short l, ident; // ident might be a Boolean.
  Short j, k, chance;
  inven_type *i_ptr; // A pointer to the wand in question
  //  Char spname_buf[MAXLEN_SPELLNAME];

  //  free_turn_flag = true;
  //  if (inven_ctr == 0)
  //    message("But you are not carrying anything.");
  //  else if (!find_range(TV_WAND, TV_NEVER, &j, &k))
  //    message("You are not carrying any wands.");
  //  else if (get_item(&item_val, "Aim which wand?", j, k, CNIL, CNIL)) {
  // item_val now contains the index, in 'inventory' array, of the wand
  if (inven_ctr <= item_val) { // for MODE_AGAIN
    message("You'll need to select a wand again.");
    return;
  }
  i_ptr = &(inventory[item_val]);
  if (i_ptr->tval != TV_WAND) { // for MODE_AGAIN
    message("You'll need to select a wand again.");
    return;
  }
  free_turn_flag = false;

  if (pyflags.confused > 0) {
    message("You are confused.");
    do {
      dir = randint(9);
    } while (dir == 5);
  }
  ident = false;
  // What on earth is this doing?
  // it is calculating the chance that you use the wand correctly.
  // (but how?)
  chance = pymisc.save + stat_adj(A_INT) - (Short)i_ptr->level
    + (class_level_adj[pymisc.pclass][CLA_DEVICE] * pymisc.lev / 3);
  if (pyflags.confused > 0)
    chance = chance / 2;
  if ((chance < USE_DEVICE) && (randint(USE_DEVICE - chance + 1) == 1))
    chance = USE_DEVICE; /* Give everyone a slight chance */
  if (chance <= 0)  chance = 1;
  // Decide whether we used the wand correctly
  if (randint(chance) < USE_DEVICE)
    message("You failed to use the wand properly.");
  else if (i_ptr->p1 > 0) {
    i = i_ptr->flags; // I think this indicates the effect of the wand?
    invy_set_p1(inventory, item_val, i_ptr->p1-1); // decrement # of charges
    while (i != 0) {
      j = bit_pos(&i) + 1;
      k = char_row;
      l = char_col;
      /* Wands */
      switch(j) {
      case 1:
	message("A line of blue shimmering light appears.");
	light_line(dir, char_row, char_col);
	ident = true;
	break;
      case 2:
	fire_bolt(GF_LIGHTNING, dir, k, l, damroll(4, 8), spell_names[8]);
	ident = true;
	break;
      case 3:
	fire_bolt(GF_FROST, dir, k, l, damroll(6, 8), spell_names[14]);
	ident = true;
	break;
      case 4:
	fire_bolt(GF_FIRE, dir, k, l, damroll(9, 8), spell_names[22]);
	ident = true;
	break;
      case 5:
	ident = wall_to_mud(dir, k, l);
	break;
      case 6:
	ident = poly_monster(dir, k, l);
	break;
      case 7:
	ident = hp_monster(dir, k, l, -damroll(4, 6));
	break;
      case 8:
	ident = speed_monster(dir, k, l, 1);
	break;
      case 9:
	ident = speed_monster(dir, k, l, -1);
	break;
      case 10:
	ident = confuse_monster(dir, k, l);
	break;
      case 11:
	ident = sleep_monster(dir, k, l);
	break;
      case 12:
	ident = drain_life(dir, k, l);
	break;
      case 13:
	ident = td_destroy2(dir, k, l);
	break;
      case 14:
	fire_bolt(GF_MAGIC_MISSILE, dir, k, l, damroll(2, 6), spell_names[0]);
	ident = true;
	break;
      case 15:
	ident = build_wall(dir, k, l);
	break;
      case 16:
	ident = clone_monster(dir, k, l);
	break;
      case 17:
	ident = teleport_monster(dir, k, l);
	break;
      case 18:
	ident = disarm_all(dir, k, l);
	break;
      case 19:
	fire_ball(GF_LIGHTNING, dir, k, l, 32, "Lightning Ball");
	ident = true;
	break;
      case 20:
	fire_ball(GF_FROST, dir, k, l, 48, "Cold Ball");
	ident = true;
	break;
      case 21:
	fire_ball(GF_FIRE, dir, k, l, 72, spell_names[28]);
	ident = true;
	break;
      case 22:
	fire_ball(GF_POISON_GAS, dir, k, l, 12, spell_names[6]);
	ident = true;
	break;
      case 23:
	fire_ball(GF_ACID, dir, k, l, 60, "Acid Ball");
	ident = true;
	break;
      case 24:
	i = 1L << (randint(23) - 1);
	break;
      default:
	message("Internal error in wands()"); // how cheery.
	break;
      } // end switch
    } // end while i
    // did we recognize (identify) the wand effect? gain experience.
    if (ident) {
      if (!known1_p(i_ptr)) {
	/* round half-way case up */
	pymisc.exp += (i_ptr->level +(pymisc.lev >> 1)) / pymisc.lev;
	prt_experience();
	identify(&item_val);
	i_ptr = &(inventory[item_val]);
      }
    } else if (!known1_p(i_ptr))
      sample(i_ptr); // you still don't know it.?
    desc_charges(item_val);
    // we are done with charged wands
  } else {
    message("The wand has no charges left.");
    if (!known2_p(i_ptr))
      add_inscribe(inventory, item_val, ID_EMPTY);
  } // done with charged or empty
  //  }
}
