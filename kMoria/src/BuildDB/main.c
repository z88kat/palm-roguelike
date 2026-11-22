#include "palm.h"
//#include <Globals.h>
#include "kMoriaBuildRsc.h"
#include "kMoria.h"

DmOpenRef MoriaDB;
static Boolean OpenDatabase();
/*****************************************************************************
 *                                                                           *
 *                      RebuildDatabase                                      *
 *                                                                           *
 *****************************************************************************/
// returns false on error, true otherwise.
/*
void newrec_objectlist();
void newrec_creaturelist();
void newrec_sortedobjects();
void newrec_tlevel();
void newrec_mlevel();
void newrec_haggle_texts();
void newrec_magics_names();
void newrec_player_title();
static Boolean RebuildDatabase()
{
  // open the database
  if (!OpenDatabase())
    return false;

  // CHANGE THIS PART WHEN YOU CHANGE WHAT OBJECTS ARE COMPILED IN.
  // 1.
  //  newrec_objectlist(); // create the object_list
  // 2.
  //  newrec_creaturelist(); // create the c_list
  // 3.
  //  newrec_tlevel();
  //  newrec_sortedobjects();
  //  newrec_mlevel(); // create the t_level, sorted_objects, and m_level. one shot
  // 4.
  //  newrec_haggle_texts();
  // 5.
  //  newrec_magics_names(); // might be too big.  might need split up.  nope.
  // 6.
  //  newrec_player_title();
  // END OF CHANGE THIS PART

  if (MoriaDB != NULL) {
    DmCloseDatabase(MoriaDB);
    MoriaDB = NULL;
  }
  return true;
}

*/

Short monster_index;
Char monster_char;
RGBColorType rgbcolor;
IndexedColorType black, white;
Boolean bg_is_white = true;

/* this part is from ../lock.c */
creature_type *c_list = NULL;
Char *c_names = NULL;
IndexedColorType *c_colors = NULL; // added

static void creaturelist_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_CREATURELIST;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    c_list = (creature_type *) p;
  } else {
    // unlock record
    if (c_list != NULL) {
      c_list = NULL;
      MemHandleUnlock(vh);
    }
  }
}
static void cnames_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_CLIST_NAMES;
  UInt index;
  static VoidHand vh;
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) return;
    vh = DmQueryRecord(MoriaDB, index);
    p = MemHandleLock(vh);
    c_names = (Char *) p;
  } else {
    // unlock record
    if (c_names != NULL) {
      c_names = NULL;
      MemHandleUnlock(vh);
    }
  }
}

#define MAX_ASCII 95
#define ASCII_START 32 
#define MAX_DEFAULTS 11
#define MAX_MY_COLORS (MAX_CREATURES + MAX_ASCII + MAX_DEFAULTS)
// added...
static void ccolors_record_lock(Boolean unlocked)
{
  ULong uniqueID = REC_CLIST_COLORS;
  static UInt index;
  static VoidHand vh;
  ULong size = MAX_MY_COLORS * sizeof(IndexedColorType);
  VoidPtr p = NULL;
  if (unlocked) {
    // lock record
    if (0!=DmFindRecordByID(MoriaDB, uniqueID, &index)) {
      // create new record if it doesn't exist
      index = DmNumRecords(MoriaDB);
      vh = DmNewRecord(MoriaDB, &index, size);
      p = MemHandleLock(vh);
      DmSet(p, 0, size, black); // happily, it's the right size, UChar.
    } else {
      vh = DmGetRecord(MoriaDB, index);
      //vh = DmResizeRecord(MoriaDB, index, size); // uncomment to resize it!
      p = MemHandleLock(vh);
    }
    c_colors = (IndexedColorType *) p;
  } else {
    // unlock record
    if (c_colors != NULL) {
      c_colors = NULL;
      MemHandleUnlock(vh);
      DmReleaseRecord(MoriaDB, index, true);
      DmSetRecordInfo(MoriaDB, index, NULL, &uniqueID); // set its uid!
    }
  }
}
/* end of stuff from lock.c */

/******************************************
 *  For title of "thing"
 **************************************/

