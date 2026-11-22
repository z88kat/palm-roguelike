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

// Form to select a spell book, then select a spell.
static void show_books(FormPtr frm) SEC_4;
static void free_book_select(FormPtr frm) SEC_4;
static void count_spells(Short selected_book) SEC_4;
void show_spells(FormPtr frm, Word lst_i, Short first_spell) SEC_4;
static void print_spell_ss(FormPtr frm, Word lst_i, Short start, Short rows, Boolean browse) SEC_4;
static void print_spell_status(Short i, Short j, Short top, Short side) SEC_4;
static void print_spell_stats(Short i, Short j, Short top, Short side) SEC_4;
static void hit_spell_by_alpha(Char c, Word lst_index) SEC_4;
static void cast_spell_if_ok(Short first_spell) SEC_4;
void free_spell_select(ListPtr lst) SEC_4;
static void learn_the_spell(Short j, ListPtr lst) SEC_4;

Short selected_book = -1;
Short selected_spell = -1;
Char **book_display; // used for books or spells
Char **spell_display; // used for books or spells
Short book_ctr; // number of strings in book_display
Short spell_ctr; // number of strings in spell_display
extern spell_type magic_spell[MAX_CLASS-1][MAX_SPELLS]; // magic.c
extern Short spells[MAX_SPELLS]; // spell_ctr of them will be filled in.
//extern ULong spell_learned;
//extern ULong spell_worked;
//extern ULong spell_forgotten;

// why we're in the spell form
Boolean just_browsing_book = false;

Boolean book_btnsHandleEvent(Short btn_i)
{
  FormPtr frm;
  ListPtr lst;
  if (btn_i < 0 || btn_i >= book_ctr)
    return false;
  frm = FrmGetActiveForm();
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
  LstSetSelection(lst, btn_i);
  // also, act like we pressed ok.
  hit_button_if_usable(frm, btn_book_ok);
  return true;
}


Boolean Book_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  CharPtr label;
  Short s;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    selected_book = -1;
    show_books(frm);
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    // no need to make the right buttons visible - they are by default.
    // Also, print your current mana!
    show_mana_at_bottom();
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

    // No need to scroll - there are only 8 distinct books,
    // and, no character class can use more than 4 of them anyway.
  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
    s = LstGetSelection(lst); // might be -1
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (--s < 0) s = 0;
      LstSetSelection(lst, s);
      handled = true;
      break;
    case pageDownChr:
      if (++s >= book_ctr) s = book_ctr - 1;
      LstSetSelection(lst, s);
      handled = true;
      break;
      // Actually, if you are a mage, and for some daft reason you
      // acquire prayer books, your first book listed will NOT be a!!!
      // If you have 2 books (of a kind that you can use) don't worry,
      // it will only appear once (it is one inventory entry).
    case 'a':
      LstSetSelection(lst, 0);
      hit_button_if_usable(frm, btn_book_ok);
      handled = true;
      break;
    case 'b':
      LstSetSelection(lst, 1);
      hit_button_if_usable(frm, btn_book_ok);
      handled = true;
      break;
    case 'c':
      LstSetSelection(lst, 2);
      hit_button_if_usable(frm, btn_book_ok);
      handled = true;
      break;
    case 'd':
      LstSetSelection(lst, 3);
      hit_button_if_usable(frm, btn_book_ok);
      handled = true;
      break;
    case '\n':
      // make like we hit 'read book'
      hit_button_if_usable(frm, btn_book_ok);
      handled = true;
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_book_ok:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
      s = LstGetSelection(lst);
      if (s != -1) {
	label = LstGetSelectionText(lst, s);
	if (label) {
	  selected_book = label[0] - 'a';
	  count_spells(selected_book);
	  if (spell_ctr > 0) {
	    just_browsing_book = false;
	    FrmPopupForm(SpellForm);
	  } else
	    show_brief_comment("You don't know any in that book.");
	}
      }
      handled = true;
      break;
    case btn_book_cancel:
      free_book_select(frm);
      LeaveForm();
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}



