#include "iRogue.h" // for my EMDASH and FOODCHAR
Boolean have_read_color = false;
Boolean have_read_tiles = false;

#ifdef I_AM_COLOR
DmOpenRef MemoDB;
// this seems weird to me, but what the heck.
typedef struct {
  Char note;           // null terminated
} MemoDBRecordType;
typedef MemoDBRecordType * MemoDBRecordPtr;
#define memoDBType 'DATA'

// So.  what are the symbols.  From explain_it():
// A-Z a-z ? ! * FOODCHAR / ] ) = , . + # | EMDASH ^ % @   (and ' ')
// HERE IS WHAT WE DO to look up the color for Char c.
// If c == EMDASH then c = '-'
// If c == FOODCHAR then c = ':'
// Take the ASCII representation and subtract 33; that is your index.
/*  THese are things I'm not using at the moment after all:
  // I will also take the unused symbol '"' and use it for PIE SLICES.
  // I will also take the unused symbol '_' and use it for BACKGROUND.
  // I will also take the unused symbol '`' and use it for FOREGROUND secretly.
  //#define FG_COL_IX    ('`' - 33)
  //#define BG_COL_IX    ('_' - 33)
 */
// These are color symbols that get special cased in some way:
#define FOOD_COL_SYM ':'
#define DASH_COL_SYM '-'
#define NUM_SYMBOLS 92
IndexedColorType symbol_colors[NUM_SYMBOLS];
IndexedColorType black, white;
static Short grok_hex(Char *buf, Short len);
#endif // I_AM_COLOR


/* 
 * This is ONLY called if the OS version is at least 3.5.
 * If a memo entitled "iRogue-RGB" exists, read from it into symbol_colors
 * and set IsColor = true.  (Any symbol missing from the memo will be black.)
 * Otherwise IsColor is false (and symbol_colors remains black).
 */
