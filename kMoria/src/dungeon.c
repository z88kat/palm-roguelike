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
/* A note on EvtSysEventAvail(true) ... It's inefficient to check it
   every turn, I guess, but it doesn't matter much if we slow down a counted
   command a little bit.  the slower it is, the easier it is to abort
   earlier, anyway. */

void try_print_status() SEC_5;

static Boolean non_againable_command(Char c) SEC_7;
static ULong valid_command(Char c) SEC_7;
static Boolean start_use_item() SEC_7;
static void regenhp(Short percent) SEC_6;
static void regenmana(Short percent) SEC_6;
static Boolean enchanted (inven_type *t_ptr) SEC_5;
static void go_up() SEC_5;
static void go_down() SEC_5;
static void do_verb_generic(void (*verb)(Short)) SEC_3;
static void do_search() SEC_6;
static void do_force() SEC_6;
static void do_disarm() SEC_6;
static void do_open() SEC_6;
static void do_dig() SEC_6;
static void do_spike() SEC_6;
static void jamdoor(Short dir) SEC_5;
static void exchange_weap() SEC_5;
//static Char map_roguedir(Char comval) SEC_5;
static void ident_char(Char command) SEC_5;
// static void ident_monster(Char c) SEC_6;
static Boolean user_interrupted() SEC_5;
static Boolean check_for_shovel() SEC_6;

extern Short recall_this_monster;
extern Boolean recall_list;

extern UChar invequ_mode; // for aim,zap

UInt need_print_stats;
extern Boolean unverbing;

// In the original,
// you exit the dungeon loop when it's time to start a new dungeon level.

Short find_count; // this was a local variable in dungeon() so was regen_amount
extern Boolean do_pickup;
/* 
 * Call this every time you enter a new level.  THis was the stuff at the
 * start of dungeon() before you enter the giant Do-While loop.
 */
//void dungeon()
void enter_new_level()
{
  /* Main procedure for dungeon.			-RAK-	*/
  /* Note: There is a lot of preliminary magic going on here at first*/
  // This procedure is just all the preliminary magic.
  need_print_stats = 0;

  /* Check light status for setup	   */
  if (inventory[INVEN_LIGHT].p1 > 0)
    player_light = true;
  else
    player_light = false;
  /* Check for a maximum level		   */
  if (dun_level > pymisc.max_dlv)
    pymisc.max_dlv = dun_level;

  /* Reset flags and initialize variables  */
  //  curr_state.count = 0;
  find_count = 0;
  new_level_flag = false;
  find_flag = 0; // false;
  teleport_flag = false;
  mon_tot_mult = 0;
  cave_set(char_row, char_col, CAVE_cptr, 1); // put the '@' where you are!!!
  /* Ensure we display the panel. Used to do this with a global var. -CJS- */
  panel_row = panel_col = -1;
  /* Light up the area around character	   */ // also move panel if needed.
  if (!check_view())
    recalc_screen(); // and_refresh(); // fix the screen-not-redrawn bug
  
  /* must do this after panel_row/col set to -1, because search_off() will
     call check_view(), and so the panel_* variables must be valid before
     search_off() is called */
  if (pyflags.status & PY_SEARCH)
    search_off();
  /* Light,  but do not move critters	    */
  creatures(false);
  /* Print the depth			   */
  //  prt_depth(); // XXXXX dude, just print_stats(mumble)
  if (!need_print_stats) need_print_stats |= STATS_NONE; // XXXXXXX &= ?

}

/*************************************************************************
 * This is stuff from the beginning of dungeon(),
 * the stuff that gets done BEFORE we require player input.
 * (which, remember, will be an EVENT for us and not a polled char.)
 *************************************************************************/
