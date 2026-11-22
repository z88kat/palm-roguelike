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
// why is IsDigit (CharAttr.h) not working for 3.1??


extern owner_type owners[MAX_OWNERS]; // lock.c

Short STORE_LINES = 10;
#define MAX_STORE_LINES 14 /* xxx If screen height increases, check on this */
// for multiplication of current-tens:
#define Five_Or_Ten (my_prefs.store_is_compacted ? STORE_LINES : STORE_LINES/2)
// for division of list row index:
#define Two_Or_One  (my_prefs.store_is_compacted ? 1 : 2)

static void store_draw() SEC_6;

Short in_store; // Need to know which store we are in!
Short store_item; // so that Haggle knows what we're bidding on
Short store_current_tens;
Boolean store_youbuying; // so Haggle knows why we're bidding
Short test = -1;
static Boolean isgold = false;
static void store_gold_owner(Boolean forcegold)
{
  Char buf[40];

  if (forcegold || isgold) {
    StrPrintF(buf, "(You have: %ld gold pieces)", pymisc.au);
    isgold = false;
  } else {
    StrPrintF(buf, "%s", owners[store[in_store].owner].owner_name);
    isgold = true;
  }
  show_brief_comment(buf);
}
// show weight of cur. selected store item (if in range) -- dan_van 19 Jul 2001
static void show_store_item_weight()
{
  FormPtr frm;
  ListPtr lst;
  Short store_item, total_weight;
  inven_record *storeinv;
  inven_type sold_obj;
  Char buf[40];
  //get the selected item:
  frm = FrmGetActiveForm();
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
  store_item = LstGetSelection(lst) / Two_Or_One;
  // make sure it's in range
  if (store_item >= 0 && store_item < STORE_LINES) {
    storeinv = store_inven[in_store];
    store_item += Five_Or_Ten * store_current_tens;
    take_one_item(&sold_obj, &storeinv[store_item].sitem); // 1-count item
    total_weight = sold_obj.weight * sold_obj.number;

    StrPrintF(buf, "Weight: %d.%d lb", total_weight / 10, total_weight % 10);
    show_brief_comment(buf);
  } else {
    isgold = !isgold; // kludge; don't toggle this time.
    store_gold_owner(false);
  }
}


#define STORE_NONE -1
extern Boolean in_a_store; 

Char *store_display[MAX_STORE_LINES];
extern Boolean Version_GE_OS35;
Boolean Store_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  //  CharPtr label;
  Short objid, i;
    // CALLBACK_PROLOGUE

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    /* this must be AFTER draw form */
    // [draw stuff]
    isgold = true;//false;
    store_current_tens = 0; // first screen
    store_display[0] = NULL; // to let store_list_tens know not to free it
    store_list_tens(0);
    FrmCopyTitle(frm, owners[store[in_store].owner].store_name);
    store_gold_owner(false); // this is done by store_list_tens now!
    //    store_btns(frm);
    // if there are <= 1 screen full of items in the store, DISABLE BUTTONS
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

#ifdef I_AM_COLOR
  case menuCmdBarOpenEvent:
    // this will only work if we are on an OS3.5 or greater device.
    if (Version_GE_OS35) {
      MenuCmdBarAddButton(2, bmp_cmd_compact, menuCmdBarResultMenuItem,
			  menu_stoCompact, NULL);
      MenuCmdBarAddButton(1, (my_prefs.store_is_haggling ? bmp_cmd_hglOff :
			                                   bmp_cmd_hgl),
			  menuCmdBarResultMenuItem,
			  menu_stoHaggle, NULL);
    }
    return false;