#ifdef I_AM_COLOR
void look_for_memo()
{
  UInt recordNum;
  VoidHand recH;
  UInt mode = dmModeReadOnly;
  Char magic_title[20];
  Short title_len, memo_len, i, j, symbol;
  Char *memo, c;
  RGBColorType rgbcolor;
  IndexedColorType icolor;
  //  Boolean no_foreground = true;
  //  Boolean symbol_set[NUM_SYMBOLS];

  // initialize the symbol_colors array
  have_read_color = false;
  rgbcolor.r = rgbcolor.g = rgbcolor.b = 255; // white
  white = WinRGBToIndex(&rgbcolor);
  rgbcolor.r = rgbcolor.g = rgbcolor.b = 0; // black
  black = WinRGBToIndex(&rgbcolor);
  for (i = 0 ; i < NUM_SYMBOLS ; i++) {
    symbol_colors[i] = black;
    //    symbol_set[i] = false;
  }
  //  symbol_colors[BG_COL_IX] = white;
  //  symbol_set[BG_COL_IX] = true;

  // FIND KITTEN^H^H^H^H^H^HMEMO
  // first, what is our magic title:
  StrPrintF(magic_title, "iRogue-RGB%c", linefeedChr);
  title_len = StrLen(magic_title);
  // try to open the memo database
  if (!(MemoDB = DmOpenDatabaseByTypeCreator(memoDBType, sysFileCMemo, mode))){
    have_read_color = false;
    return;
  }
  recordNum = 0;
  // get a record (stop if we run out)
  while (!have_read_color &&
	 (recH = DmQueryNextInCategory(MemoDB, &recordNum, dmAllCategories))) {
    MemoDBRecordPtr memoRecP = MemHandleLock (recH);
    memo = &(memoRecP->note);
    // Does the title match our magic title?
    if (0 == StrNCompare(memo, magic_title, title_len)) {
      have_read_color = true;
      memo_len = StrLen(memo);
      i = title_len; // skip past the title!
      while (i < memo_len - 1) {
	Char tmp;
	symbol = memo[i]; // this is the symbol whose color we'll define
	tmp = EMDASH;
	if (memo[i] == tmp) symbol = DASH_COL_SYM;
	tmp = FOODCHAR;
	if (memo[i] == tmp) symbol = FOOD_COL_SYM;
	symbol -= 33; // now it is an index into symbol_colors[]
	if (symbol >= 0 && symbol < NUM_SYMBOLS) {
	  // ok now skip some (probably) whitespace to get to the rrggbb
	  Boolean found = false;
	  for ( i = i+1 ; i < memo_len - 1 ; i++) {
	    c = memo[i];
	    if (Is_Hex(memo[i])) { found = true; break; }
	    if (memo[i] == linefeedChr) { i++; break; }
	  }
	  if (!found) continue; // we've hit end of line or, maybe, EOF
	  if (memo_len < i + 6) break; // we'll hit EOF before end of rrggbb
	  // Read rrggbb color value.  First make sure they're all hexy.
	  for (j = 0 ; j < 6 ; j++)
	    if (!Is_Hex(memo[i+j]))
	      found = false;

	  // Ok if they're all hexy then understand it as a color and set it.
	  if (found) {
	    rgbcolor.r = grok_hex(&memo[i], 2);
	    rgbcolor.g = grok_hex(&memo[i+2], 2);
	    rgbcolor.b = grok_hex(&memo[i+4], 2);
	    icolor = WinRGBToIndex(&rgbcolor);
	    symbol_colors[symbol] = icolor;  // HOORAY!
	    //	    symbol_set[symbol] = true;
	    i += 6;
	    // A little extra stuff to make sure black bg gets white fg.
	    //	    if ((symbol == BG_COL_IX) && !symbol_set[FG_COL_IX])
	    //	      if ((rgbcolor.r + rgbcolor.g + rgbcolor.b < 128*3))
	    //		symbol_colors[FG_COL_IX] = white;
	  } // else the rrggbb is garbage and we skip to "Skip to the newline".
	} // else it's some freakish character we don't care about
	// We've sucked all the juice out of this line.  Skip to the newline
	while ((i < memo_len - 1) && (memo[i] != linefeedChr))
	  i++;
	if (memo[i] == linefeedChr) i++;
      } // end while(not EOF)
    } // else memo does not match, try the next memo
    MemHandleUnlock(recH);
    recordNum++;
  } // end while(records)
  DmCloseDatabase(MemoDB);
  /*
   * DONE READING MEMO
   * At this point either IsColor is true, meaning we found the memo and,
   * unless user is taunting us, set some entries of symbol_colors[];
   * or it is false, meaning we did not find a color definition memo.
   */
  // Oh yeah if we have a non-black fg, need to fix the defaulting symbols.
  //  for (i = 0 ; i < NUM_SYMBOLS ; i++)
  //    if (!symbol_set[i]) // symbol_set[BG] is always true to avoid a duh.
  //      symbol_colors[i] = symbol_colors[FG_COL_IX];
  /* Ok - since this is only called when color is turned ON (like,
   * either initially or when the user turns on that preference),
   * this is also a good place to change foreground-background color
   * maybe.  I might regret this in other forms, though.
   */

}
#endif // I_AM_COLOR


