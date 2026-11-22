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

static void blink_me() SEC_6;
static void screen_map1() SEC_6;
static void screen_map2() SEC_6;
#ifdef I_AM_COLOR
static void map_draw_row(UChar *map, Short orow,
			 IndexedColorType *map_color) SEC_6;
#else
static void map_draw_row(UChar *map, Short orow) SEC_6;
#endif

  /* Display highest priority object in the RATIO_W by RATIO_H area */
#define RATIO_W 4
#define RATIO_H 3
// we can make ratio_w 4 by drawing the map in two sections.
// dungeon is (cells) 198 w, 66 h. --> overview map is (chars) 50 w, 22 h.

#define vc_h visible_char_h_itsy
#define vc_w visible_char_w_itsy

Boolean left_side = true;
Boolean map_mode_chars = true; // perhaps should be a pref.  nahhh.

Short me_x, me_y;

Boolean Map_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short bot_y;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    left_side = (char_col < MAX_WIDTH/2);
    me_x = me_y = -1;
    /* this must be AFTER draw form */
    map_draw();
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case penDownEvent:
    {
      Short half = SCR_WIDTH/2, ten = SCR_WIDTH/16;
      if (map_mode_chars) bot_y = (MAX_HEIGHT/RATIO_H + 1) * vc_h;
      else                bot_y = MAX_HEIGHT*2+1;
      if (e->screenY > bot_y) {
	if (e->screenX > half + ten) // was 90 for 160
	  LeaveForm();
	else if (e->screenX < half - ten) { // was 70 for 160
	  map_mode_chars = !map_mode_chars;
	  map_draw();
	} else {
	  blink_me(); // blink the @'s position
	}
      } else if ((left_side && e->screenX > half) ||
		 (!left_side && e->screenX < half)) {
	if (!IsVGA || map_mode_chars) {
	  left_side = !left_side;
	  map_draw();
	}
      }
    }
    handled = true;
    break;

    // Keyboard support.  Woo hoo.
  case keyDownEvent:
    frm = FrmGetActiveForm();
    switch(e->data.keyDown.chr) {
    case '@': case '?':
      blink_me();
      handled = true;
      break;
    case '\n': case 'x': case 'X': // "exit"
      LeaveForm();
      handled = true;
      break;
    case 'm': case 'M': // "mode"
      map_mode_chars = !map_mode_chars;
      map_draw();
      handled = true;
      break;
    case '<': case 'h': case 'H': // show other half
    case '>': case 'l': case 'L':
    case ' ':
      if (!IsVGA || map_mode_chars) {
	left_side = !left_side;
	map_draw();
      }
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}


/*  160 BY 160 CALCULATIONS
  The dungeon is, at maximum, 66 high by 198 wide.
  A 160 by 160 screen, in tiny font chars, is 160/5 wide by 160/6 high.
  that's 32 wide by 26 high.
     if I use _2_ screens wide, then it's 320/5 wide by 160/6 high.
     that's 64 wide by 26 high.  so if I use 2 screens of characters it's
     3 cells high by (just barely more than 3, round up to) 4 cells wide.
  It would be better to allow 2 (rectangle) or 4 (square) screens.
  With one character representing a 4w by 3h area, that's 50w by 22h chars.
*/
/* 240 BY 240 (or 305) CALCULATIONS
  The dungeon is, at maximum, 66 high by 198 wide.
  A 240 by 240 screen, in "small font" chars, is 240/8 wide by 240/10 high.
   that's 30 wide by 24 high... a bit less than usual.  (30*2)x24 will work.
  (A 240 by 240 screen, in flyspeck font chars, is 240/5 wide by 240/6 high.
   that's 48 wide by 40 high ... ALMOST wide enough for 50x22 on one screen.)
*/   

/* definitions used by map_draw() */
/* index into border character array */
#define TL 0	/* top left */
#define TR 1
#define BL 2
#define BR 3
#define HE 4	/* horizontal edge */
#define VE 5
#   define CH(x)	(screen_border[x])

