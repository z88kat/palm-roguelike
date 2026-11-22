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

// in my_object.c
Short wiz_form_object_type = -1;
Boolean Wiz_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short whatkind = -1, lvis;
  ListPtr lst;
  ControlPtr btn;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    init_wiz_list(frm, list_wiza, -1);
    wiz_form_object_type = -1;
#ifndef I_AM_OS_2
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_wiz_ok, y_diff, false);
      PrvMoveObject(frm, btn_wiz_cancel, y_diff,false);
      PrvResizeList(frm, list_wiza, btn_wiz_ok, y_diff);
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
      PrvMoveObject(frm, btn_wiz_ok, y_diff, true);
      PrvMoveObject(frm, btn_wiz_cancel, y_diff, true);
      PrvResizeList(frm, list_wiza, btn_wiz_ok, y_diff);
      FrmDrawForm(frm);
    }
    break;
#endif

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_wiz_ok:
      if (wiz_form_object_type != -1) {
	lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_wiza));
	whatkind = LstGetSelection(lst);
      }
      LeaveForm();
      /* It is important to do this after LeaveForm for msg to display */
      if (wiz_form_object_type != -1 && whatkind != -1) {
	if (wiz_form_object_type > 7) {
	  whatkind += (wiz_form_object_type > 8 ? 1 : 27);
	  create_monster(whatkind);
	} else {
	  new_object_for_wizard(wiz_form_object_type, whatkind);
	}
      }
      handled = true;
      break;
    case btn_wiz_cancel:
      LeaveForm();
      handled = true;
      break;
    }
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_wiza));
    lvis = LstGetVisibleItems(lst) - 1;
    if (lvis < 10) lvis = 10; // minimum of 11 items visible at a time
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, winUp, lvis))
	LstSetSelection(lst, -1);
      //	wiz_form_object_type = -1;
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, winDown, lvis))
	LstSetSelection(lst, -1);
      //	wiz_form_object_type = -1;
      handled = true;
      break;
    }
    break;

  case lstSelectEvent:
    if (e->data.lstSelect.listID == list_wiza) {
      if (wiz_form_object_type == -1) {
	wiz_form_object_type = e->data.lstSelect.selection;
	if (wiz_form_object_type != -1) {
	  frm = FrmGetActiveForm();
	  init_wiz_list(frm, list_wiza, wiz_form_object_type);
#ifndef I_AM_OS_2
	  if (IsVGA) // because number of items have changed...
	    PrvResizeList(frm, list_wiza, btn_wiz_ok, 0);
#endif
	  FrmDrawForm(frm);
	  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_wiz_ok));
	  CtlShowControl(btn);    
	  handled = true;
	}
      }
    }
    break;


  default:
    break;
  }

  return handled;
}
