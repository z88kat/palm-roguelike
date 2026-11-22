/**********************************************************************
 * kMoria - Moria adapted for the PalmPilot.                          *
 * This file is                                                       *
 * copyright (C) 1985-2001 Robert A. Koeneke, Jimmey Wayne Todd,      *
 * James E. Wilson, Bridget Spitznagel. (see main.c for full credits) *
 *                                                                    *
 * This program is freeware.  You can redistribute it and/or modify   *
 * it (provided that this notice and the full credits are retained),  *
 * but it CANNOT be distributed for profit.                           *
 **********************************************************************/
#include "kMoria.h"

/*
 *  Ok, here is the deal.  This will be a real trip.
 *  We will call 'look' EVERY TIME to find "the next thing to print".
 *  When we enter the form, set the already_printed to 0.
 *  When you are in the look_see printing part, if gl_nseen == already_printed
 *  then show the message and ABORT, otherwise skip printing it!
 *  oh and also increment already_printed appropriately.
 */

Short look_message_next;    // what thing is the first message still unprinted
Short look_message_current; // what thing we're looking at
Boolean found;
Boolean look_finished;
extern Short recall_this_monster;
extern Boolean recall_list; 
struct highlight_me_s {
  Short x;
  Short y;
  Boolean valid;
} highlight_me;

static void next_puppy(FormPtr frm) SEC_6;
static void frob_look_btns(FormPtr frm) SEC_6;
static void look_message(Char *buf) SEC_6;
static void look2(Short dir) SEC_6;
static Short look_ray(Short y, Short from, Short to) SEC_6;
static Boolean look_see(Short x, Short y, Boolean *transparent) SEC_6;

Short look_see_already_printed;

Short look_form_dir;
// look is in moria4.c
Boolean Look_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    look_message_next = 0;
    look_message_current = 0;
    set_statsbar_loc(false);
    look_draw(true);
    frob_look_btns(frm);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_look_exit:
      set_statsbar_loc(my_prefs.stats_at_top);
      LeaveForm();
      handled = true;
      break;
    case btn_look_recall:
      // recall monster
      recall_list = false;
      FrmPopupForm(RecallForm);
      handled = true;
      break;
    }
    break;

  case keyDownEvent:
    {
      Word objid = -1;
      frm = FrmGetActiveForm();
      switch(e->data.keyDown.chr) {
      case pageUpChr:
      case pageDownChr:
      case '\n': case 'n': case ' ': // "next"
	objid = -1;
	if (!look_finished) next_puppy(frm);
	handled = true;
	break;
      case 'x': case 'X': // "exit"
	objid = btn_look_exit;
	break;
      case 'r': case 'R': // "recall"
	objid = btn_look_recall;
	break;
      }
      if (objid != -1) {
	hit_button_if_usable(frm, objid);
	handled = true;
      }
    }
    break;

  case penDownEvent:
    frm = FrmGetActiveForm();
    if (e->screenY < MAP_HEIGHT) {
      if (look_finished) {
	set_statsbar_loc(my_prefs.stats_at_top);
	LeaveForm();
      } else 
	next_puppy(frm);
      handled = true;
    }
    break;

  default:
    break;
  }

  return handled;
}

Boolean look_btnsHandleEvent(Short btn_num)
{
  FormPtr frm = FrmGetActiveForm();
  // pageup/pagedown does "next".
  // app buttons exit.  calc/find do recall.
  switch(btn_num) {
  case HW_hard1Chr: case HW_hard2Chr:
    hit_button_if_usable(frm, btn_look_recall);
    return true;
  case HW_hard3Chr: case HW_hard4Chr:
    hit_button_if_usable(frm, btn_look_exit);
    return true;
  case HW_pageUpChr: case HW_pageDownChr:
    if (!look_finished) next_puppy(frm);
    return true;
  default:
    return false;
  }
}

void look_draw(Boolean first)
{
  recalc_and_refresh();
  WinDrawLine(0, MAP_HEIGHT, SCR_WIDTH, MAP_HEIGHT);
  WinDrawChars("Look", 4, 0, 0);
  if (first) look2(look_form_dir);
  if (highlight_me.valid)    // highlight the dude
    animate_char(highlight_me.y, highlight_me.x, 0, true);
}

static void next_puppy(FormPtr frm)
{
  if (highlight_me.valid)      // unhighlight the previous dude
    animate_char(highlight_me.y, highlight_me.x, 0, false);
  look_message_current = 0;
  look2(look_form_dir);
  if (highlight_me.valid)      // highlight the dude
    animate_char(highlight_me.y, highlight_me.x, 0, true);
  frob_look_btns(frm);
}

