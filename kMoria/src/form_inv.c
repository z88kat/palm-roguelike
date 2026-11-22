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


/* Ok.  I am going to try to design inventory and equipment forms
   so that (1) you can stay in them after some actions
   and (2) you can use them in a store for a subset of those actions.
   This means that for those stay-in-inv actions, I will need
   a different message routine, and I had better not print
   stats until I exit.
   That, or message and print_stats will have to figure out that
   I am in the inventory form.  That might be easier.
   ...Have the monster-printing functions figure that out too, so
   that I can move monsters while the player makes actions.

   What are the store actions: wear/wield, takeoff. (and exchange)
   What are the stay-in-inv actions: those plus drop.
   Make sure that these don't use message or print_stats.
   Hey, I think I will make 'drop' into 'sell' in the store!

   wear/wield: wear_wield_etc
       inven_destroy
       inven_carry
       takeoff
       py_bonuses
       check_strength
   drop:       invOrEqu_drop
       inven_drop
       check_strength
   takeof:     equ_takeoff
       inven_carry
       takeoff
       check_strength

  What about making time pass while doing inventory actions?
  I can't move monsters if it draws them to the screen.
  I guess screen-drawing things will have to secretly check
  whether they are in the main form or not.
*/

Boolean just_throwing = false; // for entering this form via 't' (throw)
//extern Char last_command; // for throwing selected item
//extern Short needed_direction_item; // for throwing selected item
extern previous_state curr_state; // replaces last_command & co.

static Boolean handle_invbtn_frob() SEC_6;
static Boolean handle_invbtn_drop() SEC_6;
static Boolean handle_invbtn_throw() SEC_6;
static Boolean handle_invbtn_cancel() SEC_6;
static Boolean handle_invbtn_eq() SEC_6;

static void show_weight() SEC_7; // was no section
static void select_best_missile(FormPtr frm) SEC_6;
static void dwimify_buttons(FormPtr frm, Short item) SEC_7;
static Boolean dwim_dispatch(Short item) SEC_7;
static Boolean match_and_fire(FormPtr frm, Char c, Short num_btns) SEC_7;
static void show_inven(FormPtr frm) SEC_7;
static void free_inventory_select(FormPtr frm) SEC_7;
static void wear_wield_etc(Short item) SEC_2;
static void examine_book(Short item_val, Short spellcaster_class) SEC_7;
Short wear_low, wear_high;
extern Short in_store; // Need to know which store we are in!

Short inventory_item;
Boolean in_a_store;
extern Short store_item;

Char **inventory_display;
Short inv_display_ctr; /* how many of inventory_display are used; value may be
			  from inven_ctr to 2*inven_ctr, inclusive. */
Short invequ_lines = 10;
extern Boolean thrown_out;
Short old_inven_weight;

/*
#define SHOW_NONE   0x0
#define SHOW_FROB   0x1
#define SHOW_DROP   0x2
#define SHOW_THROW  0x4
#define SHOW_CANCEL 0x8
#define SHOW_EQ     0x16
#define SHOW_DEFAULT     (SHOW_CANCEL | SHOW_EQ)
UInt show_invbtns; // flags
*/

extern Boolean Version_GE_OS35;
Boolean Inv_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  CharPtr label;
  Char c;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    inventory_item = -1;
    inventory_display = NULL;
    show_inven(frm);
    if (just_throwing)
      select_best_missile(frm);
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    show_weight();
    //    show_invbtns = SHOW_DEFAULT;
    dwimify_buttons(frm, inventory_item);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

#ifdef I_AM_COLOR
  case menuCmdBarOpenEvent:
    // this will only work if we are on an OS3.5 or greater device.
    if (Version_GE_OS35) {
      MenuCmdBarAddButton(2, bmp_cmd_compact, menuCmdBarResultMenuItem,
			  menu_invCompact, NULL);
      MenuCmdBarAddButton(1, (my_prefs.inv_show_weight ? bmp_cmd_wtsOff :
			                                 bmp_cmd_wts),
			  menuCmdBarResultMenuItem,
			  menu_invWeights, NULL);
    }
    return false;