const Char *ident_char[MAX_ASCII] =
{
  "  - An open pit.",
  "! - A potion.",
  //  "\" - An amulet, periapt, or necklace.", // Too wide.
  "\" - An amulet or necklace",
  "# - A stone wall.",
  "$ - Treasure.",
  "% - A magma or quartz vein.",
  "& - Treasure chest.",
  "' - An open door.",
  "( - Soft armor.",
  ") - A shield.",
  "* - Gems.",
  "+ - A closed door.",
  ", - Food or mushroom patch.",
  "- - A wand",
  ". - Floor.",
  "/ - A pole weapon.",
  "0 - Not used.",
  "1 - Entrance to General Store.",
  "2 - Entrance to Armory.",
  "3 - Entrance to Weaponsmith.",
  "4 - Entrance to Temple.",
  "5 - Entrance to Alchemy shop.",
  // "6 - Entrance to Magic-Users store.", // Too wide.
  "6 - Magic-Users store.",
  "7 - Not used.",
  "8 - Not used.",
  "9 - Not used.",
  ": - Rubble.",
  "; - A loose rock.",
  "< - An up staircase.",
  "= - A ring.",
  "> - A down staircase.",
  "? - A scroll.",
  "@ - You.",
  "A - Giant Ant Lion.",
  "B - The Balrog.",
  "C - Gelatinous Cube.",
  //  "D - An Ancient Dragon (Beware).",// Too wide.
  "D - An Ancient Dragon, beware",
  "E - Elemental.",
  "F - Giant Fly.",
  "G - Ghost.",
  "H - Hobgoblin.",
  "I - Invisible Stalker.",
  "J - Jelly.",
  "K - Killer Beetle.",
  "L - Lich.",
  "M - Mummy.",
  "N - Not used.",
  "O - Ooze.",
  "P - Giant humanoid.",
  "Q - Quylthulg.",
  //  "Q - Quylthulg (Pulsing Flesh Mound).", // Tragically Too Wide
  "R - Reptile.",
  "S - Giant Scorpion.",
  "T - Troll.",
  "U - Umber Hulk.",
  "V - Vampire.",
  "W - Wight or Wraith.",
  "X - Xorn.",
  "Y - Yeti.",
  "Z - Not used.",
  "[ - Hard armor.",
  "\\ - A hafted weapon.",
  "] - Misc. armor.",
  "^ - A trap.",
  "_ - A staff.",
  "` - Not used.",
  "a - Giant Ant.",
  "b - Giant Bat.",
  "c - Giant Centipede.",
  "d - Dragon.",
  "e - Floating Eye.",
  "f - Giant Frog.",
  "g - Golem.",
  "h - Harpy.",
  "i - Icky Thing.",
  "j - Jackal.",
  "k - Kobold.",
  "l - Giant Louse.",
  "m - Mold.",
  "n - Naga.",
  "o - Orc or Ogre.",
  "p - Person (Humanoid).",
  "q - Quasit.",
  "r - Rodent.",
  "s - Skeleton.",
  "t - Giant Tick.",
  "u - Not used.",
  "v - Not used.",
  "w - Worm or Worm Mass.",
  "x - Not used.",
  "y - Yeek.",
  "z - Zombie.",
  "{ - Arrow, bolt, or bullet.",
  "| - A sword or dagger.",
  "} - Bow, crossbow, or sling.",
  "~ - Miscellaneous item.",
};

#define X_0 1
#define Y_0 1
#define CREAT_COLS 20
#define dX (160/CREAT_COLS)
#define dY 11

void draw_ascii()
{
  Short i, x=X_0, y=Y_0;
  Char buf[2];
  IndexedColorType mycolor;
  RectangleType r;
  RctSetRectangle(&r, 0, 0, 160, 160-22);
  WinPushDrawState();
  if (!bg_is_white)
    WinSetBackColor(black);
  WinEraseRectangle(&r, 0);
  for (i = 0 ; i < MAX_ASCII ; i++) {
    if (i && (i % CREAT_COLS == 0)) {
      x = X_0;
      y += dY;
    }
    mycolor = c_colors[i+MAX_CREATURES];
    if (mycolor == black && !bg_is_white) mycolor = white;
    WinSetTextColor(mycolor);
    buf[0] = (Char) (i + ASCII_START);
    WinDrawChars(buf, 1, x, y);
    x += dX;
  }
  WinPopDrawState();
}
void write_color(Short monster_index, IndexedColorType icolor);
void update_drawing(Short monster_index);
Boolean supportUIPickColor = true; /* this should be set at startup, to check
                                      for cranky prisms, but it's not, yet. */
