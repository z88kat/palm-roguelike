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
 * inventory.c
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



extern Short task; // main.c; used here lots
extern Short direction; // main.c; used here in inv select to throw thing
extern void LeaveForm(); // iRogue.c


static Short what_do_i_mean(Short index, Short button) SEC_1;
static void do_what_i_mean(FormPtr frm) SEC_1;
static void init_inventory_select(FormPtr frm, Short task) SEC_1;
static void free_inventory_select(FormPtr frm) SEC_1;
static void deduce_task(Short index, Short *primary, Short *secondary,
			Boolean *call) SEC_1;
static void specialize_labels(FormPtr frm, Short index) SEC_1;
static void toggle_ring_buttons(FormPtr frm, Boolean hide) SEC_1;
static Short get_ring_finger(FormPtr frm) SEC_1;

VoidHand view_TextHandle = NULL;
//#define INV_BUF_SIZE 160
//Char inv_buf[INV_BUF_SIZE];
Char ** inventory_select_items;
Short inventory_select_numItems;
extern Short inventory_item;


/**********************************************************************
                       INIT_INVENTORY_SELECT
 IN:
 frm = the form to initialize part of
 task = either it's INV_IDENTIFY or it's not (general use)
 various globals
 OUT:
 nothing
 PURPOSE:
 This initializes part of the form that's used to list inventory items
 (and possible actions on items, if the task is not to 'identify')
 for selection by the user.
 I sense that it could stand some cleaning up.
 Do not use the inventory form if the pack is empty.
 **********************************************************************/
extern Short ScrWidth;
static void init_inventory_select(FormPtr frm, Short task)
{
  ListPtr lst;
  ControlPtr pushbtn;
  Short numItems, i, k;
  object *obj;
  Char my_buf[80];
  Char cur_ichar = 'a';

  /* one little special-case */
  if (task == INV_IDENTIFY) {
    FrmCopyTitle(frm, "...identify what?");
    for (i = pbtn_if_frob ; i <= pbtn_if_call ; i++) {
      /* drop,eat,quaff,read,wield,puton,remove,wear,takeoff,zap,throw. call.*/
      /* the L/R is already hidden by default */
      pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
      CtlHideControl(pushbtn);
    }
    // Also, disable the cancel button!  Since you can't unread the scroll.
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_i_cancel));
    CtlHideControl(pushbtn);
  }
  // (hey what if the inventory scroll is the only item you have?)

  /* set the stuff in the list */
  numItems = 0;
  for (obj = sotu->roguep->pack.next_object ;
       obj != NULL ;
       obj = obj->next_object) {
    numItems++;
  }

  if (numItems == 0)
    ; /* XXXXXX */
  obj = sotu->roguep->pack.next_object;
  inventory_select_items = (Char **) md_malloc(sizeof(Char *) * numItems);

  // the pack is empty!  the caller is supposed to be checking for that.
  ErrNonFatalDisplayIf ( (inventory_select_items == NULL),
			 "XXXXXX SERIOUS EVIL");

  for (i = 0 ; i < numItems && obj ; i++, obj = obj->next_object ) 
    {
      /* if (obj->what_is & mask) */
      obj->ichar = cur_ichar++; /* re-order the inventory letters */
      if (cur_ichar > 'z')
	cur_ichar = 'A';
      my_buf[0] = obj->ichar; 
      my_buf[1] = ((obj->what_is & ARMOR) && obj->is_protected) ? ']' : ')';
      my_buf[2] = ' ';
      k = 3;
      if (obj->in_use_flags & BEING_USED) {
	if ( (obj->in_use_flags & BEING_WIELDED)
	     || (obj->in_use_flags & BEING_WORN) ) {
	  my_buf[2] = my_buf[1];
	  my_buf[3] = ' ';
	  k=4;
	} else if (obj->in_use_flags & ON_LEFT_HAND) {
	  my_buf[1] = 171;
	} else if (obj->in_use_flags & ON_RIGHT_HAND) {
	  my_buf[1] = 187;
	}
      }
      get_desc_brief(obj, my_buf + k); /* er.  will it overflow? */


      // in 160x160 mode, the list is 122 pixels wide.
      // scale it to the real size of the screen (badly... but...
      // there isn't an API call to ask the List how wide it is...)
      // (the /2's and /4's are to make the calculation fit in Short)
      /*      make_it_fit(my_buf, 122, 80); */
      make_it_fit(my_buf, ((122/2)*(ScrWidth/2))/(160/4) ,
		  80); // 80 is size of my_buf

      /* ok done truncating */
      inventory_select_items[i] = md_malloc(sizeof(Char)*(StrLen(my_buf)+1));

      // (malloc failed.)
      ErrNonFatalDisplayIf ( (inventory_select_items[i] == NULL),
			     "XXXXXX SERIOUS EVIL");

      StrCopy(inventory_select_items[i], my_buf);
    } /* end for */
  if (i < numItems) {
    numItems = i; /* Just In Case! */
  }
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_i));
  LstSetListChoices(lst, inventory_select_items, numItems);
  LstSetSelection(lst, -1);
  inventory_select_numItems = numItems;
  /* And just who is planning to free this memory I'd like to know? */
}