#define CHECKPOINT_TIME 200
void start_of_turn()
{
  //  inven_type *i_ptr;
  Short plight, regen_amount, i;

  do {
    turn++;
    /* leave out game-hours restriction, unless your Palm is a mainframe */
    if ((!pyflags.rest && (turn % CHECKPOINT_TIME) == 0) ||
	(turn % (CHECKPOINT_TIME * 10)) == 0)
      maybe_checkpoint();

    /* turn over the store contents every, say, 1000 turns */
    if ((dun_level != 0) && ((turn % 1000) == 0))
      store_maint();

    /* Check for creature generation		*/
    if (randint(MAX_MALLOC_CHANCE) == 1)
      alloc_monster(1, MAX_SIGHT, false);

    /* Check light status			       */
    //  i_ptr = &inventory[INVEN_LIGHT];
    plight = inventory[INVEN_LIGHT].p1;
    if (player_light) {
      if (plight > 0) {
	plight--;
	invy_set_p1(inventory, INVEN_LIGHT, plight); // used a turn of light
	if (plight == 0) {
	  player_light = false;
	  message("Your light has gone out!");
	  disturb(false, true);
	  /* unlight creatures */
	  creatures(false);
	} else if ((plight < 40) && (randint(5) == 1) &&
		   (pyflags.blind < 1)) {
	  disturb(false, false);
	  message("Your light is growing faint.");
	}
      } else {
	player_light = false;
	disturb(false, true);
	/* unlight creatures */
	creatures(false);
      }
    } else if (plight > 0) {
      invy_set_p1(inventory, INVEN_LIGHT, plight); // used a turn of light
      player_light = true;
      disturb(false, true);
      /* light creatures */
      creatures(false);
    } /* end of Check light status                  */
  
    /* Update counters and messages (quite long)			*/

    /* Heroism (must precede anything that can damage player)      */
    if (pyflags.hero > 0) {
      if ((PY_HERO & pyflags.status) == 0) {
	pyflags.status |= PY_HERO;
	disturb(false, false);
	pymisc.mhp += 10;
	pymisc.chp += 10;
	pymisc.bth += 12;
	pymisc.bthb+= 12;
	message("You feel like a HERO!");
	need_print_stats |= STATS_HP; // was QUO
      }
      pyflags.hero--;
      if (pyflags.hero == 0) {
	pyflags.status &= ~PY_HERO;
	disturb (false, false);
	pymisc.mhp -= 10;
	if (pymisc.chp > pymisc.mhp) {
	  pymisc.chp = pymisc.mhp;
	  pymisc.chp_frac = 0;
	  // need_print_stats |= STATS_HP;// omit because we do it below
	}
	pymisc.bth  -= 12;
	pymisc.bthb -= 12;
	message("The heroism wears off.");
	need_print_stats |= STATS_HP; // was QUO
      }
    } /* end Heroism                  */

    /* Super Heroism */
    if (pyflags.shero > 0) {
      if ((PY_SHERO & pyflags.status) == 0) {
	pyflags.status |= PY_SHERO;
	disturb (false, false);
	pymisc.mhp += 20;
	pymisc.chp += 20;
	pymisc.bth += 24;
	pymisc.bthb+= 24;
	message("You feel like a SUPER HERO!");
	need_print_stats |= STATS_HP; // was QUO
      }
      pyflags.shero--;
      if (pyflags.shero == 0) {
	pyflags.status &= ~PY_SHERO;
	disturb (false, false);
	pymisc.mhp -= 20;
	if (pymisc.chp > pymisc.mhp) {
	  pymisc.chp = pymisc.mhp;
	  pymisc.chp_frac = 0;
	  // need_print_stats |= STATS_HP; // omit, we do it below
	}
	pymisc.bth  -= 24;
	pymisc.bthb -= 24;
	message("The super heroism wears off.");
	need_print_stats |= STATS_HP; // was QUO
      }
    } /* end Super Heroism              */

    /* Check food status	       */
    regen_amount = PLAYER_REGEN_NORMAL;
    if (pyflags.food < PLAYER_FOOD_ALERT) {
      if (pyflags.food < PLAYER_FOOD_WEAK) {
	if (pyflags.food < 0)
	  regen_amount = 0;
	else if (pyflags.food < PLAYER_FOOD_FAINT)
	  regen_amount = PLAYER_REGEN_FAINT;
	else if (pyflags.food < PLAYER_FOOD_WEAK)
	  regen_amount = PLAYER_REGEN_WEAK;
	if ((PY_WEAK & pyflags.status) == 0) {
	  pyflags.status |= PY_WEAK;
	  message("You are getting weak from hunger.");
	  disturb (false, false);
	  print_stats(STATS_WORD);//	prt_hunger(); // XXXXX not implemented
	}
	if ((pyflags.food < PLAYER_FOOD_FAINT) && (randint(8) == 1)) {
	  pyflags.paralysis += randint(5);
	  message("You faint from the lack of food.");
	  disturb (true, false);
	}
      } else if ((PY_HUNGRY & pyflags.status) == 0) {
	pyflags.status |= PY_HUNGRY;
	message("You are getting hungry.");
	disturb (false, false);
	print_stats(STATS_WORD);//   prt_hunger(); // XXXXXXX not implemented
      }
    } /* end of Check food status          */

    /* Food consumption
     * Note: Speeded up characters really burn up the food!  */
    if (pyflags.speed < 0)
      pyflags.food -= pyflags.speed * pyflags.speed;
    pyflags.food -= pyflags.food_digested;
    if (pyflags.food < 0) {
      take_hit (-pyflags.food/16, "starvation");   /* -CJS- */
      // XXXX can you die here?
      disturb(true, false);
    } /* end Food consumption */

    /* Regenerate	       */
    if (pyflags.regenerate)  regen_amount *= 3 / 2;
    if ((pyflags.status & PY_SEARCH) || pyflags.rest != 0)
      regen_amount *= 2;
    if ((pyflags.poisoned < 1) && (pymisc.chp < pymisc.mhp))
      regenhp(regen_amount);
    if (pymisc.cmana < pymisc.mana)
      regenmana(regen_amount);
    /* end Regenerate */

    /* Blindness	       */
    if (pyflags.blind > 0) {
      if ((PY_BLIND & pyflags.status) == 0) {
	pyflags.status |= PY_BLIND;
	//      prt_map(); // XXXX not implemented
	print_stats(STATS_WORD);//      prt_blind(); // XXXX not implemented
	disturb (false, true);
	/* unlight creatures */
	creatures (false);
      }
      pyflags.blind--;
      if (pyflags.blind == 0) {
	pyflags.status &= ~PY_BLIND;
	print_stats(STATS_WORD);//      prt_blind(); // XXXX not implemented
	//      prt_map(); // XXXX not implemented
	recalc_and_refresh(); //    recalc_screen();  // refresh(true);
	/* light creatures */
	disturb (false, true);
	creatures(false);
	message("The veil of darkness lifts.");
      }
    } /* end Blindness */

    /* Confusion	       */
    if (pyflags.confused > 0) {
      if ((PY_CONFUSED & pyflags.status) == 0) {
	pyflags.status |= PY_CONFUSED;
	print_stats(STATS_WORD);//      prt_confused(); // XXXX not implemented
      }
      pyflags.confused--;
      if (pyflags.confused == 0) {
	pyflags.status &= ~PY_CONFUSED;
	print_stats(STATS_WORD);//      prt_confused(); // XXXX not implemented
	message("You feel less confused now.");
	if (pyflags.rest != 0)
	  rest_off();
      }
    } /* end Confusion */

    /* Afraid		       */
    if (pyflags.afraid > 0) {
      if ((PY_FEAR & pyflags.status) == 0) {
	if ((pyflags.shero + pyflags.hero) > 0)
	  pyflags.afraid = 0;
	else {
	  pyflags.status |= PY_FEAR;
	  print_stats(STATS_WORD);//	prt_afraid(); // XXXX not implemented
	}
      } else if ((pyflags.shero + pyflags.hero) > 0)
	pyflags.afraid = 1;
      pyflags.afraid--;
      if (pyflags.afraid == 0) {
	pyflags.status &= ~PY_FEAR;
	print_stats(STATS_WORD);//      prt_afraid(); // XXXX not implemented
	message("You feel bolder now.");
	disturb (false, false);
      }
    } /* end Afraid */

    /* Poisoned	       */
    if (pyflags.poisoned > 0) {
      if ((PY_POISONED & pyflags.status) == 0) {
	pyflags.status |= PY_POISONED;
	print_stats(STATS_WORD);//      prt_poisoned(); // XXXX not implemented
      }
      pyflags.poisoned--;
      if (pyflags.poisoned == 0) {
	pyflags.status &= ~PY_POISONED;
	print_stats(STATS_WORD);//      prt_poisoned(); // XXXX not implemented
	message("You feel better.");
	disturb (false, false);
      } else {
	i = 0;
	switch(con_adj()) {
	case -4:  i = 4;  break;
	case -3:
	case -2:  i = 3;  break;
	case -1:  i = 2;  break;
	case 0:	  i = 1;  break;
	case 1: case 2: case 3:
	  i = ((turn % 2) == 0);
	  break;
	case 4: case 5:
	  i = ((turn % 3) == 0);
	  break;
	case 6:
	  i = ((turn % 4) == 0);
	  break;
	}
	take_hit (i, "poison");
	// XXX can you die here?
	disturb (true, false);
      }
    } /* end Poisoned */

    /* Fast		       */
    if (pyflags.fast > 0) {
      if ((PY_FAST & pyflags.status) == 0) {
	pyflags.status |= PY_FAST;
	change_speed(-1);
	message("You feel yourself moving faster.");
	disturb (false, false);
      }
      pyflags.fast--;
      if (pyflags.fast == 0) {
	pyflags.status &= ~PY_FAST;
	change_speed(1);
	message("You feel yourself slow down.");
	disturb (false, false);
      }
    } /* end Fast */

    /* Slow		       */
    if (pyflags.slow > 0) {
      if ((PY_SLOW & pyflags.status) == 0) {
	pyflags.status |= PY_SLOW;
	change_speed(1);
	message("You feel yourself moving slower.");
	disturb (false, false);
      }
      pyflags.slow--;
      if (pyflags.slow == 0) {
	pyflags.status &= ~PY_SLOW;
	change_speed(-1);
	message("You feel yourself speed up.");
	disturb (false, false);
      }
    } /* end Slow */

    /* Resting is over?      */
    // First, check to see if the user has interrupted it...
    if ((pyflags.rest != 0) && user_interrupted())
      rest_off();
    // User has not interrupted the rest, check other termination conditions
    if (pyflags.rest > 0) {
      pyflags.rest--;
      if (pyflags.rest == 0)		      /* Resting over	       */
	rest_off();
    } else if (pyflags.rest < 0) {
      /* Rest until reach max mana and max hit points.  */
      pyflags.rest++;
      if ((pymisc.chp == pymisc.mhp && pymisc.cmana == pymisc.mana) ||
	  pyflags.rest == 0)
	rest_off();
    }

    /* Hallucinating?	 (Random characters appear!)*/
    if (pyflags.image > 0) {
      end_find ();
      pyflags.image--;
      if (pyflags.image == 0)
	recalc_and_refresh(); //    recalc_screen();  // refresh(true);
    } /* end Hallucinating */

    /* Paralysis	       */
    if (pyflags.paralysis > 0) {
      /* when paralysis true, you can not see any movement that occurs */
      pyflags.paralysis--;
      if (!pyflags.paralysis)
	print_stats(STATS_WORD);
      disturb (true, false);
    } /* end Paralysis */

    /* Protection from evil counter*/
    if (pyflags.protevil > 0) {
      pyflags.protevil--;
      if (pyflags.protevil == 0)
	message ("You no longer feel safe from evil.");
    } /* end Protection from evil */

    /* Invulnerability	*/
    if (pyflags.invuln > 0) {
      if ((PY_INVULN & pyflags.status) == 0) {
	pyflags.status |= PY_INVULN;
	disturb (false, false);
	pymisc.pac += 100;
	pymisc.dis_ac += 100;
	need_print_stats |= STATS_AC;      //      prt_pac(); // XXXX
	message("Your skin turns into steel!");
      }
      pyflags.invuln--;
      if (pyflags.invuln == 0) {
	pyflags.status &= ~PY_INVULN;
	disturb (false, false);
	pymisc.pac -= 100;
	pymisc.dis_ac -= 100;
	need_print_stats |= STATS_AC;      //      prt_pac(); // XXX
	message("Your skin returns to normal.");
      }
    } /* end Invulnerability */

    /* Blessed       */
    if (pyflags.blessed > 0) {
      if ((PY_BLESSED & pyflags.status) == 0) {
	pyflags.status |= PY_BLESSED;
	disturb (false, false);
	pymisc.bth += 5;
	pymisc.bthb+= 5;
	pymisc.pac += 2;
	pymisc.dis_ac+= 2;
	message("You feel righteous!");
	need_print_stats |= STATS_AC;      //      prt_pac(); // XXXX
      }
      pyflags.blessed--;
      if (pyflags.blessed == 0) {
	pyflags.status &= ~PY_BLESSED;
	disturb (false, false);
	pymisc.bth -= 5;
	pymisc.bthb-= 5;
	pymisc.pac -= 2;
	pymisc.dis_ac -= 2;
	message("The prayer has expired.");
	need_print_stats |= STATS_AC;      //      prt_pac(); // XXXX
      }
    } /* end Blessed */

    /* Resist Heat   */
    if (pyflags.resist_heat > 0) {
      pyflags.resist_heat--;
      if (pyflags.resist_heat == 0)
	message ("You no longer feel safe from flame.");
    } /* end Resist heat */

    /* Resist Cold   */
    if (pyflags.resist_cold > 0) {
      pyflags.resist_cold--;
      if (pyflags.resist_cold == 0)
	message ("You no longer feel safe from cold.");
    } /* end Resist cold */

    /* Detect Invisible      */
    if (pyflags.detect_inv > 0) {
      if ((PY_DET_INV & pyflags.status) == 0) {
	pyflags.status |= PY_DET_INV;
	pyflags.see_inv = true;
	/* light but don't move creatures */
	creatures (false);
      }
      pyflags.detect_inv--;
      if (pyflags.detect_inv == 0) {
	pyflags.status &= ~PY_DET_INV;
	/* may still be able to see_inv if wearing magic item */
	calc_bonuses();
	/* unlight but don't move creatures */
	creatures (false);
      }
    } /* end Detect invisible */

    /* Timed infra-vision    */
    if (pyflags.tim_infra > 0) {
      if ((PY_TIM_INFRA & pyflags.status) == 0) {
	pyflags.status |= PY_TIM_INFRA;
	pyflags.see_infra++;
	/* light but don't move creatures */
	creatures (false);
      }
      pyflags.tim_infra--;
      if (pyflags.tim_infra == 0) {
	pyflags.status &= ~PY_TIM_INFRA;
	pyflags.see_infra--;
	/* unlight but don't move creatures */
	creatures (false);
      }
    } /* end Timed infra-vision */

    /* Word-of-Recall  Note: Word-of-Recall is a delayed action	 */
    if (pyflags.word_recall > 0) {
      if (pyflags.word_recall == 1) {
	new_level_flag = true;
	pyflags.paralysis++;
	pyflags.word_recall = 0;
	if (dun_level > 0) {
	  dun_level = 0;
	  level_message("You feel yourself yanked upwards!");
	} else if (pymisc.max_dlv != 0) {
	  dun_level = pymisc.max_dlv;
	  level_message("You feel yourself yanked downwards!");
	}
      } else
	pyflags.word_recall--;
    } /* end Word-of-recall */

    /* Random teleportation  */
    if ((pyflags.teleport) && (randint(100) == 1)) {
      disturb (false, false);
      teleport(40);
    } /* end Random teleportation */

    /* end of Update counters and messages (told you it was long)     */  
    /* ok there's not much more stuff left to do... */

    /* See if we are too weak to handle the weapon or pack.  -CJS- */
    if (pyflags.status & PY_STR_WGT)
      check_strength();

    /* Decide whether to print status-words on the screen. */
    /*  // XXXXXXXX This needs to be revised for the tiny PalmOS screen
	if (pyflags.status & PY_STUDY)
	prt_study();
	if (pyflags.status & PY_SPEED) {
	pyflags.status &= ~PY_SPEED;
	prt_speed();
	}
	if ((pyflags.status & PY_PARALYSED) && (pyflags.paralysis < 1)) {
	prt_state();
	pyflags.status &= ~PY_PARALYSED;
	}
	else if (pyflags.paralysis > 0) {
	prt_state();
	pyflags.status |= PY_PARALYSED;
	}
	else if (pyflags.rest != 0)
	prt_state();
    */  
    if (pyflags.status & PY_ARMOR) {
      need_print_stats |= STATS_AC;    //    prt_pac();
      pyflags.status &= ~PY_ARMOR;
    }
    if (pyflags.status & PY_STATS) {
      for (i = 0; i < 6; i++)
	if ((PY_STR << i) & pyflags.status)
	  need_print_stats |= STATS_STR;	// prt_stat(i);
      pyflags.status &= ~PY_STATS;
    }
    if (pyflags.status & PY_HP) {
      need_print_stats |= STATS_HP; //    prt_mhp();  prt_chp();
      pyflags.status &= ~PY_HP;
    }
    if (pyflags.status & PY_MANA) {
      need_print_stats |= STATS_MANA;    // prt_cmana();
      pyflags.status &= ~PY_MANA;
    }
    /* End of printing status type stuff. */

    /* Allow for a slim chance of detect enchantment -CJS- */
    /* for 1st level char, check once every 2160 turns    // (* 16 135)
       for 40th level char, check once every 416 turns */ // (* 16 26)
    if  ( ((turn & 0xF) == 0) &&
	  (pyflags.confused == 0) &&
	  (randint((Short)(10 + 750 / (5 + pymisc.lev))) == 1)) {

      Char tmp_str[80];
    
      for (i = 0; i < INVEN_ARRAY_SIZE; i++) {
	if (i == inven_ctr) // skip space between "inventory" and "equipment"
	  i = 22;
	// if in inventory,      succeed 1 out of 50 times,
	// if in equipment list, success 1 out of 10 times
	if ( (&inventory[i].tval != TV_NOTHING) &&
	     enchanted(&inventory[i]) &&
	     (randint(i < 22 ? 50 : 10) == 1) ) {
	  StrPrintF(tmp_str,
		    "There's something about what you are %s...",
		    describe_use(i));
	  disturb(false, false);
	  message(tmp_str);
	  add_inscribe(inventory, i, ID_MAGIK);//add_inscribe(i_ptr, ID_MAGIK);
	}
      }
    }

    /* Check the state of the monster list, and delete some monsters if
       the monster list is nearly full.  This helps to avoid problems in
       creature.c when monsters try to multiply.  Compact_monsters() is
       much more likely to succeed if called from here, than if called
       from within creatures().  */
    if (MAX_MALLOC - mfptr < 10)
      compact_monsters();

    // OK - We have now reached a comment that says "Accept a command?"

    // But in some cases we don't want to accept a command:
    // If still alive but you're paralyzed or resting, keep on do-whileing...
    if (!death && !(pyflags.paralysis <= 0 && pyflags.rest == 0)) {
      /*
      show_messages();//(this is kind of flickery, might be better to, instead,
      refresh(false); // call try_print_status() after the end_of_turn.)
      */
      end_of_turn(); // Move the monsters, then spin your wheels again
      
      if (pyflags.rest == 99) update_status_words();
      else if (0 == pyflags.rest % 10 || abs(pyflags.rest) < 99)
	try_print_status();
      // will we miss anything w/o show_msgs&refresh ?

      // let's try some stats updating, too.  Testing....
      if ((turn & 0xF) == 0) { // every 16 turns
	unverbing = true; // secret switch to turn off Words (once).
	show_messages();
      }

    }
  } while (!death && (pyflags.paralysis >= 1 || pyflags.rest != 0));

  /* if paralyzed, resting, or dead, flush output */
  // no idea how to do that
  /* but first move the cursor onto the player, for aesthetics */
  // I'm not doing that now but it would be cool.

  // these happen Right Before we accept the command i.e. not if paralyzed
  do_pickup = true;
  free_turn_flag = false;
  //  refresh(false);
}