void do_uiPickColor()
{
  IndexedColorType icolor;
  RGBColorType rgb;
  Boolean picked;
  const Char *title;
  if (supportUIPickColor) {
    icolor = c_colors[monster_index];
    WinIndexToRGB(icolor, &rgb);
    if (monster_index < MAX_CREATURES)
      title =  c_names + c_list[monster_index].name_offset;
    else
      title = ident_char[monster_index-MAX_CREATURES];

    picked = UIPickColor(&icolor, &rgb, UIPickColorStartPalette,
			 title, NULL);
    if (picked) {
      write_color(monster_index, icolor);
      update_drawing(monster_index);
    }
  }
  else {
    FrmPopupForm(ColorForm);
  }
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
    draw_ascii();

    handled = true;
    break;

  case keyDownEvent:
    if (e->data.keyDown.chr == pageUpChr) {
      bg_is_white = !bg_is_white;
      draw_ascii();
      handled = true;
    } else if (e->data.keyDown.chr == pageDownChr) {
      FrmGotoForm(MainForm);
      handled = true;
    }
    break;

  case penDownEvent:
    x = e->screenX - X_0;
    y = e->screenY - Y_0;
    // calculate monster_index
    row = y / dY;
    col = x / dX;
    monster_index = col + row*CREAT_COLS;
    if (monster_index < MAX_ASCII) {
      monster_char = (Char) (monster_index + ASCII_START);
      monster_index += MAX_CREATURES;
      do_uiPickColor();
      handled = true;
    }
    break;

  default:
    break;
  }

  return handled;
}


/*****************************************************************************
 *                     little event handler                                  *
 *****************************************************************************/

// I could let purple == violet == grape.
// quantities:  25, 12, 8, 6, 17, 9, (2,) 21, 4, 20, 7
const Char *default_colors[MAX_DEFAULTS] = 
{
  "Black", "Blue", "Brown", "Clear",
  "Green", "Grey", "Purple", 
  "Red", "Silver", "White", "Yellow"
};
#define SYNON 7
// the last four of these are questionable, since either there are very
// few of them, or, a cousin already is using the proposed synonym color.
const Char *synonyms[SYNON] = {
  "Grape", "Violet", "Emperor", "Bloodshot", "Crimson", "Crystal", "Fire" };
const Short synonym_idx[SYNON] = {
  6, 6, 6, 7, 7, 3, 7 };

