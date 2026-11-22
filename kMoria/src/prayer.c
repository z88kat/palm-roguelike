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

void bump_state() SEC_7;

/* Needed:

data:
prayer.c:24: `class' undeclared (first use in this function)
prayer.c:35: `magic_spell' undeclared (first use in this function)
prayer.c:161: `spell_worked' undeclared (first use in this function)

functions:
`no_light'
`find_range'
`get_item'
`cast_spell'
`detect_evil'
`hp_player'
`bless'
`remove_fear'
`light_area'
`detect_trap'
`detect_sdoor'
`slow_poison'
`get_dir'
`confuse_monster'
`teleport'
`sleep_monsters1'
`create_food'
`cure_poison'
`fire_ball'
`detect_inv2'
`protect_evil'
`earthquake'
`map_area'
`turn_undead'
`dispel_creature'
`warding_glyph'

 */


/* Pray like HELL.					-RAK-	*/
extern Boolean just_browsing_book; 
//extern Char last_command; // in dungeon.c
extern previous_state curr_state; // replaces last_command & co.


void pray()
{
  //  Short i, j, dir;
  //  Short choice, chance, result;
  //  spell_type *s_ptr;
  //  struct misc *m_ptr;
  //  struct flags *f_ptr;
  //  inven_type *i_ptr;

  free_turn_flag = true;
  //  curr_state.cmd = -1;
  if (pyflags.blind > 0)
    message("You can't see to read your prayer!");
  else if (no_light())
    message("You have no light to read by.");
  else if (pyflags.confused > 0)
    message("You are too confused.");
  else if (class[pymisc.pclass].spell != PRIEST)
    message("Pray hard enough and your prayers may be answered.");
  //  else if (inven_ctr == 0)
  //    message ("But you are not carrying anything!");
  //  else if (!find_range(TV_PRAYER_BOOK, TV_NEVER, &i, &j))
  //    message ("You are not carrying any Holy Books!");
  //  else if (get_item(&item_val, "Use which Holy Book?", i, j, CNIL, CNIL)) {
  else {
    Short i, ctr = 0;
    for (i = 0 ; i < inven_ctr ; i++)
      if (inventory[i].tval == TV_PRAYER_BOOK)
	ctr++;
    if (ctr == 0)
      message("But you have no Holy Book!");
    else {
      just_browsing_book = false;
      FrmPopupForm(BookForm);
    }
    // Launch the recite-which-prayer form....
    // It should call pray_helper(Short choice, Short chance)
  }
}

