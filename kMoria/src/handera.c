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

/* note a couple of things are from a HandEra example... (ok to use) */

#ifndef I_AM_OS_2

#define DROWS MAX_HEIGHT
#define DCOLS MAX_WIDTH


extern Short WalkMagic;
Short StatsHeight;

static void PrvMoveObject(FormPtr frm, UInt oid, Coord x, Coord y,
			  Boolean draw) SEC_7;
static Short PrvResizeList(FormPtr frm, UInt oid, UInt bot_oid, Coord x,
			   Coord y) SEC_7;
static void PrvResizeField(FormPtr frm, UInt objid, Coord y_diff) SEC_7;
static void update_display_size() SEC_7;
static void adjust_form_size(Boolean draw, Short *dx, Short *dy,
			     Boolean move) SEC_7;

static Short message_y_ctr(FormPtr frm, UInt top_objid, UInt bot_objid) SEC_7;

/* This routine moves an object vertically within a form. */
static void PrvMoveObject(FormPtr frmP, UInt objid,
			  Coord x_diff, Coord y_diff, Boolean draw)
{
  RectangleType r;
  UInt objIndex = FrmGetObjectIndex(frmP, objid);
  if (y_diff == 0) return;    

  FrmGetObjectBounds(frmP, objIndex, &r);
  if (draw) {
    RctInsetRectangle(&r, -2);   //need to erase the frame as well
    WinEraseRectangle(&r, 0);
    RctInsetRectangle(&r, 2);
  }    
  r.topLeft.x += x_diff;
  r.topLeft.y += y_diff;
  FrmSetObjectBounds(frmP, objIndex, &r);
}


// Vertically resize a list.  (extra_y is like 0 or LINE_HEIGHT.)
// It will pull the list bottom down to the top of the boy_objid button,
// leaving at least extra_y pixels of space between them
static Short PrvResizeList(FormPtr frm, UInt objid, UInt bot_objid,
			   Coord add_x, Coord subtract_y)
{
  // Wonder if I need to erase the old space.
  Short lines, x, y, y_bot;
  ListPtr lst;
  UInt objIndex = FrmGetObjectIndex(frm, objid);
  RectangleType r;
  lst = FrmGetObjectPtr(frm, objIndex);
  // Ok, the lists I'm calling this on all are positioned just below the
  // title bar, and end just above a single row of buttons, one of which
  // is supplied in bot_objid.  Make the list as big as possible!
  //
  // In 160x160 these (x, y)'s start out at (0, 15) and (5, 141).
  // We really only care about the y's.
  FrmGetObjectPosition(frm, objIndex, &x, &y);
  FrmGetObjectPosition(frm, FrmGetObjectIndex(frm, bot_objid), &x, &y_bot);
  // Ok.  y_bot - y is the absolute max space available to the list.
  // Figure out how many lines we can put into that space (yay int division).
  lines = (y_bot - y - subtract_y) / FntLineHeight();
  if (objid == list_st) lines = (lines / 2) * 2; // keep the store list EVEN.
  LstSetHeight(lst, lines);
  if (add_x) {
    FrmGetObjectBounds(frm, objIndex, &r);
    r.extent.x += add_x;
    FrmSetObjectBounds(frm, objIndex, &r);
  }
  return lines;
  /*
    // This did not work:  ... maybe because I had objid instead of objIndex!
  FrmGetObjectBounds(frm, objIndex, &r);
  h = FntLineHeight();
  vis_items = r.extent.y / h; // calculate height INCLUDING blank slots thanks!
  vis_items = vis_items + y_diff / h; // add (or subtract) change
  // Hey - if subtracting - will int division work right, or fencepost?
  LstSetHeight(lst, vis_items);
  */
}