void end_of_turn()
{
  /* Teleport?		       */
  if (teleport_flag)  teleport(100);
  /* Move the creatures	       */
  if (!new_level_flag)
    creatures(true);
  else {
    // this used to be taken care of by exiting 'while' when (new_level_flag).
    generate_cave();
    enter_new_level();
    // In case the user has gotten impatient..
    EvtFlushKeyQueue();
    EvtFlushPenQueue();
  }
}

// a c d e f i m o p s t x z ' ' G R S < > ^

/*      commands:        NOT commands:
 *   a  aim/fire wand        browse: inv
 *   c  close door           drop: inv
 *   e  equip list           look: menu
 *   t  fire/throw           quaff: inv
 *   i  inventory            read scroll: inv
 *   S# jam/spike door       take off: inv
 *   m  magic spell          version: menu
 *   o# open door/chest      wear,wield: inv
 *   p  pray                 your stats: menu
 *   s# search for trap/door eat: inv
 *   z  zap a staff          fill: inv
 *   x  exchange weapons     locate-on-map: menu
 *   <  ascend               map: menu
 *   >  descend              search mode: menu?
 *   f# force/bash           view scores: menu?
 *   ^# disarm trap/chest    set options: menu
 *   G  gain spells          help: menu
 *   R# rest                 inscribe: ????
 *   d# dig tunnel           prev msg: menu
 *    # move w/o pickup ' '  /identify char: menu   */
