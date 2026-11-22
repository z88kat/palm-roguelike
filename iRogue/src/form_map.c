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

extern Short ScrWidth;
extern Short ScrHeight;
extern Short LineHeight;
extern Short WalkMagic;

Boolean Map_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
#ifndef I_AM_OS_2
    if (IsVGA) {
      VgaFormModify(frm, vgaFormModify160To240);
      adjust_form_size(false);
    }
#endif
    FrmDrawForm(frm);
    /* this must be AFTER draw form */
    show_dungeon();
    handled = true;
    break;


#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) {
      update_display_size();
      adjust_form_size(true);
      // redraw the "new" screen too...  don't ask me whether this will work
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      show_dungeon();
    }
    break;
#endif

  case penDownEvent:
    LeaveForm();
    handled = true;
    break;

  default:
    break;
  }

  return handled;
}

/*
enum patterns { blackPattern, whitePattern, grayPattern, customPattern };
typedef enum patterns PatternType;
typedef Word CustomPatternType [4];
*/
/*static CustomPatternType bombPattern = { 0x8855, 0x2255, 0x8855, 0x2255 };*/
/*static CustomPatternType graePattern = { 0xaa55, 0xaa55, 0xaa55, 0xaa55 };*/
/*static CustomPatternType diagPattern = { 0x8844, 0x2211, 0x8844, 0x2211 };*/

/**********************************************************************
                       SHOW_DUNGEON
 IN:
 sotu = various globals (dungeon, rooms, wizard, cur_level, rogue)
 OUT:
 nothing
 PURPOSE:
 The "real" dungeon is 24 wide x 80 high, and doesn't all fit on the
 (156^2 pixel) screen at once when it's "full size".  So I have a
 form to display a tiny version of the map (just rooms/mazes/tunnels,
 the rogue, and the stairs), where each square is 2 x 3 pixels.
 Maybe I could put flyspecks for monsters/objects too.  Maybe.
 **********************************************************************/