// maybe I should do a sort of radio buttons in combination with
// the sliders. handmade radio buttons so I can make them change color.
#define XYZZY 27
void draw_color_names()
{
  Short i, x=X_0,y=Y_0;
  IndexedColorType mycolor;
  RectangleType r;
  RctSetRectangle(&r, x-1, y-1, XYZZY+1, MAX_DEFAULTS*12+1);
  WinEraseRectangle(&r, 0);
  RctSetRectangle(&r, x+XYZZY, y, XYZZY+1, MAX_DEFAULTS*12+1);
  WinDrawRectangle(&r, 0);
  WinPushDrawState();
  for (i = 0 ; i < MAX_DEFAULTS ; i++, y += 12) {
    mycolor = c_colors[i+MAX_CREATURES+MAX_ASCII];
    WinSetTextColor(mycolor);
    WinSetBackColor(white);
    WinDrawChars(default_colors[i], StrLen(default_colors[i]), x, y+1);
    WinSetBackColor(black);
    WinDrawChars(default_colors[i], StrLen(default_colors[i]), x+XYZZY, y+1);
  }
  WinPopDrawState();
}
Short selected_color_name;
void update_color_name()
{
  Short i=selected_color_name, x=X_0,y=Y_0;
  IndexedColorType mycolor;
  y += i * 12;
  WinPushDrawState();
  mycolor = c_colors[i+MAX_CREATURES+MAX_ASCII];
  WinSetTextColor(mycolor);
  WinSetBackColor(white);
  WinDrawChars(default_colors[i], StrLen(default_colors[i]), x, y+1);
  WinSetBackColor(black);
  WinDrawChars(default_colors[i], StrLen(default_colors[i]), x+XYZZY, y+1);
  WinPopDrawState();
}
void highlight_color_name()
{
  Short x=X_0,y=Y_0;
  if (selected_color_name < 0) return;
  y += selected_color_name * 12;
  WinInvertLine(x-1, y, x+2*XYZZY, y);
  WinInvertLine(x-1, y+12, x+2*XYZZY, y+12);
  WinInvertLine(x-1, y, x-1, y+12);
  WinInvertLine(x+2*XYZZY, y, x+2*XYZZY, y+12);
}
Boolean in_color_name(Short x, Short y)
{
  if ((x < X_0) || (y < Y_0) ||
      (x > X_0 + 2*XYZZY) || (y > Y_0 + MAX_DEFAULTS*12))
    return false;
  y -= Y_0;
  if (selected_color_name >= 0) highlight_color_name();
  selected_color_name = y / 12;
  return true;
}
#undef XYZZY
//void write_color(Short monster_index, IndexedColorType icolor);
void convert_color_to_critter(Boolean override)
{
  Short i, j, k;
  Boolean done;
  for (i = 0 ; i < MAX_CREATURES ; i++) {
    j = c_list[i].cchar - ASCII_START; // what symbol represents this monster
    if (override || c_colors[i] == black || c_colors[i] == white ||
	c_colors[i] == c_colors[j + MAX_CREATURES]) {
      done = false;
      for (k = 0 ; (k < MAX_DEFAULTS) && !done ; k++) {
	// Decide whether it matches a Color, and if so, write it.
	if (NULL != StrStr(c_names + c_list[i].name_offset,
			   default_colors[k])) {
	  write_color(i, c_colors[k+MAX_CREATURES+MAX_ASCII]); // copy color
	  done = true;
	}
      }
      for (k = 0 ; (k < SYNON) && !done ; k++) {
	// Decide whether it matches a purple, and if so, write it.
	if (NULL != StrStr(c_names + c_list[i].name_offset,
			   synonyms[k])) {
	  write_color(i, c_colors[synonym_idx[k] + MAX_CREATURES + MAX_ASCII]);
	  done = true;
	}
      }
      // done strstr'ing all possible candidates for this creature
    }
  } // done coloring all creatures
}
void update_defaults_listies()
{
  ListPtr lst;
  FormPtr frm = FrmGetActiveForm();
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_def_1));
  LstSetSelection(lst, (255-rgbcolor.r)/51);
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_def_2));
  LstSetSelection(lst, (255-rgbcolor.g)/51);
  lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_def_3));
  LstSetSelection(lst, (255-rgbcolor.b)/51);
}
Boolean Defaults_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  //  Short x, y, row, col;
  IndexedColorType icolor;
  //  ListPtr lst;
  FormPtr frm;
  Short foo;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    draw_color_names();
    selected_color_name = -1;
    // Really I should pretend to have selected "Black" initially.
    selected_color_name = 0;
    highlight_color_name();
    // also need to frob those little lists
    icolor = c_colors[selected_color_name+MAX_CREATURES+MAX_ASCII];
    WinIndexToRGB(icolor, &rgbcolor); // initialize to old val
    update_defaults_listies();
    handled = true;
    break;

  case penDownEvent:
    if (in_color_name(e->screenX, e->screenY)) {
      // (that also sets selected_color_name to some valid value)
      highlight_color_name();
      // also need to frob those little lists
      icolor = c_colors[selected_color_name+MAX_CREATURES+MAX_ASCII];
      WinIndexToRGB(icolor, &rgbcolor);
      update_defaults_listies();
      handled = true;
    }
    break;

  case lstSelectEvent:
    switch (e->data.lstSelect.listID) {
    case list_def_1:
      rgbcolor.r = 255 - 51 * e->data.lstSelect.selection;
      break;
    case list_def_2:
      rgbcolor.g = 255 - 51 * e->data.lstSelect.selection;
      break;
    case list_def_3:
      rgbcolor.b = 255 - 51 * e->data.lstSelect.selection;
      break;
    }
    //
    if (selected_color_name >= 0) {
      icolor = WinRGBToIndex(&rgbcolor);
      write_color(selected_color_name+MAX_CREATURES+MAX_ASCII, icolor);
      update_color_name(icolor);
    }
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_def_uipick:
      if (selected_color_name >= 0) {
	const Char *title;
	Boolean picked;
	RGBColorType rgb;
	title = default_colors[selected_color_name];
	icolor = c_colors[selected_color_name+MAX_CREATURES+MAX_ASCII];
	WinIndexToRGB(icolor, &rgb);
	picked = UIPickColor(&icolor, &rgb, UIPickColorStartPalette,
			     title, NULL);
	if (picked) {
	  WinIndexToRGB(icolor, &rgbcolor);
	  update_defaults_listies();
	  write_color(selected_color_name+MAX_CREATURES+MAX_ASCII, icolor);
	  update_color_name(icolor);
	}
      }
      handled = true;
      break;
    case btn_def_ok:
      // Propagate colors to creatures....
      foo = FrmAlert(Convert2P);
      if (0 == foo) {
	convert_color_to_critter(true);
	LeaveForm();
      } else if (1 == foo) {
	convert_color_to_critter(false);
	LeaveForm();
      }
      // else do nothing
      handled = true;
      break;
    case btn_def_cancel:
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