// I should really let the books have 2 lines each.
static void show_books(FormPtr frm)
{
  Short i, j;
  Char tmp_val[160], c;
  Int width_str, length_str;
  Boolean fits;
  ListPtr lst;
  UChar tval;

  if (class[pymisc.pclass].spell == PRIEST)
    tval = TV_PRAYER_BOOK;
  else if (class[pymisc.pclass].spell == MAGE)
    tval = TV_MAGIC_BOOK;
  else
    return;// should not happen!

  book_ctr = 0;
  for (i = 0 ; i < inven_ctr ; i++)
    if (inventory[i].tval == tval)
      book_ctr++;

  if (book_ctr == 0)
    return;// caller should have checked!

  // there must be at least one book.
  book_display = (Char **) md_malloc(sizeof(Char *) * book_ctr);

  for (i = 0, j = 0, c = 'a' ; i < inven_ctr ; i++, c++) {
    /* Print the items	  */
    //    if (mask == CNIL || mask[i]) {
    if (inventory[i].tval == tval) {
      //      objdes(tmp_val+3, &inventory[i], true); // XXXXxxx fix this.
      StrPrintF(tmp_val, "%c) %s", c,
		object_names + object_list[inventory[i].index].name_offset);

      // Truncate if too wide for the screen..... we only have 156 pixels...
      // length_str gets set to the # of chars that will fit in 156
      // width_str gets set to the # of pixels that those chars will use(<=156)
      width_str = SCR_WIDTH - 4; //156;
      length_str = StrLen(tmp_val); // max chars to allow
      FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
      tmp_val[length_str] = '\0';
      // ok, now like put it somewhere.
      book_display[j] = (Char *) md_malloc(1 + length_str);
      StrCopy(book_display[j], tmp_val);
      j++;
    }
  }

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
  LstSetListChoices(lst, book_display, book_ctr);
  selected_book = -1;
  LstSetSelection(lst, -1);
}

/*
 *  Free the strings in the list
 */
static void free_book_select(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_book));
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < book_ctr ; i++) {
    h = MemPtrRecoverHandle(book_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(book_display);
  if (h) 
    MemHandleFree(h);
}

////////////////////////////////////////////////////////////////////////
extern Short magic_spell_choice; // dungeon.c
Boolean Spell_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  //  CharPtr label;
  Char c;
  Short s;
  static Short first_spell; // remember which spell is 1st in this book
  ULong j;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    j = inventory[selected_book].flags;
    first_spell = bit_pos(&j); // this modifies j
    show_spells(frm, list_spell, first_spell);
    spell_draw();
    // also fix the buttons:
    // if browse: hide "ok", change cancel's buttonlabel to "done"
    // if learn: hide "ok", *show "learn"*
    // create a new "learn" button, initially disabled,
    // and make its handler cause you to learn the selected spell.
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

    // No need to scroll - the books each have 10 or fewer spells.  I _think_.
    // I will make Learning a separate form.
    // (Hm, did I?  yeah, GainSpForm)
  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
    s = LstGetSelection(lst); // might be -1
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      // see if we bound it to a particular spell
      if (spell_btnsHandleEvent(HW_pageUpChr)) return true;
      // nope..
      if (--s < 0) s = 0;
      LstSetSelection(lst, s);
      handled = true;
      break;
    case pageDownChr:
      // see if we bound it to a particular spell
      if (spell_btnsHandleEvent(HW_pageDownChr)) return true;
      // nope..
      if (++s >= spell_ctr) s = spell_ctr - 1;
      LstSetSelection(lst, s);
      handled = true;
      break;
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'i':
      // Find the line starting with this letter. Lines are in order with gaps.
      c = e->data.keyDown.chr;
      hit_spell_by_alpha(c, list_spell);
      // make like we hit 'recite'! could be dangerous... hm...
      hit_button_if_usable(frm, btn_spell_ok);
      handled = true;
      break;
    case '\n': case 'R':
      // make like we hit 'recite'
      hit_button_if_usable(frm, btn_spell_ok);
      handled = true;
      break;
    case 'C': case 27:
      hit_button_if_usable(frm, btn_spell_cancel);
      handled = true;
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_spell_ok:
      cast_spell_if_ok(first_spell);
      handled = true;
      break;
    case btn_spell_cancel:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
      free_spell_select(lst); // hm, I get a Nil Pointer here. // fixed
      // (don't forget to NOT free something you allocated *0* bytes for)
      LeaveForm();
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}

