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


Boolean About_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
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
    switch(e->data.ctlSelect.controlID) {
    case btn_about_ok:
      LeaveForm();
      handled = true;
      break;
    case btn_about_more:
      FrmHelp(LicenseStr);
      handled = true;
      break;
    case btn_about_credits:
      FrmHelp(CreditStr);
      handled = true;
      break;
    }
    break;

  case keyDownEvent:
    switch(e->data.keyDown.chr) {
    case '\n': case 'd':
      LeaveForm();
      handled = true;
      break;
    case 'v': case 'c': case 'i':      
      FrmHelp(AboutStr);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}



