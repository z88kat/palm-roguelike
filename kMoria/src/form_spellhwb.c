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

/***********************************************************************
 * This form allows the player to bind/unbind spells to hardware buttons
 *
 * Here's the plan
 * there are pushbtns on the left.  exactly one is always selected
 * (open event must select one initially)
 * there is a list on the right.  when the user selects an item on it,
 * we assign that item to the hwb represented by the selected pushbtn.
 * also we write the index letter of that item next to the pushbtn
 * (this should also be done initially).
 ***********************************************************************/
Short canonical_book; /* this must be an index into object_list!!! not inv!
			like, all the stuff about an object. */
Char btn_is_spell_char[8];

extern Char **spell_display; // see form_spell.c
extern Short spell_ctr; // number of strings in spell_display
extern Short spells[MAX_SPELLS]; // 'spell_ctr' of them must be filled in.
void free_spell_select(ListPtr lst) SEC_4;
void show_spells(FormPtr frm, Word lst_i, Short first_spell) SEC_4;

// These values are assumed anyway by create.c
// So they might as well make themselves useful
#define FIRST_MAGE_BOOK 318
#define FIRST_PRIEST_BOOK 322

static void init_spells() SEC_7;
static void setup_shwb_for(Short my_book) SEC_7;
static void write_spell_binding(Short hwb_i, UChar spell_i) SEC_7;

Boolean SpellHwBtns_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  ControlPtr pushbtn, ctl;
  CharPtr label;
  Short i, k;
  //  ULong j;
  Char c;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_shwb_2));
    ctl = FrmGetObjectPtr (frm, FrmGetObjectIndex(frm, popup_shwb_2));
    // start up with the book appropriate to your class, or else mage.
    i = ( (class[pymisc.pclass].spell == PRIEST) ? 4 : 0 );
    LstSetSelection(lst, i);
    label = LstGetSelectionText(lst, i);
    CtlSetLabel(ctl, label);

#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    // Select the first pushbtn to ensure one is always selected.
    pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_shwb_0));
    CtlSetValue(pushbtn, true);
    setup_shwb_for(FIRST_MAGE_BOOK + i);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case lstSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.lstSelect.listID) {
    case list_shwb_1:
      i = e->data.lstSelect.selection;
      if (i == -1 || i >= spell_ctr) { handled = true; break; }
      for (k = 0 ; k < 8 ; k++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_shwb_0+k));
	if (CtlGetValue(pushbtn))
	  break;
      }
      if (k >= 8) break;
      c = i + 'a';
      btn_is_spell_char[k] = c;
      show_spell_char(k);
      // also, RECORD THIS VALUE
      write_spell_binding(k, i+1);
      break;
    }
    handled = true;
    break;

  case popSelectEvent:
    frm = FrmGetActiveForm();
    i = e->data.popSelect.selection;
    if (i == -1 || i >= 8 ) { handled = true; break; }
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_shwb_1));
    free_spell_select(lst);
    setup_shwb_for(FIRST_MAGE_BOOK + i);
    LstDrawList(lst);
    // do not set 'handled' ?
    break;
    

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_shwb_clear:
      // feedback in list
      frm = FrmGetActiveForm();
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_shwb_1));
      LstSetSelection(lst, -1);
      // find the affected pushbtn
      for (k = 0 ; k < 8 ; k++) {
	pushbtn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, pbtn_shwb_0+k));
	if (CtlGetValue(pushbtn))
	  break;
      }
      if (k >= 8) break;
      btn_is_spell_char[k] = '-';
      show_spell_char(k);
      // also, RECORD THIS VALUE
      write_spell_binding(k, 0);
      handled = true;
      break;
    case btn_shwb_ok:
      frm = FrmGetActiveForm();
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_shwb_1));
      free_spell_select(lst);
      writeMoriaPrefs();
      LeaveForm();
      handled = true;
      break;
    }
    break;
  default:
    break;
  }

  return handled;
}



// cribbed from examine_book in form_inv.c
static void init_spells()
{
  Short i=0, k;
  ULong j;
  j = object_list[canonical_book].flags;
  while (j) {
    k = bit_pos(&j);
    /*      if (magic_spell[my_class][k].slevel < 99) { */
    spells[i] = k;
    i++;
    /*      } // else it's a spell your class never gets; sorry dude
    */
  }
  spell_ctr = i;
}

// Call this initially (after FrmDrawForm) and also call it (AFTER you
// free the list) to change books.
static void setup_shwb_for(Short my_book)
{
  FormPtr frm;
  //  ControlPtr pushbtn;
  Short i, first_spell, k;
  UChar which_spell;
  ULong j;
  canonical_book = my_book;
  // 1. Print spell names into the list, including index char (with gaps)
  init_spells();
  frm = FrmGetActiveForm();
  j = object_list[canonical_book].flags;
  first_spell = bit_pos(&j); // this modifies j
  show_spells(frm, list_shwb_1, first_spell); // this is in form_spell.c
  // 2. Dig up the bindings we already have for this book.  (If any.)
  k = 8 * ((my_book - FIRST_MAGE_BOOK) % 4);// index in my_prefs.hw_spells.
  for (i = 0 ; i < 8 ; i++, k++) {
    which_spell = my_prefs.hw_spells[k];
    if (my_book < FIRST_PRIEST_BOOK)
      which_spell = which_spell & 0x0F;
    else {
      which_spell = which_spell >> 4; // unsigned, so high 4 bits become 0s
      which_spell = which_spell & 0x0F; // except it seems not to be working
    }
    if (which_spell == 0)
      btn_is_spell_char[i] = '-'; // not bound...
    else
      btn_is_spell_char[i] = (which_spell - 1) + 'a';
  }
  // 3. Print the index letter of ALL spells next to the pbtn stack.
  show_spell_char(-1);
}
static void write_spell_binding(Short hwb_i, UChar spell_i)
{
  Short k;
  UChar hw_spell_pair;
  k = 8 * ((canonical_book - FIRST_MAGE_BOOK) % 4);//index my_prefs.hw_spells
  k += hwb_i;
  hw_spell_pair = my_prefs.hw_spells[k];
  // now, frob it
  if (canonical_book < FIRST_PRIEST_BOOK) {
    hw_spell_pair = (hw_spell_pair & 0xF0); // erase the old lower-4
    hw_spell_pair = (hw_spell_pair | spell_i); // add the new lower-4
  } else {
    hw_spell_pair = (hw_spell_pair & 0x0F); // erase the old higher-4
    hw_spell_pair = (hw_spell_pair | (spell_i << 4)); // and add new
  }
  // well, I suppose that's ok
  my_prefs.hw_spells[k] = hw_spell_pair;  
}


// Prints 1 if index is in range; prints all if out of range.
//#define SPELL_CHAR_X 3+35+1+2 /* for 160x160 screen */
void show_spell_char(Short i)
{
  FormPtr frm = FrmGetActiveForm();
  RectangleType r;
  Short x, y, dy;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, pbtn_shwb_0), &r);
  x = r.topLeft.x + r.extent.x + 1+2; // 3 + 35 + a little space (3)
  y = r.topLeft.y;  //y = ROW_1 + LINE_HEIGHT;
  dy = r.extent.y + 1;
  if (i >= 0 && i < 8)
    WinDrawChars(btn_is_spell_char+i, 1,  x, y + dy*i);
  else
    for (i = 0 ; i < 8 ; i++)
      WinDrawChars(btn_is_spell_char+i, 1,  x, y + dy*i);
}
