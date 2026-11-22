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


static void show_equip(FormPtr frm) SEC_7;
static void free_equipment_select(FormPtr frm) SEC_7;
static void dwimify_buttons(FormPtr frm, Short item) SEC_7;
static void equ_takeoff(Short item) SEC_7;
extern Short wear_low, wear_high; // these live in form_inv.c
extern Boolean in_a_store; // ditto
extern Short in_store; // Need to know which store we are in!
extern Short store_item; // lives in form_store.c
Short *equip_indices;
Boolean equ_is_compacted = false; /* Should long-titled items be given 2 lines.
				     This needs to be saved like a Pref... */

Char **equipment_display;
Short equips_used;
Short listitems_used;
extern Short invequ_lines;


Boolean Equ_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short s, e_item;
  ListPtr lst;
  Char c;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    equipment_display = NULL;
    equip_indices = NULL;
    show_equip(frm);
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    dwimify_buttons(frm, -1);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    s = e->data.lstSelect.selection;
    if (s != -1) {
      clear_brief_comment();
      e_item = equip_indices[s];
      dwimify_buttons(frm, e_item);
    }
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
    switch(e->data.keyDown.chr) {
    case pageUpChr: case 8: case 127:
      if (LstScrollList(lst, winUp, invequ_lines)) { // usually 10
	LstSetSelection(lst, -1);
	dwimify_buttons(frm, -1);
      }
      handled = true;
      break;
    case pageDownChr: case ' ':
      if (LstScrollList(lst, winDown, invequ_lines)) {
	LstSetSelection(lst, -1);
	dwimify_buttons(frm, -1);
      }
      handled = true;
      break;
    default:
      c = e->data.keyDown.chr;
      if ((LstGetSelection(lst) != -1) &&  (c == 't' || c == 'd')) {
	// If item is selected, let t and d fire the takeoff and drop btns.
	hit_button_if_usable(frm, (c == 't') ? btn_eq_off : btn_eq_drop);
	handled = true;
	break;
      }
      if (c >= 'a' && c <= 'z') {
	CharPtr label;
	Short s = e->data.keyDown.chr - 'a';
	// Find the line starting with this letter. in order; some have 2 lines
	// so search from s to 2*s (or, in practice, to the end of the list
	// because no letter-indices are skipped, e.g. can't have 'f' w/o 'e').
	e_item = -1;
	do {
	  label = LstGetSelectionText(lst, s);
	  if (label && (label[0] == c)) {
	    LstSetSelection(lst, s); // index in list,
	    e_item = equip_indices[s]; //e_item = c - 'a';
	    break;
	  }
	} while (++s < listitems_used);
	if (e_item == -1) LstSetSelection(lst, -1);
	dwimify_buttons(frm, e_item);         // ...index in inventory.
	handled = true;	
      } else {
	// Keyboard shortcuts for the buttons.  Woo hoo.
	Word objid = -1;
	switch(c) {
	case '1': objid = btn_eq_off; break;
	case '2': objid = btn_eq_drop; break;
	case '9': case 27: case 'D': objid = btn_eq_cancel; break;
	case '0': case 'I': objid = btn_eq_if; break;
	default: objid = -1; break;
	}
	if (objid != -1) {
	  hit_button_if_usable(frm, objid);
	  handled = true;
	} // else ignore the keystroke
      }
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
    s = LstGetSelection(lst);
    if (s == -1) e_item = -1;
    else e_item = equip_indices[s];
    switch(e->data.ctlSelect.controlID) {
    case btn_eq_off:
      if (e_item != -1) {
	free_equipment_select(frm);
	equ_takeoff(e_item);
	show_equip(frm);
	LstDrawList(lst);	//FrmDrawForm(frm);
	dwimify_buttons(frm, -1);
	// Note: need to take a turn
	//calc_bonuses(); end_of_turn(); start_of_turn();
	take_a_turn(false);
      }
      handled = true;
      break;
    case btn_eq_drop:
      if (e_item != -1) {
	free_equipment_select(frm);
	if (!in_a_store) {
	  invOrEqu_drop(e_item);
	  show_equip(frm);
	  LstDrawList(lst);	  //FrmDrawForm(frm);
	  dwimify_buttons(frm, -1);
	  // Note: need to take a turn
	    //calc_bonuses(); end_of_turn(); start_of_turn();
	  take_a_turn(false);
	}
	/* It's easier if we don't let you sell things from equipment.
	   The original doesn't and inven_destroy would have to be frobbed.
	else {
	  LeaveForm();
	  store_item = e_item;
	  // Note: store, when done, must call 	  calc_bonuses();
	  if (store_sell_init(in_store, store_item))
	    FrmPopupForm(HaggleForm);
	  // else:  may have been thrown out... xxxx need to deal with that
	}
	*/
      }
      handled = true;
      break;
    case btn_eq_cancel:
      free_equipment_select(frm);
      LeaveForm();
#ifndef I_AM_OS_2
      if (in_a_store && IsVGA) PrvMoveForm(true);
#endif
      show_messages(); // because time passed
      refresh(false); // because creatures may have moved
      handled = true;
      break;
    case btn_eq_if:
      free_equipment_select(frm);
      LeaveForm();
      FrmPopupForm(InvForm);
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}

void init_equ(FormPtr frm)
{
  // Set up the equipment foo
  // Select the first item so we never have to make all buttons invisible?
  // (for useless items, the first button will be invisible)
}


// Unlike inventory, the buttons DON'T really change meaning here.
static void dwimify_buttons(FormPtr frm, Short item)
{
  ControlPtr btn;

  // based on type of item,
  // make button 1 usable or not, set its text, and set dwim_action.
  // actually dwim_action may be unneeded.
  // it could be a function pointer if I felt evil, though,

  // also, hide the '[I]' button if !inven_ctr.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_if));
  if (inven_ctr > 0)
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
  // hide takeoff/drop if nothing selected
  if (item == -1) {
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_off));
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_drop));
    CtlHideControl(btn);
    return;
  }
  // disable the 'take off' button if there's no room in the pack.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_off));
  if (inven_check_num(&inventory[item])) 
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
  // disable the 'drop' button if there's already stuff on the ground
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_eq_drop));
  if (in_a_store) {
    CtlHideControl(btn);
    /* It's easier if we don't let you sell things from equipment.
    // only allow the sell button if you are in the right KNID of store.
    Boolean flag;
    flag = is_store_buy(in_store, inventory[item].tval);
    if (flag) {
      CtlSetLabel(btn, "sell");
      CtlShowControl(btn);
    } else {
      CtlHideControl(btn);
    }
    */
  } else if (cave_tptr[char_row*MAX_WIDTH+char_col] == 0)
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
}