// I think for now I will ignore the command_count nature of MOVE and REST.
// but I will try for S, o, s, f, ^, and d
// A note on things that take a command_count:
// The various you-moving and monster_moving parts of the turn can
// 'disturb' your repetition and set command_count to 0.
// in fact, anything that prints a message will set command_count to 0
// (unless it calls count_message instead.)
// So you may end the repetition prematurely.  Make sure this works ok.
Char last_command = -1;
extern previous_state curr_state; // in main.c.  The new paradigm (heh)
extern Boolean just_throwing;
extern Boolean draw_directional_p;
extern Boolean undraw_directional_p;
extern UChar current_print_stats; // display.c; for toggle
static const Char map_arrows[4] = { 'h','l','k','j' };


// If a command should not be repeatable by AGAIN,
// then take care of it here.  Else return false & we'll put it in the struct.
static Boolean non_againable_command(Char c)
{
  switch(c) {
  case 'R':    
#ifndef I_AM_OS_2
    if (IsVGA) handera_rotate();      // Rotate screen!!!
    else message("can't rotate");
#endif
    free_turn_flag = true;
    return true;    
  case 'g':
    // the AGAIN command ...
    do_again(); // I should copy prev_cmd back to curr_cmd and call do...
    return true;
  case ' ':
    /* hack for move without pickup.  Map ' ' to a movement command. */
    // Hm.  This will need to be FIXED for the new curr_state/mode paradigm:
    do_pickup = false;
    // XXXXX Remember command_count; Draw a get-direction pie and move.
    // This turn should be a freebie.
    free_turn_flag = true;
    return true;
  case '{':		/* ({) inscribe an object    */
    free_turn_flag = true;
    if (inven_ctr == 0 && equip_ctr == 0)
      message("You are not carrying anything.");
    else {
      invequ_mode = ACT_SCRIBE;
      FrmPopupForm(InvSelectForm);
    }
    return true;
  case 'M':
    FrmPopupForm(MapForm);
    free_turn_flag = true;
    return true;    
  case 'G':		/* (G)ain magic spells */
    gain_spells();
    return true;
  case '!':
    if (current_print_stats & CURSTAT_NOT_STR)
      print_stats(STATS_STR);
    else
      print_stats(STATS_AC | STATS_GOLD);
    free_turn_flag = true;
    return true;
  case '#':		/* (#) search toggle	(S)earch toggle */
  case '"':		/* for people who have trouble writing graffiti # */
    if (pyflags.status & PY_SEARCH)
      search_off();
    else
      search_on();
    free_turn_flag = true;
    return true;
  case '/':		// (/) identify a symbol
    message("Tap (or write) a symbol to identify.");
    curr_state.mode = MODE_SLASH;
    free_turn_flag = true;
    return true;
  }
  return false;
}


#define C_NONE         0x00
#define C_VALID        0x01
#define C_COUNTABLE    0x02
#define C_DIRECTABLE   0x04
#define C_USEITEM      0x08
#define C_SPECIAL      0x10
static ULong valid_command(Char c)
{
  ULong attributes = C_NONE;
  switch(c) {
  case 'b': case 'n': case 'y': case 'u': case 'h': case 'j': case 'k':
  case 'l': case 'B': case 'N': case 'Y': case 'U': case 'H': case 'J':
  case 'K': case 'L': 
    attributes |= C_COUNTABLE;
    // fall through

  case '<': case '>': case 'x': case '.':

  case 'G': case '{': // these two pop up a form..
  case '!': case '#': case '"': // hey, these should take no time
    attributes |= C_VALID;
    break;
  case 'c': case '?': case '-':
    attributes |= C_VALID | C_DIRECTABLE;
    break;
  case 'a': case 't':
    attributes |= C_VALID | C_DIRECTABLE | C_USEITEM;
    break;
  case 'z':
    attributes |= C_VALID | C_USEITEM;
    break;
  case 'r': case 's':
    attributes |= C_COUNTABLE | C_VALID;
    break;
  case ' ': case 'f': case '^': case 'S': case 'o': case 'd':
    attributes |= C_COUNTABLE | C_VALID | C_DIRECTABLE;
    break;
  case 'e': case 'i': case 'm': case 'p': // these can turn into something else
    attributes |= C_VALID | C_SPECIAL;
    break;
  default:
    return C_NONE;
  }
  return attributes;
}

