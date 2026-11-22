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

#include "iRogue.h"

Boolean HwButtons_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;    // popup list
  ControlPtr ctl; // popup trigger
  CharPtr label;
  Short i;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    // my stuff
    ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_hwb));
    CtlSetValue(ctl, (my_prefs.use_hardware ? 1 : 0));
    for (i = 0 ; i < 8 ; i++) {
      lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_hwb_1 + i));
      ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, popup_hwb_1 + i));
      LstSetSelection(lst, my_prefs.hardware[i]);
      label = LstGetSelectionText(lst, my_prefs.hardware[i]);
      CtlSetLabel(ctl, label);
    }
    // end my stuff
#ifndef I_AM_OS_2
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_hwb_ok, y_diff, false);
      PrvMoveObject(frm, btn_hwb_cancel, y_diff,false);
    }
#endif
    FrmDrawForm(frm);
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
      PrvMoveObject(frm, btn_hwb_ok, y_diff, true);
      PrvMoveObject(frm, btn_hwb_cancel, y_diff, true);
      FrmDrawForm(frm);
    }
    break;
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_hwb_ok:
      frm = FrmGetActiveForm();
      ctl = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_hwb));
      my_prefs.use_hardware = (CtlGetValue(ctl) != 0);	
      for (i = 0 ; i < 8 ; i++) {
	lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_hwb_1 + i));
	my_prefs.hardware[i] = LstGetSelection(lst);
      }
      writeRoguePrefs();
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