/**********************************************************************
                       FREE_INVENTORY_SELECT
 IN:
 frm = the inventory select form, again
 various globals
 OUT:
 nothing
 PURPOSE:
 This is called when the form is exited, I guess,
 and tries to free up some memory allocated by its initializer.
 **********************************************************************/
static void free_inventory_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_i));
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < inventory_select_numItems ; i++) {
    h = MemPtrRecoverHandle(inventory_select_items[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(inventory_select_items);
  if (h) 
    MemHandleFree(h);
}

/**********************************************************************
                       WHAT_DO_I_MEAN
 IN:
 index of inventory-object currently selected
 index of button currently selected
 various globals
 OUT:
 what "I mean"
 PURPOSE:
 Called when the player taps "ok" in the inventory-frobbing dialog,
 this will return the task that should be performed on the object.
**********************************************************************/
static Short what_do_i_mean(Short index, Short button)
{
  Short first = 0, fourth = 0;
  Boolean call = false;
  deduce_task(index, &first, &fourth, &call);
  switch (button) {
  case pbtn_if_frob:
    return first;
  case pbtn_if_drop:
    return INV_DROP;
  case pbtn_if_throw:
    return INV_THROW;
  case pbtn_if_wield:
    return fourth;
  case pbtn_if_call:
    return INV_CALL; // perhaps I should bother to check call
  }
  return 0;
}
/**********************************************************************
                       WHAT_DO_I_MEAN
 IN:
 form, various globals including inventory_item and task.
 PURPOSE:
 Called when the player taps "ok" in the inventory-frobbing dialog,
 this will perform the task on the object and exit the form.
**********************************************************************/
static void do_what_i_mean(FormPtr frm)
{
  Boolean more;
  Short finger;
  free_inventory_select(frm); // must come before LeaveForm, I reckon
  if (task != INV_PUTON)
    LeaveForm(); // must come before calls to message()
  switch (task) {
  case INV_DROP:
    if (check_drop_item())
      drop(inventory_item); // make sure it's not cursed + there's room
    break;
  case INV_THROW:
  case INV_ZAP:
    /* leave 'inventory_item' set, go on to set 'direction'
       and have that form "do the right thing" */
    direction = NO_DIRECTION;
    FrmPopupForm(DirectionForm);
    break;
  case INV_WIELD:
    if (sotu->roguep->weapon && sotu->roguep->weapon->is_cursed)
      message("already wielding a cursed weapon");
    else
      wield(inventory_item);
    break;
  case INV_UNWIELD:
    if (sotu->roguep->weapon) {
      if (sotu->roguep->weapon->is_cursed)
	message("you can't, it's cursed");
      else
	unwield(sotu->roguep->weapon, sotu->roguep);
    }
    break;
  case INV_EAT:
    if (eat(inventory_item))
      reg_move();
    break;
  case INV_QUAFF:
    if (quaff(inventory_item))
      reg_move();
    break;
  case INV_READ:
    if (read_scroll(inventory_item, &more))
      reg_move();
    if (more) {
      task = INV_IDENTIFY; // here we go again
      FrmPopupForm(InvSelectForm);
    }
    break;
  case INV_REMOVE:
    remove_ring(inventory_item);
    reg_move();
    break;
  case INV_PUTON:
    finger = get_ring_finger(frm);
    LeaveForm(frm);
    if (finger != -1) {
      if (put_on_ring(inventory_item, finger))
	reg_move();
    }
    break;
  case INV_TAKEOFF:
    take_off();
    break;
  case INV_WEAR:
    if (sotu->roguep->armor) {
      take_off();
    }
    wear(inventory_item);
    break;
  case INV_CALL:
    // form will consult 'inventory_item'.
    FrmPopupForm(CallForm);
    break;
  default:
    message("doing nothing");
    break;
  }
}

// this is in iRogue.c
#ifndef I_AM_OS_2
void PrvMoveObject(FormPtr frmP, UInt objid, Coord y_diff, Boolean draw);
void PrvResizeList(FormPtr frmP, UInt objid, UInt bot_objid, Coord y_diff);
#endif
Boolean InvSelect_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr pushbtn;
  ListPtr lst;
  Short i, selected_btn, lvis;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* mine */
#ifndef I_AM_OS_2
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_i_ok, y_diff, false);
      PrvMoveObject(frm, btn_i_cancel, y_diff, false);
      // should resize list too: add y_diff / 11 lines to it
      init_inventory_select(frm, task);
      PrvResizeList(frm, list_i, btn_i_ok, y_diff);
    } else