#endif

  case menuEvent:
    MenuEraseStatus(NULL); /* clear menu status from display */
    switch(e->data.menu.itemID) {
    case menu_invCompact:
      my_prefs.inv_is_compacted = !my_prefs.inv_is_compacted;
      writeMoriaPrefs();
      // redraw!  yeehah.
      frm = FrmGetActiveForm();
      free_inventory_select(frm);
      show_inven(frm);
      FrmDrawForm(frm);
      dwimify_buttons(frm, inventory_item);
      break;
    case menu_invWeights:
      my_prefs.inv_show_weight = !my_prefs.inv_show_weight;
      writeMoriaPrefs();
      // redraw!  yeehah.
      frm = FrmGetActiveForm();
      free_inventory_select(frm);
      show_inven(frm);
      FrmDrawForm(frm);
      dwimify_buttons(frm, inventory_item);
      break;
    }

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
    label = LstGetSelectionText(lst, e->data.lstSelect.selection);
    // may need to retreat one row to find the letter
    if (label && label[0] == ' ')
      if (e->data.lstSelect.selection > 0)
	label = LstGetSelectionText(lst, e->data.lstSelect.selection - 1);
    // ok this row should have an index letter 
    if (label) inventory_item = label[0] - 'a';
    else       inventory_item = -1; // bug if this happens
    dwimify_buttons(frm, inventory_item);
    // also clear the message space
    if (old_inven_weight == inven_weight) {
      clear_brief_comment();
    } else
      show_weight(0);
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
    switch(e->data.keyDown.chr) {
    case pageUpChr: case 8: case 127:
      if (LstScrollList(lst, winUp, invequ_lines)) {//  usually 10
	LstSetSelection(lst, -1);
	inventory_item = -1;
	dwimify_buttons(frm, inventory_item);
      }
      handled = true;
      break;
    case pageDownChr: case ' ':
      if (LstScrollList(lst, winDown, invequ_lines)) {
	LstSetSelection(lst, -1);
	inventory_item = -1;
	dwimify_buttons(frm, inventory_item);
      }
      handled = true;
      break;
    default:
      c = e->data.keyDown.chr;
      if ((LstGetSelection(lst) != -1) && match_and_fire(frm, c, 3)) {
	// if we already selected in lst, and c is the first letter of a
	// button, we fire the button.
	handled = true;
	break;
      }
      if (c >= 'a' && c <= 'z') {
	Short s = e->data.keyDown.chr - 'a';
	// Find the line starting with this letter. in order; some have 2 lines
	// so search from s to 2*s (or, in practice, to the end of the list
	// because no letter-indices are skipped, e.g. can't have 'f' w/o 'e').
	inventory_item = -1;
	do {
	  label = LstGetSelectionText(lst, s);
	  if (label && (label[0] == c)) {
	    LstSetSelection(lst, s); // index in list,
	    inventory_item = c - 'a';
	    break;
	  }
	} while (++s < inv_display_ctr);
	if (inventory_item == -1) LstSetSelection(lst, -1);
	dwimify_buttons(frm, inventory_item);         // ...index in inventory.
	handled = true;	
      } else {
	// Keyboard shortcuts for the buttons.  Woo hoo.
	Word objid = -1;
	switch(c) {
	  // Need to make it accept the first letter of the button IFF
	  // an inventory item is already selected.
	case '1': case '\n': objid = btn_if_frob; break;
	case '2': case 'S': objid = btn_if_drop; break; // 'S' for Sell!!
	case '3': case 'T': objid = btn_if_throw; break;
	case '9': case 'D': case 'X': case 27: objid = btn_if_cancel; break;
	case '0': case 'E': objid = btn_if_eq; break;
	default: break;
	}
	if (objid != -1) {
	  hit_button_if_usable(frm, objid); // kind of silly, I could just dispatch it..
	  handled = true;
	} // else ignore the keystroke
      }
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_if_frob:
      handled = handle_invbtn_frob();
      break;
    case btn_if_drop:
      handled = handle_invbtn_drop();
      break;
    case btn_if_throw:
      handled = handle_invbtn_throw();
      break;
    case btn_if_cancel:
      handled = handle_invbtn_cancel();
      break;
    case btn_if_eq:
      handled = handle_invbtn_eq();
      break;
    }

  default:
    break;
  }
  return handled;
}

