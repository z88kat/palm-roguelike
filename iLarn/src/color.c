// Borrowed copiously from iRogue

#include "palm.h"
#include "sections.h"
#include "defines.h"
#include "types.h"
#include "color.h"
//#include "rogue.h" // for my EMDASH and FOODCHAR
Boolean have_read_color = false;

#ifdef I_AM_COLOR
extern LarnPreferenceType my_prefs;
DmOpenRef MemoDB;
// this seems weird to me, but what the heck.
typedef struct {
  Char note;           // null terminated
} MemoDBRecordType;
typedef MemoDBRecordType * MemoDBRecordPtr;
#define memoDBType 'DATA'
#define Is_Hex(x) ((('0' <= (x)) && ((x) <= '9')) || (('A' <= (x)) && ((x) <= 'F')) || (('a' <= (x)) && ((x) <= 'f')))

// So.  what are the symbols.  From explain_it():
/*
 * These are the monsters:
 * "lGHJKOScjtAELNQRZabhiCTYdegmvzFWflorXV pqsyUkMwDDPxnDDuD1234567890";
 *  i.e. 1 each of 0-9A-Za-z with these exceptions:
 *  0 of B, 5 of D, 0 of I (that's i as in India), 2 of l (that's L as in Lima)
 *
 * These are the objects:
 * "AToP%^F&^+M=%^$$f*OD#~][[)))(((||||||||{?!BC.o...<<<<EVV))([[]]](^ [H*** ^^ S tsTLcu...//))]:::::@
 *  i.e. !#$%&()*+./:<=?@ABCDEFHLMOPSTV[]^cfostu{|~
 *
 * Got some serious overlap on the letters, there.
 * Ok, suppose we had:
 * an array for regular ascii ! thru ~ (33 thru 126)    i = (Short) (c - 33);
 * plus "[A-Za-z]_" e.g. T_ is a Throne, say an extra 65 thru..um..117.
 *   i = (Short) (c - 65) + (126-33);
 * total space-wasting array is 126-33 + 117-65 = 145 * sizeof(UInt).  290 B.
 *
 * put_char_at will do:
 *    if (!bold) i = c - 33; else i = c + 28;    // i is index into my array.
 * Sounds ok to me.  put_char_at doesn't know what kind of dragon you're seeing
 * so, sorry, all dragons are the same dam' color For Now.
 */



// symbol_colors[] layout (see symbol_index):
//   0..93     plain characters '!'..'~' (monsters, and map things that
//             aren't objects: '#' walls, ',' unexplored, '@' you)
//   94..145   "bold" letters A-Z then a-z: objects, drawn as an inverted
//             block (a memo writes these as e.g. "T_")
//   146..150  one each for the five kinds of dragon 'D' (memo "Db" etc.)
// (Before 0.40 the bold and dragon slots overlapped, so the urn's colour
// changed bronze dragons, and bold 'z' had no slot at all.)
#define NUM_PLAIN 94
#define NUM_BOLD 52
#define SKIP_TO_BOLD NUM_PLAIN
#define SKIP_TO_DRAGONS (NUM_PLAIN + NUM_BOLD)
#define NUM_DRAGONS 5
#define NUM_SYMBOLS (SKIP_TO_DRAGONS + NUM_DRAGONS)
#define BOLD_SYM '_'
#define SUBTRACT_BANG 33
IndexedColorType symbol_colors[NUM_SYMBOLS];
IndexedColorType black, white;
static Short grok_hex(Char *buf, Short len) SEC_4;
const Char dragon_color_syms[NUM_DRAGONS] = { 'b', 'g', 's', 'p', 'r' };
/* Need to fix it so that I accept "d " (default for dragon), "dw", "db", "dg",
   "ds", "dp", "dr" */
Boolean dragon_colors[NUM_DRAGONS];

// Index into symbol_colors for character c, or -1 if it has none.
static Short symbol_index(Char c, Boolean bold)
{
  if (bold && c >= 'A' && c <= 'Z') return SKIP_TO_BOLD + (c - 'A');
  if (bold && c >= 'a' && c <= 'z') return SKIP_TO_BOLD + 26 + (c - 'a');
  if (c >= '!' && c <= '~') return c - SUBTRACT_BANG;
  return -1;
}

