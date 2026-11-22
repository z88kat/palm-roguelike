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
 * This form lets you view and modify your preferences.
 * There are a bunch of checkboxes and things in it,
 * and an ok and a cancel button, as you might expect.
 ***********************************************************************/
#ifdef I_AM_COLOR
extern Boolean found_color;
#endif

static void init_checkboxes_1() SEC_3;
static void init_checkboxes_2() SEC_3;
static void init_lists(Short rw, Short ws) SEC_3;
static void set_side_left(Boolean left) SEC_3;
static void show_lists(Boolean show) SEC_3;
static void show_boxes(const Int8 *id, Short num, Boolean show) SEC_3;
static void show_box_color(Boolean show) SEC_3;
static void prefs_form_save_prefs(Short rw, Short ws) SEC_3;
static Boolean prefs_save_lists(Short rw, Short ws) SEC_3;
static Boolean prefs_save_checkboxes_1() SEC_3;
static Boolean prefs_save_checkboxes_2() SEC_3;

Boolean prefs_show_left = true;

static const Int8 id1[8] = { 1, 4, 11, 12, 13, 15, 16, 17 };
static const Int8 id2[8] = { 2, 3,  5,  6,  7,  8,  9, 10 };

Boolean Prefs_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  static Short rw = 0;
  static Short ws = 0;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    rw = max(1, my_prefs.run_walk_border - my_prefs.walk_center_border);
    ws = max(1, my_prefs.walk_center_border);
    init_checkboxes_1();
    init_checkboxes_2();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    init_lists(rw, ws);

    set_side_left(prefs_show_left);
    {
      Word btnid = (prefs_show_left ? pbtn_prf_0 : pbtn_prf_1);
      ControlPtr pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btnid));
      CtlSetValue(pushbtn, true);
    }

    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case lstSelectEvent:
    switch (e->data.lstSelect.listID) {
    case list_prf_1:
      ws = max(1, 1 + e->data.lstSelect.selection);      /* "hole" */
      break;
    case list_prf_2:
      rw = max(1, 1 + e->data.lstSelect.selection);      /* "donut" */
      break;
    }
    handled = true;
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case pbtn_prf_0:
      if (!prefs_show_left) set_side_left(true);
      handled = true;
      break;
    case pbtn_prf_1:
      if (prefs_show_left) set_side_left(false);
      handled = true;
      break;
    case btn_prf_ok:
      prefs_form_save_prefs(rw, ws);
      handled = true;
      break;
    case btn_prf_cancel:
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

static void set_side_left(Boolean left)
{
  Boolean right = !left;
  if (left) {
    show_boxes(id2, 8, right);
    show_lists(right);
    show_boxes(id1, 8, left);
    show_box_color(left);
  } else {
    show_boxes(id1, 8, left);
    show_box_color(left);
    show_boxes(id2, 8, right);
    show_lists(right);
  }
  prefs_show_left = left;
}

static void init_checkboxes_1()
{
  FormPtr frm;
  ControlPtr checkbox;
  frm = FrmGetActiveForm();
  // 1, 4, 11, 12, 13, 14, 15
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_1));
  CtlSetValue(checkbox, (my_prefs.big_font ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_4));
  CtlSetValue(checkbox, (my_prefs.sound ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_11));
  CtlSetValue(checkbox, (my_prefs.highlight_minerals ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_12));
  CtlSetValue(checkbox, (my_prefs.stats_at_top ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_13));
  CtlSetValue(checkbox, (my_prefs.black_bg ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_14));
  CtlSetValue(checkbox, (my_prefs.color_on ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_15));
  CtlSetValue(checkbox, (my_prefs.no_animations ? 0 : 1)); // REVERSED
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_16));
  CtlSetValue(checkbox, ((my_prefs.determine_recent_user & RESTORE_PROMPT)
			 ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_17));
  CtlSetValue(checkbox, ((my_prefs.determine_recent_user & RESTORE_RECENT)
			 ? 1 : 0));
}
static void init_checkboxes_2()
{
  FormPtr frm;
  ControlPtr checkbox;
  frm = FrmGetActiveForm();
  // 2, 3, 5, 6, 7, 8, 9, 10
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_2));
  CtlSetValue(checkbox, (my_prefs.relative_move ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_3));
  CtlSetValue(checkbox, (my_prefs.overlay_circles ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_5));
  CtlSetValue(checkbox, (my_prefs.run ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_6));
  CtlSetValue(checkbox, (my_prefs.run_cut_corners ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_7));
  CtlSetValue(checkbox, (my_prefs.run_examine_corners ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_8));
  CtlSetValue(checkbox, (my_prefs.run_stop_sector ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_9));
  CtlSetValue(checkbox, (my_prefs.run_thru_doors ? 1 : 0));
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_10));
  CtlSetValue(checkbox, (my_prefs.prompt_pickup ? 1 : 0));
}
static void show_box_color(Boolean show)
{
  FormPtr frm;
  ControlPtr checkbox;
  frm = FrmGetActiveForm();
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_14));
#ifdef I_AM_COLOR
  if (found_color && show)
    CtlShowControl(checkbox);
  else
#endif
    CtlHideControl(checkbox);
}

static void init_lists(Short rw, Short ws)
{
  FormPtr frm;
  ListPtr lst;
  frm = FrmGetActiveForm();
  /* set initial settings for lists */
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_prf_1));
  LstSetSelection(lst, ws-1);
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_prf_2));
  LstSetSelection(lst, rw-1);
}