/////////////////////////////////////////////////////////////

// Handlers for the form buttons

static Boolean handle_invbtn_frob()
{
  FormPtr frm = FrmGetActiveForm();
  free_inventory_select(frm);
  if (inventory_item != -1) {
    Boolean mainform = dwim_dispatch(inventory_item);
    // Note: need to take a turn
    take_a_turn(mainform);
  }
  just_throwing = false;
  return true;
}

static Boolean handle_invbtn_drop()
{
  FormPtr frm = FrmGetActiveForm();
  //      LeaveForm();
  if (inventory_item != -1) {
    free_inventory_select(frm);
    if (!in_a_store) {
      invOrEqu_drop(inventory_item);
      show_inven(frm);
      //FrmDrawForm(frm); // we really just need to redraw the list:
      LstDrawList(FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if)));
      dwimify_buttons(frm, inventory_item);
      // Note: need to take a turn
      //calc_bonuses(); end_of_turn(); start_of_turn();
      take_a_turn(false);
    } else {
      LeaveForm();
#ifndef I_AM_OS_2
      if (IsVGA) PrvMoveForm(true);
#endif
      store_item = inventory_item;
      // Note: store, when done, must call 	  calc_bonuses();
      if (store_sell_init(in_store, store_item))
	FrmPopupForm(HaggleForm);
      else if (thrown_out)
	// due to that LeaveForm, we are now in the StoreForm... so...
	store_form_boot();
    }
  }
  just_throwing = false;
  return true;
}

static Boolean handle_invbtn_throw()
{
  FormPtr frm = FrmGetActiveForm();
  free_inventory_select(frm);
  LeaveForm();
  // Note: thrower, when done, must call 	  calc_bonuses();
  // also, need to take a turn.
  if (inventory_item != -1) {
    curr_state.cmd = 't';
    curr_state.mode = MODE_DIRECTIONAL;
    curr_state.item = inventory_item;
    draw_directional();
    free_turn_flag = true;
  }
  just_throwing = false;
  return true;
}

static Boolean handle_invbtn_cancel()
{
  FormPtr frm = FrmGetActiveForm();
  free_inventory_select(frm);
  LeaveForm();
  show_messages(); // because time passed
  refresh(false); // because creatures may have moved
  just_throwing = false;
  return true;
}

static Boolean handle_invbtn_eq()
{
  FormPtr frm = FrmGetActiveForm();
  free_inventory_select(frm);
  LeaveForm();
  FrmPopupForm(EquForm);
  just_throwing = false;
  return true;
}

/////////////////////////////////////////////////////////////

static void show_weight()
{
  Char buf[80];
  Short w;
  if (just_throwing) return;
  old_inven_weight = inven_weight;
  if (!my_prefs.inv_show_weight)
    StrPrintF(buf, "You are carrying %d.%d pounds.", 
	      inven_weight / 10, inven_weight % 10);
  else {
    w = weight_limit();
    StrPrintF(buf, "Carrying %d.%d lb, capacity %d.%d lb.", 
	      inven_weight / 10, inven_weight % 10,
	      w / 10, w % 10);
  }
  message(buf); // note - we don't want this one in the log.
}

// set list selection if we find a missile.
// will try to deduce what kind of missile the player prefers.
const UChar bow_p1_ammo[7] = { TV_SPIKE,
			       TV_SLING_AMMO, TV_ARROW, TV_ARROW, TV_ARROW,
			       TV_BOLT, TV_BOLT };// see facts().
