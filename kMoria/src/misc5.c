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

// This is stuff related to obtaining a DIRECTION from the player.

Short needed_direction_for = DIR_NONE;
//Short needed_direction_item;

//extern Boolean stats_dirty;
extern UInt need_print_stats; // instead of stats_dirty
extern UChar current_print_stats;

Boolean draw_directional_p;
Boolean undraw_directional_p;
//extern Char last_command;
extern previous_state curr_state; // replaces last_command & co.

static void xor_directional() SEC_3;

static void xor_directional()
{
  Short x, y, d[4], scr_min = min(SCR_WIDTH,SCR_HEIGHT);
  x = SCR_WIDTH/2;
  y = SCR_HEIGHT/2;
  d[0] = scr_min/2 - 1;  // 0, 159:  80
  d[1] = scr_min/4;      // 40, 120: 40
  d[2] = scr_min/8;      // 60, 100: 20
  d[3] = scr_min/16;     // 70, 90:  10
  WinInvertLine(x-d[0], y-d[1], x-d[2], y-d[3]); //  was(0,  40, 60, 70);
  WinInvertLine(x+d[2], y+d[3], x+d[0], y+d[1]); //  was(100,90,160,120);
  WinInvertLine(x-d[0], y+d[1], x-d[2], y+d[3]); //  was(0,  120,   60, 90);
  WinInvertLine(x+d[2], y-d[3], x+d[0], y-d[1]); //  was(100, 70,  160, 40);
  WinInvertLine(x-d[1], y-d[0], x-d[3], y-d[2]); //  was(40,  0, 70, 60);
  WinInvertLine(x+d[3], y+d[2], x+d[1], y+d[0]); //  was(90,100,120,159);
  WinInvertLine(x+d[1], y-d[0], x+d[3], y-d[2]); //  was(120,  0,90, 60);
  WinInvertLine(x-d[3], y+d[2], x-d[1], y+d[0]); //  was(70, 100,40,159);    
}
void erase_runwalk_circles() SEC_5;
void draw_directional()
{
  Char *p = NULL;
  //  XOR some lines onto the screen
  // centered around 80,80
  erase_runwalk_circles();
  //  switch(last_command) {
  switch(curr_state.cmd) {
  case 'a':
    p = "Aiming wand...";
    break;
  case 't':
    p = "Throwing...";
    break;
  case 'm':
    p = "Casting spell...";
    break;
  case 'p':
    p = "Targeting prayer...";
    break;
  case 'f':
    p = "Forcing/bashing...";
    break;
  case '^':
    p = "Disarming trap...";
    break;
  case 'c':
    p = "Closing...";
    break;
  case 'S':
    p = "Spiking...";
    break;
  case 'o':
    p = "Opening...";
    break;
  case 'd':
    p = "Digging...";
    break;
  case '?':
    p = "Looking...";
    break;
  case '-':
    p = "Shifting the display...";
    break;
  }
  if (p != NULL) {
    // alert_message(p);
    count_message(p);
    show_messages();
  }
  xor_directional();
  //stats_dirty = false;
  need_print_stats = STATS_NONE;
  draw_directional_p = false;
  undraw_directional_p = true;
}
void undraw_directional()
{
  xor_directional(); //  XOR the lines again.. hopefully nothing has changed!
  //stats_dirty = true;
  need_print_stats = ((current_print_stats & CURSTAT_NOT_STR) ?
		      STATS_ALLG : STATS_ALL);
  undraw_directional_p = false;
}

//#define abs(a)                                  (((a) >= 0) ? (a) : (-(a)))
// ignore_center is the 'radius' of a central square that is 'ignored'
Short convert_to_dir(Short x, Short y, Short ignore_center)
{
  //    x = e->screenX - 80;
  //    y = e->screenY - 80;
  // x and y each range from -80 to +80
  // -80,-80 is the top left corner of the screen

  // almost in the center?  heck, let's make it a real (circle) radius
  if (x*x + y*y < ignore_center*ignore_center)
    return 5; // the NOOP direction
  // mostly x?
  if ((abs(x)) > 2 * (abs(y)))
    return ( (x > 0) ? 6 : 4 ); // east, west
  // mostly y?
  if ((abs(y)) > 2 * (abs(x)))
    return ( (y > 0) ? 2 : 8 ); // south, north
  // diagonal?  (x ==approx== y)
  if (y > 0)      // southish
    return ( (x > 0) ? 3 : 1 ); // se, sw
  else            // northish
    return ( (x > 0) ? 9 : 7 ); // ne, nw
}

void get_default_username()
{
  Char *tmp, *first_wspace;
  VoidHand h;
  tmp = md_malloc(sizeof(Char) * (dlkMaxUserNameLength + 1));
  DlkGetSyncInfo(NULL, NULL, NULL, tmp, NULL, NULL);
  /* if it's too long, use the first name only */
  if (StrLen(tmp) > PLAYER_NAME_SIZE-1) {
    first_wspace = StrChr(tmp, spaceChr);
    if (first_wspace)
      *(first_wspace) = '\0';
    else
      tmp[PLAYER_NAME_SIZE-1] = '\0';
  }
  if (StrLen(tmp))
    StrNCopy(pymisc.name, tmp, PLAYER_NAME_SIZE);
  else
    StrPrintF(pymisc.name, (pymisc.male ? "'tis Himself" : "'tis Herself"));
  h = MemPtrRecoverHandle(tmp);
  if (h) MemHandleFree(h);  
}


/**********************************************************************
                       DO_FEEP
 IN:
 frq = frequency
 dur = duration
 OUT:
 nothing
 PURPOSE:
 This will produce a beep of the requested frequency and duration,
 if the pilot owner has "game sound" turned on in the Preferences app.
 (Try to keep it down to chirps...)
 **********************************************************************/
/* SoundLevelType: slOn, slOff
   gameSoundLevel found in SystemPreferencesType
 */
void do_feep(Long frq, UInt dur)
{
  SystemPreferencesChoice allgamesound;

  if (!my_prefs.sound)
    return;

#ifdef I_AM_OS_2
  allgamesound = prefGameSoundLevel;
#else
  allgamesound = prefGameSoundLevelV20;
#endif

  if (PrefGetPreference(allgamesound) != slOff) {
    /* click: 200, 9
       confirmation: 500, 70  */
    SndCommandType sndCmd;
    sndCmd.cmd = sndCmdFreqDurationAmp; /* "play a sound" */
    sndCmd.param1 = frq; /* frequency in Hz */
    sndCmd.param2 = dur; /* duration in milliseconds */
    sndCmd.param3 = sndDefaultAmp; /* amplitude (0 to sndMaxAmp) */
    SndDoCmd(0, &sndCmd, true);
  }
}