void map_draw()
{
  Short y, y0, ten = SCR_WIDTH/16;
  Short left = SCR_WIDTH/2-ten;
  RectangleType r;  
#ifdef I_AM_COLOR
  if (IsColor) start_color_all(); // gar!
#endif //I_AM_COLOR
  RctSetRectangle(&r, 0, 0, SCR_WIDTH, SCR_HEIGHT);
  WinEraseRectangle(&r, 0);
  if (map_mode_chars) y = (MAX_HEIGHT/RATIO_H + 1) * vc_h;
  else                y = MAX_HEIGHT*2+1;
  WinDrawLine(0, y-1, SCR_WIDTH, y-1);
  y0 = SCR_HEIGHT-LINE_HEIGHT;
#ifdef I_AM_COLOR
  if (IsVGA && map_mode_chars) {
    y0 = SCR_HEIGHT - vc_h;
    FntSetFont(SmallFont);
  }
#endif //I_AM_COLOR
  WinDrawChars_ctr("Mode", 4, left/2, y0);
  WinDrawChars_ctr("@", 1, SCR_WIDTH/2, y0);
  WinDrawChars_ctr("Exit", 4, SCR_WIDTH-left/2, y0);
  RctSetRectangle(&r, left, y, SCR_WIDTH/8, SCR_HEIGHT-y);
  WinInvertRectangle(&r, 0);
  me_x = me_y = -1;
  if (map_mode_chars)
    screen_map1();
  else
    screen_map2();

#ifdef I_AM_COLOR
  if (IsColor)
    WinSetTextColor((my_prefs.black_bg) ? white : black);
#endif //I_AM_COLOR

  if (!IsVGA || map_mode_chars) {
    y0 = y-1 - FntCharHeight()/2;
    if (left_side) WinDrawChars_rj(">>", 2, SCR_WIDTH-1, y0);
    else           WinDrawChars("<<", 2, 1, y0);
  }
  
#ifdef I_AM_COLOR
  if (IsColor) stop_color();
  if (IsVGA && map_mode_chars) FntSetFont(BigFont);
#endif //I_AM_COLOR
}

static void blink_me()
{
  Short i;
  RectangleType r;  
  RctSetRectangle(&r, me_x, me_y, vc_w, vc_h);
  if (me_x == -1 || me_y == -1) return;
  for (i = 0 ; i < 6 ; i++) {
    WinInvertRectangle(&r, 0);
    SysTaskDelay(SysTicksPerSecond()/5);
    WinInvertRectangle(&r, 0);
    SysTaskDelay(SysTicksPerSecond()/5);
  }
}



