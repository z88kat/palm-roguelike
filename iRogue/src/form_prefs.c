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

extern Short ScrWidth; // display.c
extern Short ScrHeight;
extern Short WalkMagic;


void sort_pack(object *pack);
extern Boolean have_read_memo; // in color.c
Boolean Prefs_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  ControlPtr checkbox;
  static Short rw = 0;
  static Short ws = 0;
  Boolean redraw = false;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    rw = max(1, my_prefs.run_walk_border - my_prefs.walk_search_border);
    ws = max(1, my_prefs.walk_search_border);
    /* and checkboxes */
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_1));
    CtlSetValue(checkbox, (my_prefs.run_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_2));
    CtlSetValue(checkbox, (my_prefs.autosave_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_3));
    CtlSetValue(checkbox, (my_prefs.dave_mode ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_4));
    CtlSetValue(checkbox, (my_prefs.scroll_mode ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_5));
    CtlSetValue(checkbox, (my_prefs.sound_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_6));
    CtlSetValue(checkbox, (my_prefs.stay_centered ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_7));
    CtlSetValue(checkbox, (my_prefs.font_small ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_8));
    CtlSetValue(checkbox, (my_prefs.rogue_relative ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_9));
    CtlSetValue(checkbox, (my_prefs.crosshairs_on ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_11));
    CtlSetValue(checkbox, (my_prefs.sort_pack ? 1 : 0));
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_12));
    CtlSetValue(checkbox, (my_prefs.black_bg) ? 1 : 0);
    /* set player name */
    usr_form_init(frm, field_usr, sotu->username, 30);
#ifndef I_AM_OS_2
    if (IsVGA) {
      Short y_diff;
      VgaFormModify(frm, vgaFormModify160To240);
      y_diff = adjust_form_size(false);
      // move buttons
      PrvMoveObject(frm, btn_bul_ok, y_diff, false);
      PrvMoveObject(frm, btn_bul_cancel, y_diff,false);
    }
#endif
    FrmDrawForm(frm);
#ifdef I_AM_COLOR
    {
      DWord version;
      FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
      if (version >= 0x03503000L) {
	checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_10));
	CtlSetValue(checkbox, (my_prefs.color_on) ? 1 : 0);
	CtlShowControl(checkbox);
	checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_13));
	CtlSetValue(checkbox, (my_prefs.tiles_on) ? 1 : 0);
	CtlShowControl(checkbox);
      }
    }
#endif // COLOR
    /* set initial settings for lists */
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_bul_1));
    LstSetSelection(lst, ws-1);
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_bul_2));
    LstSetSelection(lst, rw-1);
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
      PrvMoveObject(frm, btn_bul_ok, y_diff, true);
      PrvMoveObject(frm, btn_bul_cancel, y_diff, true);
      FrmDrawForm(frm);
    }
    break;
#endif

  case lstSelectEvent:
    switch (e->data.lstSelect.listID) {
    case list_bul_1:
      ws = max(1, 1 + e->data.lstSelect.selection);      /* "hole" */
      break;
    case list_bul_2:
      rw = max(1, 1 + e->data.lstSelect.selection);      /* "donut" */
      break;
    }


    handled = true;
    break;
  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
      /*     case btn_bul_save: */
      /*       handled = true; */
      /*       break; */
    case btn_bul_ok:
      my_prefs.run_walk_border = rw+ws;
      my_prefs.walk_search_border = ws;
      /* read run/walk checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_1));
      my_prefs.run_on = (CtlGetValue(checkbox) != 0);	
      /* read save/don't checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_2));
      my_prefs.autosave_on = (CtlGetValue(checkbox) != 0);	
      /* read dave-mode checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_3));
      my_prefs.dave_mode = (CtlGetValue(checkbox) != 0);	
      /* read scroll-mode checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_4));
      my_prefs.scroll_mode = (CtlGetValue(checkbox) != 0);	
      /* read sound checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_5));
      my_prefs.sound_on = (CtlGetValue(checkbox) != 0);	
      /* read centered checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_6));
      my_prefs.stay_centered = (CtlGetValue(checkbox) != 0);	
      /* read font checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_7));
      my_prefs.font_small = (CtlGetValue(checkbox) != 0);	
      /* read rogue-relative checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_8));
      my_prefs.rogue_relative = (CtlGetValue(checkbox) != 0);	
      /* read crosshairs checkbox... */
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_9));
      my_prefs.crosshairs_on = (CtlGetValue(checkbox) != 0);	
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_bul_11));
      if (!my_prefs.sort_pack && CtlGetValue(checkbox))
	sort_pack(&(sotu->roguep->pack));
      my_prefs.sort_pack = (CtlGetValue(checkbox) != 0);	
      checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,check_bul_12));
      if ((CtlGetValue(checkbox) != 0) != my_prefs.black_bg)
	redraw = true;
      my_prefs.black_bg = (CtlGetValue(checkbox) != 0);
#ifdef I_AM_COLOR
      {
	/* read color checkbox... */
	DWord version;
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
	if (version >= 0x03503000L) {
	  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,check_bul_10));
	  if ((CtlGetValue(checkbox) != 0) != my_prefs.color_on)
	    redraw = true;
	  my_prefs.color_on = (CtlGetValue(checkbox) != 0);
	  if (!have_read_color && my_prefs.color_on)
	    // we'll need to read the memo.
	    // I used to check that the old mp.color_on was false.. need to????
	    look_for_memo(); // will set have_read_color true if found

	  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm,check_bul_13));
	  if ((CtlGetValue(checkbox) != 0) != my_prefs.tiles_on)
	    redraw = true;
	  my_prefs.tiles_on = (CtlGetValue(checkbox) != 0);
	  if (!have_read_tiles && my_prefs.tiles_on) {
	    look_for_tiles();
	    if (!have_read_tiles) {
	      FrmAlert(NoTiles);
	      my_prefs.tiles_on = false;
	    }

	  }
	}
      }
#endif // COLOR

      /* read player name... */
      usr_form_update(frm, field_usr, sotu->username, 30);
      /* write bools and shorts to a PREFERENCES data thingy. */
      writeRoguePrefs();
      LeaveForm();
      if (redraw) { // Basically same as the MainForm re-center menu item...
	move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
	print_stats(STAT_ALL); 
	check_message();  // clear the old message
      }
      /* It is important to do this after LeaveForm for msg to display */
      /* <anything with message> */
      handled = true;
      break;
    case btn_bul_cancel:
      LeaveForm();
      handled = true;
      break;
      /*
    case btn_bul_hw:
      FrmPopupForm(HwButtonsForm);
      handled = true;
      break;
      */
    case btn_bul_draw:
      draw_circle(ScrWidth/2, ScrHeight/2, (rw+ws)*WalkMagic, true);
      draw_circle(ScrWidth/2, ScrHeight/2, ws*WalkMagic, false);
      handled = true;
      break;
    case btn_bul_clear:
      //draw_circle(ScrWidth/2, ScrHeight/2, 160, false); // why?
      // let's try it without
      FrmDrawForm(frm);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}