#endif
      init_inventory_select(frm, task);
    /* end mine */	
    FrmDrawForm(frm);
    toggle_ring_buttons(frm, true); // must be _after_ form is drawn


    handled = true;
    break;


#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      // move buttons
      PrvMoveObject(frm, btn_i_ok, y_diff, true);
      PrvMoveObject(frm, btn_i_cancel, y_diff, true);
      // should resize list too: add y_diff / 11 lines to it
      PrvResizeList(frm, list_i, btn_i_ok, y_diff);
      FrmDrawForm(frm);
    }
    break;
#endif

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    //    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_i));
    inventory_item = e->data.lstSelect.selection;
    if (task != INV_IDENTIFY)
      specialize_labels(frm, inventory_item);
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_i));
    lvis = LstGetVisibleItems(lst);
    if (lvis < 11) lvis = 11; // minimum of 11 items visible at a time
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, winUp, lvis)) {
	LstSetSelection(lst, -1);
	inventory_item = -1;
	if (task != INV_IDENTIFY)
	  specialize_labels(frm, inventory_item);
      }
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, winDown, lvis)) {
	LstSetSelection(lst, -1);
	inventory_item = -1;
	if (task != INV_IDENTIFY)
	  specialize_labels(frm, inventory_item);
      }
      handled = true;
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_i_ok:
      handled = true;
      if (inventory_item == -1) {
	break;
      }
      if (task == INV_IDENTIFY) {
	/* this will only be prompted by INV_READ */
	free_inventory_select(frm);
	LeaveForm();
	idntfy(inventory_item);
	task = 0;
      } else {
	for (i = pbtn_if_frob, selected_btn = 0 ; i <= pbtn_if_call ; i++) {
	  pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
	  if (CtlGetValue(pushbtn))
	    selected_btn = i;
	}
	task = what_do_i_mean(inventory_item, selected_btn);
	do_what_i_mean(frm);
      }
      break;
    case btn_i_cancel:
      free_inventory_select(frm);
      LeaveForm();
      handled = true;
      break;
    case pbtn_if_frob:
    case pbtn_if_drop:
    case pbtn_if_throw:
    case pbtn_if_wield:
    case pbtn_if_call:
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}



/**********************************************************************
                       DEDUCE_TASK
 IN:
 index of inventory-object currently selected
 return value -- primary DWIM-task
 return value -- secondary DWIM-task
 various globals
 OUT:
 nothing
 PURPOSE:
 Called when an inventory object is selected; it figures out what
 tasks are appropriate to it.
 **********************************************************************/