#endif

  case menuEvent:
    MenuEraseStatus(NULL); /* clear menu status from display */
    switch(e->data.menu.itemID) {
    case menu_stoCompact:
      if (my_prefs.store_is_compacted)
	store_current_tens *= 2; // the item on ith page moves to 2*ith page
      else
	store_current_tens /= 2; // the item on ith page moves to i/2th page
      my_prefs.store_is_compacted = !my_prefs.store_is_compacted;
      writeMoriaPrefs();
      // need to redraw! ....
      store_list_tens(0);
      break;
    case menu_stoHaggle:
      my_prefs.store_is_haggling = !my_prefs.store_is_haggling;
      writeMoriaPrefs();
      break;
    }

  case penDownEvent:
    if (in_brief_comment(e->screenX, e->screenY)) {
      store_gold_owner(false);
      handled = true;
      break;
    }
    break;

  case lstSelectEvent:
    show_store_item_weight();
    handled = true;
    break;

  case keyDownEvent:
    // hardware button -- or else graffiti.
    frm = FrmGetActiveForm();
    objid = -1;
    switch(e->data.keyDown.chr) {
    case pageUpChr: case '<':
      store_list_tens(-1);
      handled = true;
      break;
    case pageDownChr: case '>':
      store_list_tens(1);
      handled = true;
      break;
    case ' ':
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
      i = LstGetSelection(lst);
      if (i == -1) i = 0;
      else i += Two_Or_One;
      if (i >= STORE_LINES) {
	store_list_tens(1);
	i = i % STORE_LINES;
      }
      LstSetSelection(lst, i);
      show_store_item_weight(); // hm.. this duplicates the getselection work..
      handled = true;
      break;
    case 'b': case 'B': objid = btn_st_buy;  break;
    case 's': case 'S': objid = btn_st_sell; break;
    case 'x': case 'X': case '9': case '0': case 27: objid=btn_st_exit; break;
    }
    if (objid != -1) {
      hit_button_if_usable(frm, objid);
      handled = true;
    }
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_st_buy:
      frm = FrmGetActiveForm();
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
      store_item = LstGetSelection(lst) / Two_Or_One;
      // make sure it's in range
      if (store_item >= 0 && store_item < STORE_LINES) {
	store_item += Five_Or_Ten * store_current_tens;
	if (store_item < store[in_store].store_ctr) {
	  store_youbuying = true;
	  if (store_purchase_init(in_store, store_item))
	    FrmPopupForm(HaggleForm);
	}
      }
      handled = true;
      break;
    case btn_st_prev:
      store_list_tens(-1);
      handled = true;
      break;
    case btn_st_next:
      store_list_tens(1);
      handled = true;
      break;
    case btn_st_sell:
      //      frm = FrmGetActiveForm();
      //      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
      //      store_item = LstGetSelection(lst);
      //      if (store_item >= 0 && store_item < STORE_LINES) {
      store_youbuying = false;
      if (inven_ctr != 0)
	FrmPopupForm(InvForm);
      else if (equip_ctr != 0)
	FrmPopupForm(EquForm);
      // The rest will be in exiting the InvForm, I guess.
      //      }
      handled = true;
      break;
    case btn_st_exit:
      free_store_list(FrmGetActiveForm());
      LeaveForm();
      in_store = STORE_NONE;
      in_a_store = false;
      handled = true;
      // Need to REDRAW our gold!
      print_stats(STATS_GOLD);
      show_messages(); // to redraw stats.
      break;
    }
    break;

  default:
    break;
  }

    // CALLBACK_EPILOGUE

  return handled;
}



