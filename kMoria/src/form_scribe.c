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

/* Form for inscribing an already selected object. 
   Note that inscribing should not take a turn whether cancelled or not. */
//extern Short needed_direction_item; // this is the selected item
extern previous_state curr_state; // replaces needed_directional_item

static void init_scribe_fld(FormPtr frm) SEC_6;
static void commit_scribe_fld(FormPtr frm, Short item_val) SEC_6;

Boolean Scribe_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Char chr;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    init_scribe_fld(frm);
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    scribe_draw();
    FrmSetFocus(frm, FrmGetObjectIndex(frm, field_sb));
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case keyDownEvent:
    frm = FrmGetActiveForm();
    chr = e->data.keyDown.chr;
    if (chr == '\n') {
      hit_button_if_usable(frm, btn_sb_ok);
      handled = true;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_sb_ok:
      commit_scribe_fld(frm, curr_state.item);
      LeaveForm();
      //      needed_direction_item = -1;
      handled = true;
      break;
    case btn_sb_cancel:
      LeaveForm();
      //      needed_direction_item = -1;
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}


void scribe_draw()
{
  Char buf[160], *p;
  Word wwlen, wwpixels=SCR_WIDTH-4;//156;
  Short x=2, y=ROW_1, lines = 0;

  Short item_val = curr_state.item;

  // Print up to 3 lines of "Inscribing <original object>"
  StrPrintF(buf, "Inscribing ");
  p = buf + StrLen(buf);
  objdes(p, &inventory[item_val], true);
  p = buf;
  wwlen = 0;
  wwpixels -= 2*x; // leave margins
  while (lines < 3 && StrLen(p) > 0) {
    wwlen = FntWordWrap(p, wwpixels);
    WinDrawChars(p, wwlen, x, y+LINE_HEIGHT*lines);
    p += wwlen;
    lines++;
  }
  // print old inscription if any
  if (inventory[item_val].inscrip[0] != '\0') {
    StrPrintF(buf, "Replace %s", inventory[item_val].inscrip);
    WinDrawChars(buf, StrLen(buf), x, y+3*LINE_HEIGHT);
    WinDrawChars("New", 3,  3, y+4*LINE_HEIGHT);
  }
}


static void init_scribe_fld(FormPtr frm)
{
  VoidHand h;
  VoidPtr p;
  FieldPtr fld;
  // allocate memory for field, I hope.
  h = MemHandleNew(sizeof(Char) * INSCRIP_SIZE);
  p = MemHandleLock(h);
  ((Char *)p)[0] = '\0';
  MemPtrUnlock(p);
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_sb));
  FldSetTextHandle(fld, (Handle) h);
}

static void commit_scribe_fld(FormPtr frm, Short item_val)
{
  FieldPtr fld;
  CharPtr textp;
  // Copy contents of field
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_sb));
  textp = FldGetTextPtr(fld); // if (textP) ...
  textp[INSCRIP_SIZE - 1] = '\0';
  DmStrCopy(inventory, item_val*sizeof(inven_type)+I_inscrip, textp);
  // Get rid of the field
  FldReleaseFocus(fld);
  FldSetSelection(fld, 0, 0);
  FldFreeMemory(fld);
}