static void deduce_task(Short index, Short *primary, Short *secondary,
			Boolean *call)
{
  object *obj = get_nth_object(index, &(sotu->roguep->pack));
  ErrNonFatalDisplayIf ( !obj,
			 "XXXXXX Something Is Very Wrong");  // serious error.
  *primary = 0;  // 'primary' is the all-purpose button
  *secondary = (obj->in_use_flags & BEING_WIELDED) ? INV_UNWIELD : INV_WIELD;
  *call = false;
  switch (obj->what_is) {
  case FOOD:
    *primary = INV_EAT;
    break;
  case POTION:
    *primary = INV_QUAFF;
    *call = true;
    break;
  case SCROLL:
    *primary = INV_READ;
    *call = true;
    break;
  case WAND:
    *primary = INV_ZAP;
    *call = true;
    break;
  case RING:
    *secondary = 0; // can't wield a ring.
    if (obj->in_use_flags & ON_EITHER_HAND)
      *primary = INV_REMOVE;
    else if (sotu->r_rings < 8)
      *primary = INV_PUTON; // there are still fingers available
    *call = true;
    break;
  case ARMOR:
    *secondary = 0; // can't wield armor.
    *primary = (obj->in_use_flags & BEING_WORN) ? INV_TAKEOFF : INV_WEAR;
    // (if an armor is already on, you will spend an extra turn taking it off)
    break;
  case WEAPON:
    *primary = *secondary;
    *secondary = (obj->o_flags & O_ZAPPED) ? INV_ZAP : 0;
    break;
  default:
    *primary = 0;
    *secondary = 0;
    break;
  }
}

/**********************************************************************
                       SPECIALIZE_LABELS
 IN:
 inventory-frobbing form
 index of inventory-object currently selected
 various globals
 OUT:
 nothing
 PURPOSE:
 Called when an inventory object is selected to set/hide/show the button
 labels to represent the set of tasks appropriate to that object.
**********************************************************************/
Char specialized_labels[13][9] = {
  "NO-OP",
  "drop",
  "eat",
  "read",
  "put on",
  "wield",
  "zap",
  "quaff",
  "wear",
  "remove",
  "throw",
  "unwield",
  "unwear" //  "take off"
};
static void specialize_labels(FormPtr frm, Short index)
{
  Short first = 0, fourth = 0, i;
  ControlPtr btn;
  fighter *rogue = sotu->roguep;
  UShort ** dungeon = sotu->dungeon;
  Boolean call;

  // Also, hide the ring-fingers pushbutton panel!!!
  // Reveal it IFF first==INV_PUTON.
  if (index == -1) {
    for (i = pbtn_if_frob ; i <= pbtn_if_call ; i++) {
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
      CtlHideControl(btn);
    }
    toggle_ring_buttons(frm, true);
    return;
  }
  deduce_task(index, &first, &fourth, &call);
  toggle_ring_buttons(frm, (first != INV_PUTON)); // wheeee!
  // set or hide the first button
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_if_frob));
  CtlSetLabel(btn, specialized_labels[first]);
  if (first) {
    CtlShowControl(btn);    
    CtlSetValue(btn, 1); // to turn it on...
  }
  else
    CtlHideControl(btn);
  // set or hide the fourth button
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_if_wield));
  CtlSetLabel(btn, specialized_labels[fourth]);
  if (fourth)
    CtlShowControl(btn);    
  else
    CtlHideControl(btn);
  // show or hide the fifth button... for scroll, potion, wand, ring only.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_if_call));
  if (call)
    CtlShowControl(btn);    
  else
    CtlHideControl(btn);
  // I need some better way to refresh the lines between them!!!
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_if_drop));
  if (dungeon[rogue->row][rogue->col] & (OBJECT | STAIRS | TRAP))
    CtlHideControl(btn); // "there's already something there"
  else 
    CtlShowControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_if_throw));
  CtlShowControl(btn);
  // Hey, maybe I should do something clever,
  // like have it default to "first button ON if we're "show"ing it"
  // This might deal well with the common case.
  // ....later.....

}

/**********************************************************************
                       TOGGLE_RING_BUTTONS
 IN:
 frm = the form that has these buttons in it
 hide = whether to hide or display the buttons
 remove = whether the player wants to 'remove' or 'put on' a ring
 various globals
 OUT:
 nothing
 PURPOSE:
 There is a form that allows the user to perform actions on an item
 selected from an inventory list.  If the action selected is related
 to rings, the buttons for 'left' and 'right' should be displayed
 so the user can select which hand to put the ring on; otherwise
 they should be hidden.  If the action selected is to remove a ring,
 a hand's button should be displayed iff there's a ring on that hand;
 if the action selected is to put on a ring, display a hand's button
 only if there's no ring on that hand.
 **********************************************************************/
