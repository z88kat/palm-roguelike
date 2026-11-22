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

// these live in lock.c
extern Short cur_height, cur_width;


static void put_char_at(Short row, Short col, Char ch, Boolean bold) SEC_6;
static void refresh_2() SEC_6; // refresh top 2 lines only
static void print_stats_large(UInt which_stats);
static void print_stats_itsy(UInt which_stats);
static void print_stats_real(UInt which_stats, Boolean large) SEC_5;
static void print_stats_hp(Short width_used) SEC_5;
void try_print_status() SEC_5;
static void draw_circle(Short radius, Short x0, Short y0) SEC_5;
static void xor_runwalk_circles() SEC_5;

// Ok.  I have how much screen?
// Top line sometimes taken by messages
// Bottom two lines taken
// *** I moved the #define's (for font character sizes) into constant.h

Boolean itsy_on = false;
Boolean xored_runwalk_on = false;

/* This is the offset to convert character-positions in
   the "visible" part of the screen represented by 'terminal'
   to the larger "real" dungeon represented by 'buffer'. */
Short visible_x = 0, visible_y = 0;
Short WalkMagic = 10;

#define DROWS MAX_HEIGHT
#define DCOLS MAX_WIDTH


// These are display-dependent now: see constant.h for more comments.
Int8 visible_h = 13;
Int8 visible_w = 20;
Int8 visible_char_h = 10;
Int8 visible_char_w = 8;
Int8 visible_h_itsy = 22;
Int8 visible_w_itsy = 32;
Int8 visible_char_h_itsy = 6;
Int8 visible_char_w_itsy = 5;
Int8 LINE_HEIGHT = 11;
Short SCR_WIDTH = 160;
Short SCR_HEIGHT = 160;
Short MAP_HEIGHT = 133; /* in main screen */
Short HAG_Y = 80; /* in store2.c */
Short COL_1 =    0;
Short COL_2 =   50;
Short COL_3 =   66;
Short COL_3a =  98;
Short COL_31 =  88;
Short COL_4 =  116;
Short COL_5 =  136;
Short ROW_1 =   15;
Short ROW_2 =   26;
Short ROW_3 =   70;
Short ROW_4 =   94;
// end of formerly-constant things

// MAX_VIS_H and MAX_VIS_W values are for HandEra 330 expanded/rotated screen.
#define MAX_VIS_H 27 /* xxx Change this if I do Sony 320x320, */
#define MAX_VIS_W 38 /* xxx or if I do Font56.txt on a HandEra. */
//Char terminal[visible_h_itsy][visible_w_itsy];
Char terminal[MAX_VIS_H][MAX_VIS_W];/*relative: model of the screen*/
Char buffer[DROWS][DCOLS]; /* absolute - a model of the dungeon */
Boolean screen_dirty; //, stats_dirty = true;
Boolean lines_dirty[DROWS]; /* absolute */
Boolean screen_recalced = false;


Short words_dirty_x[2];
Boolean pending_messages;
Char *old_messages[SAVED_MSGS];
Short last_old_message_shown;
extern UInt need_print_stats;
UChar current_print_stats = CURSTAT_NONE;





/*
 * Switch between the small font and the regular font.
 * (Do NOT allow the small font if the SDK that I compiled with
 * is inappropriate for your operating system version.)
 */
Boolean toggle_itsy()
{
  DWord version;
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  if (version < 0x03000000l) {
    itsy_on = !itsy_on;
    return true;
  } else itsy_on = false;
#else
  if (version >= 0x03000000l) {
    itsy_on = !itsy_on;
    return true;
  } else itsy_on = false;
#endif
  return false;
}

Boolean stats_bar_on_bottom = true;
Short DunTopY = 0; // 0 or <DunLineY+1>
Short DunBotY = 0; // <DunLineY-1> or 160
Short DunLineY = 133;//MAP_HEIGHT //where to draw The Line: 133 or 160-133= 27
Short StatsTopY = 134; // add 4 to be where large stats start... hm
Short StatsBotY = 160;//add -12 to start itsy stats: 160 or <DunLineY-1>
// note: the 'word' stats will still be printed at DunTopY.

// caller should decide whether to redraw screen.
void set_statsbar_loc(Boolean stats_at_top)
{
  if (stats_at_top) {
    StatsTopY = 0;
    DunLineY = SCR_HEIGHT-MAP_HEIGHT; // 160-133
    StatsBotY = DunLineY-1;
    DunTopY = DunLineY+1;
    DunBotY = SCR_HEIGHT;
  } else {
    DunTopY = 0;
    DunLineY = MAP_HEIGHT; // 133
    DunBotY = DunLineY-1;
    StatsTopY = DunLineY+1;
    StatsBotY = SCR_HEIGHT;
  }
  stats_bar_on_bottom = !stats_at_top;
}


// Translate a tap on the screen into absolute-dungeon coordinates.
// So that the user can ask "what's in this cell?"
void where_in_dungeon(Short scr_x, Short scr_y, Short *dun_x, Short *dun_y)
{
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
  if (!stats_bar_on_bottom)
    scr_y -= DunTopY; // need a slight vertical shift
  *dun_x = scr_x / vc_w;
  *dun_y = scr_y / vc_h;
  *dun_x += visible_x;
  *dun_y += visible_y;
  return;
}

// Draw 1 cell on screen (here row,col are NOT screen-relative.)
// For making animatedness. If c = 0, it will set cell to 'original' value
void animate_char(Short y, Short x, Char c, Boolean bold)
{
  Short row, col, ch;
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;

  if (y < visible_y || x < visible_x ||
      y >= visible_y + v_h || x >= visible_x + v_w)
    return; // the cursor position is not visible on the screen!

  if (c != 0 && FrmGetActiveFormID() == MainForm)
    lines_dirty[y] = screen_dirty = true;// worst case, refresh will clean up

  row = y - visible_y;
  col = x - visible_x;
  
  ch = terminal[row][col];
  if (c == 0) c = ch;

#ifdef I_AM_OS_2
  if (itsy_on) FntSetFont(ledFont);
#else
  if (itsy_on) FntSetFont(SmallFont);
#endif
#ifdef I_AM_COLOR
  if (IsColor) { start_color(); }
#endif //I_AM_COLOR

  put_char_at(row, col, c, bold);


#ifdef I_AM_COLOR
  if (IsColor) { stop_color(); }
#endif //I_AM_COLOR
#ifdef I_AM_OS_2
  if (itsy_on) FntSetFont(stdFont);
#else
  if (itsy_on) FntSetFont(BigFont);
#endif

  terminal[row][col] = ch;
}


/* 
 *  This will clear the part of the display that belongs to
 *  "the dungeon".  It also clears 'terminal' since that is
 *  a representation of the display.
 *  This should be called before the offset for 'terminal' is
 *  moved to show a different part of 'buffer'.
 */
// Actually, the Moria equivalent of this clears the WHOLE screen.
void clear_screen() 
{
  Short i, j;
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  //  RectangleType r;
  if (FrmGetActiveFormID() != MainForm) return;
  v_w = min(v_w, DROWS);
  v_h = min(v_h, DCOLS);

  /* Clear the physical screen. */
  //  RctSetRectangle(&r, 0, 0, 160, 134-1);
  //  WinEraseRectangle(&r, 0);

  /* Update 'terminal' to represent the cleared screen. */
  screen_dirty = true;
  for (i = 0 ; i < v_h; i++) {
    lines_dirty[i+visible_y] = true;
    for (j = 0; j < v_w; j++) {
      terminal[i][j] = ' ';
    }
  }
}