void spell_draw()
{
  print_spell_ss(FrmGetActiveForm(), list_spell, 0, 10, just_browsing_book);
  // Also, print your current mana!
  show_mana_at_bottom();
}

static void count_spells(Short selected_book)
{
  Short k;
  ULong j;
  spell_type *s_ptr;

  // set spell_ctr to the number of known SPELLS from the given book
  //  j = inventory[selected_book].flags;
  //  first_spell = bit_pos(&j); // this modifies j
  j = inventory[selected_book].flags & spell_learned;
  s_ptr = magic_spell[pymisc.pclass-1];
  spell_ctr = 0;
  while (j) {
    k = bit_pos(&j);
    if (s_ptr[k].slevel <= pymisc.lev) {
      spells[spell_ctr] = k;
      spell_ctr++;
    }
  }
}

// For "again"ed spells.
// Make sure player has the book that this spell comes from, else return false.
// And make sure the player knows the spell still.
Boolean has_book(Short spell)
{
  ULong j;
  Short i,k;
  UChar tval;
  spell_type *s_ptr;

  switch(class[pymisc.pclass].spell) {
  case PRIEST: tval = TV_PRAYER_BOOK; break;
  case MAGE: tval = TV_MAGIC_BOOK; break;
  default: return false; // "impossible"
  }
  for (i = 0 ; i < inven_ctr ; i++) {
    if (inventory[i].tval != tval) continue;
    j = inventory[i].flags & spell_learned;
    s_ptr = magic_spell[pymisc.pclass-1];
    while (j) {
      k = bit_pos(&j);
      if (k == spell) return true; // found it!
      if (k > spell || s_ptr[k].slevel > pymisc.lev) return false; // too high
    }
  }
  return false;
}


/* Note - may also want to use this form for:
   browsing a book (list all n<10 spell names, and status instead of #s)
   [selecting a spell to learn (there might be >10, rats.  how likely??)] */