/**********************************************************
****** HEISENBUG - null string passed to stringmgr ********
**************** fixed now? *******************************
***********************************************************/
static void toggle_ring_buttons(FormPtr frm, Boolean hide)
{
  static Boolean ring_buttons_visible = true;
  ControlPtr pushbtn;
  Short i;
  Boolean set_me = true;
  if (!hide) {
    // Re-show buttons even if they're (unlikely) already visible
    ring_buttons_visible = true;
    for (i = pbtn_i_left1 ; i <= pbtn_i_right4 ; i++) {
      pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
      // Show only buttons corresponding to ring-free fingers ...
      if (!sotu->roguep->rings[i-pbtn_i_left1]) {
	CtlShowControl(pushbtn);
	CtlSetValue(pushbtn, set_me);
	set_me = false;
      } else {
	CtlHideControl(pushbtn);
	CtlSetValue(pushbtn, 0);
      }
    }
  } else if (ring_buttons_visible) {
    // Don't bother hiding the buttons if they're already hidden.
    ring_buttons_visible = false;
    for (i = pbtn_i_left1 ; i <= pbtn_i_right4 ; i++) {
      pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
      CtlHideControl(pushbtn);
      CtlSetValue(pushbtn, 0);
    }
  } 
}

/**********************************************************************
                       get_ring_finger
 IN:
 inventory form
 various globals
 OUT:
 what ring finger is currently selected
 PURPOSE:
 called when the player wants to put on a ring
 **********************************************************************/
// I could use bitwise shift but I might be shooting myself in the foot.
// oh, what the hell, let's try it. ("u=u<<1;" is the same as "u*=2;")
// yes, I realize this is grotesque even if it works
/*
ULong Old_get_ring_finger(FormPtr frm)
{
  ControlPtr pushbtn;
  Short i;
  UShort u; // i THINK this will be long enough...
  //   0000 0000 0000 0100 // left 1
  //   0000 0010 0000 0000 // right 1
  u = 4;
  for (i = pbtn_i_left1 ; i <= pbtn_i_right4 ; i++) {
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
    if (CtlGetValue(pushbtn))
      return (ULong) u;
    u = u << 1;
  }
  return 0;
}
*/
static Short get_ring_finger(FormPtr frm)
{
  ControlPtr pushbtn;
  Short i;
  for (i = pbtn_i_left1 ; i <= pbtn_i_right4 ; i++) {
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
    if (CtlGetValue(pushbtn))
      return (i - pbtn_i_left1);
  }
  return -1;
}

/*****************************************************************
 *****************************************************************
 ********************  random initialization   *******************
 *****************************************************************
 *****************************************************************/


//const Char *wand_materials[WAND_MATERIALS]
// added some metals and woods from urogue
// Strangely it makes no difference which way it is declared...
static const Char wand_materials[WAND_MATERIALS][12] = {
  "steel ",
  "bronze ",
  "gold ",
  "silver ",
  "copper ",
  "nickel ",
  "cobalt ",
  "tin ",
  "iron ",
  "magnesium ",
  "chrome ",
  "carbon ",
  "platinum ",
  "silicon ",
  "titanium ",
  "aluminum ",
  "bone ",
  "brass ",
  "lead ",
  "pewter ",
  "zinc ",
  "ivory ",
  "arenak ",

  "teak ",
  "oak ",
  "cherry ",
  "birch ",
  "pine ",
  "cedar ",
  "redwood ",
  "balsa ",
  "walnut ",
  "maple ",
  "mahogany ",
  "elm ",
  "palm ",
  "zebra-wood ",
  "banyan ",
  "cinnabari ",
  "dogwood ",
  "driftwood ",
  "ebony ",
  "eucalyptus ",
  "hemlock ",
  "ironwood ",
  "manzanita ",
  "rosewood "
};
// Added: alum., bone, brass, lead, pewter, zinc.
// Moved ivory to "metals".
// Added: bany, cinnabari, dogw, driftw, ebony, heml, ironw, manz, rose, eu
// Changed 'wooden' to 'zebra-wood'
// Leaving out: avocado wood, persimmon wood