#ifdef I_AM_COLOR
// lacking a scanf function, and being uncertain whether StrAToI likes hex,..
// Requires:  There are at least len characters in buf, and those len
//            characters satisfy Is_Hex (that is they are 0-9a-fA-F)
static Short grok_hex(Char *buf, Short len)
{
  Short i, val;
  Short total = 0;
  for (i = 0 ; i < len ; i++) {
    total *= 16;
    if (buf[i] >= '0' && buf[i] <= '9')
      val = buf[i] - '0';
    else if (buf[i] >= 'a' && buf[i] <= 'f')
      val = 10 + (buf[i] - 'a');
    else if (buf[i] >= 'A' && buf[i] <= 'F')
      val = 10 + (buf[i] - 'A');
    else // silly rabbit, this is not hex
      val = 0;
    total += val;
  }
  return total;
}
#endif // I_AM_COLOR
// This might be pretty slow.  Maybe should inline it,
// Also compare how long it takes w/o bounds check.
// If inlined this would go in put_char_at() in lib_curses.c
// put_char_at is called only by refresh() so the Push and Pop should go there.
#ifdef I_AM_COLOR
IndexedColorType get_color(Char c)
{
  IndexedColorType ict = black;
  Short i;
  Char tmp;
  tmp = EMDASH;
  if (c == tmp) c = DASH_COL_SYM;
  tmp = FOODCHAR;
  if (c == tmp) c = FOOD_COL_SYM;
  i = c - 33;
  // Let's have some bounds checking here.
  if (i >= 0 && i < NUM_SYMBOLS)
    ict = symbol_colors[i];
  if (my_prefs.black_bg && ict == black)
    return white;
  else if (!my_prefs.black_bg && ict == white)
    return black;
  else
    return ict;
}

void set_color(Char c, IndexedColorType ict)
{
  Short i;
  Char tmp;
  tmp = EMDASH;
  if (c == tmp) c = DASH_COL_SYM;
  tmp = FOODCHAR;
  if (c == tmp) c = FOOD_COL_SYM;
  i = c - 33;
  // inverting is screwed up, not sure how to fix.
  if (i >= 0 && i < NUM_SYMBOLS)
    symbol_colors[i] = ict;
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//                           Tiles
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
extern DmOpenRef RogueTileDB;

// This will set RogueTileDB and have_read_tiles,
// if it can find a database that looks like the right thing.
void look_for_tiles()
{
  LocalID dbID;
  ULong supported_depths, depth;
  Short i;

  if (RogueTileDB!=NULL) {
    DmCloseDatabase(RogueTileDB);
    RogueTileDB = NULL;
    have_read_tiles = false;
  }
  // Find the tiles DB, if it exists
  if (0 == (dbID = DmFindDatabase(0, RogueTileDBName))) return;
  if (0 == (RogueTileDB = DmOpenDatabase(0,dbID,dmModeReadOnly))) return;
  // Make sure there are "enough" tiles in it..
  if (DmNumResources(RogueTileDB) < END_TILE - START_TILE) {
    DmCloseDatabase(RogueTileDB);
    RogueTileDB = NULL;
    return;
  }    
  have_read_tiles = true; // yay

  // try setting the screen depth

  WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL,
		&supported_depths, NULL); // all allowed settings.
  // supported_depths :==  2^(depth-1)   for each supported value of 'depth' 
  // e.g. 2^(4-1) + 2^(2-1) + 2^(1-1) = 11, depth == { 4 or 2 or 1 }, 3.5-en-ez
  depth = supported_depths << 1;
  i = 0; 
  while (depth > 1) {
    depth = depth >> 1;
    i++;
  }
  depth = i;
  WinScreenMode(winScreenModeSet, 0, 0, &depth, 0);

}


/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
// adding some stuff so that you can edit color w/in the app.
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/
/*************************************************************************/

RGBColorType rgbcolor;
IndexedColorType black, white;
//Boolean bg_is_white = true;

Boolean supportUIPickColor = true; /* this should be set at startup, to check
                                      for cranky prisms, but it's not, yet. */

Short symbol_index;
// you can rearrange the chars in this string in any order you like:
UChar *colored_symbols = "!#%)*+\x97/\xb0=?]^|abcdefghijklmnopqrstuvxyzABCDEFGHJKLMNOQRSTUVZ@.";
//IndexedColorType *c_colors = NULL;
// same length as colored_symbols, please:
IndexedColorType c_colors[61];


#define X_0 1
#define Y_0 1
//#define COLOR_COLS 20
#define COLOR_COLS 10
#define dX (160/COLOR_COLS)
//#define dY 11
#define dY 20