//
// Caller must SET spell_ctr and spells[] to an appropriate value
// before calling show_spells.
void show_spells(FormPtr frm, Word lst_i, Short first_spell)
{
  Short i, offset, j, lst_width, width_str;
  //  ULong j;
  Char tmp_val[160], c;
  Int length_str;
  Boolean fits;
  UInt obj_i = FrmGetObjectIndex(frm, lst_i);
  ListPtr lst = FrmGetObjectPtr(frm, obj_i);
  RectangleType r;
  FrmGetObjectBounds(frm, obj_i, &r);
  lst_width = r.extent.x; // = 112; // = 156;

  // spell_ctr, spells[] have ALREADY BEEN SET by caller.

  // malloc spell_display - there must be at least 1 spell.
  spell_display = (Char **) md_malloc(sizeof(Char *) * spell_ctr);
  // print strings
  // this was in:      print_spells (spell, num, FALSE, first_spell);
  offset = (class[pymisc.pclass].spell==MAGE ? SPELL_OFFSET : PRAYER_OFFSET);
  for (i = 0 ; i < spell_ctr ; i++) {
    j = spells[i];
    // comment == false, so no "forgotten/unknown/untried" here
    if (first_spell != -1)
      c = 'a' + j - first_spell;
    else
      c = 'a' + i;
    StrPrintF(tmp_val, "%c) %s", c, spell_names[j+offset]);
    width_str = lst_width;
    length_str = StrLen(tmp_val); // max chars to allow
    FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
    tmp_val[length_str] = '\0';
    // ok, now like put it somewhere.
    spell_display[i] = (Char *) md_malloc(1 + length_str);
    StrCopy(spell_display[i], tmp_val);
  }
  // set ui foo
  //  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
  LstSetListChoices(lst, spell_display, spell_ctr);
  selected_spell = -1;
  LstSetSelection(lst, -1);
  LstDrawList(lst);
}
// Print something to the right side of all the spells.
static void print_spell_ss(FormPtr frm, Word lst_i, Short start,
			   Short rows, Boolean browse)
{
  RectangleType r;
  Short i, j;
  Short list_side, top, list_top; //Short list_side = 112+1;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, lst_i), &r);
  list_side = r.topLeft.x + r.extent.x + 1; // = 112; // = 156;
  list_top = r.topLeft.y; // = 26;
  top = LINE_HEIGHT;
  //  RctSetRectangle(&r, list_side, top, SCR_WIDTH-list_side, 10*LINE_HEIGHT);
  RctSetRectangle(&r, list_side, list_top - top, SCR_WIDTH-list_side,
		  r.extent.y + top);
  WinEraseRectangle(&r, 0);

  if (rows > spell_ctr) rows = spell_ctr;
  if (!browse) {
    WinDrawChars("Lv-M-Fail", 9,   list_side+1, list_top-top);
    for (i = start ; i < rows ; i++) {
      j = spells[i];
      print_spell_stats(i-start, j, list_top, list_side); // where to draw, what to draw
    }
  } else {
    WinDrawChars("Status", 6,   list_side+1, list_top-top);
    for (i = start ; i < rows ; i++) {
      j = spells[i];
      print_spell_status(i-start, j, list_top, list_side); // words instead of numbers
    }
  }
}

// show for browsing
static void print_spell_status(Short i, Short j, Short lst_top, Short lst_side)
{
  Short x = lst_side+FntCharWidth(' '), y = lst_top;
  Char *p;
  if ((spell_forgotten & (1L << j)) != 0)
    p = "forgotten";
  else if ((spell_learned & (1L << j)) == 0)
    p = "unknown";
  else if ((spell_worked & (1L << j)) == 0)
    p = "untried";
  else
    return;
  WinDrawChars(p, StrLen(p), x, y + LINE_HEIGHT*i);
}
// show this if you are Casting [or Learning] a spell (not browsing)
// shows <slevel> <smana> <spell_chance>%, each is 1 or 2 digits.
static void print_spell_stats(Short i, Short j, Short lst_top, Short lst_side)
{
  Short x = lst_side+FntCharWidth(' '), y = lst_top;
  Short five = FntCharWidth('0'); // 5 in 160
  Short dx = FntCharsWidth("00 ", 3); // 13 in 160
  spell_type *s_ptr;
  Char tmp_val[10];
  s_ptr = &magic_spell[pymisc.pclass-1][j];
  StrPrintF(tmp_val, "%d", s_ptr->slevel);
  if (s_ptr->slevel > 9) WinDrawChars(tmp_val, 2, x,      y + LINE_HEIGHT*i);
  else                   WinDrawChars(tmp_val, 1, x+five, y + LINE_HEIGHT*i);
  x += dx;
  StrPrintF(tmp_val, "%d", s_ptr->smana);
  if (s_ptr->smana > 9) WinDrawChars(tmp_val, 2, x,      y + LINE_HEIGHT*i);
  else                  WinDrawChars(tmp_val, 1, x+five, y + LINE_HEIGHT*i);
  x += dx;
  StrPrintF(tmp_val, "%d%%", spell_chance(j));
  x += five * (3 - StrLen(tmp_val));
  WinDrawChars(tmp_val, StrLen(tmp_val), x, y + LINE_HEIGHT*i);
}

