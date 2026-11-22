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


/***********************************************************************
 * This form allows player to view the last N message that went by..
 * useful if he/she acted too fast and missed one.
 * Note that the old_messages array is probably going to be combined with
 * the splash array and a position-in-array int to enable "--more--"
 ***********************************************************************/

extern Char *old_messages[SAVED_MSGS];
VoidHand msglog_TextHandle = NULL;
static void init_messagelog_view(FormPtr frm) SEC_5;
Boolean MsgLog_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    init_messagelog_view(frm);
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    while (FldScrollable(fld, winDown)) {
      page_scroll_field(frm, fld, winDown);
    }
    update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_ml_ok:
      LeaveForm(); // LeaveForm(true);
      handled = true;
      break;
    }
    break;

  case ctlRepeatEvent:
    /*     "Repeating controls don't repeat if handled is set true." */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
    switch(e->data.ctlRepeat.controlID) {
    case repeat_ml_up:
      FldScrollField(fld, 1, winUp);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      break;
    case repeat_ml_down:
      FldScrollField(fld, 1, winDown);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      break;
    }
    break;

  case keyDownEvent:
    /* hardware button -- or else graffiti. */
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      page_scroll_field(frm, fld, winUp);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      handled = true;
      break;
    case pageDownChr: case ' ':
      page_scroll_field(frm, fld, winDown);
      update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
      handled = true;
      break;
    case '\n': // Keyboard support.  Woo hoo.
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



/**********************************************************************
                       INIT_MESSAGELOG_VIEW
 IN: frm = a form that has a text-view widget
 PURPOSE:
 Initializes the displayed form 'frm' with a list of the last
 SAVED_MSGS messages.  Called when frm is being initialized.
 **********************************************************************/
static void init_messagelog_view(FormPtr frm)
{
  FieldPtr fld;
  CharPtr txtP;
  Short i, len = 10; // some extra space to be sure
  // Get the text field
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
  // Create a mem. handle and lock it
  for (i = 0 ; i < SAVED_MSGS ; i++) len += StrLen(old_messages[i]);
  msglog_TextHandle = MemHandleNew(len);
  txtP = MemHandleLock(msglog_TextHandle);
  // Use MemMove and/or MemSet to copy text to the mem. handle
  for (i = 0, len = 0 ; i < SAVED_MSGS ; i++) {
    MemMove(txtP+len, old_messages[i], StrLen(old_messages[i])+1);
    len += StrLen(old_messages[i]);
  }
  // this is to get rid of the last newline:
  if ((len > 0) && (txtP[len-1] == '\n')) MemSet(txtP+len-1, 1, '\0');

  // Unlock the handle.  Set the field to display the handle's text.
  MemHandleUnlock(msglog_TextHandle);
  FldSetTextHandle(fld, (Handle) msglog_TextHandle);
}

