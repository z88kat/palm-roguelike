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

/* This is a form for just selecting an inventory or equipment item,
   and leaving (to perform some previously selected action like "inscribe".)
*/

static void invequ_dispatch(Short item) SEC_6;
static void show_invequ(FormPtr frm, Short mode) SEC_6;
static void free_invequ_select(FormPtr frm) SEC_6;
#ifdef I_AM_COLOR
Err beam_object(Short item) SEC_7; // this is in form_beam.c
#endif

//extern Char last_command; // for aiming selected wand
//extern Short needed_direction_item; // for aiming selected wand
extern previous_state curr_state; // replaces last_command & co.


UChar invequ_mode = ACT_NONE; // what action we are supposed to be taking
Short invequ_scroll = -1; // the item_val of the scroll used to get here, or -1
Short invequ_charge; // so recharge knows how much juice to use

Short invequ_item;
Short *invequ_indices; //f. x = line selected ==> inventory[f(x)] = item wanted
Char **invequ_display;
Short items_used;
Short listitems_used;
Short invsel_lines = 11;

Boolean InvS_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  CharPtr label;
  Char c;
  Short s;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    invequ_item = -1;
    invequ_display = NULL;
    invequ_indices = NULL;
    show_invequ(frm, invequ_mode);
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    // also set the title based on invequ_mode ...
    switch(invequ_mode) {
    case ACT_AIM:
      FrmCopyTitle(frm, "Aim a Wand"); break;
    case ACT_ZAP:
      FrmCopyTitle(frm, "Zap a Staff"); break;
    case ACT_RECHARGE:
      FrmCopyTitle(frm, "Recharge which?"); break;
    case ACT_IDENTIFY:
      FrmCopyTitle(frm, "Identify which?"); break;
    case ACT_SCRIBE:
      FrmCopyTitle(frm, "Inscribe which?"); break;
    case ACT_BEAM:
      FrmCopyTitle(frm, "Send which?"); break;
    }
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
    invequ_item = (s != -1) ? invequ_indices[s] : -1;
    // forget all that label[0] - 'a' stuff...
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_is));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, winUp, invsel_lines)) {// usually 11
	LstSetSelection(lst, -1);
	invequ_item = -1;
      }
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, winDown, invsel_lines)) {
	LstSetSelection(lst, -1);
	invequ_item = -1;
      }
      handled = true;
      break;
    default:
      c = e->data.keyDown.chr;
      if (c >= 'a' && c <= 'z') {
	// Find the line starting with this letter. Lines in order with gaps.
	s = listitems_used;
	invequ_item = -1;
	while (s > -1) {
	  label = LstGetSelectionText(lst, s);
	  if (label && (label[0] == e->data.keyDown.chr)) {
	    LstSetSelection(lst, s);
	    invequ_item = invequ_indices[s];
	    break;
	  }
	  s--;
	}
	if (invequ_item == -1) LstSetSelection(lst, -1);
	handled = true;
      } else {
	// Keyboard support for buttons.  Woo hoo.
	Word objid = -1;
	switch(c) {
	case '1': case '\n':  objid = btn_is_frob;   break;
	case '9': case '0' :  case 27: objid = btn_is_cancel; break;
	}
	if (objid != -1) {
	  hit_button_if_usable(frm, objid);
	  handled = true;
	}
      }
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_is_frob:
      if (invequ_item != -1) {
	free_invequ_select(frm);
	LeaveForm();
	// Then do something, and decide whether to take a turn about it.
	// aim, zap, recharge, identify, scribe.
	invequ_dispatch(invequ_item);
	// take_a_turn(true);
	invequ_scroll = -1;
      }
      handled = true;
      break;
    case btn_is_cancel:
      free_invequ_select(frm);
      LeaveForm();
      invequ_scroll = -1;
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}