// return true if 'handled'
Boolean do_command(Char com_val) // really 'parse and then do' command
{
  // Fill in curr_state and then call do_parsed_command.
  Short dir;
  ULong attributes;
  //  Char buf[40];

  /* This has been moved to map_roguedir */
  dir = map_roguedir(&com_val); // dir==5 if noop or non-direction.
  free_turn_flag = false; // by default


  // Take care of NON-AGAINable commands first.
  // these are    g {M
  if (non_againable_command(com_val))
    return true;

  // All remaining commands are AGAINable, or are invalid.
  // Build them into a structure.

  attributes = valid_command(com_val);
  // Check for invalid command.
  if (!attributes) {
    free_turn_flag = false;
    //    StrPrintF(buf, "invalid command '%c'", com_val);
    //    message(buf);
    return false;
  }
  // Check for needing more information for curr_state.
  curr_state.cmd = com_val;
  curr_state.dir = dir;
  if (!(attributes & C_COUNTABLE)) curr_state.count = 0; //else set by main.c
  if (attributes & C_USEITEM) {
    // need item (and probably direction) before we go to do_parsed_command
    return start_use_item();
  } else if (attributes & C_DIRECTABLE) {
    // need direction before we go to do_parsed_command
    draw_directional_p = true;
    curr_state.mode = MODE_DIRECTIONAL;
    free_turn_flag = true;
    return true;
  } else
    return do_parsed_command(&curr_state);
}

// This will popup a form to set the inventory counter for curr_state.
static Boolean start_use_item()
{
  Short iii, jjj;
  free_turn_flag = true;
  switch(curr_state.cmd) {
  case 'a':		/* (a)im a wand		(a)im a wand */
    if (!find_range(TV_WAND, TV_NEVER, &iii, &jjj)) {
      message("You are not carrying any wands.");
      break;
    }
    invequ_mode = ACT_AIM;
    FrmPopupForm(InvSelectForm);
    break;
  case 'z':		/* (z)ap a staff	(u)se a staff */
    if (!find_range(TV_STAFF, TV_NEVER, &iii, &jjj)) {
      message("You are not carrying any staffs.");
      break;
    }
    invequ_mode = ACT_ZAP;
    FrmPopupForm(InvSelectForm);
    break;
  case 't':		/* (t)hrow something	(f)ire something */
    if (inven_ctr == 0) {
      message("You are not carrying anything.");
      break;
    }
    just_throwing = true;
    FrmPopupForm(InvForm);
    break;
  default:
    return false;
  }
  return true;
}

// The command should be previously parsed into a previous_state struct.
// I assume it's at least kinda valid.
// return true if 'handled'.   Note 'ps' is READ ONLY here
Boolean do_parsed_command(previous_state *ps)
{
  Char buf[80];
  if (ps->mode != MODE_DEFAULT && ps->mode != MODE_AGAIN) {
    StrPrintF(buf, "hm, non-default mode %d", ps->mode);
    message(buf);
    return false;
  }

  switch(ps->cmd) {
  // ----- NON DIRECTIONAL COMMANDS ------------------------------------
  case 'x': exchange_weap(); break;
  case 'r': rest();          break; // rest will use and zero command_count
  case '<': go_up();         break;
  case '>': go_down();       break;
  case 's': do_search();     break;

  case 'h':case 'j':case 'k':case 'l':case 'y':case 'u':case 'b':case 'n':
    if (curr_state.count <= 0) {
      move_char(ps->dir, do_pickup);
      break;
    }
    // else FALL THROUGH!  'find_run' will do counting now in moria2.c.
  case 'H':case 'J':case 'K':case 'L':case 'Y':case 'U':case 'B':case 'N':    
    find_init(ps->dir);
    if (find_flag > 0) find_run();
    break;

  case '.':
    move_char (5, do_pickup);
    if (curr_state.count > 1) {
      command_count = curr_state.count - 1;
      rest();
    }
    break;

  case 'e':		/* (e)quipment list */
    free_turn_flag = true;
    if (equip_ctr == 0)
      message("You are not using anything.");
    else
      FrmPopupForm(EquForm);
    break;
  case 'i':		/* (i)nventory list */
    free_turn_flag = true;
    if (inven_ctr == 0)
      message("You are not carrying anything.");
    else
      FrmPopupForm(InvForm);
    break;
  // ----- DIRECTIONAL COMMANDS ------------------------------------------
 // hmmmm, what to do about m and p???

  case 'm':		/* (m)agic spells */
    free_turn_flag = true; // cast_helper will decide to set it
    cast();
    break;
  case 'p':		/* (p)ray */
    free_turn_flag = true; // pray_helper will decide to set it
    pray();
    break;
  default:
   return false;
  }
  return true;
}


/******************************************************
 * Finish up a command that needed a location on screen
 * This is only used from MODE_SLASH, it's non-againable
 ********************************************************/
Boolean do_xy_command(Short x, Short y)
{
  UChar monptr;
  switch(curr_state.mode) {
  case MODE_SLASH:		// (/) identify a symbol
    free_turn_flag = true;
    // If there's a monster there,
    // set recall_this_monster to some index of c_list.
    // Otherwise do ident_char().
    monptr = cave_cptr[y * MAX_WIDTH + x];
    // hopefully adding the m_list[].ml will weed out invisible monsters
    if (monptr > 1 && m_list[monptr].ml) {
      recall_list = false;
      recall_this_monster = m_list[monptr].mptr;
      if (recall_this_monster < 0 || recall_this_monster >= MAX_CREATURES) {
	message("m_list[].mptr assertion failed"); // this better not happen
	break;
      }
      FrmPopupForm(RecallForm);
    } else
      ident_char((Char) loc_symbol(y,x));
    break;
  default:
    return false;
  }
  // Caller will bump the state.
  return true;
}

/******************************************************
 * Finish up a command that needed a compass direction
 ********************************************************/

extern Short selected_spell;

//Short needed_direction_item;
extern Short look_form_dir;
// note cast_helper and pray_helper rely on valid direction never being -1
Boolean do_dir_command(Short dir, Short item)
{
  /*Short XXX_item=0; / * This is a tmp argument.  Really these things need
		       to be setting a global variable, which you select
		       from some form or other when we do Z, z, or t. */
  if (undraw_directional_p) undraw_directional();

  if (curr_state.cmd == -1) return false;

  switch(curr_state.cmd) {

  case 'm': cast_helper(curr_state.spell, dir);   break;
  case 'p': pray_helper(curr_state.spell, dir);   break;

  case 't': throw_object(curr_state.item, dir);   break;
  case 'a': aim(curr_state.item, dir);            break;
  // case 'z': the item-selection form itself calls use(item)!
  // we only get to here if we're doing 'again'.
  case 'z': use(curr_state.item);                 break;

  case 'c': closeobject(curr_state.dir); break;
  case 'f': do_force();    break;
  case '^': do_disarm();   break;
  case 'S': do_spike();    break;
  case 'o': do_open();     break;
  case 'd': do_dig();      break;
  case '?':		/* look or examine */
    //    look(dir);
    if (pyflags.blind > 0)
      message("You can't see a damn thing!");
    else if (pyflags.image > 0)
      message("You can't believe what you are seeing! It's like a dream!");
    else {
      look_form_dir = dir;
      FrmPopupForm(LookForm);
    }
    free_turn_flag = true;
    break;
  case '-':
    slide_dir(dir);
    free_turn_flag = true;
    break;

  default:
    {
      //      Char buf[20];
      //      StrPrintF(buf, "Unknown command '%c'.", curr_state.cmd);
      //      message(buf);
      free_turn_flag = true;
      return false;
    }
  }
  // Caller will bump the state.
  return true;
}