// const Char *gems[GEMS]
// Added some more from urogue.
//static const Char gems[GEMS][16] = {
// It takes less space declared this way:
static const Char *gems[GEMS] = {
  "diamond ",
  "stibotantalite ",
  "lapis-lazuli ",
  "ruby ",
  "emerald ",
  "sapphire ",
  "amethyst ",
  "quartz ",
  "tiger-eye ",
  "opal ",
  "agate ",
  "turquoise ",
  "pearl ",
  "garnet ",

  "alexandrite ",
  "azurite ",
  "carnelian ",
  "chrysoberyl ",
  "chrysoprase ",
  "citrine ",
  "hematite ",
  "jacinth ",
  "jade ",
  "kryptonite ",
  "malachite ",
  "moonstone ",
  "obsidian ",
  "olivine ",
  "onyx ",
  "peridot ",
  "rhodochrosite ",
  "sardonyx ",
  "serpentine ",
  "spinel ",
  "topaz ",
  "tourmaline ",

  "faidon "
};
// diffs:  no "stibotantalite "; "Lapus lazuli", "Tiger eye".


// I think I like the scrolls the way they are.
// Note urogue has a fairly different syllable set.
// const Char *syllables[MAXSYLLABLES]
// It takes less space declared this way:
static const Char syllables[MAXSYLLABLES][7] = {
  "blech ",
  "foo ",
  "barf ",
  "rech ",
  "bar ",
  "blech ",
  "quo ",
  "bloto ",
  "woh ",
  "caca ",
  "blorp ",
  "erp ",
  "festr ",
  "rot ",
  "slie ",
  "snorf ",
  "iky ",
  "yuky ",
  "ooze ",
  "ah ",
  "bahl ",
  "zep ",
  "druhl ",
  "flem ",
  "behil ",
  "arek ",
  "mep ",
  "zihr ",
  "grit ",
  "kona ",
  "kini ",
  "ichi ",
  "niah ",
  "ogr ",
  "ooh ",
  "ighr ",
  "coph ",
  "swerr ",
  "mihln ",
  "poxi "
};

/* inventory(pack, mask) changed to a popup dialog */


/**********************************************************************
                       MIX_COLORS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Called whenever a new rogue is started; this mixes up the colors
 assigned to the various kinds of potions.
 **********************************************************************/
/* 32 * 3 StrCopy's.... ugh */
void mix_colors() {
  Short i, j, k;
  Char t[32];

  for (i = 0; i <= 32; i++) {
    j = get_rand(0, (POTIONS - 1));
    k = get_rand(0, (POTIONS - 1));
    if (j != k) {
      StrCopy(t,                         sotu->id_potions[j].title);
      StrCopy(sotu->id_potions[j].title, sotu->id_potions[k].title);
      StrCopy(sotu->id_potions[k].title, t);
    }
    // "it was like that when I got here" (i.e. commented out)
    //		swap_string(id_potions[j].title, id_potions[k].title);
  }
}


/**********************************************************************
                       MAKE_SCROLL_TITLES
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Called whenever a new rogue is started; this makes up the titles
 assigned to the various kinds of scrolls.
 **********************************************************************/
/* Also string-intensive but I think it's ok. */
void make_scroll_titles() {
  Short i, j, n;
  Short sylls, s;

  for (i = 0; i < SCROLLS; i++) {
    sylls = get_rand(2, 5);
    /*		id_scrolls[i].title=(char *)malloc(128);
		if(id_scrolls[i].title==(char *)0)
		clean_up("Panic: no memory.",0);*/
    StrCopy(id_scrolls[i].title, "'");

    for (j = 0; j < sylls; j++) {
      s = get_rand(1, (MAXSYLLABLES-1));
      StrCat(id_scrolls[i].title, syllables[s]);
    }
    n = StrLen(id_scrolls[i].title);
    StrCopy(id_scrolls[i].title+(n-1), "' ");
  }
}