static void select_best_missile(FormPtr frm)
{
  Short bow = 0, i, s=-1, best;//, first;
  UChar t;
  ListPtr lst;
  CharPtr label;
  Char c;

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
  // What's the best ammo for your weapon.
  if (inventory[INVEN_WIELD].tval == TV_BOW) {
    bow = inventory[INVEN_WIELD].p1;
    if (bow > 6) bow = 0; // (bounds check prob. unnecessary)
  }
  // Try to find that ammo, or (failing that) any ammo.
  for (best = -1, i = 0 ; i < inven_ctr ; i++) {
    t = inventory[i].tval;
    if ((t >= TV_SLING_AMMO) && (t <= TV_SPIKE)) {
      if (best == -1) best = i;    // default:   _first_ ammo 
      if (t == bow_p1_ammo[bow]) { // preferred: _appropriate_ ammo
	best = i;
	break;
      }
    }
  }
  // If we found ammo, set the list selection to it.
  if (best != -1) {
    s = best; // found start of missiles...
    c = best + 'a';
    do {
      label = LstGetSelectionText(lst, s);
      if (label && (label[0] == c)) {
	LstSetSelection(lst, s);
	inventory_item = best;
	LstMakeItemVisible(lst, s); // yay!  this does the right thing.
	break;
      }
    } while (++s < inv_display_ctr);
  }
  // The caller will dwimify the buttons.
}

// This determins what string is printed on the frob button.
static Short do_what(Short item_type)
{
  if (item_type >= TV_MIN_WEAR && item_type <= TV_MAX_WEAR) {
    if (item_type < TV_MID_WEAR)
      return ACT_WIELD;
    return ACT_WEAR;
  }
  switch(item_type) {
  case TV_STAFF:
    return ACT_ZAP;
  case TV_WAND:
    return ACT_AIM;
  case TV_FLASK:
    return ACT_REFILL;
  case TV_SCROLL1: case TV_SCROLL2:
    return ACT_SCROLL;
  case TV_POTION1: case TV_POTION2:
    return ACT_QUAFF;
  case TV_FOOD:
    return ACT_EAT;
  case TV_PRAYER_BOOK:
    return ACT_PRAYRB;
  case TV_MAGIC_BOOK:
    return ACT_SPELLB;
  }
  return ACT_NONE;
}
const Char if_button_labels[10][7] = {
  "eat", "refill", "wield", "wear", "read",
  "read", "read", "quaff", "aim", "zap"
};
static void dwimify_buttons(FormPtr frm, Short item)
{
  Short i;
  ControlPtr btn;

  if (just_throwing) {
    // enable throw, disable [E], everything else ok by default.
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_throw));
    if (item == -1) CtlHideControl(btn);
    else CtlShowControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_eq));
    CtlHideControl(btn);
    return;
  }

  // hide the '[E]' button if !equip_ctr.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_eq));
  CtlHideControl(btn);
  for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE ; i++)
    if (inventory[i].tval != TV_NOTHING) {
      CtlShowControl(btn);
      break;
    }
  // hide drop/throw if nothing selected
  if (item == -1 || in_a_store) {
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_frob));
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_throw));
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_drop));
    if (item == -1) {
      CtlHideControl(btn);
      return;
    } else {
    // only allow the sell button if you are in the right KNID of store.
      Boolean flag;
      flag = is_store_buy(in_store, inventory[item].tval);
      if (flag) {
	CtlSetLabel(btn, "sell");
	CtlShowControl(btn);
      } else {
	CtlHideControl(btn);
      }
    }
  } else {
    // also, disable the 'drop' button if cave_tptr[char_row][char_col]!=0
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_drop));
    if (cave_tptr[char_row*MAX_WIDTH+char_col] == 0)
      CtlShowControl(btn);
    else
      CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_throw));
    CtlShowControl(btn);
  }
  // based on type of item,
  // make button 1 usable or not, set its text.
  // it could be a function pointer if I felt evil, though,
  i = do_what(inventory[item].tval);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_if_frob));
  if (i == ACT_NONE || (in_a_store && i != ACT_WIELD && i != ACT_WEAR))
    CtlHideControl(btn);
  else {
    CtlSetLabel(btn, if_button_labels[i-1]);
    CtlShowControl(btn);    
  }
  // In store, you are allowed to wear/wield (inv), and to take off (equ).
  // And also to exchange weapons.  Disable anything else.
}