//static CustomPatternType yourPattern = { 0xaaaa, 0x5555, 0xaaaa, 0x5555 };
// The passage pattern got messed up so I moved it inside.
static const Char* map_foo[6] = {
  "You Can't Get There From Here",
  "(Tap  Anywhere  When  Done)",
  "Strange, This Doesn't Look Like ...",
  "Oops, I Was Holding It Upside Down.",
  "Here There Be Dragons",
  "(Nothing You Can't Handle, Eh?)",
};
void show_dungeon()
{
  RectangleType r;
  Word col, row, i, w, h, len;
  UShort mask;
  Short unit, hunit, map_top;
  Char c;
  /*
#ifndef I_AM_COLOR
  CustomPatternType origPattern;
  CustomPatternType yourPattern = { 0xaaaa, 0x5555, 0xaaaa, 0x5555 };
#else // I_AM_COLOR
  // XXXXX
  // const CustomPatternType yourPattern = { 0xaa, 0xaa, 0x55, 0x55,
  //					  0xaa, 0xaa, 0x55, 0x55 };
#endif // I_AM_COLOR
  */
  UShort ** dungeon = sotu->dungeon;
  room * rooms = sotu->rooms;

  RctSetRectangle(&r, 0,0, ScrWidth, ScrHeight);
  WinDrawRectangle(&r, 6);
  unit = ScrWidth/DCOLS;
  hunit = unit+1;
  // width of map will be DCOLS units.  height of map will be DROWS hunits.
  // that's 80 cols, 24 rows

  /* This is just some fluff to put in the extra space above and below. 
     I could probably think of some more-useful fluff to put there. */
  if (!IS_WIZARD)
    i = (sotu->cur_level % 2 == 1) ? 0 : 1; // alternate the fluff
  else
    i = 2;
  len = StrLen(map_foo[i]);
  w = (ScrWidth - FntCharsWidth(map_foo[i],len) )/2;
  WinDrawInvertedChars(map_foo[i], len, w, 1);
  len = StrLen(map_foo[i+1]);
  w = (ScrWidth - FntCharsWidth(map_foo[i+1],len) )/2;
  WinDrawInvertedChars(map_foo[i+1], len, w, ScrHeight-(LineHeight+2));

  /* Each tile gets UNIT pixels/col and HUNIT pixels/row. 
     for 160x16,      =2                   =3              */
  map_top = (ScrHeight-DROWS*hunit)/2;

  /* This is just some decoration. three white horizontal lines above map. */
  for (i = 2 ; i <= 4 ; i++)
    WinEraseLine(i*unit, map_top-i*unit, ScrWidth-(i+2)*unit, map_top-i*unit);
  
  RctSetRectangle(&r, 0, map_top, ScrWidth, DROWS*hunit);
  WinEraseRectangle(&r, 0);
  WinDrawRectangleFrame(simpleFrame, &r); /* simple,round,popup,boldRound */

  /* Draw the rooms of the dungeon as rectangles */
  for (i = 0 ; i < MAXROOMS ; i++) {
    if (rooms[i].is_room & R_ROOM) {
      col = rooms[i].left_col;
      row = rooms[i].top_row;
      c = peek_me(row, col); /* this is a gross hack */
      if ( IS_WIZARD || (c && c != ' ') ) {
	w = rooms[i].right_col - col;
	h = rooms[i].bottom_row - row;
	/* col * 2, row * 3 + 42, w * 2, h * 3... -1 all round. */
	RctSetRectangle(&r, col * unit + 2, row * hunit + map_top + 2, 
			    w * unit - 2,   h * hunit - 1);
	WinDrawRectangleFrame(simpleFrame, &r);
      }
    }
  }
  /* Draw the rest of the dungeon square by square */
  for (col = 0 ; col < DCOLS ; col++) {
    for (row = 0 ; row < DROWS ; row++) {
      RctSetRectangle(&r, col*unit, row*hunit + map_top, unit,hunit);
      c = peek_me(row, col); /* this is a gross hack */
      if ( IS_WIZARD || (c && c != ' ') ) {
	mask = dungeon[row][col];
	if ( (mask & STAIRS) || (mask & TUNNEL) ) {
	  /*
#ifndef I_AM_COLOR
	  WinGetPattern(origPattern);
	  WinSetPattern(yourPattern);
	  WinFillRectangle(&r, 0);
	  WinSetPattern(origPattern);
#else // I_AM_COLOR //
	  //	  WinPushDrawState();
	  //	  WinSetPattern(&yourPattern);
	  WinFillRectangle(&r, 0);
	  //	  WinPopDrawState();
#endif // I_AM_COLOR
	  */
	  // Forget it!!, just do this:
	  /*
	  WinDrawLine(col*unit + 1, row*hunit + map_top + 1,
		      col*unit + 2, row*hunit + map_top + 2);
	  WinDrawLine(col*unit + 2, row*hunit + map_top + 2,
		      col*unit + 1, row*hunit + map_top + 3);
	  */
	  WinDrawLine(col*unit, row*hunit + map_top,
		      col*unit + unit, row*hunit + map_top + hunit);
	} else if (mask & FLOOR) {
	  /* 	WinDrawRectangle(&r, 0); */
	} else if (0 && (mask & HORWALL) ) {
	  WinDrawLine(col*unit + 1, row*hunit + map_top + 1,
		      col*unit + unit+1, row*hunit + map_top + 1);
	} else if (0 && (mask & VERTWALL) ) {
	  WinDrawLine(col*unit + 1, row*hunit + map_top + 1,
		      col*unit + 1, row*hunit + map_top + hunit+1);
	} else if (mask & DOOR) {
	  if (! (mask & HIDDEN)) {
	    WinInvertRectangle(&r, 0);
	  }
	}
      }
      /* used to have "else" (peekme==' ') "erase rectangle" */
    } /* end for row */
  } /* end for col */

  /* don't forget to put the rogue somewhere */
  col = sotu->roguep->col * unit - 1;
  if (col < 0) col = 0;
  row = sotu->roguep->row * hunit + map_top - 1;
  RctSetRectangle(&r, col, row, 
		      unit+1, hunit+1);
  WinDrawRectangle(&r, 0);
  return;
}