/*****************************************************************************
 *                     little event handler                                  *
 *****************************************************************************/

void draw_creatures()
{
  Short i, x=X_0, y=Y_0;
  Char buf[2];
  IndexedColorType mycolor;
  RectangleType r;
  RctSetRectangle(&r, 0, 0, 160, 160);
  WinPushDrawState();
  if (!bg_is_white)
    WinSetBackColor(black);
  WinEraseRectangle(&r, 0);
  for (i = 0 ; i < MAX_CREATURES ; i++) {
    if (i && (i % CREAT_COLS == 0)) {
      x = X_0;
      y += dY;
    }
    mycolor = c_colors[i];
    if (mycolor == black && !bg_is_white) mycolor = white;
    WinSetTextColor(mycolor);
    buf[0] = c_list[i].cchar;
    WinDrawChars(buf, 1, x, y);
    x += dX;
  }
  WinPopDrawState();
}

void update_drawing(Short monster_index)
{
  Short i, x=X_0, y=Y_0, row, col;
  Char buf[2];
  WinPushDrawState();
  if (!bg_is_white)
    WinSetBackColor(black);
  
  i = monster_index;
  if (i >= MAX_CREATURES) i -= MAX_CREATURES;

  row = i / CREAT_COLS;
  col = i % CREAT_COLS;
  x += col * dX;
  y += row * dY;
  WinSetTextColor(c_colors[monster_index]);

  if (monster_index < MAX_CREATURES)
    buf[0] = c_list[monster_index].cchar;
  else
    buf[0] = (Char) (i + ASCII_START);

  WinDrawChars(buf, 1, x, y);
  WinPopDrawState();
}


Boolean Critter_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  Short x, y, row, col;
  FormPtr frm;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    draw_creatures();

    handled = true;
    break;

  case keyDownEvent:
    if (e->data.keyDown.chr == pageUpChr) {
      bg_is_white = !bg_is_white;
      draw_creatures();
      handled = true;
    } else if (e->data.keyDown.chr == pageDownChr) {
      FrmGotoForm(MainForm);
      handled = true;
    }
    break;

  case penDownEvent:
    x = e->screenX - X_0;
    y = e->screenY - Y_0;
    // calculate monster_index
    row = y / dY;
    col = x / dX;
    monster_index = col + row*CREAT_COLS;
    if (monster_index < MAX_CREATURES) {
      monster_char = c_list[monster_index].cchar;
      do_uiPickColor();
      handled = true;
    }
    break;

  default:
    break;
  }

  return handled;
}
/*****************************************************************************
 *                     little event handler                                  *
 *****************************************************************************/