void print(Short ch, Short row, Short col) {
  buffer[row][col] = ch;
  lines_dirty[row] = true;
  screen_dirty = true;
}



/*
 * Draw the whole screen, as the player knows it.
 * On second thought, maybe update but DON'T draw.
 */
void recalc_screen()
{
  Short row, col;// absolute
  if (FrmGetActiveFormID() != MainForm) return;
  clear_screen();
  for (row = 0 ; row < cur_height ; row++)
    for (col = 0 ; col < cur_width ; col++)
      buffer[row][col] = loc_symbol(row, col);
  //      buffer[row][col] = '^';
  //  for (row = visible_y, vr = 0 ; vr < visible_h_itsy ; row++, vr++)
  //    for (col = visible_x, vc = 0 ; vc < visible_w_itsy ; col++, vc++)
  //      terminal[vr][vc] = buffer[row][col];
  /*refresh();*/
  screen_recalced = true;
}
/*
 * The funtion to actually draw a character ch at a col,row position
 * (row, col are RELATIVE i.e. this is used in model-of-screen)
 * note the row+1 offset is so that we have a line for msgs.
 * The character will be drawn in either the small or normal font.
 *
 * if color is available and is on,
 * put_char_at is only called with the draw state pushed,
 * and it gets popped when finished calling put_char_at.
 */
static void put_char_at(Short row, Short col, Char ch, Boolean bold)
{
  //  Short cheat, vcheat = itsy_on ? 14 : 5; // center the dungeon vertically
  //Short cheat, vcheat = itsy_on ? 0 : 0; // center the dungeon vertically
  Short cheat, vcheat = stats_bar_on_bottom ? 0 : DunTopY;
  /* the space has already been cleared with clear_line */
  /* on second thought, no it hasn't. */
  RectangleType r;
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;

  RctSetRectangle(&r, col * vc_w, row*vc_h+vcheat,
		  vc_w, vc_h); /* Left, Top, W, H */

  if (!my_prefs.black_bg || IsColor)
    WinEraseRectangle(&r, 0);
  else
    WinDrawRectangle(&r, 0);
    
  // calculate pixel position of "row, col" and put char there
  cheat = vc_w - FntCharWidth(ch); // center the variable width characters

  if (cheat <= 1)   cheat = 0;
  else              cheat /= 2;

  if (ch != ' ') {

#ifdef I_AM_COLOR
  if (IsColor) {
    WinSetTextColor(get_color(ch, col+visible_x, row+visible_y));
  }
#endif

    if (!itsy_on && (ch== 'g' || ch== 'j' || ch== 'p' ||ch == 'q' ||ch == 'y'))
      vcheat--; // unfortunately, letters with dangling bits are a pain.
    
    if (!my_prefs.black_bg || IsColor)
      WinDrawChars(&ch, 1, col * vc_w + cheat, row * vc_h+vcheat);
    else
      WinDrawInvertedChars(&ch, 1, col * vc_w + cheat, row * vc_h+vcheat);

    if (bold)  WinInvertRectangle(&r, 0); /* 0 for square corners */
  }
  terminal[row][col] = ch;

}

/*
 *  Actually redraw the screen.  (if it is 'dirty'.)
 */
static void refresh_tail() SEC_5;
//extern Char last_command; // use this to un-show "detect <foo>" creatures 
extern previous_state curr_state; // replaces last_command...
void refresh(Boolean clearall)
{
  Short col, line, absline;
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
  Short vcheat = stats_bar_on_bottom ? 0 : DunTopY;
  RectangleType r;
  if (curr_state.mode == MODE_SHOWINVIS) return; // don't overwrite nifty detected creatures
  if (FrmGetActiveFormID() != MainForm) return;
  if (!screen_dirty) return;
  screen_dirty = false;
  if (screen_recalced) { recalc_and_refresh(); return; }

  if (xored_runwalk_on) {
    xor_runwalk_circles(); // erase the old one
    xored_runwalk_on = false;
  }

#ifdef I_AM_OS_2
  if (itsy_on) FntSetFont(ledFont);
#else
  if (itsy_on) FntSetFont(SmallFont);
#endif
#ifdef I_AM_COLOR
  if (IsColor) { start_color(); }
#endif //I_AM_COLOR


  v_w = min(v_w, cur_width  - visible_x);
  v_h = min(v_h, cur_height - visible_y);
  if (v_h > DROWS) v_h = DROWS;
  if (v_w > DCOLS) v_h = DCOLS;

  for (line = 0 ; line < v_h ; line++) {
    /* line is RELATIVE, line+visible_y is ABSOLUTE. */
    absline = line + visible_y;
    if (absline < 0 || !lines_dirty[absline]) continue;
    //// this rectangle's y coords must match with put_char_at's...
    if (clearall) {
      RctSetRectangle(&r, 0, line*vc_h+vcheat, SCR_WIDTH, vc_h);
      qWinEraseRectangle(&r, 0);
    }
    //// ...ok maybe that worked
    lines_dirty[absline] = false;
    for (col = 0; col < v_w; col++) {
      /* col is RELATIVE, col+visible_x is ABSOLUTE. */
      if (col+visible_x >= cur_width) break;
      if (col+visible_x < 0) continue;
      if (buffer[absline][col+visible_x] != terminal[line][col]) {
	put_char_at(line, col, buffer[absline][col+visible_x], false);
      }
    }
  }
  refresh_tail();
}
// This is an "optimized" (redundant bits taken out) combo of recalc + refresh
// to spped up the transition from screen to screen I hope
void recalc_and_refresh()
{
  Short col, line, absline;
  Short v_h = itsy_on ? visible_h_itsy : visible_h;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  Word this_form = FrmGetActiveFormID();
  //  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
  //  RectangleType r;
  if (this_form != MainForm &&
      this_form != LookForm) return;
  screen_recalced = false;
  screen_dirty = false;

#ifdef I_AM_OS_2
  if (itsy_on) FntSetFont(ledFont);
#else
  if (itsy_on) FntSetFont(SmallFont);
#endif
#ifdef I_AM_COLOR
  if (IsColor) { start_color(); }
#endif //I_AM_COLOR


  if (!itsy_on && this_form == MainForm) {
    // erase that last 1-2 pixels of turds
    RectangleType r;
    RctSetRectangle(&r, 0, DunBotY-2, SCR_WIDTH, 2); // xxx still 2 in handera?
    qWinEraseRectangle(&r, 0);
  }
  xored_runwalk_on = false;

  v_w = min(v_w, cur_width  - visible_x);
  v_h = min(v_h, cur_height - visible_y);
  v_w = min(v_w, DROWS);
  v_h = min(v_h, DCOLS);

  for (line = 0 ; line < v_h ; line++) {
    /* line is RELATIVE, line+visible_y is ABSOLUTE. */
    absline = line + visible_y;
    //// this rectangle's y coords must match with put_char_at's...
    //    RctSetRectangle(&r, 0, line*vc_h, 160, vc_h);
    //    WinEraseRectangle(&r, 0);
    //// ...ok cleared
    lines_dirty[absline] = false; // keep this
    for (col = 0; col < v_w; col++) {
      /* col is RELATIVE, col+visible_x is ABSOLUTE. */
      if (col+visible_x >= cur_width) break;
      buffer[absline][col+visible_x] = loc_symbol(absline, col+visible_x);
      put_char_at(line, col, buffer[absline][col+visible_x], false);
      // (put_char_at takes care of setting terminal[][].)
    }
  }
  refresh_tail();
}

