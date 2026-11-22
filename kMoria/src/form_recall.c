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

// this will have to be an extern so we know which monster to look up:
Short recall_this_monster = 0;
Short recall_next_monster = 0;
Boolean recall_list;

static void frob_recall_btn(FormPtr frm) SEC_7;
static void set_recall_text(FormPtr frm, FieldPtr fld) SEC_7;
static void init_recall_form(FormPtr frm, FieldPtr fld) SEC_7;


/*
static const Char *mon_recall_titles[] = {
  "$ - Treasure.",
  ", - Mushroom patch.",
  "A - Giant Ant Lion.",
  "B - The Balrog.",
  "C - Gelatinous Cube.",
  "D - An Ancient Dragon.",
  "E - Elemental.",
  "F - Giant Fly.",
  "G - Ghost.",
  "H - Hobgoblin.",
  "I",
  "J - Jelly.",
  "K - Killer Beetle.",
  "L - Lich.",
  "M - Mummy.",
  "N",
  "O - Ooze.",
  "P - Giant humanoid.",
  "Q - Quylthulg",
  "R - Reptile.",
  "S - Giant Scorpion.",
  "T - Troll.",
  "U - Umber Hulk.",
  "V - Vampire.",
  "W - Wight or Wraith.",
  "X - Xorn.",
  "Y - Yeti.",
  "Z",
  "a - Giant Ant.",
  "b - Giant Bat.",
  "c - Giant Centipede.",
  "d - Dragon.",
  "e - Floating Eye.",
  "f - Giant Frog.",
  "g - Golem.",
  "h - Harpy.",
  "i - Icky Thing.",
  "j - Jackal.",
  "k - Kobold.",
  "l - Giant Louse.",
  "m - Mold.",
  "n - Naga.",
  "o - Orc or Ogre.",
  "p - Person/Humanoid.",
  "q - Quasit.",
  "r - Rodent.",
  "s - Skeleton.",
  "t - Giant Tick.",
  "u",
  "v",
  "w - Worm / Worm Mass.",
  "x",
  "y - Yeek.",
  "z - Zombie."
};
*/


// If we're in list-of-monsters mode,
// and there's anohter monster in the list, enable the Next button.
static void frob_recall_btn(FormPtr frm)
{
  Char c = c_list[recall_this_monster].cchar;
  Short i;
  Boolean more = false;
  ControlPtr btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,
							  btn_recall_next));
  if (recall_list) {
    // Is there another monster of this kind, further up the list.
    for (i = recall_this_monster-1 ; i >= 0 ; i--)
      if ((c_list[i].cchar == c) && bool_roff_recall(i)) {
	recall_next_monster = i;
	more = true;
	break;
      }
  }
  if (more)
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
}

// Spit out all the text about the monster.
// This can be called multiple times, I hope.
VoidHand recall_handle = NULL;
static void set_recall_text(FormPtr frm, FieldPtr fld)
{
  CharPtr txtP;
  txtP = MemHandleLock(recall_handle);
  txtP[0] = '\0';
  roff_recall(recall_this_monster, txtP); // print info to this text buffer
  MemHandleUnlock(recall_handle);
  FldSetTextHandle(fld, (Handle) recall_handle);

  // There's a bug in this
  update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
}

/**********************************************************************
                       INIT_RECALL_FORM
 IN:
 fld = the text field
 assumes 'recall_this_monster' is in bounds
 PURPOSE:
 Initialize the recall form which will display information known about
 a particular monster type.  Call this only initially.
 **********************************************************************/

static void init_recall_form(FormPtr frm, FieldPtr fld)
{
  Char c = c_list[recall_this_monster].cchar;
  Char buf[16];
  //  Short i = -1;
  //  Char buf[40];
  /*  VoidHand recall_handle = NULL; */

  if (recall_list) {
    StrPrintF(buf, "recall: %c", c);
    FrmCopyTitle(frm, buf);
  } else
    FrmCopyTitle(frm, c_names + c_list[recall_this_monster].name_offset);    
  /*
  if (c == '$') i = 0;
  else if (c == '.') i = 1;
  else if (c >= 'A' && c <= 'Z') i = (c - 'A') + 2;
  else if (c >= 'a' && c <= 'z') i = (c - 'a') + 2 + 26;
  if (i != -1)
    FrmCopyTitle(frm, mon_recall_titles[i]);
  else
    FrmCopyTitle(frm, c_names + c_list[recall_this_monster].name_offset);
  */

  if (recall_handle != NULL) {
    FldSetTextHandle(fld, (Handle) 0);
    /* the following line is a fine way to crash the palmpilot,
       whenever someone re-visits this form */
    // MemHandleFree(recall_handle);
  }
  recall_handle = MemHandleNew(2700); // this SHOULD be enough, but who knows
  set_recall_text(frm, fld);
  // Hey, this never gets freed.  Leak!

  //  StrPrintF(buf, "%s:", c_names + c_list[recall_this_monster].name_offset);
  //  FrmCopyTitle(frm, buf);
}


/**********************************************************************
                       INIT_RECALL_FORM

 This form will display information (gathered by the current character
 and dead predecessors) about a particular monster type.
 Before you jump to this form, you should set recall_this_monster to
 the index of the desired monster type!
 The form is just a big boring text field that can be scrolled.
 Assumes 'recall_this_monster' is in bounds
 **********************************************************************/
extern Boolean Version_GE_OS35;
Boolean Recall_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
    //    recall_list = true; // TESTING!! Take out this line!!!!
    init_recall_form(frm, fld);
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    frob_recall_btn(frm); // after form is drawn.
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
      MenuCmdBarAddButton(1, bmp_cmd_amnesia, menuCmdBarResultMenuItem,
			  menu_recClear, NULL);
    }
    return false;
#endif

  case menuEvent:
    MenuEraseStatus(NULL); /* clear menu status from display */
    switch(e->data.menu.itemID) {
    case menu_recClear:
      if (0 == FrmAlert(RecallClearP))
	if (0 == FrmAlert(RecallClear2P))
	  blank_recall(); // erase the monster memory! woo hoo.
      LeaveForm();
      handled = true;
      break;
    }

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_recall_ok:
      LeaveForm();
      handled = true;
      break;
    case btn_recall_next:
      frm = FrmGetActiveForm();
      fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
      recall_this_monster = recall_next_monster;
      frob_recall_btn(frm);
      FldEraseField(fld);
      set_recall_text(frm, fld); // Hope this works..
      FldDrawField(fld);
      handled = true;
      break;
    }
    break;

  case ctlRepeatEvent:
    /*     "Repeating controls don't repeat if handled is set true." */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
    switch(e->data.ctlRepeat.controlID) {
    case repeat_recall_up:
      FldScrollField(fld, 1, winUp);
      update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
      break;
    case repeat_recall_down:
      FldScrollField(fld, 1, winDown);
      update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
      break;
    }
    break;

  case keyDownEvent:
    /* hardware button -- or else graffiti. */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      page_scroll_field(frm, fld, winUp);
      update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
      handled = true;
      break;
    case pageDownChr: case ' ':
      if (FldScrollable(fld, winDown)) {
	page_scroll_field(frm, fld, winDown);
	update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
	handled = true;
	break;
      } // else FALL THROUGH...
    case 'n': // ... act like we hit the 'Next' button - iff it's visible.
      hit_button_if_usable(frm, btn_recall_next);
      handled = true;
      break;
    case '\n': case 'x':
      LeaveForm();
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}