////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
static void store_draw() // call this after drawing the store list.
{
  FormPtr frm = FrmGetActiveForm();
  Char buf[10];
  Short i, iitem, j, len, list_edge, x, y = ROW_1, five = FntCharWidth('0');
  Long v;
  RectangleType r;
  inven_record *storeinv;

 // print digit between < - and + > buttons
  StrPrintF(buf, "%d", store_current_tens);
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_st_prev), &r);
  x = r.topLeft.x + r.extent.x;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_st_next), &r);
  WinDrawChars_ctr(buf, 1, (x + r.topLeft.x)/2, r.topLeft.y);

  // figure out where the right edge of the list is
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, list_st), &r);
  list_edge = r.topLeft.x + r.extent.x + 1; // = 126;

  // erase the rectangle (126,15) to (156,15+110) // 126, 15, 30, 110.
  RctSetRectangle(&r, list_edge, ROW_1, 6*five, STORE_LINES*LINE_HEIGHT);
  WinEraseRectangle(&r, 0);

  storeinv = store_inven[in_store];
  for (i = 0, j = 0 ; i < Five_Or_Ten ; i++, j++) {
    iitem = i + store_current_tens * Five_Or_Ten;
    // Print the price.
    if (iitem < store[in_store].store_ctr) {
      v = storeinv[iitem].scost;
      if (v <= 0) {
	Long value = -v;
	value = value * chr_adj() / 100;
	if (value <= 0)
	  value = 1;
	StrPrintF(buf, "%ld", value);
	len = StrLen(buf);
	WinDrawChars(buf, len, list_edge + (6-len)*five, y);
      } else {
	StrPrintF(buf, "%ld", v);
	len = StrLen(buf);
	WinDrawInvertedChars(buf, len, list_edge + (6-len)*five, y);
      }
    }
    y += LINE_HEIGHT;
    if (!my_prefs.store_is_compacted)
      y += LINE_HEIGHT; // don't print a price on next line.
  }

  // There is no selection in the list now, so make sure no Weight is shown.
  /*
  isgold = !isgold; // (kludge; don't toggle this time.)
  store_gold_owner(false);
  */

  // Also, let the user know when the store is full, if on the last page.
  if (store[in_store].store_ctr >= STORE_INVEN_MAX)
    // Store is full!
    if (store_current_tens == (store[in_store].store_ctr - 1) / Five_Or_Ten)
      // We are on the last page!
      WinDrawChars("*** store is full ***", 21, 2,
		   ROW_1 + (STORE_LINES-1)*LINE_HEIGHT);
}
void store_list_tens(Short plus)
{
  Short i, j, iitem, uncut_len;
  Char tmp_val[160], ctmp=' ';
  // length_str gets set to the # of chars that will fit in 124 pixels
  // width_str gets set to the # of pixels that those chars will use
  Int width_str, length_str;
  Boolean fits;
  ListPtr lst;
  FormPtr frm = FrmGetActiveForm();
  inven_type *i_ptr;
  inven_record *storeinv;
  RectangleType r;
  Short list_width;
  ControlPtr btn;

  storeinv = store_inven[in_store];

  // store_ctr == #items in store.  we're trying to scroll but nothing happens:
  if (plus != 0 && store[in_store].store_ctr <= Five_Or_Ten)
    return; 

  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, list_st), &r);
  list_width = r.extent.x; // = 126;

  store_current_tens += plus;
  // if we overflow or underflow, wrap-around.
  if (store_current_tens * Five_Or_Ten >= store[in_store].store_ctr) // "..."
    store_current_tens = 0; // go to first screen
  else if (store_current_tens < 0) // go to last screen
    //store_current_tens = store[in_store].store_ctr / Five_Or_Ten; // Nope,
    store_current_tens = (store[in_store].store_ctr - 1) / Five_Or_Ten; // Yep

  // Also, decide whether we can free the old store_display[i]'s.
  if (store_display[0] != NULL)
    free_store_list(frm);

  for (i = 0, j = 0 ; i < Five_Or_Ten ; i++, j++) {
    //    c = i + 'a';
    //    StrPrintF(tmp_val, "%c) ", c);
    iitem = i + store_current_tens * Five_Or_Ten;
    i_ptr = &storeinv[iitem].sitem;
    if (iitem >= store[in_store].store_ctr) { // yes, I really do mean >=
      if (store[in_store].store_ctr < STORE_INVEN_MAX)
	StrPrintF(tmp_val, "..."); // because store_ctr is "first empty space"
      else StrPrintF(tmp_val, "   ");
      uncut_len = length_str = StrLen(tmp_val); // max chars to allow
    } else {
      objdes(tmp_val, i_ptr, true); // was tmpval+3
      uncut_len = length_str = StrLen(tmp_val); // max chars to allow
      // Truncate if too wide for the screen..... we only have 124 pixels...
      width_str = list_width-2; //124-2;
      FntCharsInWidth(tmp_val, &width_str, &length_str, &fits);
      ctmp = tmp_val[length_str];
      tmp_val[length_str] = '\0';
    }
    // ok, now like put it somewhere.
    // XXXX  MEMORY LEAK  MEMORY LEAK  MEMORY LEAK  MEMORY LEAK:
    store_display[j] = (Char *) md_malloc(1 + length_str);
    StrCopy(store_display[j], tmp_val);
    // ok, and if we're not compact, print the leftovers
    if (!my_prefs.store_is_compacted) {
      j++;
      if (uncut_len > length_str) {
	Char *p;
	tmp_val[length_str] = ctmp;
	p = &tmp_val[length_str];
	store_display[j] = (Char *) md_malloc(1 + StrLen(p));
	StrCopy(store_display[j], p);	
      } else {
	store_display[j] = (Char *) md_malloc(2);
	StrPrintF(store_display[j], " ");
      }
    }
    // ok, all done (compacted or otherwise).
  }

  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
  LstSetListChoices(lst, store_display, STORE_LINES);
  LstSetSelection(lst, -1);
  LstDrawList(lst);

  // Also, if you have nothing to sell, disable the sell button.
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_sell));
  if (inven_ctr > 0 || equip_ctr > 0) CtlShowControl(btn);
  else CtlHideControl(btn);

  store_draw(); // print the prices!!!
}