// 'item' should be an index into inventory[].
// returns true if we left the form
static Boolean dwim_dispatch(Short item)
{
  Short i;
  FormPtr frm;
  if (item == -1) return false;
  i = do_what(inventory[item].tval);
  // Actions that let you stay in inventory:
  switch(i) {
  case ACT_WIELD: case ACT_WEAR:
    wear_wield_etc(item);
    frm = FrmGetActiveForm();
    show_inven(frm);
    //FrmDrawForm(frm); // Nah, we really only need to redraw the list, so...
    LstDrawList(FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if)));
    //    inventory_item = -1;
    dwimify_buttons(frm, inventory_item);
    return false;
  }
  // Actions that require you to leave inventory:
  LeaveForm();
  switch(i) {
  case ACT_QUAFF:
    quaff(item);
    break;
  case ACT_EAT:
    eat(item);
    break;
  case ACT_REFILL: // this ignores 'item'!
    refill_lamp();
    break;
  case ACT_AIM:
    curr_state.cmd = 'a';
    curr_state.mode = MODE_DIRECTIONAL;
    curr_state.item = inventory_item;
    draw_directional();
    free_turn_flag = true;
    break;
  case ACT_ZAP:
    use(item);
    break;
  case ACT_SCROLL:
    read_scroll(item); // note it fails if dark, blind, or confused
    break;
  case ACT_PRAYRB:
    examine_book(item, PRIEST);
    break;
  case ACT_SPELLB:
    // This button will only have been enabled if you're the right class.
    // also consider disabling if:
    // pyflags.blind > 0; no_light(); pyflags.confused > 0.
    // (right now you just get a message saying you can't dig it.)
    examine_book(item, MAGE);
    break;
    // This should be like Browse
    //    pray(item); // note this could fail if dark, blind, or confused;
    //                  and it should pop up a "which prayer?" form, itself.
    //    message("read prayer book"); break;
    // This should be like Browse
    //    cast(item); // note this could fail if dark, blind, or confused;
    //                 and it should pop up a "which spell?" form, itself.
    //                 I should combine cast/pray and examine_book
    //                 probably.
    //    message("read spell book"); break;
  default:
    message("not implemented yet");
  }
  return true;
}

static const Word btn_ids[4] = {
  btn_if_frob, btn_if_drop, btn_if_throw, btn_if_cancel
};
static Boolean match_and_fire(FormPtr frm, Char c, Short num_btns)
{
  ControlPtr btn;
  Short i;
  const Char *btnlabel;
  // Determine whether 'c' is the initial letter of a button in the form.
  // If so then returh the button objid.
  for (i = 0 ; i < num_btns ; i++) {
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_ids[i]));
    btnlabel = CtlGetLabel(btn);
    if (btnlabel[0] == c) {
      hit_button_if_usable(frm, btn_ids[i]);
      return true;
    }
  }
  return false;
}