static void frob_look_btns(FormPtr frm)
{
  ControlPtr btn;
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_look_exit));
  CtlShowControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_look_recall));
  if (recall_this_monster != -1)
    CtlShowControl(btn);
  else
    CtlHideControl(btn);
}

/*
  we see 0.  we print 0.  set message_next to 0+1.  abort.
  we see 0.
  we see 1.  we print 1.  set message_next to 1+1.  abort.
  we see 0.
  we see 1.
  we see 2.  we print 2.  set message_next to 2+1.  abort.
*/
static void look_message(Char *buf)
{
  RectangleType r;
  Short len_str, len_line, y = MAP_HEIGHT+1;
  if (!found && (look_message_current == look_message_next)) {
    RctSetRectangle(&r, 0, y, SCR_WIDTH, 2*LINE_HEIGHT);
    WinEraseRectangle(&r, 0); /* 0 for square corners */
    
    len_str = StrLen(buf);
    len_line = FntWordWrap(buf, SCR_WIDTH);
    WinDrawChars(buf, len_line, 0, y);
    if (len_line < len_str) // we have not finished the string!
      WinDrawChars(buf+len_line, StrLen(buf+len_line), 0, y + LINE_HEIGHT);

    look_message_next = look_message_current + 1;
    found = true;
  }
  look_message_current++;
}

/* An enhanced look, with peripheral vision. Looking all 8	-CJS-
   directions will see everything which ought to be visible. Can
   specify direction 5, which looks in all directions.

   For the purpose of hindering vision, each place is regarded as
   a diamond just touching its four immediate neighbours. A
   diamond is opaque if it is a wall, or shut door, or something
   like that. A place is visible if any part of its diamond is
   visible: i.e. there is a line from the view point to part of
   the diamond which does not pass through any opaque diamonds.

   Consider the following situation:

     @....			    X	X   X	X   X
     .##..			   / \ / \ / \ / \ / \
     .....			  X @ X . X . X 1 X . X
				   \ / \ / \ / \ / \ /
				    X	X   X	X   X
	   Expanded view, with	   / \ / \ / \ / \ / \
	   diamonds inscribed	  X . X # X # X 2 X . X
	   about each point,	   \ / \ / \ / \ / \ /
	   and some locations	    X	X   X	X   X
	   numbered.		   / \ / \ / \ / \ / \
				  X . X . X . X 3 X 4 X
				   \ / \ / \ / \ / \ /
				    X	X   X	X   X
	- Location 1 is fully visible.
	- Location 2 is visible, even though partially obscured.
	- Location 3 is invisible, but if either # were
	  transparent, it would be visible.
	- Location 4 is completely obscured by a single #.

   The function which does the work is look_ray. It sets up its
   own co-ordinate frame (global variables map back to the
   dungeon frame) and looks for everything between two angles
   specified from a central line. It is recursive, and each call
   looks at stuff visible along a line parallel to the center
   line, and a set distance away from it. A diagonal look uses
   more extreme peripheral vision from the closest horizontal and
   vertical directions; horizontal or vertical looks take a call
   for each side of the central line. */

/* Globally accessed variables: gl_nseen counts the number of places where
   something is seen. gl_rock indicates a look for rock or objects.

   The others map co-ords in the ray frame to dungeon co-ords.

   dungeon y = char_row	 + gl_fyx * (ray x)  + gl_fyy * (ray y)
   dungeon x = char_col	 + gl_fxx * (ray x)  + gl_fxy * (ray y) */
static Short gl_fxx, gl_fxy, gl_fyx, gl_fyy;
static Short gl_nseen, gl_noquery;
static Short gl_rock;
/* Intended to be indexed by dir/2, since is only relevant to horizontal or
   vertical directions. */
static Short set_fxy[] = { 0,  1,	 0,  0, -1 };
static Short set_fxx[] = { 0,  0, -1,  1,	 0 };
static Short set_fyy[] = { 0,  0,	 1, -1,	 0 };
static Short set_fyx[] = { 0,  1,	 0,  0, -1 };
/* Map diagonal-dir/2 to a normal-dir/2. */
static Short map_diag1[] = { 1, 3, 0, 2, 4 };
static Short map_diag2[] = { 2, 1, 0, 4, 3 };

#define GRADF	10000	/* Any sufficiently big number will do */

/* Look at what we can see. This is a free move.

   Prompts for a direction, and then looks at every object in
   turn within a cone of vision in that direction. For each
   object, the cursor is moved over the object, a description is
   given, and we wait for the user to type something. Typing
   ESCAPE will abort the entire look.

   Looks first at real objects and monsters, and looks at rock
   types only after all other things have been seen.  Only looks at rock
   types if the highlight_seams option is set. */