// Vertically resize a text field.
static void PrvResizeField(FormPtr frm, UInt objid, Coord y_diff)
{
  // Wonder if I need to erase the old space.  Probably I DO.
  RectangleType r;
  Short h;
  FieldPtr fld;
  UInt objIndex = FrmGetObjectIndex(frm, objid);
  if (y_diff == 0) return;
  fld = FrmGetObjectPtr(frm, objIndex);
  FldGetBounds(fld, &r); // same as using FrmGetObjectBounds, I b'lieve
  h = FntLineHeight();
  r.extent.y = r.extent.y + h * (y_diff / h); // could just add y_diff...
  FldSetBounds(fld, &r);
  // Hmmm.  It says "don't change the WIDTH of the object while it's visible"
}


// Set all the "constants" based on actual display size
extern Short statwidth_large[6];
extern Short statwidth_small[6];
extern Short statwidth_misc[8];
extern const Char *stat_abbrev[6];
void init_display_size()
{
  Short i;
  FntSetFont(BigFont);
  LINE_HEIGHT = FntCharHeight(); // "11"... 16 for handera
  visible_char_h = FntCharHeight() - 1; // "10".. really 16-1 = 15 ???
  visible_char_w = FntCharWidth('M'); // "8".. really 11.  (not 12??)
  // note: perhaps I should change 'M' to '@' it seems wider in handera.

  FntSetFont(SmallFont);
  //StatsHeight = 26; // "26".  133 + 1 + 26 = 160.  3*itsy+2 or 2*normal+4.
  StatsHeight = 3*FntCharHeight() + 2; // "26".  3*itsy+2 or 2*normal+4.
  visible_char_h_itsy = FntCharHeight() - 1; // 10, for real
  visible_char_w_itsy = FntCharWidth('M');   //  8, for real

  update_display_size();

  // Update all the things that were once #defines

  // in itsy stats, shove Mana 123 and HP 123/123 as far right as possible.
  FntSetFont(SmallFont);
  statwidth_misc[6] = FntCharsWidth("HP 000/000 ", 11) + 1; // 160 - 109
  statwidth_misc[7] = FntCharsWidth("Mana 000 ", 9) + 1 + statwidth_misc[6];
  // in large and itsy stats, calculate the positioning in advance
  for (i = 0 ; i < 6 ; i++)
    statwidth_small[i] = FntCharsWidth(stat_abbrev[i], StrLen(stat_abbrev[i]));
  FntSetFont(BigFont);
  for (i = 0 ; i < 6 ; i++)
    statwidth_large[i] = FntCharsWidth(stat_abbrev[i], StrLen(stat_abbrev[i]));
  statwidth_misc[0] = 1;
  statwidth_misc[1] = FntCharsWidth("AC 000 ", 7) + statwidth_misc[0]; // 30
  for (i = 0, statwidth_misc[2] = 0 ; i < 6 ; i++)
    statwidth_misc[2] += statwidth_large[i] + FntCharsWidth("00 ", 3);
  statwidth_misc[2] += FntCharWidth(' '); // 115
  statwidth_misc[3] = FntCharsWidth("L 00 ", 6) + statwidth_misc[0] + 1; // 21
  statwidth_misc[4] = FntCharsWidth("x 1234567 ", 10) + statwidth_misc[3];//45
  statwidth_misc[5] = FntCharsWidth("mana 000 ", 9) + statwidth_misc[4] +1;//43
  // now the stuff for Your Stats screen.
  // some of it is used in places other than form_chargen.c:
  // ROW_1 is used everywhere, ROW_4 is used in create.c.  (that's all.)
  ROW_1 = (15*3)/2; // VgaFrmGetTitleHeight(); ... NUTS. // 15
  ROW_2 = ROW_1 + LINE_HEIGHT; // 26
  ROW_3 = ROW_2 + 4*LINE_HEIGHT; // 70
  ROW_4 = ROW_3 + 2*LINE_HEIGHT + 2; // 94
  COL_1 = 0;
  COL_2 = COL_1 + FntCharsWidth("+ To Hit: -10", 13) - 1; // 50
  COL_3 = COL_1 + FntCharsWidth("Social Class: 100 ", 18); // 66
  COL_3a = COL_3 + FntCharsWidth("Weight: ", 8) - 1; // 98
  COL_4 = COL_3a + FntCharsWidth("000 ", 4); // 116
  COL_5 = SCR_WIDTH - FntCharsWidth("18/00", 5) - 1;  // 136
  COL_31 = COL_1 + FntCharsWidth("Experience:  1234567  ", 22); // 88
  //  COL_6 = ; // 146 // should really be ROW_
}