void LeaveForm()
{
   FormPtr frm;
   frm = FrmGetActiveForm();
   FrmEraseForm (frm);
   FrmDeleteForm (frm);
   FrmSetActiveForm (FrmGetFirstForm ());
}

void write_color(Short monster_index, IndexedColorType icolor)
{
  // ... Write to the database record.
  DmWrite(c_colors, monster_index * sizeof(IndexedColorType),
	  &icolor, sizeof(IndexedColorType));
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
  foo = monster_char;
  for (i = 0 ; i < 8 ; i++)
    WinDrawChars(&foo, 1, 5+(i*dX), 113);
  WinSetBackColor(black);
  for (i = 0 ; i < 8 ; i++)
    WinDrawChars(&foo, 1, 5+(i*dX), 125);
  WinPopDrawState();
}
// Keep this around and use it in case of prism.
Boolean Color_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  IndexedColorType icolor;
  ListPtr lst;
  
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    if (monster_index < MAX_CREATURES)
      FrmCopyTitle(frm, c_names + c_list[monster_index].name_offset);
    else
      FrmCopyTitle(frm, ident_char[monster_index-MAX_CREATURES]);
    FrmDrawForm(frm);
    icolor = c_colors[monster_index];
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
      write_color(monster_index, icolor);
      LeaveForm();
      // draw it in new color!!!
      //      if (monster_index < MAX_CREATURES)
      update_drawing(monster_index);
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

/****************************************************************
 *                                                              *
 *     main form                                                *
 *                                                              *
 ****************************************************************/

void convert_thing_to_critter(Boolean override)
{
  Short i, j;
  for (i = 0 ; i < MAX_CREATURES ; i++) {
    j = c_list[i].cchar - ASCII_START; // what symbol represents this monster
    if (override || c_colors[i] == black || c_colors[i] == white)
      write_color(i, c_colors[j + MAX_CREATURES]); // copy color from symbol
  }
}

Boolean Main_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Short foo;
  
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    handled = true;
    break;

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_things:
      FrmGotoForm(ThingForm);
      handled = true;
      break;
    case btn_ttom:
      foo = FrmAlert(ConvertP);
      if (0 == foo)
	convert_thing_to_critter(true);
      else if (1 == foo)
	convert_thing_to_critter(false);
      // else do nothing
      handled = true;
      break;
    case btn_ctom:
      FrmPopupForm(DefaultsForm);
      handled = true;
      break;
    case btn_mons:
      FrmGotoForm(CritterForm);
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}

Boolean Luser_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    handled = true;
    break;

  default:
    break;
  }

  return handled;
}

/*****************************************************************************
 *                                                                           *
 *                      OpenDatabase                                         *
 *                                                                           *
 *****************************************************************************/
// returns false on error, true otherwise.
static Boolean OpenDatabase()
{
  LocalID dbID;

  dbID = DmFindDatabase(0, MoriaDBName); // card#, name
  /*
  if (!dbID) {
    // create database IFF it does not exist
    if (DmCreateDatabase(0, MoriaDBName, MoriaAppID, MoriaDBType, false))
      return false;
    dbID = DmFindDatabase(0, MoriaDBName); // card#, name
    if (!dbID) return false;
  }
  */
  if (!dbID) return false;

  MoriaDB = DmOpenDatabase(0, dbID, dmModeReadWrite);

  creaturelist_record_lock(true);
  cnames_record_lock(true);

  rgbcolor.r = rgbcolor.g = rgbcolor.b = 0; // black
  black = WinRGBToIndex(&rgbcolor);
  rgbcolor.r = rgbcolor.g = rgbcolor.b = 255; // white
  white = WinRGBToIndex(&rgbcolor);

  ccolors_record_lock(true);

  return true;
}

/*****************************************************************************
 *                      ApplicationHandleEvent                               *
 *****************************************************************************/
