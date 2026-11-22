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



Boolean Snapshot_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short filenum, new_i;
  ListPtr lst;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* end mine */
#ifndef I_AM_OS_2
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      PrvMoveObject(frm, btn_snap_delete, y_diff, false);
      PrvMoveObject(frm, btn_snap_save, y_diff, false);
      PrvMoveObject(frm, btn_snap_cancel, y_diff, false);
    }
#endif
    FrmDrawForm(frm);
    // Hey! Disable the "delete" button if there is only autosave.
    // (even though the button handler is smart enough to not do it).
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_snap));
    snapshot_update_list(lst, true); // true == whether to include 'new'
    display_saved_rogue(0);
    snapshot_frob_btns(frm, false);
    handled = true;
    break;


#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) {
      Short y_diff;
      update_display_size();
      y_diff = adjust_form_size(true);
      frm = FrmGetActiveForm();
      // redraw the "new" screen too...  don't ask me whether this will work
      PrvMoveObject(frm, btn_snap_delete, y_diff, true);
      PrvMoveObject(frm, btn_snap_save, y_diff, true);
      PrvMoveObject(frm, btn_snap_cancel, y_diff, true);
      FrmDrawForm(frm);
      lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_snap));
      filenum = LstGetSelection(lst);
      display_saved_rogue(filenum);
    }
    break;
#endif

  case popSelectEvent:
    filenum = e->data.popSelect.selection; // you also know priorSelection
    new_i = get_file_count();
    undisplay_saved_rogue();
    if (filenum < new_i) {
      display_saved_rogue(filenum);
      // if filenum == 0, disable 'delete' button..
      snapshot_frob_btns(FrmGetActiveForm(), (filenum!=0));      
    }
    ////    handled = true; // ?
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_snap));
    filenum = LstGetSelection(lst);
    new_i = get_file_count();
    switch(e->data.ctlSelect.controlID) {
    case btn_snap_cancel:
      LstSetListChoices(lst, NULL, 0);
      LeaveForm();
      handled = true;
      break;
    case btn_snap_delete:
      // figure out which is selected, if NEW or autosave then feep & ignore
      // ..or else have the got-selected code disable/enable delete..
      // Popup an "deleting foo, are you SURE?"
      // Do not leave the form.
      handled = true;
      if (filenum <= 0 || filenum >= new_i) break; // o.o.b.
      if (0 == FrmAlert(SaveDeleteP)) {
	delete_saved_rogue(filenum);
	snapshot_delete_listi(frm, filenum);
	snapshot_frob_btns(frm, false);
      }
      // then REDRAW the list!!
      // actually, just free the ith string and pack the array,
      // then tell it to redraw itself.
      break;
    case btn_snap_save:
      // if something other than autosave or NEW is selected,
      // Popup an "overwriting foo, are you SURE?"
      handled = true;
      if (filenum < 0 || filenum > new_i) break; // out of bounds
      if (filenum == 0 || filenum == new_i || 0 == FrmAlert(SaveOverP)) {
	save_character(filenum);
	LstSetListChoices(lst, NULL, 0);
	LeaveForm();
	message("saved.");
      }
      break;
    }
    break;
  default:
    break;
  }

  return handled;
}

