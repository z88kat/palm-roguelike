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

#ifndef __IROGUE_GLOBALS_H__
#define __IROGUE_GLOBALS_H__

#define RogueAppType 'RoGe'
#define RogueDBType  'Data'
#define RogueAppID   'RoGe'
//extern DmOpenRef       RogueDB;
/*extern char            RecipeDBName[];*/
#define RogueDBName "iRogueDB"
#define RogueTileDBName "iRogueTileDB"
//#define RogueLibraryName "iRogue_Library"

#define RogueAppPrefID 0x00
//#define RogueAppPrefVersion 0x02
//#define RogueAppPrefVersion 0x03 /* iRogue Version 0.42 */
//#define RogueAppPrefVersion 0x03 /* iRogue Version 0.45 (color) */
#define RogueAppPrefVersion 0x04 /* iRogue Version 1.1.1 (icons/conducts) */


#include "palm.h"
#ifndef I_AM_OS_2
#include "handera.h" /* some need this too */
#endif
#include "iRogueRsc.h"
#include "sections.h"
#include "constant.h"
#include "types.h"
#include "prototypes.h"
#include "globals.h"

#endif