/*
static void dwim_dispatch(Short item)
{
  // remove it or something.
  message("take off");
}
*/

// I have left off all of the original arguments, and return value...
static void show_equip(FormPtr frm)
{
  Short i, j, uncut_len;
  Char tmp_val[160], c, *prt1, ctmp=' ';
  Int width_str, length_str;
  Boolean fits;
  ListPtr lst;
  //  Char buf[80];

  equips_used = 0;
  listitems_used = 0;
  for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE ; i++)
    if (inventory[i].tval != TV_NOTHING)
      equips_used++;

  if (equips_used <= 0)
    return; // dwimify should take care of disabling buttons.

  // first, figure out whether we should free existing list.
  if (equipment_display != NULL)
    free_equipment_select(frm);

  equipment_display = (Char **) md_malloc(sizeof(Char *) * 2*equips_used);
  equip_indices = (Short *) md_malloc(sizeof(Short) * 2*equips_used);

  c = 'a';
  j = 0;
  for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE && j < 2*equips_used ; i++) {
    if (inventory[i].tval != TV_NOTHING) {
      equip_indices[j] = equip_indices[j+1] = i;
      switch(i) {
/*
  (mapcar (lambda (x) (- 59 x)) '(43 34 34 50 35 31 38 56 51 32 47 50 57))
  (16 25 25 9 24 28 21 3 8 27 12 9 2)
*/
	/*
      case INVEN_WIELD:
	if (pystats.use_stat[A_STR]*15 < inventory[i].weight)
	  prt1 = "Just lifting   \x80\x80"; // 43 + 16
	else
	  prt1 = "Wielding\x80\x80\x80\x80\x80"; // 34 + 25
	break;
      case INVEN_HEAD:	prt1 = "On head\x80\x80\x80\x80\x80"; break; // 34 + 25
      case INVEN_NECK:	prt1 = "Around neck  \x80"; break; // 50 + 9
      case INVEN_BODY:	prt1 = "On body  \x80\x80\x80\x80"; break; // 35 + 24
      case INVEN_ARM:	prt1 = "On arm    \x80\x80\x80\x80"; break; // 31 + 28
      case INVEN_HANDS:	prt1 = "On hands   \x80\x80\x80"; break; // 38 + 21
      case INVEN_RIGHT:	prt1 = "On right hand "; break; // 56 + 3
      case INVEN_LEFT:	prt1 = "On left hand    "; break; // 51 + 8
      case INVEN_FEET:	prt1 = "On feet \x80\x80\x80\x80\x80"; break;// 32 + 27
      case INVEN_OUTER:	prt1 = "About body \x80\x80"; break; // 47 + 12
      case INVEN_LIGHT:	prt1 = "Light source  \x80"; break; // 50 + 9
      case INVEN_AUX:	prt1 = "Spare weapon "; break; // 57 + 2
      default:  	prt1 = "Unknown value "; break; // 61; sign of bug
      }
      // whitespace: '\x20' = 2, '\x80' = 5
      // c will range from a thru l.... width 5:abdeghk 4:cf 3:j 2:il
      switch(c) { // make it line up a bit better
      case 'i': case 'l': StrPrintF(tmp_val, "%c)\x80%s", c, prt1); break;
      case 'j':           StrPrintF(tmp_val, "%c)  %s", c, prt1); break;
	// case 'c': case 'f': it will just have to be a pixel short.
      default:            StrPrintF(tmp_val, "%c) %s", c, prt1); break;
      }
	*/
      case INVEN_WIELD:
	if (pystats.use_stat[A_STR]*15 < inventory[i].weight)
	  prt1 = "Just lifting        "; // 43 + 16
	else
	  prt1 = "Wielding            "; // 34 + 25 *
	break;
      case INVEN_HEAD:	prt1 = "On head            "; break; // 34 + 25 *
      case INVEN_NECK:	prt1 = "Around neck    "; break; // 50 + 9 *
      case INVEN_BODY:	prt1 = "On body            "; break; // 35 + 24
      case INVEN_ARM:	prt1 = "On arm              "; break; // 31 + 28
      case INVEN_HANDS:	prt1 = "On hands          "; break; // 38 + 21 *
      case INVEN_RIGHT:	prt1 = "On right hand "; break; // 56 + 3
      case INVEN_LEFT:	prt1 = "On left hand    "; break; // 51 + 8
      case INVEN_FEET:	prt1 = "On feet             "; break;// 32 + 27 *
      case INVEN_OUTER:	prt1 = "About body      "; break; // 47 + 12
      case INVEN_LIGHT:	prt1 = "Light source    "; break; // 50 + 9 *
      case INVEN_AUX:	prt1 = "Spare weapon "; break; // 57 + 2
      default:  	prt1 = "Unknown value "; break; // 61; sign of bug
      }
      // whitespace: '\x20' = 2, '\x80' = 5
      // But, unfortunately, \x80 is not whitespace on the Visor.
      // c will range from a thru l.... width 5:abdeghk 4:cf 3:j 2:il
      switch(c) { // make it line up a bit better
	// i, l, c, f are a pixel short, as are *'ed strings above.
	// If I were in a perfectionist mood  I would set a boolean at *s
	// and (if it were true at i, l, c, f) add another ' ' after ')'.
      case 'i': case 'l': case 'j':
	StrPrintF(tmp_val, "%c)  %s", c, prt1); break;
	// case 'c': case 'f': it will just have to be a pixel short.
      default:
	StrPrintF(tmp_val, "%c) %s", c, prt1); break;
      }
      if (my_prefs.inv_show_weight) {
	Short total_weight = inventory[i].weight * inventory[i].number;
	StrPrintF(tmp_val + StrLen(tmp_val), "%d.%d lb : ",
		  (total_weight) / 10,
		  (total_weight) % 10);
      }
      objdes(tmp_val + StrLen(tmp_val), &inventory[i], true);
      uncut_len = StrLen(tmp_val);
      // width_str = 156-9;// subtract the width of those danged scroll-arrows.
      width_str = SCR_WIDTH-4-1; // 156-1 // leave a little 
      length_str = StrLen(tmp_val);
      FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
      if (!fits) {
	Short k;
	k = 0; // try to find a ' ' to cut at. don't look back real far though.
	while (tmp_val[length_str-k] != ' ' && k < 10) k++;
	if (k < 10) length_str -= k; // found a reasonable ' '.
	ctmp = tmp_val[length_str];
      }
      tmp_val[length_str] = '\0';
      equipment_display[j] = (Char *) md_malloc(1 + length_str);
      StrCopy(equipment_display[j], tmp_val);
      j++;
      // now the (mandatory) second row.  the first character should be ' '.
      if (!fits) {
	tmp_val[length_str] = ctmp;
	equipment_display[j] = (Char *) md_malloc(4 + uncut_len - length_str);
	StrPrintF(equipment_display[j], "   %s", &tmp_val[length_str]);
      } else {
	equipment_display[j] = (Char *) md_malloc(2);
	StrPrintF(equipment_display[j], " ");
      }
      j++;
      c++;
    }
  }
  
  listitems_used = j;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
  LstSetListChoices(lst, equipment_display, listitems_used);
  LstSetSelection(lst, -1);
}