void write_color(Short symbol_index, IndexedColorType icolor)
{
  c_colors[symbol_index] = icolor;
}
void read_color()
{
  Short i, len = StrLen(colored_symbols);
  for (i = 0 ; i < len ; i++)
    c_colors[i] = get_color(colored_symbols[i]);
}

// 160 - 23
#define COLOR_BTN_Y (160-23)
//#define COLOR_BTN_OK_X 55
//#define COLOR_BTN_CANCEL_X 105
#define COLOR_BTN_CANCEL_X 80

void draw_ascii()
{
  Short i, x=X_0, y=Y_0;
  IndexedColorType mycolor;
  Short max_ascii = StrLen(colored_symbols);
  RectangleType r;
  RctSetRectangle(&r, 0, 0, 160, 160-22);
  WinPushDrawState();
  //  if (!bg_is_white)
  if (my_prefs.black_bg)
    WinSetBackColor(black);
  WinEraseRectangle(&r, 0);
  for (i = 0 ; i < max_ascii ; i++) {
    if (i && (i % COLOR_COLS == 0)) {
      x = X_0;
      y += dY;
    }
    mycolor = c_colors[i];
    if (mycolor == black && my_prefs.black_bg) mycolor = white;
    WinSetTextColor(mycolor);
    WinDrawChars(colored_symbols + i, 1, x, y);
    x += dX;
  }
  WinPopDrawState();

  //  WinDrawChars("invert", 6, 1, 160-22);
  //  WinDrawChars("background", 10, 1, 160-11);
  //  WinDrawChars("Ok", 2, 75, 160-(33/2));
  WinDrawChars("Ok", 2, 35, 160-(33/2));
  WinDrawChars("Cancel", 6, 110, 160-(33/2));
  WinDrawLine(0, COLOR_BTN_Y, 160, COLOR_BTN_Y);
  //  WinDrawLine(COLOR_BTN_OK_X, COLOR_BTN_Y, COLOR_BTN_OK_X, 160);
  WinDrawLine(COLOR_BTN_CANCEL_X, COLOR_BTN_Y, COLOR_BTN_CANCEL_X, 160);
}

void update_drawing(Short i)
{
  Short x=X_0, y=Y_0, row, col;
  if (i >= StrLen(colored_symbols)) return; // shouldn't happen.
  WinPushDrawState();
  //  if (!bg_is_white)
  if (my_prefs.black_bg)
    WinSetBackColor(black);
  
  row = i / COLOR_COLS;
  col = i % COLOR_COLS;
  x += col * dX;
  y += row * dY;
  WinSetTextColor(c_colors[i]);

  WinDrawChars(colored_symbols + i, 1, x, y);
  WinPopDrawState();
}

const Char *color_title;
// pass this in as an arg to explain_ch!  ha!
void FrmCopyTitle_helper(const Char *new_title)
{
  //  FormPtr frm = FrmGetActiveForm();
  //  FrmCopyTitle(frm, new_title);
  color_title = new_title;
}

void do_uiPickColor() // from kmoriautil
{
  IndexedColorType icolor;
  RGBColorType rgb;
  Boolean picked;
  explain_ch(&FrmCopyTitle_helper, colored_symbols[symbol_index]); // xxx
  if (supportUIPickColor) {
    icolor = c_colors[symbol_index]; // xxx
    WinIndexToRGB(icolor, &rgb);
    picked = UIPickColor(&icolor, &rgb, UIPickColorStartPalette,
			 color_title, NULL);
    if (picked) {
      write_color(symbol_index, icolor);
      update_drawing(symbol_index);
    }
  }
  else {
    FrmPopupForm(ColorForm);
  }
}