// clear EVERYTHING, and redraw dungeon and stats
void nuke_display()
{
  RectangleType r;
  RctSetRectangle(&r, 0, 0, SCR_WIDTH, SCR_HEIGHT);
  qWinEraseRectangle(&r, 0);
  get_panel(char_row, char_col, true); // FORCE recalculation of panel
  recalc_and_refresh(); //	recalc_screen(); refresh(true);
  //  stats_top_dirty = stats_bot_dirty = true;
  print_stats_real(STATS_ALL, true);
}

static void refresh_tail()
{
  //  WinDrawLine(0,12,160,12);
  //  spell_stats(env); // don't forget the spell-status on right hand side.
#ifdef I_AM_COLOR
  if (IsColor) { stop_color(); }
#endif //I_AM_COLOR
#ifdef I_AM_OS_2
  if (itsy_on) FntSetFont(stdFont);
#else
  if (itsy_on) FntSetFont(BigFont);
#endif

  // also clean up the line (just above bottom msgline or just above top one)
  qWinEraseLine(0, DunLineY-1, SCR_WIDTH, DunLineY-1);

  xor_runwalk_circles();
  xored_runwalk_on = true;

  // oh yeah, this might be a good idea.
  try_print_status();
}

static void refresh_2()
{
  Short col, line, absline;
  Short v_w = itsy_on ? visible_w_itsy : visible_w;
  if (FrmGetActiveFormID() != MainForm) return;

#ifdef I_AM_OS_2
  if (itsy_on) FntSetFont(ledFont);
#else
  if (itsy_on) FntSetFont(SmallFont);
#endif
#ifdef I_AM_COLOR
  if (IsColor) { start_color(); }
#endif //I_AM_COLOR

  if (xored_runwalk_on) {
    xor_runwalk_circles(); // erase the old one
    xored_runwalk_on = false;
  }

  v_w = min(v_w, cur_width  - visible_x);
  v_w = min(v_w, DROWS);

  for (line = 0 ; line < 2 ; line++) {
    /* line is RELATIVE, line+visible_y is ABSOLUTE. */
    absline = line + visible_y;
    lines_dirty[absline] = false;
    // Draw only from the edge of the status words.  Ha.
    for (col = words_dirty_x[line]; col < v_w; col++) {
      /* col is RELATIVE, col+visible_x is ABSOLUTE. */
      if (col+visible_x >= cur_width) break;
      // if (buffer[absline][col+visible_x] != terminal[line][col]) // no..
      put_char_at(line, col, buffer[absline][col+visible_x], false);
    }
  }
#ifdef I_AM_COLOR
  if (IsColor) { stop_color(); }
#endif //I_AM_COLOR
#ifdef I_AM_OS_2
  if (itsy_on) FntSetFont(stdFont);
#else
  if (itsy_on) FntSetFont(BigFont);
#endif

  xor_runwalk_circles();
  xored_runwalk_on = true;
  try_print_status();
}

// Ok... draw_directional needs this...
void erase_runwalk_circles() SEC_5;
void erase_runwalk_circles()
{
  if (xored_runwalk_on) {
    xor_runwalk_circles();
    xored_runwalk_on = false;
  }
}

static void xor_runwalk_circles()
{
  /* there's a pref to draw run/walk circles.  it's not entirely working. */
  if (my_prefs.overlay_circles) {
    static Short center_x = 80, center_y = 80;

    if (!my_prefs.relative_move) {
      center_x = SCR_WIDTH/2;
      center_y = SCR_HEIGHT/2;
    } else {
      if (!xored_runwalk_on) {
	Short vcheat = stats_bar_on_bottom ? 0 : DunTopY;
	Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
	Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
	// Center it around the '@'.  This should match relativize_move.
	center_x = vc_w/2 + (char_col - visible_x) * vc_w;
	center_y = vc_h/2 + (char_row - visible_y) * vc_h + vcheat;
      }
    }

    draw_circle(my_prefs.walk_center_border * WalkMagic, center_x, center_y);
    if (my_prefs.run)
      draw_circle(my_prefs.run_walk_border * WalkMagic, center_x, center_y);
  }
}


/*
 * Given input SCREEN coordinates x,y, with center of screen at 0,0,
 * Translate x,y such that the onscreen location of '@' is mapped to 0,0
 */
void relativize_move(Short *x, Short *y)
{
  Short center_x, center_y, vcheat = stats_bar_on_bottom ? 0 : DunTopY;
  Short vc_w = itsy_on ? visible_char_w_itsy : visible_char_w;
  Short vc_h = itsy_on ? visible_char_h_itsy : visible_char_h;
  center_x = vc_w/2 + (char_col - visible_x) * vc_w;
  center_y = vc_h/2 + (char_row - visible_y) * vc_h + vcheat;
  *x = (*x + 80) - center_x;
  *y = (*y + 80) - center_y;
}


// y,x are ABSOLUTE row,col.
void change_visible_xy(Short x, Short y)
{
  y = max(y, 0);
  x = max(x, 0);
  y = min(y, cur_height - (itsy_on ? visible_h_itsy : visible_h));
  x = min(x, cur_width - (itsy_on ? visible_w_itsy : visible_w));
  if (visible_y == y && visible_x == x) return;  // no change!
  visible_y = y;
  visible_x = x;
  recalc_and_refresh(); //  recalc_screen(); refresh(true);

  // this is like panel_bounds()
  panel_row_min = visible_y;
  panel_row_max = visible_y + (itsy_on ? visible_h_itsy : visible_h) - 1;
  panel_col_min = visible_x;
  panel_col_max = visible_x + (itsy_on ? visible_w_itsy : visible_w) - 1;

}
// x and y can be -1, 0, 1.
static Short dirx[9] = { -1,  0,  1, -1,  0,  1, -1,  0,  1 };
static Short diry[9] = {  1,  1,  1,  0,  0,  0, -1, -1, -1 };
void slide_dir(Short dir)
{
  Short x, y;
  dir = max(min(dir,9),1); // keep in bounds [1,9]
  x = dirx[dir-1];
  y = diry[dir-1];
  y *= (itsy_on ? visible_h_itsy : visible_h);
  x *= (itsy_on ? visible_w_itsy : visible_w);
  change_visible_xy(x+visible_x,y+visible_y);
}