/*
 *  Free some space that we used in the inventory-selection form
 */
void free_store_list(FormPtr frm)
{
  ListPtr lst;
  Short i;
  VoidHand h;
  if (store_display[0] == NULL) return;
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_st));
  LstSetListChoices(lst, NULL, 0);
  for (i = 0 ; i < STORE_LINES ; i++) {
    h = MemPtrRecoverHandle(store_display[i]);
    if (h) 
      MemHandleFree(h);
    store_display[i] = NULL;
  }
}


// store_item should never be -1 in haggle
static void haggle_print_obj()
{
  Char buf[160], *p;
  Word wwlen, wwpixels= SCR_WIDTH-8;//156-2-2;
  Short x, y, lines = 0;
  inven_type sold_obj; //, *i_ptr;
  inven_record *storeinv;
  RectangleType r;
  FormPtr frm = FrmGetActiveForm();
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, field_hag), &r);

  if (store_youbuying) {
    StrPrintF(buf, "Your offer:  "); // was drawn at 25,80 for 160x160...
    WinDrawChars_rj(buf, StrLen(buf), r.topLeft.x, r.topLeft.y);
    StrPrintF(buf, "Buying: ");
    p = buf + StrLen(buf);
    storeinv = store_inven[in_store];
    take_one_item(&sold_obj, &storeinv[store_item].sitem); // 1-count item
    objdes(p, &sold_obj, true);
  } else {
    StrPrintF(buf, "Asking price:  ");
    WinDrawChars_rj(buf, StrLen(buf), r.topLeft.x, r.topLeft.y);
    StrPrintF(buf, "Selling: ");
    take_one_item(&sold_obj, &inventory[store_item]); // copy item, set count=1
    p = buf + StrLen(buf);
    objdes(p, &sold_obj, true);
  }
  // All the News that Fits, We Print.
  // Actually I don't expect there to be more than two lines.
  x = y = 2;
  p = buf;
  wwlen = 0;
  while (lines < 3 && StrLen(p) > 0) {
    wwlen = FntWordWrap(p, wwpixels);
    WinDrawChars(p, wwlen, x, y);
    p += wwlen;
    y += LINE_HEIGHT;
    lines++;
  }
  StrPrintF(buf, "You have: %ld gold pieces", pymisc.au);
  WinDrawChars(buf, StrLen(buf), x, y);
  y += LINE_HEIGHT + 2;
  WinDrawGrayLine(0, y, SCR_WIDTH, y);
}