// Note some of these will also have to do which-direction (get_dir) foo....
// Those are (choice+1 == 9 || choice+1 == 18).
extern Boolean draw_directional_p;
void pray_helper(Short choice, Short dir)
{
  Short i;
  spell_type *s_ptr;
  inven_type *i_ptr;

  s_ptr = &magic_spell[pymisc.pclass-1][choice];
  free_turn_flag = false;

  if (curr_state.mode == MODE_AGAIN) {
    if (!has_book(curr_state.spell)) {
      message("But you don't have that Holy Book!");
      return;
    }
    if (!confirmed_spell(curr_state.spell)) return;
  }

  if ((dir == -1 || curr_state.mode == MODE_AGAIN) &&
      (randint(100) < spell_chance(choice)))
    message("You lost your concentration!");
  else {
    /* Check for direction, if one is needed */
    if (dir == -1) {
      switch(choice+1) {
      case 9: case 18:
	draw_directional(); // draw_directional_p = true;// draw_directional();
	curr_state.mode = MODE_DIRECTIONAL;
	curr_state.cmd = 'p';
	free_turn_flag = true;
	return;
      }
    }
    /* Prayers.					*/
    switch(choice+1) {
    case 1: // Detect Evil
      detect_evil();
      break;
    case 2: // Cure Light Wounds
      hp_player(damroll(3, 3));
      break;
    case 3: // Bless
      bless(randint(12)+12);
      break;
    case 4: // Remove Fear
      remove_fear();
      break;
    case 5: // Call Light
      light_area(char_row, char_col);
      break;
    case 6: // Find Traps
      detect_trap();
      break;
    case 7: // Detect Doors/Stairs
      detect_sdoor();
      break;
    case 8: // Slow Poison
      slow_poison();
      break;
    case 9: // Blind Creature
      confuse_monster(dir, char_row, char_col);
      break;
    case 10: // Portal
      teleport((Short)(pymisc.lev*3));
      break;
    case 11: // Cure Medium Wounds
      hp_player(damroll(4, 4));
      break;
    case 12: // Chant
      bless(randint(24)+24);
      break;
    case 13: // Sanctuary
      sleep_monsters1(char_row, char_col);
      break;
    case 14: // Create Food
      create_food();
      break;
    case 15: // Remove Curse
      for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	i_ptr = &inventory[i];
	/* only clear flag for items that are wielded or worn */
	if (i_ptr->tval >= TV_MIN_WEAR && i_ptr->tval <= TV_MAX_WEAR)
	  invy_set_flags(inventory, i, i_ptr->flags & ~TR_CURSED);
      }
      break;
    case 16: // Resist Heat and Cold
      pyflags.resist_heat += randint(10) + 10;
      pyflags.resist_cold += randint(10) + 10;
      break;
    case 17: // Neutralize Poison
      cure_poison();
      break;
    case 18: // Orb of Draining
      fire_ball(GF_HOLY_ORB, dir, char_row, char_col,
		(Short)(damroll(3, 6)+pymisc.lev),
		"Black Sphere"); // XXX this wasn't printing its message
      break;
    case 19: // Cure Serious Wounds
      hp_player(damroll(8, 4));
      break;
    case 20: // Sense Invisible
      detect_inv2(randint(24)+24);
      break;
    case 21: // Protection from Evil
      protect_evil();
      break;
    case 22: // Earthquake
      earthquake();
      break;
    case 23: // Sense Surroundings
      map_area();
      break;
    case 24: // Cure Critical Wounds
      hp_player(damroll(16, 4));
      break;
    case 25: // Turn Undead
      turn_undead();
      break;
    case 26: // Prayer
      bless(randint(48)+48);
      break;
    case 27: // Dispel Undead
      dispel_creature(CD_UNDEAD, (Short)(3*pymisc.lev));
      break;
    case 28: // Heal
      hp_player(200);
      break;
    case 29: // Dispel Evil
      dispel_creature(CD_EVIL, (Short)(3*pymisc.lev));
      break;
    case 30: // Glyph of Warding
      warding_glyph();
      break;
    case 31: // Holy Word (all-purpose!)
      remove_fear();
      cure_poison();
      hp_player(1000);
      for (i=A_STR; i<=A_CHR; i++)
	res_stat(i);
      dispel_creature(CD_EVIL, (Short)(4*pymisc.lev));
      turn_undead();
      if (pyflags.invuln < 3)
	pyflags.invuln = 3;
      else
	pyflags.invuln++;
      break;
    default:
      break;
    }
    /* End of prayers.				*/
    if (!free_turn_flag) {
      if ((spell_worked & (1L << choice)) == 0) {
	pymisc.exp += s_ptr->sexp << 2;
	prt_experience();
	spell_worked |= (1L << choice);
      }
    }
  }
  if (!free_turn_flag) {
    if (s_ptr->smana > pymisc.cmana) {
      message("You faint from fatigue!");
      pyflags.paralysis = randint((Short)(5*(s_ptr->smana-pymisc.cmana)));
      pymisc.cmana = 0;
      pymisc.cmana_frac = 0;
      if (randint(3) == 1) {
	message("You have damaged your health!");
	dec_stat (A_CON);
      }
    }
    else
      pymisc.cmana -= s_ptr->smana;
    print_stats(STATS_MANA);	//	      prt_cmana();
  }
  // don't forget to take a turn.  this checks free_turn_flag and moves mnsters
  if (dir == -1) {
    bump_state();
    take_a_turn(true); // if dir != -1, the (grand)CALLER is going to do this!
  }
}