void bump_state() SEC_7;
// 'item' should be an index into inventory[].
static void invequ_dispatch(Short item)
{
  if (item == -1) return;

  switch(invequ_mode) {
  case ACT_AIM:
    curr_state.cmd = 'a';
    curr_state.mode = MODE_DIRECTIONAL;
    curr_state.item = item;
    draw_directional();
    free_turn_flag = true;
    break;
  case ACT_ZAP:
    curr_state.cmd = 'z';
    curr_state.item = item;
    use(item);
    bump_state();
    take_a_turn(true);
    break;
  case ACT_RECHARGE:
    // check out recharge in spells.c
    free_turn_flag = false;
    recharge(item, invequ_charge);
    // If we came here from a recharge scroll, we must DESTROY IT. 
    // (do that after we recharge the object so msgs are in a good order)
    /* There is a chance of destroying wand/staff (but perhaps inventory
       order makes this step unnecessary?) : */
    while (invequ_scroll != -1 &&
	   (inventory[invequ_scroll].tval != TV_SCROLL1 ||
	    inventory[invequ_scroll].flags != 0x01000000L))
      invequ_scroll--;
    // Found the recharge scroll?  Destroy it...
    if (invequ_scroll != -1) {
      desc_remain(invequ_scroll);
      inven_destroy(invequ_scroll);
    }
    take_a_turn(true); // think I need this
    break;
  case ACT_IDENTIFY:
    // check out ident_spell in spells.c
    free_turn_flag = false;
    ident_spell(item);
    /* The identify may merge objects, causing the identify scroll
       to move to a different place.	Check for that here.  It can
       move arbitrarily far if an identify scroll was used on
       another identify scroll, but it always moves down. */
    while (invequ_scroll != -1 &&
	   (inventory[invequ_scroll].tval != TV_SCROLL1 ||
	    inventory[invequ_scroll].flags != 0x00000008L))
      invequ_scroll--;
    // Found the identify scroll?  Destroy it...
    if (invequ_scroll != -1) {
      desc_remain(invequ_scroll);
      inven_destroy(invequ_scroll);
    }
    take_a_turn(true); // think I need this
    break;
  case ACT_SCRIBE:
    // pop up a new form!  note that inscribing should NOT take a turn at all.
    // needed_direction_item = item;
    curr_state.item = item;
    FrmPopupForm(ScribeForm);
    break;
#ifdef I_AM_COLOR
  case ACT_BEAM:
    beam_object(item);
    // hey, should we make it take a turn?
    break;
#endif
  default:
    message("not implemented yet");
  }
}