/*
 *  Free some space that we used in the inventory-selection form
 */
static void free_equipment_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_eq));
  LstSetListChoices(lst, NULL, 0);
  if (!listitems_used) return;
  for (i = 0 ; i < listitems_used ; i++) {
    h = MemPtrRecoverHandle(equipment_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(equipment_display);
  if (h) 
    MemHandleFree(h);
  equipment_display = NULL;
  h = MemPtrRecoverHandle(equip_indices);
  if (h) 
    MemHandleFree(h);
  equip_indices = NULL;
}


// both Inventory and Equipment form use this.
void invOrEqu_drop(Short item)
{
  Boolean all = true;
  if (item < 0) return;
  // can't drop cursed item if it's being wielded/worn...
  if ((FrmGetActiveFormID() == EquForm) &&
      (TR_CURSED & inventory[item].flags)) {
    message("Hmmm, it seems to be cursed.");
    return;
  }
  // [moved the curse-check so that it only occurs in equipment]
  // hey, need to pop up an alert to ask all-or-one for some things
  if (inventory[item].number > 1) {
    Char buf[160];
    // The form says "Drop all ^1"
    objdes(buf, &inventory[item], true);
    buf[StrLen(buf)-1] = '?';
    all = (0 == FrmCustomAlert(DropAllP, buf, NULL, NULL));
  }
  inven_drop(item, all);
  // As a safety measure, set the player's inven weight to 0, 
  // when the last object is dropped
  if (inven_ctr == 0 && equip_ctr == 0)
    inven_weight = 0;
  check_strength();
  free_turn_flag = false;
}

// only Equipment form uses this.
static void equ_takeoff(Short item)
{
  Short slot;
  if (item < 0) return;
  if (TR_CURSED & inventory[item].flags) {
    message("Hmmm, it seems to be cursed.");
    return;
  }
  if (!inven_check_num(&inventory[item])) {
    // Note that the takeoff button is not enabled unless you have room
    // in your pack.  So this code will never be used.
    /*
    if (in_a_store || cave_tptr[char_row*MAX_WIDTH+char_col] != 0)
      message("You can't carry it.");
    else {
      Char buf[160];
      // The form says "You can't carry ^1  Drop it?"
      objdes(buf, &inventory[item], true);
      if (0 == FrmCustomAlert(DropWornP, buf, NULL, NULL))
	invOrEqu_drop(item);
    }
    */
    message("You can't carry it.");
    return;
  }
  slot = inven_carry(&inventory[item]);
  takeoff(item, slot);
  check_strength();
  free_turn_flag = false;
}