static void look2(Short dir)
{
  Short i;
  Boolean /*dummy,*/ abort;

  found = false; // Also, disable the Recall button.
  highlight_me.valid = false;
  look_finished = true;
  recall_this_monster = -1;

  abort = false;
  gl_nseen = 0;
  gl_rock = 0;
  gl_noquery = false;	/* Have to set this up for the look_see */
  do {
    abort = false;
    if (dir == 5) {
      for (i = 1; i <= 4; i++) {
	gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
	gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
	if (look_ray(0, 2*GRADF-1, 1)) {
	  abort = true;
	  break;
	}
	gl_fxy = -gl_fxy;
	gl_fyy = -gl_fyy;
	if (look_ray(0, 2*GRADF, 2)) {
	  abort = true;
	  break;
	}
      }
    } else if ((dir & 1) == 0)	/* Straight directions */ {
      i = dir >> 1;
      gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
      gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
      if (look_ray(0, GRADF, 1))
	abort = true;
      else {
	gl_fxy = -gl_fxy;
	gl_fyy = -gl_fyy;
	abort = look_ray(0, GRADF, 2);
      }
    } else {
      i = map_diag1[dir >> 1];
      gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
      gl_fxy = -set_fxy[i]; gl_fyy = -set_fyy[i];
      if (look_ray(1, 2*GRADF, GRADF))
	abort = true;
      else {
	i = map_diag2[dir >> 1];
	gl_fxx = set_fxx[i]; gl_fyx = set_fyx[i];
	gl_fxy = set_fxy[i]; gl_fyy = set_fyy[i];
	abort = look_ray(1, 2*GRADF-1, GRADF);
      }
    }
  } while (abort == false && highlight_seams && (++gl_rock < 2));
  if (abort)
    look_finished = false; // look_message("--Aborting look--");
  else {
    if (gl_nseen) {
      if (dir == 5)
	look_message("That's all you see.");
      else
	look_message("That's all you see in that direction.");
    }
    else if (dir == 5)
      look_message("You see nothing of interest.");
    else
      look_message("You see nothing of interest in that direction.");
  }
}


/* Look at everything within a cone of vision between two ray
   lines emanating from the player, and y or more places away
   from the direct line of view. This is recursive.

   Rays are specified by gradients, y over x, multiplied by
   2*GRADF. This is ONLY called with gradients between 2*GRADF
   (45 degrees) and 1 (almost horizontal).

   (y axis)/ angle from
     ^	  /	    ___ angle to
     |	 /	 ___
  ...|../.....___.................... parameter y (look at things in the
     | /   ___			      cone, and on or above this line)
     |/ ___
     @-------------------->   direction in which you are looking. (x axis)
     |
     | */
static Short look_ray(Short y, Short from, Short to)
{
  static Short max_x, x;
  static Boolean transparent;

  /* from is the larger angle of the ray, since we scan towards the
     center line. If from is smaller, then the ray does not exist. */
  if (from <= to || y > MAX_SIGHT)
    return false;
  /* Find first visible location along this line. Minimum x such
     that (2x-1)/x < from/GRADF <=> x > GRADF(2x-1)/from. This may
     be called with y=0 whence x will be set to 0. Thus we need a
     special fix. */
  x = (Short) ((Long)GRADF * (2 * y - 1) / from + 1);
  if (x <= 0)
    x = 1;

  /* Find last visible location along this line.
     Maximum x such that (2x+1)/x > to/GRADF <=> x < GRADF(2x+1)/to */
  max_x = (Short) (((Long)GRADF * (2 * y + 1) - 1) / to);
  if (max_x > MAX_SIGHT)
    max_x = MAX_SIGHT;
  if (max_x < x)
    return false;

  /* gl_noquery is a HACK to prevent doubling up on direct lines of
     sight. If 'to' is	greater than 1, we do not really look at
     stuff along the direct line of sight, but we do have to see
     what is opaque for the purposes of obscuring other objects. */
  if ((y == 0 && to > 1) || (y == x && from < GRADF*2))
    gl_noquery = true;
  else
    gl_noquery = false;
  if (look_see(x, y, &transparent))
    return true;
  if (y == x)
    gl_noquery = false;
  if (transparent)
    goto init_transparent;


  for (;;) {
    /* Look down the window we've found. */
    if (look_ray(y+1, from, (Short) ((2 * y + 1) * (Long)GRADF / x)))
      return true;
    /* Find the start of next window. */
    do {
      if (x == max_x)
	return false;
      /* See if this seals off the scan. (If y is zero, then it will.) */
      from = (Short) ((2 * y - 1) * (Long)GRADF / x);
      if (from <= to)
	return false;
      x++;
      if (look_see(x, y, &transparent))
	return true;
    } while(!transparent);
  init_transparent:
    /* Find the end of this window of visibility. */
    do {
      if (x == max_x)
	/* The window is trimmed by an earlier limit. */
	return look_ray(y+1, from, to);
      x++;
      if (look_see(x, y, &transparent))
	return true;
    } while(transparent);
  }
}