void draw_sample(IndexedColorType icolor)
{
  Char foo;
  Short i;
  RectangleType r;
  RctSetRectangle(&r, 5, 125, 8*dX, 11);
  WinDrawRectangle(&r, 0);

  WinPushDrawState();
  WinSetTextColor(icolor);
  foo = colored_symbols[symbol_index];
  for (i = 0 ; i < 8 ; i++)
    WinDrawChars(&foo, 1, 5+(i*dX), 113);
  WinSetBackColor(black);
  for (i = 0 ; i < 8 ; i++)
    WinDrawChars(&foo, 1, 5+(i*dX), 125);
  WinPopDrawState();
}
// Keep this around and use it in case of prism. // from kmoriautil.
Boolean Color_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  IndexedColorType icolor;
  ListPtr lst;
  
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmCopyTitle(frm, color_title);
    FrmDrawForm(frm);
    icolor = c_colors[symbol_index];
    WinIndexToRGB(icolor, &rgbcolor); // initialize to old val
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_col_1));
    LstSetSelection(lst, (255-rgbcolor.r)/51);
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_col_2));
    LstSetSelection(lst, (255-rgbcolor.g)/51);
    lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_col_3));
    LstSetSelection(lst, (255-rgbcolor.b)/51);
    //
    icolor = WinRGBToIndex(&rgbcolor);      
    draw_sample(icolor);
    handled = true;
    break;

  case lstSelectEvent:
    switch (e->data.lstSelect.listID) {
    case list_col_1:
      rgbcolor.r = 255 - 51 * e->data.lstSelect.selection;
      break;
    case list_col_2:
      rgbcolor.g = 255 - 51 * e->data.lstSelect.selection;
      break;
    case list_col_3:
      rgbcolor.b = 255 - 51 * e->data.lstSelect.selection;
      break;
    }
    //
    icolor = WinRGBToIndex(&rgbcolor);      
    draw_sample(icolor);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_col_ok:
      icolor = WinRGBToIndex(&rgbcolor);      
      write_color(symbol_index, icolor);
      LeaveForm();
      // draw it in new color!!!
      update_drawing(symbol_index);
      handled = true;
      break;
    case btn_col_cancel:
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




//
// Opens database by name. If it doesn't exist: error
//
// returns 0 in case of error
// returns DmOpenRef > 0 in case of success
//
static DmOpenRef NPOpenDB(Char *name)
{
  LocalID lid;
  //  Err err;
  DmOpenRef db;

  lid = DmFindDatabase(0, name);
  if (lid == 0) {
    // not found - so create it
    //    err = DmCreateDatabase(0, name, CREATOR, TYPE, 0);
    //    if (err) return 0;
    //    lid = DmFindDatabase(0, name);
    return 0;
  }
  db = DmOpenDatabase(0, lid, dmModeReadWrite);
  return db;
} /* NPOpenDB */

static void do_write_color_memo(UInt index, Char *magic_title)
{
  DmOpenRef mdb;
  Char buf[20], r[10], g[10], b[10];
  UInt memosize;
  VoidHand vh;
  Char *p;
  Short i, offset = 0, len = StrLen(colored_symbols);

  //  len = NUM_SYMBOLS; // XXXXX remove this (debugging)

  memosize = StrLen(magic_title) + 1 + 9 * len; // warning: an exact fit.
  memosize *= sizeof(Char);

  if (0 != (mdb = NPOpenDB("MemoDB"))) {
    if (0 != (vh = DmNewRecord(mdb, &index, memosize))) {
      if (0 != (p = MemHandleLock(vh))) {
	StrPrintF(buf, "%s", magic_title);
	DmStrCopy(p, offset, buf);
	offset += StrLen(buf);
	for (i = 0 ; i < len ; i++) {

	  UChar c = colored_symbols[i];
	  RGBColorType rgbcolor;
	  if (c == EMDASH) c = DASH_COL_SYM;
	  else if (c == FOODCHAR) c = FOOD_COL_SYM;
	  WinIndexToRGB(c_colors[i], &rgbcolor);

	  //	  // XXX remove these (debugging)
	  //	  c = i + 33;
	  //	  WinIndexToRGB(symbol_colors[i], &rgbcolor);
	  //	  // XXX end of remove-these

	  // I wish I could just say "%2x" or something and have it work.
	  StrPrintF(r, "%x", rgbcolor.r);
	  StrPrintF(g, "%x", rgbcolor.g);
	  StrPrintF(b, "%x", rgbcolor.b);
	  StrPrintF(buf, "%c %s%s%s\n",
		    c,
		    r+(StrLen(r)-2),
		    g+(StrLen(g)-2),
		    b+(StrLen(b)-2));
	  DmStrCopy(p, offset, buf);
	  offset += StrLen(buf);
	}
	MemPtrUnlock(p);
	DmReleaseRecord(mdb, index, true);
	DmCloseDatabase(mdb);
	return;
      }
    }
    DmCloseDatabase(mdb);    
  }
  // Report the failure... make the button go away and do a popup alert?

}