Boolean get_panel(Short y, Short x, Boolean force)
{
  Short screen_width = itsy_on ? visible_w_itsy : visible_w; // unit = cells
  Short screen_height = itsy_on ? visible_h_itsy : visible_h; // unit = cells
  Short epsilon_x = 3, epsilon_y = 2; // # of border cells you "can't get to"
  Short new_visible_x, new_visible_y;
  Boolean in_new_panel = true;
  // cur_height and cur_width are the # of cells in CURRENT floor's dimensions
  if (!(force ||
	(x < visible_x + epsilon_x) ||
	(x >= visible_x + screen_width - epsilon_x) ||
	(y < visible_y + epsilon_y) ||
	(y >= visible_y + screen_height - epsilon_y)))
    return false; // no need to change panels

  // CALCULATE X   of top left corner of the panel to move to
  // This is a little ugly in order to deal with possibly odd screen_width.
  new_visible_x = ((x - epsilon_x)/screen_width) * screen_width;
  panel_col = ((x - epsilon_x)/screen_width) * 2; // panels overlap by width/2
  if ((x - epsilon_x) % screen_width >= screen_width/2) {
    new_visible_x += screen_width/2;
    panel_col++;
  }
  // Boundary conditions.
  if (new_visible_x > cur_width - screen_width)
    new_visible_x = cur_width - screen_width;
  if (new_visible_x < 0)
    new_visible_x = 0;
  // CALCULATE Y   of top left corner of the panel to move to
  // This is a little ugly in order to deal with possibly odd screen_width.
  new_visible_y = ((y - epsilon_y)/screen_height) * screen_height;
  panel_row = ((y - epsilon_y)/screen_height) * 2; // panels overlap by h/2
  if ((y - epsilon_y) % screen_height >= screen_height/2) {
    new_visible_y += screen_height/2;
    panel_row++;
  }
  // Boundary conditions.
  if (new_visible_y > cur_height - screen_height)
    new_visible_y = cur_height - screen_height;
  if (new_visible_y < 0)
    new_visible_y = 0;
  
  if (visible_x == new_visible_x && visible_y == new_visible_y)
    in_new_panel = false; // Ha!  There was no change.  Silly rabbit.

  visible_x = new_visible_x;
  visible_y = new_visible_y;
  panel_row_min = visible_y;
  panel_row_max = visible_y + screen_height - 1;
  panel_col_min = visible_x;
  panel_col_max = visible_x + screen_width - 1;

  if (in_new_panel)
    if (find_bound)
      end_find(); // stop running, etc.

  // Caller decides whether to recalc_screen & refresh.

  return in_new_panel;
}


/*
 * Set the buffer value to what it "should" be "now"
 * but it will get actually drawn when the screen is refreshed
 */
//void update_screen_cell(Short j, Short i, struct everything *env)


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
// Here is the code formerly known as stats.c.
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*
  S00 I00 W00 D00 C00 Ch00  0000 feet
  L 00 xp 0123456 mana 000 hp 000/000
  AC 00 Gold ...
*/

// HEY - revise this so that it just queues up the messages,
// and have another routine print them at the end of the turn
// (with --more-- as necessary)
void count_message(Char *buf)
{
  Short i;
  i = command_count;
  message(buf);
  command_count = i;
}
void level_message(Char *buf)
{
  RectangleType r;
  Short y;
  RctSetRectangle(&r, 0, SCR_HEIGHT/2 - visible_char_h,
		  SCR_WIDTH, 2*visible_char_h);
  qWinEraseRectangle(&r, 0); /* 0 for square corners */
  y = SCR_HEIGHT/2 - visible_char_h;
  qWinDrawLine(0, y, SCR_WIDTH, y);
  y = SCR_HEIGHT/2 + visible_char_h;
  qWinDrawLine(0, y, SCR_WIDTH, y);
  // Perhaps this should be centered.
  qWinDrawChars(buf, StrLen(buf), 1, (SCR_HEIGHT - visible_char_h)/2);
  message(buf); // for good measure
}
Short brief_comment_y = 128;
void show_brief_comment(Char *buf) // used in store2.c, spell, inv, equ.
{
  clear_brief_comment();
  if (!buf) return;
  WinDrawChars(buf, StrLen(buf), 2, brief_comment_y); // xxx handera
}
Boolean in_brief_comment(Short x, Short y)
{ return (y > brief_comment_y && y < brief_comment_y+LINE_HEIGHT); }
void clear_brief_comment()
{
  RectangleType r;// was 0,128,156,11
  RctSetRectangle(&r, 0, brief_comment_y, SCR_WIDTH-4, LINE_HEIGHT);
  WinEraseRectangle(&r, 0);       // clear the message space
}

Boolean in_beaming = false;
void message(Char *buf)
{
  Word curfrm = FrmGetActiveFormID();
  Short i;
  Char *tmp;
  if (curfrm == MainForm ||
      curfrm == CharSummaryForm ||
      in_beaming) {    // Log a message for later display
    if (!buf) return;
    tmp = old_messages[0];
    for (i = 0 ; i < SAVED_MSGS-1 ; i++)
      old_messages[i] = old_messages[i+1];
    StrNCopy(tmp, buf, SAVED_MSG_LEN-2); /* leave room for \n.. */
    tmp[SAVED_MSG_LEN-2] = '\0'; /* making sure it's terminated.. */
    StrCat(tmp, "\n");
    old_messages[SAVED_MSGS-1] = tmp;
    // this part is for "--more--" ability:
    if ((command_count <= 0) ||
	(0 != StrCompare(old_messages[SAVED_MSGS-1],
			 old_messages[SAVED_MSGS-2])))
      // don't --more-- a count_message if it's the same as the previous one
      last_old_message_shown--;
    if (last_old_message_shown < 0) // "none of these msgs have been shown"
      last_old_message_shown = -1;
    // Remember that we have something to display
    pending_messages = true;
    command_count = 0;  // (lame hack!)
  } else if (curfrm == InvForm || curfrm == EquForm) {
    // These messages should be printed straightaway.
    // well, really I should be logging them, too.  in most cases.
    show_brief_comment(buf);
    // Make sure there's at least a moment to read it, if we have
    // several messages to print ("You wear x" "Oops, cursed!" "The foo hits")
    // SysTaskDelay(SysTicksPerSecond());
  }
}
// a mainform message that we require to be displayed in "real time"
void alert_message(Char *buf)
{
  RectangleType r;
  if (FrmGetActiveFormID() != MainForm || !buf) return;
  //  RctSetRectangle(&r,0,134,160,160-134); /* left,top, width and height */
  //RctSetRectangle(&r,0,StatsTopY,160,StatsBotY-StatsTopY);/* x,y, W, and H */
  RctSetRectangle(&r, 0, StatsTopY, SCR_WIDTH, LINE_HEIGHT+3); // was +4 xxx
  qWinEraseRectangle(&r, 0); /* 0 for square corners */
  qWinDrawChars(buf, StrLen(buf), 1, StatsTopY);
}
// Clear the message area
Boolean message_clear(Boolean really)
{
  RectangleType r;
  //  RctSetRectangle(&r,0,134,160,160-134); /* left,top, width and height */
  RctSetRectangle(&r, 0, StatsTopY, SCR_WIDTH, StatsBotY-StatsTopY);
  if (really)
    qWinEraseRectangle(&r, 0);
  qWinDrawLine(0, DunLineY, SCR_WIDTH, DunLineY);
  //print_stats(STATS_QUO);
  if (last_old_message_shown < SAVED_MSGS-1)  return true;
  else return false;
}

/*
 * Blow away the message queue
 * (Use this when you die)
 */
void preempt_messages()
{
  pending_messages = false;
  last_old_message_shown = SAVED_MSGS - 1;  
}