static Boolean look_see(Short x, Short y, Boolean *transparent)
{
  Char *dstring, *string, *monname; // , query
  //  register cave_type *c_ptr;
  Short j;
  Char out_val[160], tmp_str[160];
  UChar floor, critter, light, thing;

  if (x < 0 || y < 0 || y > x) return false; // BUG if this happens

  dstring = ((x == 0 && y == 0) ? "You are on" : "You see");

  j = char_col + gl_fxx * x + gl_fxy * y;
  y = char_row + gl_fyx * x + gl_fyy * y;
  x = j;
  if (!panel_contains(y, x)) {
    *transparent = false;
    return false;
  }

  floor = cave_fval[y*MAX_WIDTH+x];
  thing = cave_tptr[y*MAX_WIDTH+x];
  critter = cave_cptr[y*MAX_WIDTH+x];
  light = cave_light[y*MAX_WIDTH+x];
  *transparent = floor <= MAX_OPEN_SPACE;
  if (gl_noquery)
    return false; /* Don't look at a direct line of sight. A hack. */

  out_val[0] = 0;

  // Monster
  if (gl_rock == 0 && critter > 1 && m_list[critter].ml) {
    j = m_list[critter].mptr;
    monname = c_names + c_list[j].name_offset;
    //    StrPrintF(out_val, "%s %s %s. [(r)ecall]",
    StrPrintF(out_val, "%s %s %s.",
	      dstring,  is_a_vowel(monname[0]) ? "an" : "a",  monname);
    dstring = "It is on";
    look_message(out_val);
    if (found) {
      // xxxx Also, should highlight the thingy.
      highlight_me.x = x;
      highlight_me.y = y;
      highlight_me.valid = true;
      // xxxx Also, enable the Recall button.
      recall_this_monster = j; // remember which monster to recall
    }
  }

  // Thing
  if ((light & (LIGHT_tl | LIGHT_pl | LIGHT_fm)) && !found) {
    if (thing != 0) {
      if (t_list[thing].tval == TV_SECRET_DOOR)
	goto granite;
      if (gl_rock ==0 && t_list[thing].tval != TV_INVIS_TRAP) {
	objdes(tmp_str, &t_list[thing], true);
	//	StrPrintF(out_val, "%s %s ---pause---", dstring, tmp_str);
	StrPrintF(out_val, "%s %s.", dstring, tmp_str);
	dstring = "It is in";
	look_message(out_val);
	if (found) {
	  // xxxx Also, should highlight the thingy
	  highlight_me.x = x;
	  highlight_me.y = y;
	  highlight_me.valid = true;
	}
      }
    }

    // ..Thing is a rock wall
    if (((gl_rock || out_val[0]) && floor >= MIN_CLOSED_SPACE) && !found) {
      switch(floor) {
      case BOUNDARY_WALL:
      case GRANITE_WALL:
 granite:
      /* Granite is only interesting if it contains something. */
      if (out_val[0])
	string = "a granite wall";
      else
	string = CNIL;	/* In case we jump here */
      break;
      case MAGMA_WALL: string = "some dark rock";
	break;
      case QUARTZ_WALL: string = "a quartz vein";
	break;
      default: string = CNIL;
	break;
      }
      if (string) {
	//	StrPrintF(out_val, "%s %s ---pause---", dstring, string);
	StrPrintF(out_val, "%s %s.", dstring, string);
	look_message(out_val);
	// xxxx Also, should highlight the thingy
	highlight_me.x = x;
	highlight_me.y = y;
	highlight_me.valid = true;
      }
    }

  }

  if (out_val[0])
    gl_nseen++;

  return found;
}



/*
static void look_see_is_transparent(Short x, Short y, Boolean *transparent)
{
  Char *dstring, *string, *monname; // , query
  //  register cave_type *c_ptr;
  Short j;
  Char out_val[160], tmp_str[160];
  UChar floor, critter, light, thing;
  
  j = char_col + gl_fxx * x + gl_fxy * y;
  y = char_row + gl_fyx * x + gl_fyy * y;
  x = j;
  if (!panel_contains(y, x))
    *transparent = false;
  else {
    floor = cave_fval[y*MAX_WIDTH+x];
    *transparent = floor <= MAX_OPEN_SPACE;
  }
}
*/