/***************************************************************
                   GET_DESC_EITHER
               (GET_DESC, GET_DESC_BRIEF)
 IN:
 obj = the object to get a description of
 desc = space to return the description in
 sotu = various globals (wizard)
 [brief = false in GET_DESC, true in GET_DESC_BRIEF]
 OUT:
 nothing (desc)
 PURPOSE:
 Given an object, return a string describing it to the best of
 the rogue's knowledge - used in inventory lists, picking up,
 dropping, all that sort of thing
 (What 'brief' does is to leave the "entitled: " out of the scroll
 descriptions; they are more than long enough without it.)
****************************************************************/
static void get_desc_add_called(object *obj, Char *desc, const Char *item_name, struct id *id_table) SEC_L;
static void get_desc_add_called(object *obj, Char *desc, const Char *item_name, struct id *id_table)
{
  switch(obj->what_is) {
  case SCROLL:
  case POTION:
  case WAND:
  case RING:
    StrCat(desc, item_name);
    StrCat(desc, "called ");
    StrCat(desc, id_table[obj->which_kind].title);
    break;
  }
}
static void get_desc_either(object *obj, Char *desc, Boolean brief) SEC_L;
static void get_desc_either(object *obj, Char *desc, Boolean brief)
{
  const Char *item_name;
  struct id *id_table;
  Char more_info[32];
  Short i;

  if (obj->what_is == AMULET) {
    StrCopy(desc, "the amulet of Yendor ");
    return;
  }
  item_name = name_of(obj); /* exists now */

  if (obj->what_is == GOLD) {
    StrPrintF(desc, "%d pieces of gold", obj->quantity);
    return;
  }

  if (obj->what_is != ARMOR) {
    if (obj->quantity == 1) {
      StrCopy(desc, "a ");
    } else {
      StrPrintF(desc, "%d ", obj->quantity);
    }
  }
  if (obj->what_is == FOOD) {
    if (obj->which_kind == RATION) {
      if (obj->quantity > 1) {
	StrPrintF(desc, "%d rations of ", obj->quantity);
      } else {
	StrCopy(desc, "some ");
      }
    } else {
      StrCopy(desc, "a ");
    }
    StrCat(desc, item_name);
    goto ANA;
  }
  id_table = get_id_table(obj);

  if (IS_WIZARD) {
    goto ID; /* not tested */
  }
  if (obj->what_is & (WEAPON | ARMOR | WAND | RING)) {
    goto CHECK;
  }

  switch(id_table[obj->which_kind].id_status) {
  case UNIDENTIFIED:
  CHECK:
  switch(obj->what_is) {
  case SCROLL:
    StrCat(desc, item_name);
    if (!brief)
      StrCat(desc, "entitled: ");
    StrCat(desc, id_table[obj->which_kind].title);
    break;
  case POTION:
    StrCat(desc, id_table[obj->which_kind].title);
    StrCat(desc, item_name);
    break;
  case WAND:
  case RING:
    if (obj->identified ||
	(id_table[obj->which_kind].id_status == IDENTIFIED)) {
      goto ID;
    }
    /* I've disabled 'call' elsewhere, for a while */
    /* (but now working on re-enabling it, so this WILL be live code again) */
    if (id_table[obj->which_kind].id_status == CALLED) {
      // used to be "goto CALL;"
      get_desc_add_called(obj, desc, item_name, id_table);
      break;
    }
    StrCat(desc, id_table[obj->which_kind].title);
    StrCat(desc, item_name);
    break;
  case ARMOR:
    if (obj->identified) {
      goto ID;
    }
    StrCopy(desc, id_table[obj->which_kind].title);
    break;
  case WEAPON:
    if (obj->identified) {
      goto ID;
    }
    StrCat(desc, name_of(obj));
    break;
  }
  break;
  case CALLED:
    // used to be target for "goto CALL"
    get_desc_add_called(obj, desc, item_name, id_table);
  break;
  case IDENTIFIED:
ID:
    switch(obj->what_is) {
    case SCROLL:
    case POTION:
      StrCat(desc, item_name);
      StrCat(desc, id_table[obj->which_kind].real);
      break;
    case RING:
      if (IS_WIZARD || obj->identified) {
	if ((obj->which_kind == ADD_STRENGTH) ||
	    (obj->which_kind == DEXTERITY) ||
	    (obj->which_kind == R_PROTECTION)) {
	  StrPrintF(more_info, "%s%d ", ((obj->class > 0) ? "+" : ""),
		    obj->class);
	  StrCat(desc, more_info);
	}
      }
      StrCat(desc, item_name);
      StrCat(desc, id_table[obj->which_kind].real);
      break;
    case WAND:
      StrCat(desc, item_name);
      StrCat(desc, id_table[obj->which_kind].real);
      if (IS_WIZARD || obj->identified) {
	StrPrintF(more_info, "[%d]", obj->class);
	StrCat(desc, more_info);
      }
      break;
    case ARMOR:
      StrPrintF(desc, "%s%d ", ((obj->d_enchant >= 0) ? "+" : ""),
		obj->d_enchant);
      StrCat(desc, id_table[obj->which_kind].title);
      StrPrintF(more_info, "[%d] ", obj->class + obj->d_enchant);
      StrCat(desc, more_info);
      break;
    case WEAPON:
      StrPrintF(desc+StrLen(desc), "%s%d,%s%d ",
		((obj->hit_enchant >= 0) ? "+" : ""),
		obj->hit_enchant,
		((obj->d_enchant >= 0) ? "+" : ""),
		obj->d_enchant);
      StrCat(desc, name_of(obj));
      break;
    }
  break;
  }
ANA:
  if (!StrNCompare(desc, "a ", 2)) {
    if (is_vowel(desc[2])) {
      /* hm, how annoying. but not the common case, I trust */
      for (i = StrLen(desc) + 1; i > 1; i--) {
	desc[i] = desc[i-1];
      }
      desc[1] = 'n';
    }
  }
  if (brief && (obj->in_use_flags & BEING_USED)) {
    i = StrLen(desc);
    desc[i-1] = ':';
    desc[i] = ' ';
    desc[i+1] = 0;
  }
  if (obj->in_use_flags & BEING_WIELDED) {
    StrCat(desc, "in hand");
  } else if (obj->in_use_flags & BEING_WORN) {
    StrCat(desc, (brief ? "worn" : "being worn"));
  } else if (obj->in_use_flags & ON_LEFT_HAND) {
    // I don't think I'll bother saying which finger it's on.
    StrCat(desc, (brief ? "left" : "on left hand"));
  } else if (obj->in_use_flags & ON_RIGHT_HAND) {
    StrCat(desc, (brief ? "right" : "on right hand"));
  }
}
/* Return a full description. */
void get_desc(object *obj, Char *desc) {
  return get_desc_either(obj, desc, false);
}
/* Return a brief description. */
void get_desc_brief(object *obj, Char *desc) {
  return get_desc_either(obj, desc, true);
}