// Show either:
//   only wands
//   only staffs
//   just wands and staffs
//   all inventory and all equipment
// This is probably full of terrible bugs and wants proof reading
static void show_invequ(FormPtr frm, Short mode)
{
  Short i, j, k, m;
  Char tmp_val[160], c, *prt1;
  Int width_str, length_str;
  Boolean fits, ok;
  ListPtr lst;
  Short equips_used = 0, inven_used = 0;

  if (mode == ACT_IDENTIFY || mode == ACT_SCRIBE) {
    for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE ; i++)
      if (inventory[i].tval != TV_NOTHING)
	equips_used++; // all equipment items are wanted
    j = 0;
    k = inven_ctr - 1; // index of last inventory item (all are wanted)
  } else if (mode == ACT_BEAM) {
    // we won't let you beam something you're wearing/wielding...
    j = 0;
    k = inven_ctr - 1;
  } else {
    ok = false;
    if (mode == ACT_RECHARGE) ok = find_range(TV_STAFF, TV_WAND, &j, &k);
    else if (mode == ACT_AIM) ok = find_range(TV_WAND, TV_NEVER, &j, &k);
    else if (mode == ACT_ZAP) ok = find_range(TV_STAFF, TV_NEVER, &j, &k);
    if (!ok) return; // No staves or wands; this "should never happen"
  }
  inven_used = (k - j) + 1; // 'cause the valid items are j to k *inclusive*
  //  StrPrintF(tmp_val, "inven_used %d", inven_used);
  //  WinDrawChars(tmp_val, StrLen(tmp_val), 20, 20);

  // first, figure out whether we should free existing list.
  if (invequ_display != NULL)
    free_invequ_select(frm);

  m = 2 * equips_used + inven_used;  
  invequ_display = (Char **) md_malloc(sizeof(Char *) * m);
  invequ_indices = (Short *) md_malloc(sizeof(Short)  * m);

  // first the inventory items.
  for (i = j, listitems_used = 0 ; i <= k ; i++, listitems_used++) {
    StrPrintF(tmp_val, "%c) ", (Char) ('a' + i));
    objdes(tmp_val+3, &inventory[i], true);
    // Truncate if too wide for the screen..... we only have 156 pixels...
    // length_str gets set to the # of chars that will fit in 156
    // width_str gets set to the # of pixels that those chars will use (<=156)
    width_str = SCR_WIDTH - 4; // 156;
    length_str = StrLen(tmp_val); // max chars to allow
    FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
    tmp_val[length_str] = '\0';
    // ok, now like put it somewhere.
    invequ_display[listitems_used] = (Char *) md_malloc(1 + length_str);
    StrCopy(invequ_display[listitems_used], tmp_val);
    invequ_indices[listitems_used] = i;
  }

  // add equipment items also (perhaps).
  if (mode == ACT_IDENTIFY || mode == ACT_SCRIBE) {
    c = 'A';
    j = listitems_used; // just for convenience
    for (i = INVEN_WIELD ; i < INVEN_ARRAY_SIZE && j < m ; i++) {
      if (inventory[i].tval != TV_NOTHING) {
	invequ_indices[j] = invequ_indices[j+1] = i;
	switch(i) {
	case INVEN_WIELD:
	  if (pystats.use_stat[A_STR]*15 < inventory[i].weight)
	    prt1 = "Just lifting";
	  else
	    prt1 = "Wielding";
	  break;
	case INVEN_HEAD:	prt1 = "On head";       break;
	case INVEN_NECK:	prt1 = "Around neck";   break;
	case INVEN_BODY:	prt1 = "On body";       break;
	case INVEN_ARM:		prt1 = "On arm";        break;
	case INVEN_HANDS:	prt1 = "On hands";      break;
	case INVEN_RIGHT:	prt1 = "On right hand"; break;
	case INVEN_LEFT:	prt1 = "On left hand";  break;
	case INVEN_FEET:	prt1 = "On feet";       break;
	case INVEN_OUTER:	prt1 = "About body";    break;
	case INVEN_LIGHT:	prt1 = "Light source";  break;
	case INVEN_AUX:		prt1 = "Spare weapon";  break;
	default:  		prt1 = "Unknown value"; break;
	}
	invequ_display[j] = (Char *) md_malloc(3 + StrLen(prt1) + 1);
	StrPrintF(invequ_display[j], "%c) %s", c, prt1);
	j++;
	objdes(tmp_val, &inventory[i], true);
	width_str = SCR_WIDTH-4 - 3; // 156-3; // allow room for 1 space
	length_str = StrLen(tmp_val);
	FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
	tmp_val[length_str] = '\0'; // truncate at screen width
	invequ_display[j] = (Char *) md_malloc(1 + StrLen(tmp_val) + 1);
	StrPrintF(invequ_display[j], " %s", tmp_val);
	j++;
	c++;
      }
    }
    listitems_used = j;
  }
  
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_is));
  LstSetListChoices(lst, invequ_display, listitems_used);
  LstSetSelection(lst, -1);
}


/*
 *  Free some space that we used in the inventory-selection form
 */
static void free_invequ_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_is));
  LstSetListChoices(lst, NULL, 0);
  if (!listitems_used) return;
  for (i = 0 ; i < listitems_used ; i++) {
    h = MemPtrRecoverHandle(invequ_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(invequ_display);
  if (h) 
    MemHandleFree(h);
  h = MemPtrRecoverHandle(invequ_indices);
  if (h) 
    MemHandleFree(h);
  invequ_display = NULL;
  invequ_indices = NULL;
}