// The built-in "neon" colour scheme, used for every symbol that the
// iLarn-RGB memo (if any) doesn't set.  All are from the 216-colour
// web-safe part of the system palette, and vivid but readable on both the
// white and the Invert (black) background.  Objects are drawn as a block
// of their colour with the letter cut out, so theirs are mid-tones that
// show white (or black) text well.  Walls, '@' and anything not listed are
// drawn in the plain text colour.
typedef struct { Char sym; Boolean bold; UInt8 r, g, b; } DefaultColorType;
static const DefaultColorType default_colors[] = {
  // things that aren't letters (objects and traps)
  {'!',0, 0xFF,0x00,0x99}, // potion
  {'?',0, 0x00,0x99,0x99}, // scroll
  {'/',0, 0xCC,0x00,0xFF}, // wand of wonder, staff of power
  {'|',0, 0xFF,0x00,0xCC}, // ring
  {'{',0, 0xCC,0x66,0x00}, // belt
  {'[',0, 0x66,0x66,0xFF}, // light armour, shield
  {']',0, 0x33,0x33,0xCC}, // heavy armour
  {')',0, 0x00,0x66,0xFF}, // sword, axe...
  {'(',0, 0x00,0x99,0xCC}, // dagger, spear...
  {'*',0, 0xCC,0x99,0x00}, // gold
  {'<',0, 0x00,0x99,0x66}, // gem
  {'.',0, 0x99,0x33,0xFF}, // amulet, scarab, lamp, talisman...
  {'~',0, 0xFF,0x33,0x00}, // Eye of Larn
  {':',0, 0x33,0x99,0x00}, // drugs
  {'^',0, 0xFF,0x00,0x00}, // trap, teleporter, elevator
  {'%',0, 0x00,0xCC,0x00}, // stairs
  {'$',0, 0xCC,0x99,0x00}, // bank
  {'=',0, 0xFF,0x66,0x00}, // DND store
  {'+',0, 0xFF,0x66,0x00}, // college
  {'&',0, 0x99,0x99,0x99}, // statue
  {',',0, 0x99,0x99,0xCC}, // not yet explored
  // things that are letters (drawn bold)
  {'A',1, 0xCC,0x99,0x00}, // altar
  {'B',1, 0x00,0x99,0x99}, // book
  {'C',1, 0xCC,0x66,0x00}, // chest
  {'D',1, 0x99,0x66,0x33}, // closed door
  {'O',1, 0x99,0x66,0x33}, // open door
  {'E',1, 0x00,0xCC,0x00}, // dungeon entrance
  {'V',1, 0x00,0x99,0x00}, // volcano shaft
  {'F',1, 0x00,0x99,0xFF}, // fountain
  {'f',1, 0x66,0x66,0x99}, // dried-up fountain
  {'H',1, 0x33,0x99,0x33}, // home
  {'L',1, 0xCC,0x00,0x66}, // Larn Revenue Service
  {'M',1, 0x66,0x99,0xCC}, // mirror
  {'P',1, 0x66,0x33,0x99}, // pit
  {'S',1, 0xFF,0x66,0x00}, // trading post
  {'T',1, 0xCC,0x99,0x00}, // throne
  {'t',1, 0x99,0x66,0x33}, // throne without its gems
  {'c',1, 0xCC,0x66,0x33}, // fortune cookie
  {'o',1, 0x99,0x00,0xFF}, // orb
  {'s',1, 0x33,0x00,0x66}, // sphere of annihilation
  {'u',1, 0xCC,0x99,0x00}, // golden urn
  // monsters (shallow ones first)
  {'l',0, 0x99,0x66,0x33}, {'G',0, 0x99,0x66,0x00}, {'H',0, 0xCC,0x66,0x00},
  {'J',0, 0xCC,0x99,0x00}, {'K',0, 0x66,0x99,0x00}, {'O',0, 0x33,0x99,0x00},
  {'S',0, 0x00,0x99,0x00}, {'c',0, 0xCC,0x66,0x33}, {'j',0, 0x00,0x99,0x99},
  {'t',0, 0x66,0x66,0x99}, {'A',0, 0xCC,0x33,0x00}, {'E',0, 0x00,0x99,0xFF},
  {'L',0, 0x00,0x99,0x00}, {'N',0, 0xFF,0x33,0x99}, {'Q',0, 0xCC,0x00,0x66},
  {'R',0, 0xCC,0x66,0x33}, {'Z',0, 0x66,0x99,0x66}, {'a',0, 0xFF,0x33,0x00},
  {'b',0, 0x99,0x66,0x33}, {'h',0, 0xFF,0x00,0x00}, {'i',0, 0x00,0x99,0xCC},
  {'C',0, 0x99,0x66,0x00}, {'T',0, 0x33,0x99,0x33}, {'Y',0, 0x66,0x99,0xCC},
  {'d',0, 0x66,0x99,0xFF}, {'e',0, 0x33,0x99,0x99}, {'g',0, 0xFF,0x66,0x00},
  {'m',0, 0x99,0x33,0xFF}, {'v',0, 0x66,0x66,0xFF}, {'z',0, 0xCC,0x99,0x00},
  {'F',0, 0x99,0x00,0xFF}, {'W',0, 0x66,0x66,0x99}, {'f',0, 0xCC,0x00,0xCC},
  {'o',0, 0x99,0x33,0xCC}, {'r',0, 0xFF,0x66,0x00}, {'X',0, 0x99,0x66,0x33},
  {'V',0, 0xCC,0x00,0x33}, {'I',0, 0x99,0x99,0xCC}, {'p',0, 0x66,0x99,0x00},
  {'q',0, 0xFF,0x00,0x66}, {'s',0, 0x99,0x99,0x00}, {'y',0, 0xCC,0x99,0x00},
  {'U',0, 0x99,0x66,0x66}, {'k',0, 0xCC,0x00,0xFF}, {'M',0, 0x99,0x99,0x99},
  {'w',0, 0x00,0x66,0xFF}, {'P',0, 0xFF,0x00,0xCC}, {'x',0, 0x99,0x33,0x99},
  {'n',0, 0x00,0x99,0x66}, {'u',0, 0x66,0x99,0x33},
  // dragons: the white dragon is a 'd'; this is for any other 'D' (each
  // kind has its own colour too, below)
  {'D',0, 0xFF,0x00,0x00},
  // demon lords, demon prince, Lucifer (seen with the Eye of Larn)
  {'1',0, 0xFF,0x00,0x33}, {'2',0, 0xFF,0x00,0x33}, {'3',0, 0xFF,0x00,0x33},
  {'4',0, 0xFF,0x00,0x33}, {'5',0, 0xFF,0x00,0x33}, {'6',0, 0xFF,0x00,0x33},
  {'7',0, 0xFF,0x00,0x33}, {'9',0, 0xCC,0x00,0xFF}, {'0',0, 0xFF,0x00,0xFF}
};
#define NUM_DEFAULT_COLORS (sizeof(default_colors) / sizeof(default_colors[0]))
// bronze, green, silver, platinum and red dragons (dragon_color_syms)
static const UInt8 default_dragon_colors[NUM_DRAGONS][3] = {
  {0xCC,0x66,0x00}, {0x00,0x99,0x00}, {0x99,0x99,0xCC}, {0x66,0x99,0xCC},
  {0xFF,0x00,0x00}
};