#ifndef I_AM_OS_2
extern FontType *fontPtr; // was in main.c StartApplication
extern FontID MyFont;
#endif
#define OVERLAP (MAX_WIDTH / RATIO_W + 1 - visible_w_itsy)
static void screen_map1()
{
  Short i, j, row, orow, col; // , myrow, mycol = 0;
  UChar tmp, map[MAX_WIDTH / RATIO_W + 1];
#ifdef I_AM_COLOR
  IndexedColorType map_color[MAX_WIDTH / RATIO_W + 1];
#endif
  Char priority[256];

#ifdef I_AM_OS_2
  FntSetFont(ledFont);
#else
  { // Trying a Kyocera fix
    FntDefineFont(USERFONT, fontPtr);
  } // end of Kyocera fix
  FntSetFont(SmallFont);//FntSetFont(MyFont);
#endif

  for (i = 0; i < 256; i++)
    priority[i] = 0;
  priority['\t'] = -15; //this glyph is an em-space. use instead of ' '
  priority[' '] = -15;
  priority['.'] = -10;
  priority['\''] = -3;
  priority['#'] = -5;
  priority['<'] = 5;
  priority['>'] = 5;
  priority['@'] = 10;

  orow = -1;
  map[MAX_WIDTH / RATIO_W] = '\0';
  for (i = 0; i < MAX_HEIGHT; i++) {
    row = i / RATIO_H;
    if (row != orow) {
      // ** Begin Chunk (it would almost like to be an Inline Function)
      if (orow >= 0) {
#ifdef I_AM_COLOR
	map_draw_row(map, orow, map_color);
#else
	map_draw_row(map, orow);
#endif
      }
      // ** End Chunk (there is another one below)
      for (j = 0 ; j < MAX_WIDTH / RATIO_W ; j++)
	map[j] = '\t'; // use the em-width space.  // map_color doesn't matter?
      orow = row;
    }
    for (j = 0; j < MAX_WIDTH; j++) {
      col = j / RATIO_W;
      tmp = loc_symbol(i, j);
      if (tmp == ' ') tmp = '\t'; // use the em-width space.
      if (priority[map[col]] < priority[tmp]) {
	map[col] = tmp;
#ifdef I_AM_COLOR
	if (IsColor) {
	  map_color[col] = get_color(tmp, j, i); // and just hope that works eh
	}
#endif
      }
      if (map[col] == '@') { // remember location so we can blink it on request
	me_x = (left_side ? col : col-OVERLAP) * vc_w;
	me_y = orow * vc_h;//me_y = (orow+1) * vc_h;
      }
    }
  }
  // ** Begin Same Chunk
  if (orow >= 0) {
#ifdef I_AM_COLOR
    map_draw_row(map, orow, map_color);
#else
    map_draw_row(map, orow);
#endif
  }
  // ** End Same Chunk

#ifdef I_AM_OS_2
  FntSetFont(stdFont);
#else
  FntSetFont(BigFont);
#endif

}

#ifdef I_AM_COLOR
static void map_draw_row(UChar *map, Short orow,
			 IndexedColorType *map_color)
#else
static void map_draw_row(UChar *map, Short orow)
#endif
{
  Short j, len, x=0, y;
  UChar *mapptr;

  mapptr = map;
  y = orow * vc_h;
  if (y > SCR_HEIGHT-LINE_HEIGHT - vc_h) return;
  if (left_side) {
    len = visible_w_itsy;
  }
  else {
    mapptr += OVERLAP;
    len = StrLen(mapptr);
  }
#ifdef I_AM_COLOR
  if (IsColor || IsVGA) {
    for (j = 0 ; j < len ; j++) {
      if (IsColor)
	WinSetTextColor(map_color[j+(mapptr-map)]);
      if (mapptr[j] != '\t' && mapptr[j] != ' ')
	WinDrawChars(mapptr+j, 1, x+j*vc_w, y);
    }
  } else
#endif
    WinDrawChars(mapptr, len, x, y); // careful! this is "else" & "#else"
}



static void screen_map2()
{
  Short i, j; // row, col; or y, x
  Short j_Offset, j_max;
  UChar tmp;

  //  j_Offset = (left_side ? 0 : MAX_WIDTH / 2); // no fear, MAX_WIDTH is even
  //  for (j = 0; j < MAX_WIDTH/2; j++)
  if (!IsVGA)
    j_Offset = (left_side ? 0 : MAX_WIDTH - SCR_WIDTH);
  else // 198 wide by 66 high will fit on 240x240, even on 240x175.
    j_Offset = 0;
  j_max = min(SCR_WIDTH, MAX_WIDTH);

  for (j = 0; j < j_max; j++) {
    for (i = 0; i < MAX_HEIGHT; i++) {
      tmp = loc_symbol(i, j + j_Offset);
      if (tmp == '@') {
	WinDrawLine(j, i*2+1, j, i*2+1); // draw 1 measly pixel
	// also, remember location so we can blink it later.
	me_x = j-2;
	me_y = i*2-2;
	if (me_x < 0) me_x = 0;
	if (me_y < 0) me_y = 0;
	// WinDrawChars("@", 1, j, MAX_HEIGHT*2+1); // draw @ in space below
	// WinDrawChars("@", 1, j-2, i*2-2); // center the itsy character
      }
      if (tmp == '#' || tmp == '%') // wall
	WinDrawLine(j, i*2, j, i*2+1); // draw 2 measly pixels
    }
  }

}
