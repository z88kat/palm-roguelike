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
 * This form lets you view and modify a few more preferences.
 * There are a bunch of checkboxes and things in it,
 * and an ok and a cancel button, as you might expect.
 ***********************************************************************/

Boolean RepeatCmds_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr checkbox;
  //  ListPtr lst;
  //  Boolean verydirty = false, dirty = false, val;
  Short i;
  UChar x;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* and checkboxes */
    for (i = 0, x = 1 ; i < 5 ; i++, x*=2) {
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_rptc_1+i));
      CtlSetValue(checkbox, ((my_prefs.repeaters & x) ? 1 : 0));
    }
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_rptc_ok:
      my_prefs.repeaters = 0;
      for (i = 0, x = 1 ; i < 5 ; i++, x*=2) {
	checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,check_rptc_1+i));
	if (CtlGetValue(checkbox) != 0) my_prefs.repeaters += x;
      }
      writeMoriaPrefs();
      LeaveForm();
      handled = true;
      break;
    case btn_rptc_cancel:
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