static void regenhp(Short percent)
{
  Long new_chp, new_chp_frac;
  Short old_chp;

  old_chp = pymisc.chp;
  new_chp = ((Long)pymisc.mhp) * percent + PLAYER_REGEN_HPBASE;
  pymisc.chp += new_chp >> 16;	/* div 65536 */
  /* check for overflow */
  if (pymisc.chp < 0 && old_chp > 0)
    pymisc.chp = MAX_SHORT;
  new_chp_frac = (new_chp & 0xFFFF) + pymisc.chp_frac; /* mod 65536 */
  if (new_chp_frac >= 0x10000L) {
    pymisc.chp_frac = new_chp_frac - 0x10000L;
    pymisc.chp++;
  } else
    pymisc.chp_frac = new_chp_frac;

  /* must set frac to zero even if equal */
  if (pymisc.chp >= pymisc.mhp) {
    pymisc.chp = pymisc.mhp;
    pymisc.chp_frac = 0;
  }
  if (old_chp != pymisc.chp)
    pyflags.status |= PY_HP;// need_print_stats |= STATS_HP;  //    prt_chp();
}

static void regenmana(Short percent)
{
  Long new_mana, new_mana_frac;
  Short old_cmana;

  old_cmana = pymisc.cmana;
  new_mana = ((Long)pymisc.mana) * percent + PLAYER_REGEN_MNBASE;
  pymisc.cmana += new_mana >> 16;  /* div 65536 */
  /* check for overflow */
  if (pymisc.cmana < 0 && old_cmana > 0)
    pymisc.cmana = MAX_SHORT;
  new_mana_frac = (new_mana & 0xFFFF) + pymisc.cmana_frac; /* mod 65536 */
  if (new_mana_frac >= 0x10000L) {
    pymisc.cmana_frac = new_mana_frac - 0x10000L;
    pymisc.cmana++;
  } else
    pymisc.cmana_frac = new_mana_frac;

  /* must set frac to zero even if equal */
  if (pymisc.cmana >= pymisc.mana) {
    pymisc.cmana = pymisc.mana;
    pymisc.cmana_frac = 0;
  }
  if (old_cmana != pymisc.cmana)
    pyflags.status |= PY_MANA;//need_print_stats |= STATS_HP; //  prt_cmana();
}


/* Is an item an enchanted weapon or armor and we don't know?  -CJS- */
/* only returns true if it is a good enchantment */
// So, like, this is partly not working.  Sigh.  It says known non-magical
// things are magic (cloak, dagger).  What is up with that?!
static Boolean enchanted(inven_type *t_ptr)
{
  if (t_ptr->tval < TV_MIN_ENCHANT ||
      t_ptr->tval > TV_MAX_ENCHANT ||
      t_ptr->flags & TR_CURSED)
    return false;
  if (known2_p(t_ptr))
    return false;
  if (t_ptr->ident & ID_MAGIK)
    return false;
  if (t_ptr->tohit > 0 || t_ptr->todam > 0 || t_ptr->toac > 0)
    return true;
  if ((0x4000107fL & t_ptr->flags) && t_ptr->p1 > 0)
    return true;
  if (0x07ffe980L & t_ptr->flags)
    return true;

  return false;
}

// Note that examine_book is complete and has been parceled out to form_blah fu


/* Go up one level					-RAK-	*/
static void go_up()
{
  UChar thing;

  thing = cave_tptr[char_row*MAX_WIDTH+char_col];
  if (thing != 0) {
    if (t_list[thing].tval == TV_UP_STAIR) {
      dun_level--;
      new_level_flag = true;
      level_message("You enter a maze of up staircases.");
      message("You pass through a one-way door.");
      // end_of_turn will call generate_cave and enter_new_level.
      free_turn_flag = false;
      return;
    }
  }
  message("I see no up staircase here.");
  free_turn_flag = true;
}

/* Go down one level					-RAK-	*/
static void go_down()
{
  UChar thing;

  thing = cave_tptr[char_row*MAX_WIDTH+char_col];
  if (thing != 0) {
    if (t_list[thing].tval == TV_DOWN_STAIR) {
      dun_level++;
      new_level_flag = true;
      level_message("You enter a maze of down staircases.");
      message("You pass through a one-way door.");
      free_turn_flag = false;
      return;
    }
  }
  message("I see no down staircase here.");
  free_turn_flag = true;
}

static void do_search()
{
  //  Char buf[20];
  // This can take a multiplier in command_count.
  if (curr_state.count <= 0 && (my_prefs.repeaters & RPT_SEARCH))
    command_count = DEFAULT_COUNT;
  else command_count = curr_state.count;
  do {
    search(char_row, char_col, pymisc.srh); // moria2.c
    if (command_count <= 0) break;// avoid free_turn_flag for AGAINability
    end_of_turn();
    if (command_count > 0) command_count--;
    if (command_count == 90) update_status_words();
    else if (0 == command_count % 10 || command_count < 99) try_print_status();
    start_of_turn();
  } while ((command_count > 0) && !user_interrupted());
  command_count = 0;
  update_status_words(); // in case of remaining command_count msg
  //free_turn_flag = true;
}

static void do_verb_generic(void (*verb)(Short))
{
  do {
    verb(curr_state.dir); // moria4.c
    end_of_turn();
    if (command_count > 0) command_count--;
    if (command_count == 99) update_status_words();
    else if (0 == command_count % 10 || command_count < 99) try_print_status();
    start_of_turn();
  } while ((command_count > 0) && !user_interrupted());
  command_count = 0;
  update_status_words(); // in case of remaining command_count msg
  free_turn_flag = true; // 'cause we just did end_of and start_of.
}

static void do_force()
{
  if (curr_state.count <= 0 && (my_prefs.repeaters & RPT_FORCE))
    command_count = DEFAULT_COUNT;
  else command_count = curr_state.count;
  do_verb_generic(&bash);
}

static void do_disarm()
{
  if (curr_state.count <= 0 && (my_prefs.repeaters & RPT_DISARM))
    command_count = DEFAULT_COUNT;
  else command_count = curr_state.count;
  do_verb_generic(&disarm_trap);
}

static void do_open()
{
  if (curr_state.count <= 0 && (my_prefs.repeaters & RPT_OPEN))
    command_count = DEFAULT_COUNT;
  else command_count = curr_state.count;
  do_verb_generic(&openobject);
}

static void do_dig()
{
  if (curr_state.count <= 0 && (my_prefs.repeaters & RPT_DIG))
    command_count = DEFAULT_COUNT;
  else command_count = curr_state.count;
  if ((command_count > 0) && !check_for_shovel()) {
    // we checked for shovel being equipped and if not, popped up a form.
    // if the user wants to cancel the action, return.
    return;
  }
  do_verb_generic(&tunnel);
}

static void do_spike()
{
  command_count = curr_state.count;
  do_verb_generic(&jamdoor);
}

// missing: jamdoor()
// This involves destroying stuff from inventory so I will do it "later"
// i.e. "not tonight"
/* Jam a closed door					-RAK-	*/
// I now assume that we ALREADY HAVE A DIRECTION given.
static void jamdoor(Short dir)
{
  Short y, x, i, j, spikeness;
  inven_type *t_ptr, *i_ptr;
  Char tmp_str[80];
  UChar thing, critter;
  Boolean got_spikes;

  free_turn_flag = true;
  y = char_row;
  x = char_col;

  mmove(dir, &y, &x); // given direction calculate new row,col.

  // Make sure there is something there
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  if (thing == 0) {
    message("That isn't a door!");
    return;
  }
  // Make sure the thing is a closed door
  t_ptr = &t_list[thing];
  if (t_ptr->tval != TV_CLOSED_DOOR) {
    if (t_ptr->tval == TV_OPEN_DOOR)
      message("The door must be closed first.");
    else
      message("That isn't a door!");
    return;
  }
  // Make sure no monsters in the way
  if (critter != 0) {
    free_turn_flag = false;
    StrPrintF(tmp_str, "The %s is in your way!",
	      c_names + c_list[m_list[critter].mptr].name_offset);
    message(tmp_str);
    return;
  }
  // Find your spikes.  this sets i which we use below.
  got_spikes = find_range(TV_SPIKE, TV_NEVER, &i, &j);
  if (!got_spikes) {
    message("But you have no spikes.");
    return;
  }
  // Make the door jammed.
  free_turn_flag = false;
  count_message("You jam the door with a spike."); // XXXXX not impl yet
  spikeness = t_ptr->p1;
  if (spikeness > 0) spikeness = -spikeness; /* Make door 'locked to' stuck. */
  /* Successive spikes have a progressively smaller effect.
     Series is: 0 20 30 37 43 48 52 56 60 64 67 70 ... */
  spikeness -= 1 + 190 / (10 - spikeness);
  invy_set_p1(t_list, thing, spikeness); // make the door that much spiked.
  // Lose a spike from inventory.
  i_ptr = &inventory[i];
  if (i_ptr->number > 1) { 
    invy_set_number(inventory, i, i_ptr->number - 1); //== "i_ptr->number--"
    inven_weight -= i_ptr->weight;
  } else
    inven_destroy(i);
}