static void show_lists(Boolean show)
{
  FormPtr frm;
  Word lsti;
  Short i;
  frm = FrmGetActiveForm();
  for (i = 0 ; i < 4 ; i++) {
    lsti = FrmGetObjectIndex(frm, list_prf_1 + i);
    if (show) FrmShowObject(frm, lsti);
    else FrmHideObject(frm, lsti);
  }
}

static void show_boxes(const Int8 *id, Short num, Boolean show)
{
  FormPtr frm;
  ControlPtr checkbox;
  Short i, j;
  frm = FrmGetActiveForm();
  for (i = 0 ; i < num ; i++) {
    j = check_prf_1 - 1 + id[i];
    checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, j));
    if (show) CtlShowControl(checkbox);
    else CtlHideControl(checkbox);
  }
}

static Boolean prefs_save_lists(Short rw, Short ws)
{
  Boolean dirty = false;
  if (my_prefs.overlay_circles &&
      (my_prefs.run_walk_border != rw+ws ||
       my_prefs.walk_center_border != ws))
    dirty = true; // need to erase old circles
  my_prefs.run_walk_border = rw+ws;
  my_prefs.walk_center_border = ws;
  return dirty;
}

static Boolean prefs_save_checkboxes_1()
{
  FormPtr frm;
  ControlPtr checkbox;
  Boolean val, dirty = false;
  frm = FrmGetActiveForm();
  // 1, 4, 11, 12, 13, 14, 15
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_1));
  my_prefs.big_font = (CtlGetValue(checkbox) != 0);
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_4));
  my_prefs.sound = (CtlGetValue(checkbox) != 0);
  // Show different minerals:
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_11));
  val = (CtlGetValue(checkbox) != 0); // decide to draw or erase minerals?
  if (my_prefs.highlight_minerals != val)
    dirty = true;
  my_prefs.highlight_minerals = val;
  // Status bar at top or bottom:
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_12));
  val = (CtlGetValue(checkbox) != 0); // decide to move status bar
  if (my_prefs.stats_at_top != val)
    /*very*/dirty = true;
  my_prefs.stats_at_top = val;
  // Inverted background:
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_13));
  val = (CtlGetValue(checkbox) != 0);
  if (my_prefs.black_bg != val)
    /*very*/dirty = true;
  my_prefs.black_bg = val;
  // Color:
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_14));
  val = (CtlGetValue(checkbox) != 0);
  if (my_prefs.color_on != val)
    /*very*/dirty = true;
  my_prefs.color_on = val;
  // Animations
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_15));
  my_prefs.no_animations = (CtlGetValue(checkbox) == 0); // reversed
  // Prompt for save file
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_16));
  if (CtlGetValue(checkbox) == 0)
    my_prefs.determine_recent_user &= ~RESTORE_PROMPT;
  else
    my_prefs.determine_recent_user |= RESTORE_PROMPT;
  // Select most recent save file
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_17));
  if (CtlGetValue(checkbox) == 0)
    my_prefs.determine_recent_user &= ~RESTORE_RECENT;
  else
    my_prefs.determine_recent_user |= RESTORE_RECENT;
  return dirty;
}

static Boolean prefs_save_checkboxes_2()
{
  FormPtr frm;
  ControlPtr checkbox;
  Boolean val, dirty = false;
  frm = FrmGetActiveForm();
  // 2, 3, 5, 6, 7, 8, 9, 10
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_2));
  val = (CtlGetValue(checkbox) != 0); // decide to erase old circles
  if (my_prefs.overlay_circles && (val != my_prefs.relative_move))
    dirty = true;
  my_prefs.relative_move = val;
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_3));
  val = (CtlGetValue(checkbox) != 0); // decide to draw or erase circles
  if (my_prefs.overlay_circles != val)
    dirty = true;
  my_prefs.overlay_circles = val;
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_5));
  val = (CtlGetValue(checkbox) != 0); // decide to erase old 'run' circle
  if (my_prefs.overlay_circles && (val != my_prefs.run))
    dirty = true;
  my_prefs.run = val;
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_6));
  my_prefs.run_cut_corners = (CtlGetValue(checkbox) != 0);
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_7));
  my_prefs.run_examine_corners = (CtlGetValue(checkbox) != 0);
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_8));
  my_prefs.run_stop_sector = (CtlGetValue(checkbox) != 0);
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_9));
  my_prefs.run_thru_doors = (CtlGetValue(checkbox) != 0);
  checkbox = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, check_prf_10));
  my_prefs.prompt_pickup = (CtlGetValue(checkbox) != 0);
  return dirty;
}

static void prefs_form_save_prefs(Short rw, Short ws)
{
  Boolean dirty, verydirty;
  dirty = prefs_save_lists(rw, ws);
  verydirty = prefs_save_checkboxes_1();
  dirty = prefs_save_checkboxes_2() || dirty;

  useMoriaPrefs(); // map most of these back to the right moria globals
  writeMoriaPrefs();
  LeaveForm();
  if (verydirty) {
    nuke_display(); // clear-and-redraw absolutely everything
  } else if (dirty) {
    get_panel(char_row, char_col, true); // FORCE recalculation of panel
    recalc_and_refresh(); //	recalc_screen(); refresh(true);
  }

}