static void show_inven(FormPtr frm)
{
  Short i, j, invdisp_item = -1;
  Char tmp_val[160], c, ctmp = ' ';
  Int width_str, length_str, uncut_len;
  Boolean fits;
  ListPtr lst;

  if (inven_ctr <= 0) {
    just_throwing = false;
    inventory_item = -1;
    // dwimify should take care of disabling buttons.
    return;
  }

  // first, figure out whether we should free existing list.
  if (inventory_display != NULL)
    free_inventory_select(frm);

  inventory_display = (Char **) md_malloc(sizeof(Char *) * 2*inven_ctr);

  /* Print the items	  */
  for (i = 0, inv_display_ctr = 0, c = 'a' ; i < inven_ctr ; i++, c++) {
    if (i == inventory_item)
      invdisp_item = inv_display_ctr; // we will select this one, when finished

    // This will not work because \x80 is not whitespace on the Visor.
    //if (c == 'i' || c == 'l') StrPrintF(tmp_val, "%c)\x80", c); else
    StrPrintF(tmp_val, "%c) ", c);
    if (!just_throwing && my_prefs.inv_show_weight) {
      Short total_weight = inventory[i].weight * inventory[i].number;
      StrPrintF(tmp_val+3, "%d.%d lb : ",
		(total_weight) / 10,
		(total_weight) % 10);
    }
    objdes(tmp_val+StrLen(tmp_val), &inventory[i], true);
    uncut_len = StrLen(tmp_val); // max chars to allow

    // Truncate if too wide for the screen..... we only have 156 pixels...
    // length_str gets set to the # of chars that will fit in 156
    // width_str gets set to the # of pixels that those chars will use (<=156)
    width_str = SCR_WIDTH - 4; // 156
    length_str = uncut_len; // max chars to allow
    FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
    if (!fits && !my_prefs.inv_is_compacted) {
      width_str = (SCR_WIDTH - 4) - 9; // subtract the width of those danged scroll-arrows.  this may not be enough for handera, but who cares.
      FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
      j = 0; // try to find a ' ' to cut at. don't look back real far though.
      while (tmp_val[length_str-j] != ' ' && j < 10) j++;
      if (j < 10) length_str -= j; // found a reasonable ' '.
      ctmp = tmp_val[length_str];
    }
    tmp_val[length_str] = '\0';
    inventory_display[inv_display_ctr] = (Char *) md_malloc(1 + length_str);
    StrCopy(inventory_display[inv_display_ctr], tmp_val);
    inv_display_ctr++;
    if (!fits && !my_prefs.inv_is_compacted) {
      tmp_val[length_str] = ctmp;
      // use a second row!  and start it with at least one ' '.
      inventory_display[inv_display_ctr] =
	(Char *) md_malloc(3 + 1 + (uncut_len - length_str));
      StrPrintF(inventory_display[inv_display_ctr], "   %s",
		&tmp_val[length_str]);
      inv_display_ctr++;
    }
  }

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
  LstSetListChoices(lst, inventory_display, inv_display_ctr);
  LstSetSelection(lst, invdisp_item); // -1, or [0 .. inv_display_ctr-1]
}

/*
 *  Free some space that we used in the inventory-selection form
 */