void haggle_form_enable_autoinc(Short incr)
{
  Char tmp[10];
  FormPtr frm;
  ControlPtr btn;
  frm = FrmGetActiveForm();
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_incr));

  if (incr == 0) {
    CtlHideControl(btn);
    return;
  }

  if (incr > 0)
    StrPrintF(tmp, "[+%d]", incr);
  else
    StrPrintF(tmp, "[-%d]", 0 - incr);
  CtlSetLabel(btn, tmp);
  CtlShowControl(btn);
}

void store_form_boot()
{
  FormPtr frm;
  ControlPtr btn;
  frm = FrmGetActiveForm();
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_buy));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_sell));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_prev));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_next));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_st_exit));
  CtlSetLabel(btn, "OUT!!");
  CtlShowControl(btn);
}
void haggle_form_boot(Boolean kick_out, Char *done)
{
  FormPtr frm;
  ControlPtr btn;
  frm = FrmGetActiveForm();
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_ok));
  CtlHideControl(btn);
  btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_exit));
  if (kick_out) {
    CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_hag_boot));
    CtlShowControl(btn);
    return;
  } else {
    CtlSetLabel(btn, done);
    CtlShowControl(btn);
  }
}

void haggle_form_setfield(Long amt)
{
  FormPtr frm;
  FieldPtr fld;
  VoidHand h;
  VoidPtr p;
  Char buf[20];

  buf[0] = '\0';
  if (amt) StrPrintF(buf, "%ld", amt);
  frm = FrmGetActiveForm();
  fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_hag));
  h = (VoidHand) FldGetTextHandle(fld);
  if (!h)
    h = MemHandleNew(12 * sizeof(Char));
  FldSetTextHandle(fld, (Handle) 0);
  p = MemHandleLock(h);
  StrNCopy(p, buf, 12);
  MemHandleUnlock(h);
  FldSetTextHandle(fld, (Handle) h);
  FldDrawField(fld);

}

void haggle_draw()
{
#ifndef I_AM_OS_2
  if (IsVGA) {
    FormPtr frm = FrmGetActiveForm();
    RectangleType r;
    FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, field_hag), &r);
    HAG_Y = r.topLeft.y; // y pos of the haggle field
  }
#endif
  // print the name of the thing being haggled
  haggle_print_obj();
}