// Note that refill_lamp already complete (?) in form_inv.c



// missing: (earlier)
//    do_command  - this will be near MainForm handler
//    valid_countcommand  - ditto

/* benevolent dictatorship will check for potentially stupid action */
static Boolean check_for_shovel()
{
  EventType queued_event;
  inven_type *i_ptr;
  Short i, j;
  Char out_val[160];
  i_ptr = &inventory[INVEN_WIELD];
  if ((i_ptr->tval != TV_NOTHING) &&
      ((TR_TUNNEL & i_ptr->flags) || // you ARE wielding a shovel-type thing!
       (TR_CURSED & i_ptr->flags))) // weapon is cursed, no choice!
    return true; // weapon is cursed, user CANNOT wield anything else

  i_ptr = &inventory[INVEN_AUX];
  //  if (((i_ptr->tval == TV_NOTHING) || (TR_TUNNEL & i_ptr->flags)) &&
  //      (!find_range(TV_DIGGING, TV_NEVER, &i, &j))) {

  if (!(
	((i_ptr->tval != TV_NOTHING) && (TR_TUNNEL & i_ptr->flags)) ||
	(find_range(TV_DIGGING, TV_NEVER, &i, &j)) ))
    return true; // you have no better choice
  
  // ok... there's one in exchange slot, and you forgot to hit 'x',
  // or there's one in inventory, unequipped.
  // we'll return 'false' if the user decides not to dig with equipped item.

  i_ptr = &inventory[INVEN_WIELD];
  if (i_ptr->tval != TV_NOTHING) {
    objdes(out_val, i_ptr, true);
    out_val[StrLen(out_val)-1] = '?'; // change '.' to '?'
  } else {
    StrPrintF(out_val, "your bare hands?");
  }
  if (0 != FrmCustomAlert(DigSlowlyP, out_val, NULL, NULL)) {
    command_count = 0;
    update_status_words();
    free_turn_flag = true;
    return false;	  // "abort!"
  }
  // Here we are soaking up a winExit, winEnter, and penUp.  sigh.
  // Necessary now since I have changed user_interrupted().
  do {
    EvtGetEvent(&queued_event, 0);
  } while (queued_event.eType != nilEvent);
  return true;
}


static void exchange_weap()
{
  //  Char prt1[80], prt2[160];
  Char prt1[160], prt2[160];
  Boolean was_heavy = weapon_heavy;
  if (inventory[INVEN_WIELD].tval == TV_NOTHING &&
      inventory[INVEN_AUX].tval == TV_NOTHING) {
    message("But you are wielding no weapons.");
  } else if (TR_CURSED & inventory[INVEN_WIELD].flags) {
    objdes(prt1, &inventory[INVEN_WIELD], false);
    StrPrintF(prt2, "The %s you are wielding appears to be cursed.", prt1);
    message(prt2);
  } else {
    free_turn_flag = false;
    invy_swap(INVEN_AUX, INVEN_WIELD);
    //    tmp_obj = inventory[INVEN_AUX];
    //    inventory[INVEN_AUX] = inventory[INVEN_WIELD];
    //    inventory[INVEN_WIELD] = tmp_obj;
    //    if (scr_state == EQUIP_SCR)
    //      scr_left = show_equip(show_weight_flag, scr_left);
    py_bonuses(&inventory[INVEN_AUX], -1);	 /* Subtract bonuses */
    py_bonuses(&inventory[INVEN_WIELD], 1);	   /* Add bonuses    */
    if (inventory[INVEN_WIELD].tval != TV_NOTHING) {
      StrCopy(prt1, "Primary weapon: ");
      objdes(prt2, &inventory[INVEN_WIELD], true);
      message(StrCat(prt1, prt2));
    } else
      message("No primary weapon.");
    /* this is a new weapon, so clear the heavy flag */
    weapon_heavy = false;
    check_strength(); // might call calc_bonuses (if heavy)
    if (was_heavy && !weapon_heavy) calc_bonuses(); // not often enough. --bas
  }
}



/* map rogue_like direction commands into numbers */
/* this will modify the argument to one of hjklHJKL, if it's an arrow key. */
Char map_roguedir(Char *comval)
{
  // map the deeply irritating arrow keys for keyboards (e.g. stowaway, gotype)
  // the shift thing was particularly annoying.
  Char c = *comval;
  if (c >= leftArrowChr && c <= downArrowChr) {
    Boolean capslock_on, numlock_on, autoshifted;
    Short tmp_shift;
    c = map_arrows[c - leftArrowChr]; // convert to one of "hlkj"!
    GrfGetState(&capslock_on, &numlock_on, &tmp_shift, &autoshifted);
    if ((tmp_shift == grfTempShiftUpper) || capslock_on)
      c -= 'a'-'A';
    *comval = c; // we modify the arg, to fake out do_command().
  }
  if (c < 'A' || c > 'z') return 5; // catch most non-alphabetics
  if (c <= 'Z') c += ('a' - 'A');   // make it lowercase!
  switch(c) {
  case 'h': return 4;
  case 'y': return 7;
  case 'k': return 8;
  case 'u': return 9;
  case 'l': return 6;
  case 'n': return 3;
  case 'j': return 2;
  case 'b': return 1;
  case '.': return 5;
  }
  return 5;
}
static const Char hjkl_arr[10] = "bjnh.lyku";
Char unmap_roguedir(Short dir)
{
  if (dir < 1 || dir > 9) dir = 5;
  return hjkl_arr[dir-1];
}