// Called AFTER form-dependent resizing,
// On entry to a form and whenever we receive a "displayExtentChangedEvent".
static void update_display_size()
{
  Coord         x, y;
  if (!IsVGA) return;

  // Note: we must be at least OS 3.5 to call this function.
  // get the DISPLAY size
  WinGetDisplayExtent(&x, &y);

  // Update our own measurements
  SCR_WIDTH = x; // "160"  (240, 305)
  SCR_HEIGHT = y; // "160"  (240, 305)
  WalkMagic = ( (x<y) ? x : y) / 16; // i.e. 10 if 160x160.  15 if 240x240.
  MAP_HEIGHT = SCR_HEIGHT - StatsHeight - 1; // "133".  133 + 1 + 26 = 160.
  // now 240 - 32 - 1 = 207.  if screen is expanded, what then?

  visible_h = MAP_HEIGHT / visible_char_h; // 207 / 
  visible_h_itsy = MAP_HEIGHT / visible_char_h_itsy; // 207 / 10 = 20.
  visible_w = SCR_WIDTH / visible_char_w; // 240 / 11? = 21 (240/12 = 20).
  visible_w_itsy = SCR_WIDTH / visible_char_w_itsy; // 240 / 8 = 30.

  // this will be necessary if screens get larger:
  /*  if (visible_h_itsy > DROWS) visible_h_itsy = DROWS;
      if (visible_w_itsy > DCOLS) visible_w_itsy = DCOLS; */
}

static UInt16 get_gsi(FormPtr frm)
{
  UInt16 i, num;
  num = FrmGetNumberOfObjects(frm);
  for (i = 0 ; i < num ; i++)
    if (FrmGetObjectType(frm, i) == frmGraffitiStateObj)
      return i;
  return -1;
}

void move_gsi(FormPtr frm, Boolean draw)
{
  RectangleType r;
  UInt objIndex = get_gsi(frm);
  Short inset;
  FrmGetObjectBounds(frm, objIndex, &r);
  if (draw) WinEraseRectangle(&r, 0);
  // inset = max(r.extent.x, r.extent.y); // Enough on 160 but not on Handera.
  inset = visible_char_h; // Let's try this instead.  10 is correct for 160.
  WinGetWindowBounds(&r);
  r.topLeft.x = r.extent.x - inset;
  r.topLeft.y = r.extent.y - inset;
  FrmSetObjectBounds(frm, objIndex, &r);
  // Handera says we need GsiSetLocation too.  Whatever.
  GsiSetLocation(r.topLeft.x + 1, r.topLeft.y + 1);
}