Boolean Haggle_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
  FieldAttrType fieldattr;
  Word chr;
  VoidHand h;
  VoidPtr p;
  CharPtr textp;
  //  Char buf[20];
  Long gold, autobid;
  //  Boolean ok = true;
  Short outcome;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    // allocate memory for field
    h = MemHandleNew(sizeof(Char) * 12);
    p = MemHandleLock(h);
    ((Char *)p)[0] = '\0';
    MemPtrUnlock(p);
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_hag));
    FldSetTextHandle(fld, (Handle) h);
    // also set title
    //    if (store_youbuying) FrmCopyTitle(frm, "Your offer?");
    //    else FrmCopyTitle(frm, "Asking price?");
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    FrmSetFocus(frm, FrmGetObjectIndex(frm, field_hag));
    haggle_draw();
    if (store_youbuying)
      purchase_haggle_init();
    else
      sell_haggle_init();
    // haggle_form_enable_autoinc(0); // it starts out unusable anyway.
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case keyDownEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_hag));
    FldGetAttributes(fld, &fieldattr);
    // Now, either !fieldattr.hasFocus OR chr == '\n'...
    chr = e->data.keyDown.chr;
    switch(chr) {
    case '\n':
      if (fieldattr.hasFocus) FldReleaseFocus(fld);
      // If submit is enabled, do that, else try ok, else boot.
      if (hit_button_if_usable(frm, btn_hag_ok)) {
	handled = true;
	break;
      } // else FALL THROUGH to try the exit button!!
    case 'x': case 'X':
      if (fieldattr.hasFocus) FldReleaseFocus(fld);
      if (!hit_button_if_usable(frm, btn_hag_exit))
	hit_button_if_usable(frm, btn_hag_boot);
      handled = true;
      break;
    case '=':
      if (fieldattr.hasFocus) FldReleaseFocus(fld);
      hit_button_if_usable(frm, btn_hag_all);
      handled = true;
      break;
    case '+': case '-':
      if (fieldattr.hasFocus) FldReleaseFocus(fld);
      hit_button_if_usable(frm, btn_hag_incr);
      handled = true;
      break;
    default:
      if (fieldattr.hasFocus) {
	// filter out all non-handled, i.e. non-numeric, characters.
	handled = true;
	chr = e->data.keyDown.chr;
	if ( IsDigit(GetCharAttr(), chr) || (chr == backspaceChr)       ||
	     (chr == leftArrowChr)       || (chr == rightArrowChr) )
	  handled = false;
      } else {
	// automatically grab focus when numbers are typed
	if ( IsDigit(GetCharAttr(), chr) )
	  FrmSetFocus(frm, FrmGetObjectIndex(frm, field_hag));
      }
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_hag));
    autobid = 0;
    switch(e->data.ctlSelect.controlID) {
    case btn_hag_incr:
      // increment the current offer
      autobid = haggle_get_incr();
      handled = true;
      // fall through
    case btn_hag_all:
      // match the current offer
      if (!handled)
	autobid = haggle_get_ask();
      haggle_form_setfield(autobid);
      // I think that will do the right thing.
      //      FrmDrawForm(frm);
      handled = true;
      break;
    case btn_hag_ok:
      textp = FldGetTextPtr(fld); // if (textP) ...
      // Well, the only problem with StrAToI is that I want StrAToL !!
      // If the string has length greater than 4, I may have to get funky.
      gold = StrAToI(textp);
      haggle_form_setfield(0);
      if (store_youbuying)
	outcome = purchase_haggle_endturn(gold);
      else
	outcome = sell_haggle_endturn(gold);
      switch (outcome) {
      case HAG_ONGOING:
	break;
      case HAG_BOUGHT:
	// need the agreed-on price
	if (store_youbuying)
	  store_purchase_end(true);
	else
	  store_sell_end(true);
	break;
      case HAG_REJECTED:
	haggle_form_setfield(0);
	haggle_form_boot(false, "Quit");
	break;
      case HAG_BOOTED:
	haggle_form_setfield(0);
	haggle_form_boot(true, "blah");
	break;
      }
      handled = true;
      break;
    case btn_hag_exit: // cancelled the haggle or succeeded in purchase
      FldReleaseFocus(fld);
      FldSetSelection(fld, 0, 0);
      FldFreeMemory(fld);
      LeaveForm();
#ifndef I_AM_OS_2
      if (IsVGA) PrvMoveForm(true);
      else
#endif
	// also reprint items!  only needed if you succeed in buying something.
	store_list_tens(0);
      store_gold_owner(true); // reprint what gold you have
      reset_haggle_foo(); // clear globals
      handled = true;
      break;
    case btn_hag_boot: // got kicked out
      FldReleaseFocus(fld);
      FldSetSelection(fld, 0, 0);
      FldFreeMemory(fld);
      LeaveForm();
      // now leave the store form too!
      free_store_list(FrmGetActiveForm());
      LeaveForm();
      in_store = STORE_NONE;
      in_a_store = false;
      reset_haggle_foo(); // clear globals
      handled = true;
      // Need to REDRAW our gold!
      print_stats(STATS_GOLD);
      show_messages(); // to redraw stats.
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}
