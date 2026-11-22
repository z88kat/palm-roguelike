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
#ifndef __KMORIA_GLOBALS_H__
#define __KMORIA_GLOBALS_H__

/* This is the master include file that includes everything else now.
   Why?  To make it easier to use cstar (long story).
   Note not every c file needs every header file,
   e.g. some don't need kMoriaRsc.h or wacky API headers. */

#include "palm.h" /* practically everyone needs this */
#ifndef I_AM_OS_2
#include "Handera/Vga.h" /* some need this too */
#endif

/* Yes, I have registered this Creator ID.  Yay. */
#define MoriaAppType 'Mria'
#define MoriaDBType  'Data'
#define MoriaSaveDBType  'Save'
#define MoriaColorDBType  'Data'
#define MoriaScoreDBType  'Data'
#define MoriaAppID   'Mria'
extern DmOpenRef       MoriaDB;
extern DmOpenRef       MoriaSaveDB;
extern DmOpenRef       MoriaScoreDB; /* don't need it open all the time. */
#define MoriaDBName "kMoriaDB"
#define MoriaSaveDBName "kMoriaSaveDB"
#define MoriaScoreDBName "kMoriaScoreDB"
#define MoriaColorDBName "kMoriaColorDB"
#define MoriaColorDBVersion 0x01
#define MoriaSaveDBVersion 0x02

#define MoriaAppPrefID 0x00
/*#define MoriaAppPrefVersion 0x01*/
/* added stuff after name: */
//#define MoriaAppPrefVersion 0x02
/* added stuff after hw_spells: */
#define MoriaAppPrefVersion 0x03

#include "kMoriaRsc.h" /* anyone referring to UI entities needs this */

/* everyone needs these: */
#include "sections.h"
#include "constant.h"
#include "types.h"
#include "prototypes.h"
#include "records.h"
#include "lock-externs.h"

/* only a few folks need these: */
#include "win.h" /* only display.c actually needs it */
#include <CharAttr.h> /* only form_store.c and main.c, for IsDigit */
#include <DLServer.h> /* misc5.c, needed for user name */
#include <Graffiti.h> /* only dungeon.c and main.c, and just for GrfGetState */
#include <Preferences.h> /* form_{death,topscores}.c, preferred date format */
#include <SysEvtMgr.h> /* only dungeon.c, in order to interrupt 'rest' */
#ifndef I_AM_OS_2
#include "handera.h"
#endif
#ifdef I_AM_COLOR
#include "color.h" /* only display.c needs it */
#include <SystemMgr.h> /* for misc5.c; I think it's needed for user name */
#else
#include <System/Globals.h> /* only display.c and main.c need it, for fonts */
#include <SystemMgr.rh> /* for misc5.c; needed for user name */
#define found_color false
#endif /* I_AM_COLOR */



#endif