static void free_inventory_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;

  if (inven_ctr <= 0) return;

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_if));
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < inv_display_ctr ; i++) {
    h = MemPtrRecoverHandle(inventory_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(inventory_display);
  if (h) 
    MemHandleFree(h);
  inventory_display = NULL;
}




/* Refill the players lamp				-RAK-	*/
void refill_lamp()
{
  Short i, j;
  Short k, qty;
  inven_type *i_ptr;

  free_turn_flag = true;
  k = inventory[INVEN_LIGHT].subval;
  if (k != 0)
    message("But you are not using a lamp.");
  else if (!find_range(TV_FLASK, TV_NEVER, &i, &j))
    message("You have no oil.");
  else {
    free_turn_flag = false;
    i_ptr = &inventory[INVEN_LIGHT];
    qty = i_ptr->p1 + inventory[i].p1;
    if (qty > OBJ_LAMP_MAX) {
      qty = OBJ_LAMP_MAX;
      message ("Your lamp overflows, spilling oil on the ground.");
      message("Your lamp is full.");
    } else if (qty > OBJ_LAMP_MAX/2)
      message ("Your lamp is more than half full.");
    else if (qty == OBJ_LAMP_MAX/2)
      message ("Your lamp is half full.");
    else
      message ("Your lamp is less than half full.");
    invy_set_p1(inventory, INVEN_LIGHT, qty);
    desc_remain(i);
    inven_destroy(i);
  }
}


static void wear_wield_etc(Short item)
{
  Short slot, tmp2, tmp;
  Char prt1[160], prt2[160];
  inven_type tmp_obj, *i_ptr, tmp_obj2;
  Char *string;

  /* Wearing. Go to a bit of trouble over replacing
     existing equipment. */
  //  if (isupper((int)which) && !verify(prompt, item))
  //    item = -1;
  //  else
  slot = 0;
  switch(inventory[item].tval) {
    /* Slot for equipment	   */
  case TV_SLING_AMMO: case TV_BOLT: case TV_ARROW:
  case TV_BOW: case TV_HAFTED: case TV_POLEARM:
  case TV_SWORD: case TV_DIGGING: case TV_SPIKE:
    slot = INVEN_WIELD; break;
  case TV_LIGHT: slot = INVEN_LIGHT; break;
  case TV_BOOTS: slot = INVEN_FEET; break;
  case TV_GLOVES: slot = INVEN_HANDS; break;
  case TV_CLOAK: slot = INVEN_OUTER; break;
  case TV_HELM: slot = INVEN_HEAD; break;
  case TV_SHIELD: slot = INVEN_ARM; break;
  case TV_HARD_ARMOR: case TV_SOFT_ARMOR:
    slot = INVEN_BODY; break;
  case TV_AMULET: slot = INVEN_NECK; break;
  case TV_RING:
    if (inventory[INVEN_RIGHT].tval == TV_NOTHING)
      slot = INVEN_RIGHT;
    else if (inventory[INVEN_LEFT].tval == TV_NOTHING)
      slot = INVEN_LEFT;
    else {
      // Rings. Give choice over where they go.
      // I have changed this to "Put ring on which hand? Left %s Right %s"
      objdes(prt1, &inventory[INVEN_LEFT], true);
      objdes(prt2, &inventory[INVEN_RIGHT], true);
      if (0 == FrmCustomAlert(RingHandP, prt1, prt2, NULL))
	slot = INVEN_LEFT;
      else
	slot = INVEN_RIGHT;
    }
    break;
  default:
    message("I don't see how you can use that.");
    item = -1;
    break;
  }
  if (item >= 0 && inventory[slot].tval != TV_NOTHING) {
    if (TR_CURSED & inventory[slot].flags) {
      objdes(prt1, &inventory[slot], false);
      //if (slot == INVEN_HEAD) // hello???
      if (slot == INVEN_WIELD) // let's try that shall we
	FrmCustomAlert(CursedP, prt1, "wielding", NULL);
      else
	FrmCustomAlert(CursedP, prt1, "wearing", NULL);
      // "The %s you are %s appears to be cursed."
      item = -1;
    } else if (inventory[item].subval == ITEM_GROUP_MIN &&
	       inventory[item].number > 1 &&
	       !inven_check_num(&inventory[slot])) {
      /* this can happen if try to wield a torch, 
	 and have more than one in inventory */
      message("You'll have to drop something first.");
      item = -1;
    }
  }

  if (item >= 0) {
    /* OK. Wear it. */
    free_turn_flag = false;
			      
    /* first remove new item from inventory */
    tmp_obj = inventory[item];
    i_ptr = &tmp_obj;
			      
    wear_high--;
    /* Fix for torches	   */
    if (i_ptr->number > 1
	&& i_ptr->subval <= ITEM_SINGLE_STACK_MAX) {
      i_ptr->number = 1;
      wear_high++;
    }
    //      message("freak");
    //      SysTaskDelay(2*SysTicksPerSecond());
    inven_weight += i_ptr->weight*i_ptr->number;
    inven_destroy(item);	/* Subtracts weight */ // Problem for Torch!!!
    //      message("torch");
    //      SysTaskDelay(2*SysTicksPerSecond());
			      
    /* second, add old item to inv and remove from
       equipment list, if necessary */
    i_ptr = &inventory[slot];
    if (i_ptr->tval != TV_NOTHING) {
      tmp2 = inven_ctr;
      tmp_obj2 = *i_ptr;
      tmp = inven_carry(&tmp_obj2);//      tmp = inven_carry(i_ptr);
      /* if item removed did not stack 
	 with anything in inventory, 
	 then increment wear_high */
      if (inven_ctr != tmp2)
	wear_high++;
      takeoff(slot, tmp);
    }
    //

    /* third, wear new item */
    //    *i_ptr = tmp_obj; // uh, this will not work.  instead,
    DmWrite(inventory, slot*sizeof(inven_type), &tmp_obj, sizeof(inven_type));
    equip_ctr++;
    py_bonuses(i_ptr, 1);
    if (slot == INVEN_WIELD)
      string = "Wielding:";
    else if (slot == INVEN_LIGHT)
      string = "Lighted:";
    else
      string = "Wearing:";
    objdes(prt2, i_ptr, true);
    /* Get the right equipment letter. */
    tmp = INVEN_WIELD;
    item = 0;
    while (tmp != slot)
      if (inventory[tmp++].tval != TV_NOTHING)
	item++;
			      
    //    StrPrintF(prt1, "%s %s (%c)", string, prt2, 'a'+item);
    StrPrintF(prt1, "%s %s", string, prt2);
    message(prt1);
    /* this is a new weapon, so clear heavy flag */
    if (slot == INVEN_WIELD)
      weapon_heavy = false;
    check_strength(); // this also calls calc_bonuses if heaviness changed
    if (i_ptr->flags & TR_CURSED) {
      message("Oops! It feels deathly cold!");
      add_inscribe(inventory, slot, ID_DAMD);
      /* To force a cost of 0, 
	 even if unidentified. */
      invy_set_cost(inventory, slot, -1); //i_ptr->cost = -1;
    }
  }
}

/*
  'context sensitive' inventory buttons: (you can also inscribe anything)
  food: eat, throw, drop
  oil: refill, throw, drop
  weapon: wield, throw, drop
  armor: wear, throw, drop
  scroll: read, throw, drop
  potion: quaff, throw, drop
  book: read, throw, drop
  wand: aim, throw, drop
  staff: zap, throw, drop
  
  equipment: take off, exchange, drop .. ALSO wear/wield from inventory...
  you can also {t, x, d, w} in stores


  inventory:  see inven_command for {w,d}
  browse/peruse a book: examine_book()
  drop any item
  quaff a potion: quaff()
  read a scroll: read_scroll()
  wear/wield an item
  eat food: eat()
  fill lamp with oil: refill_lamp()
  (maybe) inscribe an object: scribe_object()
  
  equipment:  see inven_command for {d,t}
  drop any item
  take off / unwield any item
*/


extern Boolean just_browsing_book; 
extern spell_type magic_spell[MAX_CLASS-1][MAX_SPELLS]; // magic.c
extern Short spells[MAX_SPELLS]; // form_spell.c
extern Short spell_ctr; // ditto 
extern Short selected_book;
static void examine_book(Short item_val, Short spellcaster_class)
{
  ULong j;
  Short i, k, my_class = pymisc.pclass - 1;

  if (pyflags.blind > 0)
    message("You can't see to read your spell book!");
  else if (no_light())
    message("You have no light to read by.");
  else if (pyflags.confused > 0)
    message("You are too confused.");
  else if (class[pymisc.pclass].spell != spellcaster_class)
    message("You do not understand the language.");
  else {
    just_browsing_book = true;
    i = 0;
    j = inventory[item_val].flags;
    while (j) {
      k = bit_pos(&j);
      if (magic_spell[my_class][k].slevel < 99) {
	spells[i] = k;
	i++;
      } // else it's a spell your class never gets; sorry dude
    }
    spell_ctr = i;
    selected_book = item_val;
    //    save_screen();
    //    print_spells(spell_index, i, TRUE, -1);
    //    pause_line(0);
    //    restore_screen();
    FrmPopupForm(GainSpForm); // ok, I lied we're using THIS form
  }
}



// For use in Inventory and in Equipment, only.
// (Oh, fine, let's use it for cast_helper and pray_helper too.)
// Make time pass.  If we already left the inv/equ form, do message and refrsh.
// If we are dead, leave all forms and go to death form.
// also, you should not call LeaveForm after this puppy; before, or not at all.
void take_a_turn(Boolean mainform)
{
  if (!free_turn_flag) {
    calc_bonuses();
    end_of_turn();
    start_of_turn();
  }
  if (mainform) {
    show_messages();
    refresh(false);
  }
  if (death) {
    if (!mainform)
      LeaveForm();
    LeaveForm(); // i.e. leave MainForm too.
    FrmGotoForm(DeathForm);
  }
}