// draw should be true if form has been drawn before this is called
// draw should be false if form has not yet been drawn
// Only modal BORDERED forms should use move_y==true; it keeps their
// y dimension the same, and moves them up/down as needed.
static void adjust_form_size(Boolean draw, Short *dx, Short *dy, Boolean move)
{
  Coord         x, y, x_diff, y_diff;
  FormPtr frm = FrmGetActiveForm();
  RectangleType r;
  if (!IsVGA) { *dx = *dy = 0; return; }
  
  // There are three sizes.  DISPLAY size (WinGetDisplayExtent)
  // which is physical (modulo Handera's retractable graffiti area).
  // FORM size (FrmGetFormBounds) which includes border if any,
  // And WINDOW size (WinGetWindowBounds) that the form is drawn on.
  // Incidentally window size matches the numbers in your .rcp file.
  // If there's a 2 pixel border, then wherever the form size is
  // (0,0,160,160), there the window size would be (2,2,156,156),
  // i.e., form size for bordered forms is slightly larger than window.

  WinGetDisplayExtent(&x, &y);
  FrmGetFormBounds(frm, &r);
  x_diff = x - (r.topLeft.x + r.extent.x);
  y_diff = y - (r.topLeft.y + r.extent.y);

  // if we made the thing smaller, erase what was under silkscreen
  if (draw && (y_diff < 0)) {
    RectangleType erase_rect;
    // perhaps I should be using the later 'r' from WinGetWindowBounds.
    erase_rect           = r;
    if (move) {
      erase_rect.topLeft.y = r.extent.y + y_diff;
      erase_rect.extent.y  = -y_diff;
    }
    WinEraseRectangle(&erase_rect, 0);
  }

  // Set the WINDOW size the form is drawn on top of
  if (move) {
    r.extent.x += x_diff; // expand
    r.topLeft.y = y - r.extent.y; // move
    RctInsetRectangle(&r, 2); // adjust for frame width
  } else {
    WinGetWindowBounds(&r);
    r.extent.x += x_diff; // expand
    r.extent.y += y_diff; // expand
  }
  WinSetWindowBounds(FrmGetWindowHandle(frm), &r);
  // the form that called this function must manually REDRAW itself too.
  
  // we may need to move some objects - this will be FORM SPECIFIC.
  // let the caller know how much to move them by.
  *dx = x_diff; *dy = y_diff;//  return y_diff;
  
  // hey, what if we are on top of another form.  eh?
  // sigh.
}


static Short message_y_ctr(FormPtr frm, UInt top_objid, UInt bot_objid)
{
  Short y_0, y_1, dy;
  RectangleType r;
  // 128 = 15+110+3 or 141-LINE_WIDTH-2... hm.
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, top_objid), &r);
  y_0 = r.topLeft.y + r.extent.y;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, bot_objid), &r);
  y_1 = r.topLeft.y;
  dy = (y_1 - (y_0 + LINE_HEIGHT) + 1)/2; //(+1 to "round up" fraction)
  if (dy > LINE_HEIGHT/2) dy = LINE_HEIGHT/2; // don't let it drift much.
  return y_0 + dy;
}