void show_mana_at_bottom()
{
  Char buf[16];
  StrPrintF(buf, "mana %d ", pymisc.cmana);
  WinDrawChars_rj(buf, StrLen(buf), SCR_WIDTH-4, // was 156-...
		  SCR_HEIGHT-(3*LINE_HEIGHT)/2); // was 141, for 160
}

// caller must promise that 'a' <= c <= 'i'.   (really?  how about 'a'-'z')
static void hit_spell_by_alpha(Char c, Word lst_index)
{
  FormPtr frm;
  ListPtr lst;
  Short s;
  CharPtr label;
  frm = FrmGetActiveForm();
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, lst_index));
  s = c - 'a';
  if (s > spell_ctr) s = spell_ctr; // or is it spell_ctr - 1 ?
  while (s > -1) {
    label = LstGetSelectionText(lst, s);
    if (label && (label[0] == c))
      break;
    s--;
  }
  LstSetSelection(lst, s);
}

extern previous_state curr_state; // replaces last_command & co.
static void cast_spell_if_ok(Short first_spell)
{
  FormPtr frm;
  ListPtr lst;
  Short s;
  CharPtr label;
  frm = FrmGetActiveForm();
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_spell));
  s = LstGetSelection(lst);
  if (s != -1) {
    label = LstGetSelectionText(lst, s);
    if (label) {
      selected_spell = label[0] - 'a';
      selected_spell += first_spell; // add this to make it the right book!
      if (confirmed_spell(selected_spell)) {
	free_spell_select(lst);
	LeaveForm();
	LeaveForm(); // yes, TWICE.
	// also CALL THE SPELL CASTING HELPER now.
	curr_state.spell = selected_spell; // need this for 'again'ing
	if (class[pymisc.pclass].spell == MAGE)
	  cast_helper(selected_spell, -1);
	else
	  pray_helper(selected_spell, -1);
	// and do not reset selected_spell (until we re-enter this form)...
      }
    }
  }
}

void free_spell_select(ListPtr lst)
{
  //  ListPtr lst;
  Short i;
  VoidHand h;
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < spell_ctr ; i++) {
    h = MemPtrRecoverHandle(spell_display[i]);
    if (h) 
      MemHandleFree(h);
  }
  h = MemPtrRecoverHandle(spell_display);
  if (h) 
    MemHandleFree(h);
}

Boolean confirmed_spell(Short spell)
{
  if (magic_spell[pymisc.pclass-1][spell].smana <= pymisc.cmana)
    return true;
  if (class[pymisc.pclass].spell == MAGE)
    return (0 == FrmAlert(LowMageP));
  else
    return (0 == FrmAlert(LowPriestP));
}

Boolean spell_btnsHandleEvent(Short btn_i)
{
  // 8 books, 8 buttons per book, 4 bits per button.
  //  UChar hw_spells[32];
  // Look up by book, by button, and shift/mask 4 bits.
  // Each book gets 
  UChar which_spell;
  ULong j;
  Short first_spell, book;
  UChar *bind_array = my_prefs.hw_spells;
  Char c;
  Boolean is_prayer;

  if (just_browsing_book) return false;
  if (btn_i < HW_hard1Chr || btn_i > HW_findChr) return false;

  j = inventory[selected_book].flags;
  first_spell = bit_pos(&j); // this modifies j

  // prayer book gets high 4 bits, magic book gets low 4 bits.
  is_prayer = (inventory[selected_book].tval == TV_PRAYER_BOOK);

  // 32 uchars: 8 for each of the 4 book levels (beginner, I, II, expert)
  // figure out which book we're looking at by examining first_spell.
  book = (first_spell+1)/8; // 0, 7||8, 16, 24||25 ---> 0, 1, 2, 3.
  bind_array += 8 * book; // skip to the 8 uchars belonging to this book.
  // ok, within this book, get the uchar corresponding to this button press.
  which_spell = bind_array[btn_i];

  // now mask or shift, according to is_prayer, to get increment to firstspell.
  if (!is_prayer)
    which_spell = which_spell & 0x0F;
  else
    which_spell = which_spell >> 4; // unsigned, so high 4 bits become 0s
  if (which_spell == 0) return false; // This Button Is Not Bound.
  else which_spell--;

  // Now, act just like we wrote the graffiti character ('a'+which_spell).
  // (if the spell is not known, it is not in List, thus we will not cast it.)
  c = 'a' + which_spell;
  if (c > 'i') return false; // probably a BUG if this happens.
  hit_spell_by_alpha(c, list_spell);

  // Now, act like we pressed Ok.
  cast_spell_if_ok(first_spell);

  return true;
}