// Print up to 2 lines of messages.  make sure you word-wrap.
// 1. print first message
// 2. if necessary, word wrap
// 3. else if second message exists + does not need word wrap, print it too.
// 4. else if second message does not exist, print teensy stats.
// 5. print "--more--" if any more messages are in the queue
void show_messages()
{
  Short lines_avail = 2, len_line, len_str, lines_used = 0;
  Short str_width = SCR_WIDTH;
  Char *msg;

  RectangleType r;
  //RctSetRectangle(&r,0,134,160,160-134); /* left,top, width and height */
  RctSetRectangle(&r, 0, StatsTopY, SCR_WIDTH, StatsBotY-StatsTopY);

  // if there's nothing to print, just print stats.
  if (last_old_message_shown >= SAVED_MSGS-1) {
    //    need_print_stats &= ~STATS_WORD;
    print_stats_real(need_print_stats, true);
    //  need_print_stats = STATS_NONE; // print_stats_real does this
  } else {
    qWinEraseRectangle(&r, 0);
    qWinDrawLine(0, DunLineY, SCR_WIDTH, DunLineY);
    while (lines_avail > 0 && last_old_message_shown < SAVED_MSGS-1) {
      msg = old_messages[last_old_message_shown+1];
      len_str = StrLen(msg);
      if (lines_avail <= 1 && FntCharsWidth(msg, len_str-1) > SCR_WIDTH)
	break; // 1 line avail., message is 2 lines.  Do not print partial msg.
      while (lines_avail > 0) {
	len_str = StrLen(msg);
	len_line = FntWordWrap(msg, SCR_WIDTH);
	if (len_line >= len_str) { // we have finished the string!
	  //	  WinDrawChars(msg, len_str-1, 0, 134 + 11*lines_used);
	  qWinDrawChars(msg, len_str-1, 0, StatsTopY + LINE_HEIGHT*lines_used);
	  str_width = FntCharsWidth(msg, len_str-1); // remember for print..hp!
	  lines_avail--; lines_used++;
	  break;
	}
	//	WinDrawChars(msg, len_line, 0, 134 + 11*lines_used);
	qWinDrawChars(msg, len_line, 0, StatsTopY + LINE_HEIGHT*lines_used);
	str_width = FntCharsWidth(msg, len_line); // remember for print_s._hp!
	lines_avail--; lines_used++;
	msg += len_line;
      }    // we finished the string, or we ran out of space.
      last_old_message_shown++;
    } // finished all messages or ran out of space.
    if ((last_old_message_shown < SAVED_MSGS-1) ||
	(curr_state.mode == MODE_SHOWINVIS) || death)
      show_a_more(lines_used); // prompt for the Any Key with "--more--"
    else {
      if (lines_used > 1)	// see if there was room for hit points
	print_stats_hp(str_width); // this will decide whether to print them
    }
    if (lines_used < 2) {
      //need_print_stats &= ~STATS_WORD; // hey, why is this line here?
      // probably to prevent flickering during _combat_...
      print_stats_real(need_print_stats, false);
      // this should turn on CURSTAT_ITSY in current_print_stats.
    } else
      current_print_stats = CURSTAT_MSG;
  }
  pending_messages = false;
  //  need_print_stats = STATS_NONE; // print_stats_real does this
}
// This is what prints the MORE in lower right corner of screen.
// A couple non-display.c routines also would like to use it..
static const Char more[6] = "MORE ";
void show_a_more(Short lines_used)
{
  DWord version;
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
#ifdef I_AM_OS_2
  if (version < 0x03000000l) FntSetFont(ledFont);
#else
  if (version >= 0x03000000l) FntSetFont(SmallFont);
#endif
  //    WinDrawChars("--more--", 8, 120, 134 + 11*lines_used);
  //WinDrawChars("MORE", 8, (large ? 130 : 140), 134 + 11*lines_used - 3);
  qWinDrawChars(more, 4, SCR_WIDTH - FntCharsWidth(more, 5),
		StatsTopY + LINE_HEIGHT*lines_used - (FntCharHeight()-3));
#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(BigFont);
#endif
}