static Boolean ApplicationHandleEvent(EventPtr e)
{
    FormPtr frm;
    Word    formId;
    Boolean handled = false;

    if (e->eType == frmLoadEvent) {
	formId = e->data.frmLoad.formID;
	frm = FrmInitForm(formId);
	FrmSetActiveForm(frm);

	switch(formId) {
	case MainForm:
	    FrmSetEventHandler(frm, Main_Form_HandleEvent);
	    break;
	case LuserForm:
	    FrmSetEventHandler(frm, Luser_Form_HandleEvent);
	    break;
	case CritterForm:
	    FrmSetEventHandler(frm, Critter_Form_HandleEvent);
	    break;
	case ColorForm:
	    FrmSetEventHandler(frm, Color_Form_HandleEvent);
	    break;
	case ThingForm:
	    FrmSetEventHandler(frm, Thing_Form_HandleEvent);
	    break;
	case DefaultsForm:
	    FrmSetEventHandler(frm, Defaults_Form_HandleEvent);
	    break;
	}
	handled = true;
    }

    return handled;
}


/*****************************************************************************
 *                      StartApplication                                     *
 *****************************************************************************/
/* moved to the head of the class */
static Word StartApplication(void)
{
  Word ok;
  DWord version, hsExtVersion;

  ok = OpenDatabase();
  if (!ok) return 1;
  /*   error = checkAppInfo(); */
  /*   if (error) return error; */


  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &version);
  if (version >= 0x03503000L) {
    /*
      // HEY - try and get this working for prism (and noncrashing for palm).
      // Or else just put in the README file "prism owners, make sure
      // you have whatever that OS update was."
    FtrGet (hsFtrCreator, hsFtrIDVersion, &hsExtVersion);
    if (sysGetROMVerMajor(hsExtVersion) == 3                      //MM
	&& sysGetROMVerMinor(hsExtVersion) == 5         //m
	&& sysGetROMVerFix(hsExtVersion) == 2           //f
	&& sysGetROMVerBuild(hsExtVersion) => 0x012)    //bbb
      supportUIPickColor = true;
    else
      supportUIPickColor = false; // we should demand that the user upgrade.
    */
    FrmGotoForm(MainForm);
  } else
    FrmGotoForm(LuserForm);

  return 0;
}


/*****************************************************************************
 *                      StopApplication                                      *
 *****************************************************************************/
static void StopApplication(void)
{
  FrmSaveAllForms();
  FrmCloseAllForms();

  if (MoriaDB != NULL) {
    creaturelist_record_lock(false);
    cnames_record_lock(false);
    ccolors_record_lock(false);
    DmCloseDatabase(MoriaDB);
    MoriaDB = NULL;
  }

}


/*****************************************************************************
 *                      EventLoop                                            *
 *****************************************************************************/

/* The main event loop */


static void EventLoop(void)
{
    Word err;
    EventType e;

    do {
	EvtGetEvent(&e, evtWaitForever);
	  if (! SysHandleEvent (&e))
	    if (! MenuHandleEvent (NULL, &e, &err))
	      if (! ApplicationHandleEvent (&e))
		FrmDispatchEvent (&e);
    } while (e.eType != appStopEvent);
}


/* Main entry point; it is unlikely you will need to change this except to
   handle other launch command codes */
/*****************************************************************************
 *                      PilotMain                                            *
 *****************************************************************************/
DWord PilotMain(Word cmd, Ptr cmdPBP, Word launchFlags)
{
    Word err;

    if (cmd == sysAppLaunchCmdNormalLaunch) {

      err = StartApplication();
      if (err) return err;

      EventLoop();
      StopApplication();

    } else {
      return sysErrParamErr;
    }

    return 0;
}










/*

~ 1 bpp-white and black
~ 2 bpp-4 levels
~ 4 bpp-16 levels
~ 8 bpp:
   - White = 0
   - Black = 255
   - 216 "web safe" colors     (00,33,66,99,CC,FF)
   - 10 more grays
   - Silver, maroon, purple, green, navy 
   - 25 unassigned

Extras are special named HTML colors:
                      silver: C0C0C0
                      maroon: 800000
                      purple: 800080
                      green:  008000
                      navy:   000080 

The palette chooser says: starting at slot 216:
grays  22, 44, 55, 77, 88, aa, bb, dd, ee
then the 5 named colors.  the unassigned ones appear as black.
ee dd cc c0 bb aa 99 88 77 66 55 44 33 22

*/