////////////////////////////////////////////////////////////////////////

// This form will have only mage spells in it
// Unlike the book form and the browse/cast form, it might need scrolled.
// (damn)

// Hey - news flash.  This isn't an ok->exit form
// This is a loop until out of new spells form.
// SOOOO we need a "new spells left" display line,
// annnnd we need to be able to subtract lines from the list!!
// so, er, actually this is also the form to double-up for browsing.
extern Boolean missing_a_book;
extern ULong spell_learned;
extern UChar spell_order[MAX_SPELLS+1];
extern Short new_spells, last_known;
static Boolean stats_toggler;
Short gainspell_lines = 9;
Boolean GainSp_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  Short start;
  Char c;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    // This form will ONLY be entered if we have > 0 new_spells possible.
    // (And make sure there are still spells left to learn, eh)
    // Or.. if we are browsing a book.
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    show_spells(frm, list_gain, -1); // -1 to make it consec. starting with a
    gainsp_draw(true);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

    // need to be able to scroll - a mage of "average" ability might
    // have 12 or so LEARNABLE spells at late-teen levels.
    // (though if we're here only to browse a book we never need to scroll)
    // so, I don't know how to catch use of the little arrows in the list.
    // this is a pain.  might have to remember previous topItem
    // and check every penDownEvent,
    // or mess with LstSetDrawFunction which sets drawItemsCallback.
    // scrolling below HAS NOT BEEN TESTED and above HAS NOT BEEN IMPLEMENTED
  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_gain));
    start = lst->topItem;
    switch(e->data.keyDown.chr) {
    case pageUpChr:
      if (LstScrollList(lst, winUp, gainspell_lines)) // (probably 9)
	LstSetSelection(lst, -1);
      // xxxxx ALSO need to reprint the stats alongside.  plan how to deal..
      // heck, could just allocate an array of shorts to reprint from.
      // it's not like we use this form very often and would hate the overhead
      print_spell_ss(frm, list_gain, start,
		     min(start+gainspell_lines, spell_ctr), stats_toggler);
      handled = true;
      break;
    case pageDownChr:
      if (LstScrollList(lst, winDown, gainspell_lines))
	LstSetSelection(lst, -1);
      // xxxxx ALSO need to reprint the stats alongside.
      print_spell_ss(frm, list_gain, start,
		     min(start+gainspell_lines, spell_ctr), stats_toggler);
      handled = true;
      break;
    case 'L':
      hit_button_if_usable(frm, btn_gain_ok);
      handled = true;
      break;
    case 'D': case 27: case '\n':
      hit_button_if_usable(frm, btn_gain_cancel);
      handled = true;
      break;
    case 't':
      hit_button_if_usable(frm, btn_gain_tog);
      handled = true;
      break;
    default:
      c = e->data.keyDown.chr;
      // let user hit the index letter of a spell: In order, with gaps.
      if (c >= 'a' && c <= 'z') {
	hit_spell_by_alpha(c, list_gain); // same utility as casting a spell.
	hit_button_if_usable(frm, btn_gain_ok);
	handled = true;
      }
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_gain_tog:
      print_spell_ss(frm, list_gain, 0, gainspell_lines, stats_toggler);
      stats_toggler = !stats_toggler;
      break;
    case btn_gain_ok:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_gain));
      selected_spell = LstGetSelection(lst);
      if (selected_spell != -1) {
	learn_the_spell(selected_spell, lst);
	/*print_spell_ss(frm, list_gain, 0, gainspell_lines, false);
	// DO NOT LEAVE FORM, just unselect spell + REMOVE it from the list.
	if (new_spells <= 0) {
	  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_gain_ok));
	  CtlHideControl(btn);
	}*/
      }
      /*StrPrintF(buf, "You can learn %d %s.", new_spells,
		((new_spells == 1) ? "spell" : "spells"));
		show_brief_comment(buf);*/
      gainsp_draw(false);
      handled = true;
      break;
    case btn_gain_cancel:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_gain));
      free_spell_select(lst);
      LeaveForm();
      // It should be safe to call this whether we picked 0 or n spells.
      // the original moria did in any event.
      if (!just_browsing_book)
	gain_spell_helper();
      handled = true;
      break;
    }

  default:
    break;
  }
  return handled;
}