static void set_default_colors()
{
  RGBColorType rgb;
  Short i, j;
  rgb.index = 0;
  for (i = 0 ; i < NUM_DEFAULT_COLORS ; i++) {
    j = symbol_index(default_colors[i].sym, default_colors[i].bold);
    if (j < 0) continue;
    rgb.r = default_colors[i].r;
    rgb.g = default_colors[i].g;
    rgb.b = default_colors[i].b;
    symbol_colors[j] = WinRGBToIndex(&rgb);
  }
  for (i = 0 ; i < NUM_DRAGONS ; i++) {
    rgb.r = default_dragon_colors[i][0];
    rgb.g = default_dragon_colors[i][1];
    rgb.b = default_dragon_colors[i][2];
    symbol_colors[SKIP_TO_DRAGONS + i] = WinRGBToIndex(&rgb);
    dragon_colors[i] = true;
  }
}
#endif // I_AM_COLOR


/* 
 * This is ONLY called if the OS version is at least 3.5.
 * Loads the built-in colour scheme into symbol_colors; then, if a memo
 * entitled "iLarn-RGB" exists, the colours it lists replace those.
 * Either way have_read_color becomes true, so IsColor follows the
 * Color preference.
 */
#ifdef I_AM_COLOR
void look_for_memo()
{
  UInt recordNum;
  VoidHand recH;
  UInt mode = dmModeReadOnly;
  Char magic_title[20];
  Short title_len, memo_len, i, j, symbol;
  Char *memo;
  RGBColorType rgbcolor;
  IndexedColorType icolor;
  Boolean found_memo = false;
  Boolean memo_dragon[NUM_DRAGONS]; // dragon colours the memo has set
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
  }
  for (i = 0 ; i < NUM_DRAGONS ; i++) {
    dragon_colors[i] = false;
    memo_dragon[i] = false;
  }
  set_default_colors();
  have_read_color = true; // the defaults will do if there's no memo
  //  symbol_colors[BG_COL_IX] = white;
  //  symbol_set[BG_COL_IX] = true;

  // FIND KITTEN^H^H^H^H^H^HMEMO
  // first, what is our magic title:
  StrPrintF(magic_title, "iLarn-RGB%c", linefeedChr);
  title_len = StrLen(magic_title);
  // try to open the memo database
  if (!(MemoDB = DmOpenDatabaseByTypeCreator(memoDBType, sysFileCMemo, mode)))
    return;
  recordNum = 0;
  // get a record (stop if we run out)
  while (!found_memo &&
	 (recH = DmQueryNextInCategory(MemoDB, &recordNum, dmAllCategories))) {
    MemoDBRecordPtr memoRecP = MemHandleLock (recH);
    memo = &(memoRecP->note);
    // Does the title match our magic title?
    if (0 == StrNCompare(memo, magic_title, title_len)) {
      found_memo = true;
      memo_len = StrLen(memo);
      i = title_len; // skip past the title!
      while (i < memo_len - 1) {
	// this is the symbol whose color we'll define
	if (memo[i+1] == BOLD_SYM) {
	  // "bold" letters are objects (non-letters only mean one thing)
	  symbol = symbol_index(memo[i], true);
	  i++;
	} else {
	  symbol = symbol_index(memo[i], false);
	  // Sneakily allow multiple colors for different kinds of dragons.
	  if (memo[i] == DRAGON_CHAR) {
	    Char dchar = memo[i+1];
	    Short dindex;
	    for (dindex = 0 ; (dindex < NUM_DRAGONS) &&
		   (symbol < SKIP_TO_DRAGONS) ; dindex++) {
	      if (dchar == dragon_color_syms[dindex]) {
		i++;
		symbol = SKIP_TO_DRAGONS + dindex;
	      }
	    }
	  } // end sneakiness
	} // end else-not-bold
	// ok, now 'symbol' is an index into symbol_colors[]
	if (symbol >= 0 && symbol < NUM_SYMBOLS) {
	  // ok now skip some (probably) whitespace to get to the rrggbb
	  Boolean found = false;
	  for ( i = i+1 ; i < memo_len - 1 ; i++) {
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

	    if (symbol >= SKIP_TO_DRAGONS) {
	      dragon_colors[symbol-SKIP_TO_DRAGONS] = true; // remember it.
	      memo_dragon[symbol-SKIP_TO_DRAGONS] = true;
	    } else if (symbol == DRAGON_CHAR - SUBTRACT_BANG) {
	      // a plain "D" colours every dragon the memo doesn't colour
	      for (j = 0 ; j < NUM_DRAGONS ; j++)
		if (!memo_dragon[j])
		  dragon_colors[j] = false;
	    }

	    i += 6;
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
IndexedColorType get_color(Char c, Boolean bold)
{
  IndexedColorType ict = black;
  Short i = symbol_index(c, bold);
  // Let's have some bounds checking here.
  if (i >= 0 && i < NUM_SYMBOLS)
    ict = symbol_colors[i];
  // Indexes 230-255 of the 8-bit system palette are all black; treat any
  // of them as black, whichever one WinRGBToIndex chose.
  if (ict >= 230)
    ict = black;
  if (my_prefs.black_bg && ict == black)
    return white;
  else
    return ict;
}
#endif // I_AM_COLOR

#ifdef I_AM_COLOR
IndexedColorType get_dragon_color(Short mon)
{
  IndexedColorType ict = black;
  Short i = DRAGON_CHAR - SUBTRACT_BANG;
  Short j = -1;
  switch(mon){
    //  case WHITEDRAGON:    break; // White dragons are 'd', not 'D'. ha!
  case BRONZEDRAGON:   j = 0; break;
  case GREENDRAGON:    j = 1; break;
  case SILVERDRAGON:   j = 2; break;
  case PLATINUMDRAGON: j = 3; break;
  case REDDRAGON:      j = 4; break;
  default:
    break; // hmmm there's a bug if you hit default.
  }
  if (j >= 0 && j < NUM_DRAGONS)
    if (dragon_colors[j])
      i = SKIP_TO_DRAGONS + j; // else i remains index of default 'D' color.

  ict = symbol_colors[i];
  if (ict >= 230)
    ict = black;
  if (my_prefs.black_bg && ict == black)
    return white;
  else
    return ict;
}
#endif // I_AM_COLOR
