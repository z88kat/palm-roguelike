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
 * This form allows the player to bind/unbind actions to hardware buttons
 * It is full of popup lists and stuff.
 * and an ok and a cancel button, as you might expect.
 ***********************************************************************/
extern const Char hwb_commands[HWB_last+2];
Short tmp_hardware[8];
static void init_hwb_widgets(FormPtr frm) SEC_6;
static void do_hwb_printing(FormPtr frm, Short i) SEC_6;
static void examine_hwb_widgets(FormPtr frm, Short i) SEC_6;
Boolean HwButtons_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  ControlPtr ctl;
  Short i;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    init_hwb_widgets(frm);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    examine_hwb_widgets(frm, e->data.lstSelect.selection);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case pbtn_hwb_0: case pbtn_hwb_1: case pbtn_hwb_2:
    case pbtn_hwb_3: case pbtn_hwb_4: case pbtn_hwb_5:
    case pbtn_hwb_6: case pbtn_hwb_7:
      i = e->data.ctlSelect.controlID - pbtn_hwb_0;
      frm = FrmGetActiveForm();
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_hwb_1));
      LstSetSelection(lst, tmp_hardware[i]);
      break;
    case btn_hwb_ok:
      frm = FrmGetActiveForm();
      ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_hwb));
      my_prefs.use_hardware = (CtlGetValue(ctl) != 0);	
      for (i = 0 ; i < 8 ; i++)
	my_prefs.hardware[i] = tmp_hardware[i];
      writeMoriaPrefs();
      LeaveForm();
      handled = true;
      break;
    case btn_hwb_cancel:
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

static void init_hwb_widgets(FormPtr frm)
{
  ListPtr lst;
  ControlPtr pushbtn, ctl;
  Short i;
  // first the checkbox
  ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_hwb));
  CtlSetValue(ctl, (my_prefs.use_hardware ? 1 : 0));
  // now the 8 values
  for (i = 0 ; i < 8 ; i++)
    tmp_hardware[i] = my_prefs.hardware[i];
  // write the text.  there ought to be a less icky way to do this.
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_hwb_1));
  do_hwb_printing(frm, -1);
  LstSetSelection(lst, tmp_hardware[0]);
  // make sure a pushbutton is selected
  pushbtn = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, pbtn_hwb_0));
  CtlSetValue(pushbtn, true);
}

static void do_hwb_printing(FormPtr frm, Short i)
{
  RectangleType r;
  CharPtr label;
  ListPtr lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_hwb_1));
  Short x, y, dy, w;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, pbtn_hwb_0), &r);
  x = r.topLeft.x + r.extent.x + 1 + FntCharWidth(' '); // 3 + 35 + (3)
  y = r.topLeft.y;  //y = ROW_1 + LINE_HEIGHT;
  dy = r.extent.y + 1;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, list_hwb_1), &r);
  w = (r.topLeft.x - 1) - x;
  if (i >= 0 && i < 8) {
    RctSetRectangle(&r, x, y + dy*i, w, dy);
    WinEraseRectangle(&r, 0);
    if (!tmp_hardware[i]) return;
    label = LstGetSelectionText(lst, tmp_hardware[i]);
    WinDrawChars(label, StrLen(label), x, y + dy*i);
  } else {
    // hm... no need to erase, it's just form initialization.
    for (i = 0 ; i < 8 ; i++) {
      if (tmp_hardware[i] == 0) continue;
      label = LstGetSelectionText(lst, tmp_hardware[i]);
      WinDrawChars(label, StrLen(label), x, y + dy*i);
    }
  }
}

static void examine_hwb_widgets(FormPtr frm, Short i)
{
  ControlPtr pushbtn;
  Short k;
  // find who is affected by new value
  for (k = 0 ; k < 8 ; k++) {
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_hwb_0+k));
    if (CtlGetValue(pushbtn))
      break;
  }
  if (k >= 8) return; // no one is affected (this should not happen)
  // clear old value
  // find new value
  //    i = e->data.lstSelect.selection;
  if (i == -1 || i > HWB_last) return; // this should not happen either

  tmp_hardware[k] = i; // record.. and display..

  do_hwb_printing(frm, k);

}