/**********************************************************************
                       GET_WAND_AND_RING_MATERIALS
 IN:
 various globals
 OUT:
 nothing
 PURPOSE:
 Called whenever a new rogue is started (?); this mixes up the
 materials assigned to the different wand types and ring types.
 **********************************************************************/
void get_wand_and_ring_materials() {
  Short i, j;
  //  Boolean used[max(WAND_MATERIALS, GEMS)];
  Boolean used[WAND_MATERIALS];

  for (i = 0; i < WAND_MATERIALS; i++) {
    used[i] = 0;
  }
  for (i = 0; i < WANDS; i++) {
    do {
      j = get_rand(0, WAND_MATERIALS-1);
    } while (used[j]);
    used[j] = 1;
    /*		id_wands[i].title=(char *)malloc(128);*/
    StrCopy(id_wands[i].title, wand_materials[j]);
    id_wands[i].id_status = UNIDENTIFIED;
    sotu->is_wood[i] = (j > MAX_METAL);
  }
  for (i = 0; i < GEMS; i++) {
    used[i] = 0;
  }
  for (i = 0; i < RINGS; i++) {
    do {
      j = get_rand(0, GEMS-1);
    } while (used[j]);
    used[j] = 1;
    /*		id_rings[i].title=(char *)malloc(128);*/
    StrCopy(id_rings[i].title, gems[j]);
    id_rings[i].id_status = UNIDENTIFIED;
  }
}

/* single_inv
   perhaps this could also be a Popup  - get rid of pack_letter !    */


/***************************************************************
                   GET_ID_TABLE
 IN:
 obj = an object of some type
 sotu = various globals (id_foo)
 OUT:
 the id table for that type of object
 PURPOSE:
 Given an object, return the id table corresponding to its type
 (scroll, potion, etc.)
****************************************************************/
/* this was in inventory.c */
struct id * get_id_table(object *obj)
{
  switch(obj->what_is) {
  case SCROLL:
    return(sotu->id_scrolls);
  case POTION:
    return(sotu->id_potions);
  case WAND:
    return(sotu->id_wands);
  case RING:
    return(sotu->id_rings);
  case WEAPON:
    return(sotu->id_weapons);
  case ARMOR:
    return(sotu->id_armors);
  }
  return((struct id *) 0);
}


/* inv_armor_weapon - gone
   inv_armor - gone
   inv_weapon - gone   */


