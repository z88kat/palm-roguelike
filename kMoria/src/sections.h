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
#ifndef __MORIASECTIONS_H__
#define __MORIASECTIONS_H__

// define a section of code
#define SEC_1 __attribute__ ((section ("sec_1")))
#define SEC_2 __attribute__ ((section ("sec_2")))
#define SEC_3 __attribute__ ((section ("sec_3")))
#define SEC_4 __attribute__ ((section ("sec_4")))
#define SEC_5 __attribute__ ((section ("sec_5")))
#define SEC_6 __attribute__ ((section ("sec_6")))
#define SEC_7 __attribute__ ((section ("sec_7")))

#endif