Boolean write_color_memo()
{
  Boolean found = false;
  UInt recordNum;
  VoidHand recH;
  UInt mode = dmModeReadOnly;
  Char magic_title[20];
  Short title_len;//, memo_len, i, j, symbol;
  Char *memo;//, c;
  //  RGBColorType rgbcolor;
  //  IndexedColorType icolor;
  //  Boolean no_foreground = true;
  //  Boolean symbol_set[NUM_SYMBOLS];

  // first, what is our magic title:
  StrPrintF(magic_title, "iRogue-RGB%c", linefeedChr);
  title_len = StrLen(magic_title);
  // try to open the memo database
  if (!(MemoDB = DmOpenDatabaseByTypeCreator(memoDBType, sysFileCMemo, mode))){
    return false;
  }
  recordNum = 0;
  // get a record (stop if we run out)
  while (!found &&
	 (recH = DmQueryNextInCategory(MemoDB, &recordNum, dmAllCategories))) {
    MemoDBRecordPtr memoRecP = MemHandleLock (recH);
    memo = &(memoRecP->note);
    // Does the title match our magic title?
    if (0 == StrNCompare(memo, magic_title, title_len)) {
      found = true;
      // delete this memo and create a new one.  yeah.  at this index.
      MemHandleUnlock(recH);
      DmRemoveRecord(MemoDB, recordNum); // <--- hey, why doesn't this work?
      do_write_color_memo(recordNum, magic_title);
      DmCloseDatabase(MemoDB);
      return true;
    } // else memo does not match, try the next memo
    MemHandleUnlock(recH);
    recordNum++;
  } // end while(records)
  if (!found) {
    // create a new memo record at the end.
    do_write_color_memo(DmNumRecords(MemoDB), magic_title);
  }
  DmCloseDatabase(MemoDB);
  return true;
}



Boolean Thing_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  Short x, y, row, col;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);

    read_color();
    draw_ascii();

    handled = true;
    break;


  case penDownEvent:
    x = e->screenX - X_0;
    y = e->screenY - Y_0;
    if (y >= COLOR_BTN_Y) {
      // make like a button
      if (x >= COLOR_BTN_CANCEL_X) {
	LeaveForm();
	handled = true;
	break;
      } else {
	Short i, len = StrLen(colored_symbols);
	//	  IndexedColorType ict;
	for (i = 0 ; i < len ; i++) {
	  //	    ict = get_color(colored_symbols[i]);
	  if (c_colors[i] == white)
	    c_colors[i] = black;
	  //	    if (ict != c_colors[i])
	  set_color(colored_symbols[i], c_colors[i]);
	}
	// XXXXXX still need to write out the memo.... TODO.
	write_color_memo();
	have_read_color = true;
	LeaveForm(); // do whatever form_prefs does to a dirty screen:
	move_visible_window(sotu->roguep->col, sotu->roguep->row, true);
	//	print_stats(STAT_ALL); 
	//	check_message();
	handled = true;
	break;
      }
    } else {
      // calculate symbol_index
      row = y / dY;
      col = x / dX;
      symbol_index = col + row*COLOR_COLS;
      if (symbol_index < StrLen(colored_symbols)) {
	do_uiPickColor();
	handled = true;
      }
    }
    break;

  default:
    break;
  }

  return handled;
}





#endif // I_AM_COLOR