// call alloc_message_log initially when all stuff is being allocated..
// it has to be dynamic so we can play games with pointers in log_message.
void alloc_message_log()
{
  Short i;
  for (i = 0 ; i < SAVED_MSGS ; i++) {
    old_messages[i] = (Char *) md_malloc(sizeof(Char) * SAVED_MSG_LEN);
    old_messages[i][0] = '\0';
  }
  // the log is all null strings.. so (we pretend) all of it has been shown
  last_old_message_shown = SAVED_MSGS - 1;
}
void clear_message_log()
{
  Short i;
  for (i = 0 ; i < SAVED_MSGS ; i++)
    old_messages[i][0] = '\0';
  // the log is all null strings.. so (we pretend) all of it has been shown
  last_old_message_shown = SAVED_MSGS - 1;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
static Boolean verbing() SEC_6;
Boolean unverbing = false; // secret switch to turn off Words (once).
static Boolean verbing()
{
  if (unverbing) return (unverbing = false);
  return ((pyflags.paralysis > 1) ||
	  (PY_REST & pyflags.status) ||
	  (command_count > 0) ||
	  (PY_SEARCH & pyflags.status));
}

// See prt_stat_block() in misc3.c .. there are other things I should print
void print_stats(UInt which_stats)
{
  need_print_stats |= which_stats;
  //  if (which_stats & STATS_WORD) words_dirty = true;
  /*
  if (which_stats & (STATS_TOGGLE | STATS_GOLD_AC | STATS_STR_ETC))
    stats_top_dirty = true;
  if (which_stats & STATS_HP)
    stats_bot_dirty = true;
  */
  // what if STATS_QUO ?
  return; // let's see if this sorta works.
}

static void print_stats_real(UInt which_stats, Boolean large)
{
  /*  Short voteStrInt = 0, voteGoldAc = 0; */
  Word this_form = FrmGetActiveFormID();
  if (this_form != MainForm) return;

  if ((which_stats & STATS_WORD) || verbing()) {
    update_status_words(); // refresh the top 2 lines, then (re)print words 
  }
  which_stats &= ~STATS_WORD;
  if (which_stats & STATS_STR) {
    // eliminate the mutually incompatible...
    if (which_stats & STATS_AC) which_stats &= ~STATS_AC;
    if (which_stats & STATS_GOLD) which_stats &= ~STATS_AC;
  }
  if (large) {
    if (current_print_stats & (CURSTAT_ITSY | CURSTAT_MSG)) {
      // ... we need to refresh everything, into the same state it just was
      if ((which_stats & (STATS_AC | STATS_GOLD)) ||
	  (!(which_stats & STATS_STR) &&
	   (current_print_stats & CURSTAT_NOT_STR)))
	which_stats = STATS_ALLG;
      else
	which_stats = STATS_ALL;
    } else if (!(current_print_stats & CURSTAT_NOT_STR)) {
      if ((which_stats & STATS_AC) || (which_stats & STATS_GOLD))
	which_stats |= (STATS_AC | STATS_GOLD);
    }
    print_stats_large(which_stats);
    // remember what state we left the stats in...
    current_print_stats &= ~(CURSTAT_ITSY|CURSTAT_MSG);
    if (which_stats & STATS_STR)
      current_print_stats &= ~CURSTAT_NOT_STR;
    else if (which_stats & (STATS_AC | STATS_GOLD))
      current_print_stats |= CURSTAT_NOT_STR;      
    // else leave it like it is...
  } else {
    // itsy should always print EVERYTHING, but which_stat and
    // current_print_stats will determine which top line it prints.
    // Well, really it should NOT always print everything, because
    // when you are in combat, that is flickery too.
    // The only time it should print not-everything is if
    // current_print_stats has ITSY on and MSG off
    print_stats_itsy(which_stats);
    // remember what state we left the stats in...
    current_print_stats &= ~CURSTAT_MSG;
    current_print_stats |= CURSTAT_ITSY;
    if (which_stats & STATS_STR)
      current_print_stats &= ~CURSTAT_NOT_STR;
    else if (which_stats & (STATS_AC | STATS_GOLD))
      current_print_stats |= CURSTAT_NOT_STR;
    // else leave it like it is...
  }
  need_print_stats = STATS_NONE;
}

/*
 * Print two lines of status at the bottom of screen
 */
/*
  How many separate things are there to print:
  stats, pac, gold, depth, level, exp, cmana, mhp, chp
*/
Short statwidth_large[6] = { 5,7,8,6,5,10 };// also, width of "%2d " is 12
Short statwidth_small[6] = { 5,10,5,5,5,10 };// also, width of "%2d " is 10
const Char *stat_abbrev[6] = { "S", "In", "W", "D", "C", "Ch" };
Short statwidth_misc[8] = {1, 30, 115, 21, 21+45, 21+45+43, 160-109, 160-66};
// LEFT_X 1, GOLD_X 30, DEPTH_X 115, XP_X 21, MANA_X 21+45, HP_X 21+45+43,
// HP_AT_X 109, MANA_AT_X 66
//#define LEFT_X 1
//#define GOLD_X (6*five)/*30*/
//#define DEPTH_X (23*five)/*115*/
//#define XP_X 21
//#define MANA_X (21+45)
//#define HP_X (21+45+43)
// used in print_stats_hp _and_ print_stats_itsy ...
//#define HP_AT_X 109
//#define MANA_AT_X 66
static void print_stats_large(UInt which_stats)
{
  Char buf[80];
  Short hi, x, i, tmp, w, depth;
  Short five=FntCharWidth('0'), space=FntCharWidth(' ');
  RectangleType r;

  if (FrmGetActiveFormID() != MainForm) return;
  //  if (!stats_dirty) return;
  if (which_stats == STATS_NONE) return;

  //  RctSetRectangle(&r,0,StatsTopY,160,StatsBotY-StatsTopY);
  //  qWinEraseRectangle(&r, 0); /* 0 for square corners */

  /*
     On the first line, we will be erasing sections of rectangle
     from y0 = StatsTopY and dy = 11+4.
     On the first line, we will be erasing sections of rectangle
     from y0 = hi and dy = 11.. I think..
  */

  hi = StatsTopY + 4; // xxx 4 = magic number
  qWinDrawLine(0, DunLineY, SCR_WIDTH, DunLineY);
  if (which_stats & STATS_STR) {
    RctSetRectangle(&r,0,StatsTopY,statwidth_misc[2],(hi+LINE_HEIGHT)-StatsTopY);
    qWinEraseRectangle(&r, 0);
    x = statwidth_misc[0];
    // need to print each number at a fixed position, also
    // subtract 18 if > 18 (and print '00' if 100) and invert the display.
    //StrPrintF(buf, "S      In      W      D      C      Ch     ");//xxxx
    //qWinDrawChars(buf, StrLen(buf), x, hi);
    for (i = 0, w = x ; i < 6 ; i++, w+=2*five+space) {
      qWinDrawChars(stat_abbrev[i], StrLen(stat_abbrev[i]), w, hi);
      w += statwidth_large[i];
      tmp = pystats.use_stat[i];
      if (tmp > 18) {
	tmp -= 18;
	if (tmp > 99) tmp = 0;
	StrPrintF(buf, (tmp > 9 ? "%d" : "0%d"), tmp);
	qWinDrawInvertedChars(buf, StrLen(buf), w, hi);
      } else {
	StrPrintF(buf, "%d", tmp);
	qWinDrawChars(buf, StrLen(buf), (tmp > 9) ? w : w+five, hi);
      }
    }
  } else {
    if (which_stats & STATS_AC) {
      RctSetRectangle(&r, 0, StatsTopY, statwidth_misc[1],
		      (hi+LINE_HEIGHT)-StatsTopY);
      qWinEraseRectangle(&r, 0);
      StrPrintF(buf, "AC %d", pymisc.dis_ac);
      qWinDrawChars(buf, StrLen(buf), statwidth_misc[0], hi);
    }
    if (which_stats & STATS_GOLD) {
      RctSetRectangle(&r, statwidth_misc[1], StatsTopY,
		      statwidth_misc[2]-statwidth_misc[1],
		      (hi+LINE_HEIGHT)-StatsTopY);
      qWinEraseRectangle(&r, 0);
      StrPrintF(buf, "$ %ld", pymisc.au);
      // allow space for "AC 999  " ...
      qWinDrawChars(buf, StrLen(buf), statwidth_misc[1], hi);
    }
  }

  if (which_stats & STATS_DEPTH) {
    RctSetRectangle(&r, statwidth_misc[2], StatsTopY,
		    SCR_WIDTH-statwidth_misc[2], (hi+LINE_HEIGHT)-StatsTopY);
    qWinEraseRectangle(&r, 0);
    depth = dun_level * 50;
    if (depth > 0)
      StrPrintF(buf, "%d feet", depth);
    else 
      StrPrintF(buf, "Town");
    qWinDrawChars(buf, StrLen(buf), statwidth_misc[2], hi);
  }
  // done with first line
  hi += LINE_HEIGHT;

  if (which_stats & STATS_XP) {
    RctSetRectangle(&r, 0, hi, statwidth_misc[4], StatsBotY-hi);
    qWinEraseRectangle(&r, 0); /* 0 for square corners */
    // player level 12
    StrPrintF(buf, "L %d", (Short) pymisc.lev);
    qWinDrawChars(buf, StrLen(buf), statwidth_misc[0], hi);
    // player experience 1234567
    StrPrintF(buf, "x %ld", pymisc.exp);
    qWinDrawChars(buf, StrLen(buf), statwidth_misc[3], hi);
  }
  if (which_stats & STATS_MANA) {
    RctSetRectangle(&r, statwidth_misc[4], hi,
		    statwidth_misc[5]-statwidth_misc[4], StatsBotY-hi);
    qWinEraseRectangle(&r, 0); /* 0 for square corners */
    // mana points 123
    StrPrintF(buf, "mana %d", pymisc.cmana);
    qWinDrawChars(buf, StrLen(buf), statwidth_misc[4], hi);
  }
  if (which_stats & STATS_HP) {
    RctSetRectangle(&r, statwidth_misc[5], hi, SCR_WIDTH-statwidth_misc[5],
		    StatsBotY-hi);
    qWinEraseRectangle(&r, 0); /* 0 for square corners */
    // hit points 123/123
    //  StrPrintF(buf, "hp %d/%d", pymisc.mhp, pymisc.chp);
    x = statwidth_misc[5];
    if ((pymisc.cmana <= 99) && (pymisc.mhp > 99) && (StatsBotY == SCR_HEIGHT))
      x -= five; // Keeps the Graffiti indicator from overwriting last digit
    StrPrintF(buf, "hp %d/%d", pymisc.chp, pymisc.mhp);
    if (pymisc.chp < pymisc.mhp / 4) // might want to make this tweakable
      qWinDrawInvertedChars(buf, StrLen(buf), x, hi);
    else
      qWinDrawChars(buf, StrLen(buf), x, hi);
  }
  return;
}

static void print_stats_itsy(UInt which_stats)
{
  Char buf[80];
  Short hi, x, depth;
  Short i, w, tmp; //, five = visible_char_w_itsy, space = 2;
  Short five, space;
  RectangleType r;

  if (FrmGetActiveFormID() != MainForm) return;

#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  FntSetFont(SmallFont);
#endif
  five = FntCharWidth('0');
  space = FntCharWidth(' ');
  
  //  hi = 160-12;
  hi = StatsBotY - 2*visible_char_h_itsy;

  x = statwidth_misc[0];
  if (which_stats & STATS_STR) {
    //    StrPrintF(buf, "S      In      W      D      C      Ch     ");//xxxx
    //    qWinDrawChars(buf, StrLen(buf), x, hi);
    RctSetRectangle(&r, 0, hi, statwidth_misc[2], FntCharHeight()); // xxx
    qWinEraseRectangle(&r, 0);
    for (i = 0, w = x ; i < 6 ; i++, w+=2*five+space) {
      qWinDrawChars(stat_abbrev[i], StrLen(stat_abbrev[i]), w, hi);
      w += statwidth_small[i];
      tmp = pystats.use_stat[i];
      if (tmp > 18) {
	tmp -= 18;
	if (tmp > 99) tmp = 0;
	StrPrintF(buf, (tmp > 9 ? "%d" : "0%d"), tmp);
	qWinDrawInvertedChars(buf, StrLen(buf), w, hi);
      } else {
	StrPrintF(buf, "%d", tmp);
	qWinDrawChars(buf, StrLen(buf), (tmp > 9) ? w : w+five, hi);
      }
    }
  } else {
    StrPrintF(buf, "AC %d  $ %ld", pymisc.dis_ac, pymisc.au);
    qWinDrawChars(buf, StrLen(buf), x, hi);
  }

  x += statwidth_misc[2]; //115; a random magic number... same as large stats
  depth = dun_level * 50;
  if (depth > 0)
    StrPrintF(buf, "%d feet", depth);
  else 
    StrPrintF(buf, "Town");
  qWinDrawChars(buf, StrLen(buf), x, hi);
  // done with first line!

  x = statwidth_misc[0];
  hi = StatsBotY - visible_char_h_itsy;// hi = 160-6;

  // player level 12
  StrPrintF(buf, "L %d", (Short) pymisc.lev);
  qWinDrawChars(buf, StrLen(buf), x, hi);
  // player experience 1234567
  x += 4*five; // xxxx still works on Handera!  width(L 00 ) = 5+2+5+5+2 < 20.
  StrPrintF(buf, "/ %ld", pymisc.exp);
  qWinDrawChars(buf, StrLen(buf), x, hi);
  // mana points 123
  x = SCR_WIDTH - statwidth_misc[7]; // shove mana + hp as far right as poss.
  StrPrintF(buf, "Mana %d", pymisc.cmana);
  qWinDrawChars(buf, StrLen(buf), x, hi);
  // hit points 123/123
  x = SCR_WIDTH - statwidth_misc[6]; // shove mana + hp as far right as poss.
  //  StrPrintF(buf, "HP %d/%d", pymisc.mhp, pymisc.chp);
  if ((pymisc.cmana <= 99) && (pymisc.mhp > 99) && (StatsBotY == SCR_WIDTH))
    x -= five; // Keep the Graffiti indicator from overwriting last digit
  StrPrintF(buf, "HP %d/%d", pymisc.chp, pymisc.mhp);
  if (pymisc.chp < pymisc.mhp / 4) // might want to make this tweakable
    qWinDrawInvertedChars(buf, StrLen(buf), x, hi);
  else
    qWinDrawChars(buf, StrLen(buf), x, hi);

  // this is a test
  //  try_print_status();

#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(BigFont);
#endif

  //  stats_dirty = true;
  return;
}

// This duplicates part of the itsy-stats functionality.  "plugh"
// It will print mana if possible, and as much of hp as it can
// (at the very least it will print the current hp number).
// Inefficient (but slowness will not matter because we only do this
// once and then wait for user action) and/or inelegant at present.
static void print_stats_hp(Short width_used)
{
  Short x, w, len, hi = StatsBotY - visible_char_h_itsy; //160-6;
  Boolean low, mana = false;
  Char buf[20];
  Short five=FntCharWidth('0'), space=FntCharWidth(' ');
  if (FrmGetActiveFormID() != MainForm) return;
  //if (width_used > HP_AT_X - 2) return;
#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  FntSetFont(SmallFont);
#endif

  low = pymisc.chp < pymisc.mhp / 4; // might want to make this tweakable

  if (width_used <= (SCR_WIDTH - statwidth_misc[7]) - space) {
    StrPrintF(buf, "Mana %d", pymisc.cmana);
    qWinDrawChars(buf, StrLen(buf), SCR_WIDTH - statwidth_misc[7], hi);
    mana = true;
  }
  StrPrintF(buf, "HP %d/%d", pymisc.chp, pymisc.mhp);
  len = StrLen(buf);
  w = FntCharsWidth(buf, len);
  // If possible print at usual position, otherwise try: full string right-jus;
  // full string partly stacked; just numbers stacked.
  if (width_used < (SCR_WIDTH - statwidth_misc[6]) - space) {
    x = SCR_WIDTH - statwidth_misc[6];
    if (pymisc.mhp > 99)  // if there's room, scoot it over a bit:
      if (width_used < ((SCR_WIDTH - statwidth_misc[6]) - five) - space)
	if (!mana || (pymisc.cmana <= 99)) // can we crowd into mana's space
	  x -= five;
  } else if (width_used < SCR_WIDTH-(w+space))
    x = SCR_WIDTH - w; // print full string but right justified
  else {
    StrPrintF(buf, "%d/%d", pymisc.chp, pymisc.mhp);
    len = StrLen(buf);
    w = FntCharsWidth(buf, len);
    if (width_used < SCR_WIDTH-(w+space)) {
      x = SCR_WIDTH - w; // print both numbers but stack 'HP' above
      qWinDrawChars("HP", 2, x, hi);
    } else {
      StrPrintF(buf, "%d", pymisc.chp);  // stack cur above max. no 'HP'.
      len = StrLen(buf);
      x = SCR_WIDTH - 5*len;//==160-FntCharsWidth(buf, len);
      if (low)
	qWinDrawInvertedChars(buf, len, x, hi-visible_char_h_itsy);
      else
	qWinDrawChars(buf, len, x, hi-visible_char_h_itsy);
      StrPrintF(buf, "%d", pymisc.mhp);
      len = StrLen(buf);
      x = SCR_WIDTH - FntCharsWidth(buf, len);
    }
  }
  if (low)
    qWinDrawInvertedChars(buf, len, x, hi);
  else
    qWinDrawChars(buf, len, x, hi);


#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(BigFont);
#endif
}

// Currently this is called by refresh()
// and currently it prints at the very top of the screen.
// these are the one-word status states that were on the bottom line in moria.
// Note:
// If something other than 'refresh' calls this, it would be a good
// idea to re-print the top 2 lines of the dungeon to 'clean up' FIRST.
// in case there's a word we are no longer printing.
// (actually it could set those 2 lines to 'dirty' and refresh())
void update_status_words()
{
  try_print_status();
  refresh_2();
}
void try_print_status()
{
  Char buf[80];
  Short hi, len, x, i, maxlen=SCR_WIDTH;//maxlen=120;
  Boolean display_counts = true;


  if (FrmGetActiveFormID() != MainForm) return;

#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  FntSetFont(SmallFont);
#endif
  // Test of stats flags:
  // PoisonedSearchingVslowStudy
  // HungryBlindConfusedAfraid
  //  hi = 0;
  hi = DunTopY;
  x = 0;
  words_dirty_x[0] = words_dirty_x[1] = 0;
  // hunger
  if (PY_WEAK & pyflags.status) {
    StrPrintF(buf, "Weak"); // 20
    len = 4;
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
  } else if (PY_HUNGRY & pyflags.status) {
    StrPrintF(buf, "Hungry"); // 30
    len = 6;
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
  }
  // blindness
  if (PY_BLIND & pyflags.status) {
    StrPrintF(buf, "Blind"); // 25
    len = 5;
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
  }
  if (PY_CONFUSED & pyflags.status) {
    StrPrintF(buf, "Confused"); // 40
    len = 8;
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
    if (x > maxlen) { words_dirty_x[0] = x; x = 0; hi += visible_char_h_itsy; }
  }
  if (PY_FEAR & pyflags.status) {
    StrPrintF(buf, "Afraid"); // 30
    len = 6;
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
    if (x > maxlen) { words_dirty_x[0] = x; x = 0; hi += visible_char_h_itsy; }
  }
  if (PY_POISONED & pyflags.status) {
    StrPrintF(buf, "Poisoned"); // 40
    len = 8;
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
    if (x > maxlen) { words_dirty_x[0] = x; x = 0; hi += visible_char_h_itsy; }
  }

  // this needs to print Searching, Resting, Paralysis, or 'count' status :
  buf[0] = '\0';
  if (pyflags.paralysis > 1)
    StrPrintF(buf, "Paralysed"); // 45
  else if (PY_REST & pyflags.status) {
    if (pyflags.rest < 0)
      StrPrintF(buf, "Rest *");
    else if (display_counts)
      StrPrintF(buf, "Rest%s%d", (pyflags.rest>9?" ":" \t"), pyflags.rest);
    else
      StrPrintF(buf, "Rest");
  } else if (command_count > 0) {
    if (display_counts)
      StrPrintF(buf, "Repeat%s%d",(command_count>9?" ":" \t"),command_count);
    else
      StrPrintF(buf, "Repeat");
    // pyflags.status |= PY_REPEAT; // ?
    if (PY_SEARCH & pyflags.status)
      StrCat(buf, " Search");
  } else if (PY_SEARCH & pyflags.status)
    StrPrintF(buf, "Searching"); // 45
  if (buf[0]) {
    len = StrLen(buf);
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
    if (x > maxlen) { words_dirty_x[0] = x; x = 0; hi += visible_char_h_itsy; }
  }

  // speed
  i = pyflags.speed;
  buf[0] = '\0';
  if (PY_SEARCH & pyflags.status) i--;   /* Search mode. */
  if (i > 1)        StrPrintF(buf,"SLOW");
  else if (i == 1)  StrPrintF(buf,"Slow");
  else if (i == 0)  ;
  else if (i == -1) StrPrintF(buf,"Fast");
  else              StrPrintF(buf,"FAST");
  if (buf[0] != '\0') {
    len = StrLen(buf);
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
    if (x > maxlen) { words_dirty_x[0] = x; x = 0; hi += visible_char_h_itsy; }
  }
  // this needs fixed:
  pyflags.status &= ~PY_STUDY;
  if (pyflags.new_spells != 0) {
    StrPrintF(buf, "Study"); // 25
    len = StrLen(buf);
    WinDrawInvertedChars(buf, len, x, hi);
    x += FntCharsWidth(buf, len);
    if (x > maxlen) { words_dirty_x[0] = x; x = 0; hi += visible_char_h_itsy; }
  }

  if (hi > 0) words_dirty_x[1] = x / visible_char_w_itsy; 
  words_dirty_x[0] /= visible_char_w_itsy; 
  

#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(BigFont);
#endif

  // this may help us to tidy up later.  nope, doesn't seem to help.  hmmm.
  /*
  absline = visible_y;
  if (x > 0 || hi > 0) {
    screen_dirty = true;
    lines_dirty[absline] = true;
    if (x > 0 && hi > 0) lines_dirty[absline+1] = true;
  }
  */
}
/* The various FLAGS:
   hunger:   Weak | Hungry   23,0
   blind:    Blind           23,7
   confused: Confused        23,13
   afraid:   Afraid          23,22
   poisoned: Poisoned        23,29
   state:    Paralysed | Rest %-5d | Repeat %-3d | Search | Searching   23,38
   speed:    Very Slow | Slow | Fast | Very Fast                        23,49
   study:    Study           23,59
   depth:    Town level  was 23,65
Weak   Blind Confused Afraid Poisoned Paralysed  Very Slow Study
Hungry Blind Confused Afraid Poisoned Repeat 123 Fast      Study
 */


// Draw a circle of radius radius (pixels) without using "sqrt".
// this is a sad sad little function.  but it doesn't look all that bad really.
// I've replaced all the "WinDrawLine" with "WinInvertLine" (sigh)
//   to make 'invert mode' hopefully happy.
// Ok... this is used ONLY by the xor_runwalk_circles...
static void my_WinInvertLine(Short x, Short y) SEC_5;
static void draw_circle(Short radius, Short x0, Short y0)
{
  Short x, y, gap=2;
  if (radius >= min(SCR_HEIGHT,SCR_WIDTH)/2) return;

  for (x = 0, y = radius ; x <= y ; x += gap) {
    while (x*x + y*y > radius*radius)
      y--;
    // draw horizontalish parts
    my_WinInvertLine( x + x0, y + y0);
    my_WinInvertLine(-x + x0, y + y0);
    my_WinInvertLine( x + x0,-y + y0);
    my_WinInvertLine(-x + x0,-y + y0);
    // draw verticalish parts	    
    my_WinInvertLine( y + x0, x + y0);
    my_WinInvertLine(-y + x0, x + y0);
    my_WinInvertLine( y + x0,-x + y0);
    my_WinInvertLine(-y + x0,-x + y0);
  }
}
static void my_WinInvertLine(Short x, Short y)
{
  if (x >= 0 && y >= 0 && x <= SCR_WIDTH && y <= SCR_HEIGHT
      && (stats_bar_on_bottom ? (y < DunLineY) : (y > DunLineY)))
    WinInvertLine(x, y, x, y);
}

#define EMDASH 151
void show_time_and_volts()
{
  // dateStringLength = 9, timeStringLength = 9, voltage length = 14...
  Char buf[60];
  Short len;
  //
  UInt volts_times_100;
  UInt warn_at, critical_at, max_ticks;
  UChar battpercent = 0;
  SysBatteryKind battkind;
  Boolean plugged_in;

  
#ifndef I_AM_COLOR
  // XXXX hey, remember to TEST this, on the old xcopilot or something
  volts_times_100 = SysBatteryInfo(false, &warn_at, &critical_at, &max_ticks,
				   &battkind, &plugged_in);
#else
  volts_times_100 = SysBatteryInfo(false, &warn_at, &critical_at, &max_ticks,
				   &battkind, &plugged_in, &battpercent);
#endif

  seconds_to_string(TimGetSeconds(), buf, 0);
  len = StrLen(buf);
  buf[len++] = ' ';
  if (!plugged_in) {
    Short v = volts_times_100/100, vdec = volts_times_100 % 100;
    StrPrintF(buf+len, "  %c   %d%s%d V", EMDASH, v,
	      (vdec<10 ? ".0" : "."), vdec);
    if (battpercent) {
      len = StrLen(buf);
      StrPrintF(buf+len, " (%d%%)", battpercent);
    }
  } else {
    StrPrintF(buf+len, "  %c   [charging]", EMDASH);
  }
  message(buf);
}