void gainsp_draw(Boolean first)
{
  FormPtr frm = FrmGetActiveForm();
  Char buf[40];
  ControlPtr btn;
  print_spell_ss(frm, list_gain, 0, gainspell_lines, just_browsing_book);
  if (!just_browsing_book) {
    if (missing_a_book)
      StrPrintF(buf, "You seem to be missing a book.");
    else {
      StrPrintF(buf, "You can learn %d %s.", new_spells,
		((new_spells == 1) ? "spell" : "spells"));
      if (new_spells <= 0) {
	btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_gain_ok));
	CtlHideControl(btn);
      }
    }
  } else {
    StrPrintF(buf, "Browsing: %s", object_names +
	      object_list[inventory[selected_book].index].name_offset);
    if (first) {
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_gain_ok));
      CtlHideControl(btn);
      btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_gain_tog));
      CtlShowControl(btn);
      // also it would be a good plan to change the title bar.
      FrmCopyTitle(frm, "Browsing...");
      stats_toggler = false; // what it should display next time
    }
  }
  show_brief_comment(buf);
}

// hey, this should be in misc3.c and avoid 'extern', or else the stuff there
// should be here.
//extern ULong spell_learned;
//extern UChar spell_order[MAX_SPELLS+1];
//extern Short new_spells, last_known;
//
// XXX Note:  I have a memory leak here because I am not freeing
// the orphaned spell_display[0].
// if I do, the free_spell_select gets an invalid chunk ptr.
// same thing if I rearrange the strings.  I don't understand why.
// HYPOTHESIS: people cannot play enough moria on one set of batteries
// in one session to make this a serious leak, so just leave it in.
static void learn_the_spell(Short j, ListPtr lst)
{
  //  VoidHand h;
  //  Char *tmp;
  new_spells--; // decrement number of spells avail
  spell_learned |= 1L << spells[j]; // learn it
  spell_order[last_known++] = spells[j]; // remember order it was learned in
  //  return;
  LstSetListChoices(lst, NULL, 0);
  //  tmp = spell_display[0];
  //  h = MemPtrRecoverHandle(spell_display[0]);
  //  if (h) MemHandleFree(h); // delete the unneeded string
  for (; j < spell_ctr-1; j++) {
    spells[j] = spells[j+1]; // delete this spell from array
    spell_display[j] = spell_display[j+1]; // delete this spell from List
  }
  //  spell_display[spell_ctr-1] = tmp; // will this work?  NO.
  spell_ctr--;
  // also, redraw the freaking list,
  // and the numbers next to it.
  LstSetListChoices(lst, spell_display, spell_ctr);
  selected_spell = -1;
  LstSetSelection(lst, -1);
  LstDrawList(lst);
}
// Note - there's a fatal error in there somewhere.  No real surprise
// it's just bubble gum and baling wire.

