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

extern Short revivify_which; // main.c
extern UChar revivify_p; // main.c

extern Boolean do_make_new_level;
extern Boolean save_me;
extern Boolean i_am_dead;


/* Revivify 
   If you cancel, exit this form back to top-ten (dead) or dungeon (live).
   If you do not cancel, need to know which form you came from!!! argh.
   Well actually that should be apparent because if topten, sotu is DEAD. */
/* Undead or alive:
   50% The orange smoke slowly disappears.
   50% The cloud of orange smoke dissipates.
   make a nice piercing beep at this point.
   Undead:
   You feel.. strangely hungry.
   Alive:
   40% Someone has rifled your pockets!
   30% You don't recall how you got here.
   20% You feel shaky, weak in the knees.
   10% You feel distressingly light-headed.  */
Boolean Revivify_Form_HandleEvent(EventPtr e)
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
      PrvMoveObject(frm, btn_reviv_alive, y_diff, false);
      PrvMoveObject(frm, btn_reviv_undead, y_diff, false);
      PrvMoveObject(frm, btn_reviv_cancel, y_diff, false);
    }
#endif
    FrmDrawForm(frm);
    // Hey! Disable the "alive" button if the guy being restored already undead
    // If you can get to this form from not-top-ten-list, also,
    // disable the "undead" button if the guy being restored is alive.
    // I will do this LATER because I will have to READ THE DATABASE RECORD
    // 
    // Initialize the popup list with the available save files
    // (when deleting them: please PACK so there are no empty files.)
    // display "autosave", "<timestamp>" ...  have autosave slot selected.
    // initially and when a new one is chosen on list, print info on screen.
    // do this in a reusable function because snapshot_form needs it too
    //
    // Also do not allow more than MAX_SAVE_FILES.
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_reviv));
    snapshot_update_list(lst, false); // bool == whether to include 'new'
    display_saved_rogue(0);
    reviv_frob_btns(frm, 0);
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
      PrvMoveObject(frm, btn_reviv_alive, y_diff, true);
      PrvMoveObject(frm, btn_reviv_undead, y_diff, true);
      PrvMoveObject(frm, btn_reviv_cancel, y_diff, true);
      FrmDrawForm(frm);
      // also redraw this part of form
      lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_reviv));
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
      reviv_frob_btns(FrmGetActiveForm(), filenum);
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, list_reviv));
    switch(e->data.ctlSelect.controlID) {
    case btn_reviv_cancel:
      LstSetListChoices(lst, NULL, 0);
      LeaveForm(); // just go back to the topten form
      handled = true;
      break;
    case btn_reviv_undead:
    case btn_reviv_alive:
      revivify_which = LstGetSelection(lst);
      LstSetListChoices(lst, NULL, 0);
      LeaveForm();
      if (!save_me) LeaveForm(); // we came from top_ten, get rid of it too.
      FrmGotoForm(MainForm); // leave main form and goto main form
      check_message();
      /* \end{dangerous half-assed implementation} */
      /* Well.... it seems to work... */
      if (e->data.ctlSelect.controlID == btn_reviv_undead)
	revivify_p = STATUS_UNDEAD;
      else 
	revivify_p = STATUS_ISDEAD;
      do_make_new_level = false; // so that we load the saved character!
      save_me = true;
      i_am_dead = false;
      handled = true;
      break;
    }
    break;
  default:
    break;
  }

  return handled;
}