// draw = true: called in 'case displayExtentChangedEvent'
// draw = false: called in 'case frmOpenEvent'... BEFORE FrmDrawForm.
extern Short STORE_LINES, gainspell_lines, invequ_lines, invsel_lines;
extern Short brief_comment_y;
void PrvMoveForm(Boolean draw)
{
  Short x_diff, y_diff;
  Word curfrm;
  FormPtr frm;
  FieldPtr fld;

  if (!IsVGA) return;
  curfrm =  FrmGetActiveFormID();
  frm = FrmGetActiveForm();
  if (!draw) {
    //    WinDrawChars("111", 3, 40, 40); // TESTING
    /*
      POSE, for some reason, thinks that calling VgaFormModify is an ERROR.
      This is really irritating.  (and it's the most recent version too.)
      The older emulator from HandEra did not complain.
    */
    VgaFormModify(frm, vgaFormModify160To240); // XXX <--- FUCK.
    //    WinDrawChars("888", 3, 40, 40); // TESTING
  }
  update_display_size();

  // first, small modal forms that only want to be moved and widened.
  switch(curfrm) {
  case BeamForm: case HaggleForm: case RestForm:
    adjust_form_size(draw, &x_diff, &y_diff, true);
    if (draw) FrmDrawForm(frm);
    // haggle needs some stuff reprinted, but it isn't saved anywhere..
    return;
 case BeamCashForm:
    adjust_form_size(draw, &x_diff, &y_diff, true);
    if (draw) {
      FrmDrawForm(frm);
      beamcash_draw();
    }
    return;
  case ScribeForm:
    adjust_form_size(draw, &x_diff, &y_diff, true);
    move_gsi(frm, draw);
    if (draw) {
      FrmDrawForm(frm);
      scribe_draw();
    }
    return;
  default:
    break;
  }

  // now, forms that want to be adjusted in HEIGHT and interior layout.
  adjust_form_size(draw, &x_diff, &y_diff, false);

  //  if (x_diff == 0 && y_diff == 0) return; // XXXX Is this ok?
  // no, because we need to get to the "brief_comment_y" calls!

  switch(curfrm) {
  case MainForm:
    move_gsi(frm, draw);
    set_statsbar_loc(my_prefs.stats_at_top);
    if (draw) {
      FrmDrawForm(frm);
      slide_dir(5);
      recalc_and_refresh(); // may be redundant
      print_stats(STATS_ALL); 
      show_messages(); // probably don't need that one.
    }
    return;
  case AboutForm:
    PrvMoveObject(frm, btn_about_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_about_more, 0, y_diff, draw);
    PrvMoveObject(frm, btn_about_credits, 0, y_diff, draw);
    break;
    /*
  case BeamForm:
    PrvMoveObject(frm, btn_beam_myself, 0, y_diff, draw);
    PrvMoveObject(frm, btn_beam_scores, 0, y_diff, draw);
    PrvMoveObject(frm, btn_beam_item, 0, y_diff, draw);
    PrvMoveObject(frm, btn_beam_cash, 0, y_diff, draw);
    PrvMoveObject(frm, btn_beam_done, 0, y_diff, draw);
    break;
  case BeamCashForm:
    PrvMoveObject(frm, btn_bc_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_bc_cancel, 0, y_diff, draw);
    if (draw) {
      FrmDrawForm(frm);
      beamcash_draw();
    }
    return;
    */
  case BookForm:
    // does not need list to be any larger!
    // oh, but it might need to be SMALLER.  dang.  consider handera rotation!!
    // I should just make it fixed at 4 lines, and have done with it...
    PrvMoveObject(frm, btn_book_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_book_cancel, 0, y_diff, draw);
    brief_comment_y = message_y_ctr(frm, list_book, btn_book_ok);
    if (draw) {
      FrmDrawForm(frm);
      show_mana_at_bottom();
    }
    return;
  case CharClassForm:
    PrvMoveObject(frm, btn_class_ok2, 0, y_diff, draw);   
    PrvMoveObject(frm, btn_class_no, 0, y_diff, draw);   
    PrvMoveObject(frm, btn_class_ok, 0, y_diff, draw);   
    PrvMoveObject(frm, btn_class_reroll, 0, y_diff, draw);   
    if (draw) {
      FrmDrawForm(frm);
      charclass_draw(true, true);
    }
    return;
  case CharGenderForm:
    PrvMoveObject(frm, btn_gender_ok, 0, y_diff, draw);   
    PrvMoveObject(frm, btn_gender_no, 0, y_diff, draw);   
    if (draw) {
      FrmDrawForm(frm);
      chargender_draw();
    }
    return;
  case CharRaceForm:
    PrvMoveObject(frm, btn_race_ok, 0, y_diff, draw);   
    if (draw) {
      FrmDrawForm(frm);
      charrace_draw();
    }
    break;
  case CharSummaryForm:
    move_gsi(frm, draw);
    PrvMoveObject(frm, btn_charsum_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_charsum_no, x_diff, y_diff, draw); /* note x_diff */
    if (!draw) {
      PrvMoveObject(frm, field_charsum, 0, -1, false); // scoot field up a tad!
    }
    if (draw) {
      FrmDrawForm(frm);
      charsummary_draw();
    }
    return;
  case CrownForm:
    if (draw) {
      FrmDrawForm(frm);
      crown_draw();
    }
    return;
  case DalekForm:
    if (draw) {
      FrmDrawForm(frm);
      dalek_draw();
    }
    return;
  case DeathForm:
    PrvMoveObject(frm, btn_death_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_death_wok, 0, y_diff, draw);
    if (draw) {
      FrmDrawForm(frm);
      tombstone_draw();
    }
    return;
  case EquForm:
    PrvMoveObject(frm, btn_eq_off, 0, y_diff, draw);
    PrvMoveObject(frm, btn_eq_drop, 0, y_diff, draw);
    PrvMoveObject(frm, btn_eq_cancel, 0, y_diff, draw);
    PrvMoveObject(frm, btn_eq_if, 0, y_diff, draw);
    invequ_lines = PrvResizeList(frm, list_eq, btn_eq_off, x_diff,
				 LINE_HEIGHT);
    brief_comment_y = message_y_ctr(frm, list_eq, btn_eq_off);
    break;
  case GainSpForm:
    PrvMoveObject(frm, btn_gain_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_gain_cancel, 0, y_diff, draw);
    PrvMoveObject(frm, btn_gain_tog, 0, y_diff, draw);
    gainspell_lines = PrvResizeList(frm, list_gain, btn_gain_ok, 0,
				    LINE_HEIGHT); //list won't need to be wider
    brief_comment_y = message_y_ctr(frm, list_gain, btn_gain_ok);
    if (draw) {
      FrmDrawForm(frm);
      gainsp_draw(false);
    }
    return;
  case HwButtonsForm:
    PrvMoveObject(frm, btn_hwb_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_hwb_cancel, 0, y_diff, draw);
    PrvResizeList(frm, list_hwb_1, btn_hwb_ok, 0, 0);
    // (no need to remember the new size of the list) (no need to widen either)
    break;
  case InvForm:
    PrvMoveObject(frm, btn_if_frob, 0, y_diff, draw);
    PrvMoveObject(frm, btn_if_drop, 0, y_diff, draw);
    PrvMoveObject(frm, btn_if_throw, 0, y_diff, draw);
    PrvMoveObject(frm, btn_if_cancel, 0, y_diff, draw);
    PrvMoveObject(frm, btn_if_eq, 0, y_diff, draw);
    invequ_lines = PrvResizeList(frm, list_if, btn_if_frob, x_diff,
				 LINE_HEIGHT);
    brief_comment_y = message_y_ctr(frm, list_if, btn_if_frob);
    break;
  case InvSelectForm:
    PrvMoveObject(frm, btn_is_frob, 0, y_diff, draw);
    PrvMoveObject(frm, btn_is_cancel, 0, y_diff, draw);
    //invsel_lines = PrvResizeList(frm, list_is, btn_is_frob, LINE_HEIGHT);
    invsel_lines = PrvResizeList(frm, list_is, btn_is_frob, x_diff, 0);
    break;
  case LookForm:
    PrvMoveObject(frm, btn_look_exit, 0, y_diff, draw);
    PrvMoveObject(frm, btn_look_recall, 0, y_diff, draw);
    if (draw) {
      FrmDrawForm(frm);
      look_draw(false);
    }
    return;
  case MapForm:
    if (draw) {
      FrmDrawForm(frm);
      map_draw();
    }
    return;
  case MsgLogForm:
    PrvMoveObject(frm, btn_ml_ok, 0, y_diff, draw);
    PrvMoveObject(frm, repeat_ml_up, 0, y_diff, draw);
    PrvMoveObject(frm, repeat_ml_down, 0, y_diff, draw);
    PrvResizeField(frm, field_ml, y_diff); // orig. top 15, orig. height 121
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_ml));
    update_field_scrollers(frm, fld, repeat_ml_up, repeat_ml_down);
    break;
  case PrefsForm:
    // LOTS of widgets in this form... leave 'em there.
    PrvMoveObject(frm, btn_prf_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_prf_cancel, 0, y_diff, draw);
    break;
  case RecallForm:
    PrvMoveObject(frm, btn_recall_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_recall_next, 0, y_diff, draw);
    PrvMoveObject(frm, repeat_recall_up, 0, y_diff, draw);
    PrvMoveObject(frm, repeat_recall_down, 0, y_diff, draw);
    PrvResizeField(frm, field_recall, y_diff);// orig. top 15, orig. height 121
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_recall));
    update_field_scrollers(frm, fld, repeat_recall_up, repeat_recall_down);
    break;
  case RepeatCmdsForm:
    PrvMoveObject(frm, btn_rptc_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_rptc_cancel, 0, y_diff, draw);
    break;
  case ScoresForm:
    PrvMoveObject(frm, btn_scores_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_scores_next, 0, y_diff, draw);
    if (draw) {
      FrmDrawForm(frm);
      draw_topscores(true); // resets it to start.. hm..
    }
    return;
  case SnowCrashForm:
    break;
  case SpellHwBtnsForm:
    // the spells list does not need to be any larger, but let's move the popup
    PrvMoveObject(frm, btn_shwb_ok, 0, y_diff, draw);
    PrvMoveObject(frm, popup_shwb_2, 0, y_diff, draw);
    PrvMoveObject(frm, list_shwb_2, 0, y_diff, draw);
    if (draw) {
      FrmDrawForm(frm);
      show_spell_char(-1);
    }
    return;
  case StoreForm:
    // xxx needs the list list_st resized: (0 26 112 110)
    // XXX ... and extra prices drawn ...
    // needs about a LINE_HEIGHT between it and the button btn_st_buy ...
    PrvMoveObject(frm, btn_st_buy, 0, y_diff, draw);
    PrvMoveObject(frm, btn_st_sell, 0, y_diff, draw);
    PrvMoveObject(frm, btn_st_prev, 0, y_diff, draw);
    PrvMoveObject(frm, btn_st_next, 0, y_diff, draw);
    PrvMoveObject(frm, btn_st_exit, 0, y_diff, draw);
    free_store_list(frm); // (BEFORE changing store_lines)
    STORE_LINES = PrvResizeList(frm, list_st, btn_st_buy, x_diff, LINE_HEIGHT);
    brief_comment_y = message_y_ctr(frm, list_st, btn_st_buy);
    if (draw) {
      FrmDrawForm(frm);
      store_list_tens(0);
    }
    return;
  case SpellForm:
    // does not need list to be any larger!
    PrvMoveObject(frm, btn_spell_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_spell_cancel, 0, y_diff, draw);
    if (draw) {
      FrmDrawForm(frm);
      spell_draw();
    }
    return;
  case UserForm:
    // the list need not be resized, I think.  (hm. even if form gets smaller?)
    PrvMoveObject(frm, btn_user_ok, 0, y_diff, draw);
    PrvMoveObject(frm, btn_user_new, 0, y_diff, draw);
    PrvMoveObject(frm, btn_user_cancel, 0, y_diff, draw);
    PrvMoveObject(frm, check_user_1, 0, y_diff, draw);
    if (draw) {
      ListPtr lst;
      Short s;
      FrmDrawForm(frm);
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_user));
      if ((s = LstGetSelection(lst)) != -1)  show_user_info(frm, s);
    }
    return;
  }
  if (draw) FrmDrawForm(frm);
}
/*
  Just add this in frmOpenEvent before FrmDrawForm:
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
  And add this as a separate case after frmOpenEvent:
#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif
*/


void handera_rotate()
{
  VgaScreenModeType   screenMode;
  VgaRotateModeType   rotateMode;
  if (!IsVGA) return;
  VgaGetScreenMode(&screenMode, &rotateMode);
  VgaSetScreenMode(screenMode1To1, VgaRotateSelect(rotateMode));
  PrvMoveForm(true);
}

#endif