static void ident_char(Char command)
{
  switch(command) {
    /* every printing ASCII character is listed here, in the order in which
       they appear in the ASCII character set */
  case ' ': message("  - An open pit."); break;
  case '!': message("! - A potion."); break;
  case '"': message("\" - An amulet, periapt, or necklace."); break;
  case '#': message("# - A stone wall."); break;
  case '$': message("$ - Treasure."); break;
  case '%':
    if (highlight_seams == true)
      message("% - A magma or quartz vein.");
    else
      message("% - Not used.");
    break;
  case '&': message("& - Treasure chest."); break;
  case '\'': message("' - An open door."); break;
  case '(': message("( - Soft armor."); break;
  case ')': message(") - A shield."); break;
  case '*': message("* - Gems."); break;
  case '+': message("+ - A closed door."); break;
  case ',': message(", - Food or mushroom patch."); break;
  case '-': message("- - A wand"); break;
  case '.': message(". - Floor."); break;
  case '/': message("/ - A pole weapon."); break;
    /* case '0': message("0 - Not used."); break; */
  case '1': message("1 - Entrance to General Store."); break;
  case '2': message("2 - Entrance to Armory."); break;
  case '3': message("3 - Entrance to Weaponsmith."); break;
  case '4': message("4 - Entrance to Temple."); break;
  case '5': message("5 - Entrance to Alchemy shop."); break;
  case '6': message("6 - Entrance to Magic-Users store."); break;
    /* case '7': message("7 - Not used."); break; */
    /* case '8': message("8 - Not used."); break; */
    /* case '9': message("9 - Not used.");  break;*/
  case ':': message(": - Rubble."); break;
  case ';': message("; - A loose rock."); break;
  case '<': message("< - An up staircase."); break;
  case '=': message("= - A ring."); break;
  case '>': message("> - A down staircase."); break;
  case '?': message("? - A scroll."); break;
  case '@': message(pymisc.name); break;
  case 'A': message("A - Giant Ant Lion."); break;
  case 'B': message("B - The Balrog."); break;
  case 'C': message("C - Gelatinous Cube."); break;
  case 'D': message("D - An Ancient Dragon (Beware)."); break;
  case 'E': message("E - Elemental."); break;
  case 'F': message("F - Giant Fly."); break;
  case 'G': message("G - Ghost."); break;
  case 'H': message("H - Hobgoblin."); break;
    // case 'I': message("I - Invisible Stalker."); break;
  case 'J': message("J - Jelly."); break;
  case 'K': message("K - Killer Beetle."); break;
  case 'L': message("L - Lich."); break;
  case 'M': message("M - Mummy."); break;
    // case 'N': message("N - Not used."); break;
  case 'O': message("O - Ooze."); break;
  case 'P': message("P - Giant humanoid."); break;
  case 'Q': message("Q - Quylthulg (Pulsing Flesh Mound)."); break;
  case 'R': message("R - Reptile."); break;
  case 'S': message("S - Giant Scorpion."); break;
  case 'T': message("T - Troll."); break;
  case 'U': message("U - Umber Hulk."); break;
  case 'V': message("V - Vampire."); break;
  case 'W': message("W - Wight or Wraith."); break;
  case 'X': message("X - Xorn."); break;
  case 'Y': message("Y - Yeti."); break;
    // case 'Z': message("Z - Not used."); break;
  case '[': message("[ - Hard armor."); break;
  case '\\': message("\\ - A hafted weapon."); break;
  case ']': message("] - Misc. armor."); break;
  case '^': message("^ - A trap."); break;
  case '_': message("_ - A staff."); break;
    // case '`': message("` - Not used."); break;
  case 'a': message("a - Giant Ant."); break;
  case 'b': message("b - Giant Bat."); break;
  case 'c': message("c - Giant Centipede."); break;
  case 'd': message("d - Dragon."); break;
  case 'e': message("e - Floating Eye."); break;
  case 'f': message("f - Giant Frog."); break;
  case 'g': message("g - Golem."); break;
  case 'h': message("h - Harpy."); break;
  case 'i': message("i - Icky Thing."); break;
  case 'j': message("j - Jackal."); break;
  case 'k': message("k - Kobold."); break;
  case 'l': message("l - Giant Louse."); break;
  case 'm': message("m - Mold."); break;
  case 'n': message("n - Naga."); break;
  case 'o': message("o - Orc or Ogre."); break;
  case 'p': message("p - Person (Humanoid)."); break;
  case 'q': message("q - Quasit."); break;
  case 'r': message("r - Rodent."); break;
  case 's': message("s - Skeleton."); break;
  case 't': message("t - Giant Tick."); break;
    // case 'u': message("u - Not used."); break;
    // case 'v': message("v - Not used."); break;
  case 'w': message("w - Worm or Worm Mass."); break;
    // case 'x': message("x - Not used."); break;
  case 'y': message("y - Yeek."); break;
  case 'z': message("z - Zombie."); break;
  case '{': message("{ - Arrow, bolt, or bullet."); break;
  case '|': message("| - A sword or dagger."); break;
  case '}': message("} - Bow, crossbow, or sling."); break;
  case '~': message("~ - Miscellaneous item."); break;
  default:	message("Not Used."); break;
  }

  /* Allow access to monster memory. -CJS- */
  // nah. -sprite
}


void ident_monster(Char c)
{
  Boolean valid = false;
  Short i;
  //  Char buf[40];

  // Is it a monster...
  if (c == '$' || c == ',' ||
      (c >= 'A' && c <= 'Y' && c != 'I' && c != 'N') ||
      (c >= 'a' && c <= 'z' && c != 'u' && c != 'v' && c != 'x'))
    valid = true;
  if (!valid) {
    //    StrPrintF(buf, "'%c' - not a monster", c);
    //    message(buf);
    ident_char(c);
    return;
  }

  // Is it a monster that we remember something about...
  for (i = MAX_CREATURES-1, valid = false ; i >= 0 ; i--)
    if ((c_list[i].cchar == c) && bool_roff_recall(i)) {
      recall_this_monster = i;
      valid = true;
      break;
    }
  if (!valid) {
    //    StrPrintF(buf, "You don't recall any '%c'", c);
    //    message(buf);
    ident_char(c);
    return;
  }

  // Pop up the form... set a boolean so it decides whether to
  // have Next button turned on (if there's a next),
  // and have it set the title appropriately
  recall_list = true;
  FrmPopupForm(RecallForm);

}


extern previous_state prev_state;
void do_again()
{
  /*
  previous_state *ps = &curr_state;
  Char buf[80];
  ps = &prev_state;
  StrPrintF(buf, "mode %d cmd %c dcmd %d dir %d # %d inv %d",
	    ps->mode, ps->cmd, ps->dir_cmd, ps->dir, ps->count, ps->item);
  message(buf);
  ps = &curr_state;
  StrPrintF(buf, "mode %d cmd %c dcmd %d dir %d # %d inv %d",
	    ps->mode, ps->cmd, ps->dir_cmd, ps->dir, ps->count, ps->item);
  message(buf);
  */
  if (prev_state.cmd != -1) {
    curr_state = prev_state;
    if (curr_state.cmd == 'm')
      cast_helper(curr_state.spell, curr_state.dir);
    else if (curr_state.cmd == 'p')
      pray_helper(curr_state.spell, curr_state.dir);
    else if (!do_parsed_command(&curr_state))
      if (!do_dir_command(curr_state.dir, curr_state.item))
	message("hmmm");
  } else
    message("I can't repeat that.");
}


/*
 * Here is a nice little function that will let the user
 *  interrupt a counted command, such as resting.
 * Call this in your loop and, if it's true, then abort.
 * Note that it flushes the interrupting event (and any others)
 *  out of the queue - I hope.  Works for pen taps.
 *  Less good for graffiti or keyboard.
 *
 */
static Boolean user_interrupted()
{
  EventType queued_event;
  //  static UChar lots = 0;
  // It helps if I make it slow enough for the user to interrupt, I'm thinking.
  // but let's speed it up if they've entered a really large number.
  //  lots = (lots + 1) % 10;
  if (curr_state.count <= COUNT_THRESHOLD)// || !lots)
    SysTaskDelay(SysTicksPerSecond()/32); // XXX Remove this line? // was /10
  /*
  // this catches pen taps but not keyboard events.
  if (EvtSysEventAvail(true)) {
    EvtFlushKeyQueue(); // these don't seem to do me any good
    EvtFlushPenQueue(); // these don't seem to do me any good
    return true;
  }
  */
  // try this instead:
  do {
    EvtGetEvent(&queued_event, 0); // wait 0 ticks, i.e. just check the queue
  } while (queued_event.eType == penUpEvent);
  if (queued_event.eType != nilEvent)
    return true;
  // ok, that sucks up the event.  even works with keyboards now.
  // BUT it broke the Rest How Long form....

  return false;
}

