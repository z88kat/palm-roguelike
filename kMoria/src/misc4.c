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


// Working on first pass through this... !

/* Add a comment to an object description.		-CJS- */
// This should be handled in a form!  See form_scribe.c
/*
void scribe_object()
{
  Short item_val, j; // item_val;
  Char out_val[80], tmp_str[80];

  objdes(tmp_str, &inventory[item_val], true);
  StrPrintF(out_val, "Inscribing %s", tmp_str);
  message(out_val);
  if (inventory[item_val].inscrip[0] != '\0')
    StrPrintF(out_val, "Replace %s New inscription:",
	      inventory[item_val].inscrip);
  else
    StrCopy(out_val, "Inscription: ");
  j = 78 - StrLen(tmp_str);
  if (j > 12)
    j = 12;
  ;      //      prt(out_val, 0, 0); // xxx not implemented yet
  if(0)//      if (get_string(out_val, 0, (Short) StrLen(out_val), j)) // xxx not implemented yet
    inscribe(item_val, out_val);
}
*/

/* Append an additional comment to an object description.	-CJS- */
void add_inscribe(inven_type *i_ptr, Short index, UChar type)
{
  invy_set_ident(i_ptr, index, i_ptr[index].ident | type);
}

/* Replace any existing comment in an object description with a new one. CJS*/
// See form_scribe.c ...
//void inscribe(Short iptr, Char *str)
//{
//  if (StrLen(str) >= INSCRIP_SIZE)
//    str[INSCRIP_SIZE-1] = '\0'; // truncate!
//  DmStrCopy(inventory, iptr*sizeof(inven_type)+I_inscrip, str);
//}


/* We need to reset the view of things.			-CJS- */
Boolean check_view()
{
  Short i, j, c_ptr, d_ptr;
  Boolean recalced = false;

  c_ptr = char_row*MAX_WIDTH+char_col;
  /* Check for new panel		   */
  if (get_panel(char_row, char_col, false)) {
    recalc_screen();
    recalced = true;
  }
  /* Move the light source		   */
  move_light(char_row, char_col, char_row, char_col);
  /* A room of light should be lit.	 */
  if (cave_fval[c_ptr] == LIGHT_FLOOR) {
    if ((pyflags.blind < 1) && !(cave_light[c_ptr] & LIGHT_pl))
      light_room(char_row, char_col);
  }
  /* In doorway of light-room?		   */
  else if ((cave_light[c_ptr] & LIGHT_lr) && (pyflags.blind < 1)) {
    for (i = (char_row - 1); i <= (char_row + 1); i++)
      for (j = (char_col - 1); j <= (char_col + 1); j++) {
	d_ptr = i*MAX_WIDTH+j;
	if ((cave_fval[d_ptr] == LIGHT_FLOOR)
	    && !(cave_light[d_ptr] & LIGHT_pl))
	  light_room(i, j);
      }
  }
  return recalced;
}
