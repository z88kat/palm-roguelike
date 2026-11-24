/*********************************************************************
 * iLarn - Ularn adapted for the PalmPilot.                          *
 * Copyright (C) 2000 Bridget Spitznagel                             *
 * iLarn is derived from Ularn by Phil Cordier                       *
 * which in turn was based on Larn by Noah Morgan                    *
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
/* 
 * Here find a file full of "extern" declarations 
 * These are all defined in iLarn.c (currently)`
 */
#ifndef __ILARN_GLOBALS_H__
#define __ILARN_GLOBALS_H__

#define iLarnAppType 'Larn'
#define iLarnDBType  'Data'
#define iLarnSaveDBType  'Data'
#define iLarnAppID   'Larn'
extern DmOpenRef       iLarnDB;
extern DmOpenRef       iLarnSaveDB;
#define iLarnDBName "iLarnDB"
#define iLarnSaveDBName "iLarnSaveDB"
//#define iLarnLibraryName "iLarn_Library"
//#define iLarn2LibraryName "iLarn2_Library"

#define iLarnAppPrefID 0x00
#define PrefVersion1 0x01 // Had a different top ten format
#define PrefVersion2 0x02 // Did not have 'color' and 'black_bg'
#define PrefVersion3 0x03 // Still had top ten in DB
#define iLarnAppPrefVersion 0x04 // [We move top ten to SaveDB.]


#include "header.h"
#include "types.h"
//#include "prototypes.h"
#include "larn_prototypes.h"

extern struct everything *env;

// Palm OS 5 devices (Tungsten C, T, E...) have a 5-way navigator that this
// code predates. palmOne devices send Up/Down as vchrPageUp/Down with
// navBit*/navChange* flags in keyCode, and Left/Right/Select (and every
// release) as vchrNavChange; others send vchrRocker*. Values are from the
// PalmOne SDK (palmOneChars.h, palmOneNavigator.h), which the 3.5 SDK lacks.
#ifndef vchrNavChange
#define vchrNavChange    0x0503
#endif
#ifndef navBitUp
#define navBitUp         0x0001
#define navBitDown       0x0002
#define navBitLeft       0x0004
#define navBitRight      0x0008
#define navBitSelect     0x0010
#define navBitsAll       0x001F
#define navChangeBitsAll 0x1F00
#endif
#ifndef vchrRockerUp
#define vchrRockerUp     0x0132
#define vchrRockerDown   0x0133
#define vchrRockerLeft   0x0134
#define vchrRockerRight  0x0135
#define vchrRockerCenter 0x0136
#endif
// What fiveway_key() makes of a keyDownEvent (main.c).
#define NAV_NONE    0 // not the navigator: handle the key as usual
#define NAV_IGNORE  1 // a release, or several buttons at once: swallow it
#define NAV_UP      2
#define NAV_DOWN    3
#define NAV_LEFT    4
#define NAV_RIGHT   5
#define NAV_SELECT  6
Short fiveway_key(EventPtr e);
Boolean fiveway_list(ListPtr lst, Short nav);
Boolean fiveway_hit(FormPtr frm, UInt16 id);
Boolean fiveway_buttons(FormPtr frm, const UInt16 *ids, Short n,
			Short *focus, Short nav, UInt16 dflt);


#endif
