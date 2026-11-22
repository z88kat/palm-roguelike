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
// These are Unique ID within save db or whatever.
//
#define REC_PY           50

#define REC_MSGLOG      100
#define REC_PLAYERTITLE 101
#define REC_BG          102
#define REC_BG_STR      103
#define REC_PLAYEREXP   104
#define REC_PLAYERHP    105
#define REC_RGOLDADJ    106
#define REC_SPELLNAME   107
#define REC_SPELLORDER  108
#define REC_STOREOWN    109
#define REC_PHISTORY    111

#define REC_SAVE_VERSION 164
#define REC_SAVE_NOTE    165

#define REC_STORES      170
#define REC_STOREINV_A  171
// 171, 172, 173, 174, 175, 176: these are store inventories, 26 structs each
#define REC_STOREINV_Z  176
#define REC_OWNERS      177

#define REC_CAVE_CPTR   200
#define REC_CAVE_TPTR   201
#define REC_CAVE_FVAL   202
#define REC_CAVE_LIGHT  203
#define REC_TLIST       204
#define REC_MLIST       205
#define REC_INVENTORY   209
#define REC_OBJIDENT    210
// Note: the recall record should *NOT* be erased when a character dies!!!
#define REC_RECALL      211
//
#define REC_MAGICNAMEIDX 212
#define REC_SCROLLTITLES 213
//
#define REC_CRASHED      215

#define REC_TOPSCORES    250


// These are INDEX within the const DB
// (Previously, uid 150 151 160 161 206 207 208 180 181 214 110 112)
#define REC_OBJECTLIST     0  /*150*/
#define REC_OBJLIST_NAMES  1  /*151*/
#define REC_CREATURELIST   2  /*160*/
#define REC_CLIST_NAMES    3  /*161*/
#define REC_TLEVEL         4  /*206*/
#define REC_SORTEDOBJ      5  /*207*/
#define REC_MLEVEL         6  /*208*/
#define REC_HAGGLE         7  /*180*/
#define REC_HAGGLE_TEXT    8  /*181*/
#define REC_MAGICS         9  /*214*/
#define REC_PTITLES        10 /*110*/
#define REC_MATTACK        11 /*112*/

// These are in kMoriaColorDB - and they should change to index also,
// so that they can be beamed.  Also, have an option to beam them.
#define REC_CLIST_COLORS  0 /* 162 */
#define REC_COLOR_VERSION 1 /* 163 */



#define MAXLEN_MSG 80
#define MAX_MSGLOG 10
#define MAXLEN_PLAYERTITLE 14
#define MAXLEN_SPELLNAME 24
#define MAX_SPELLNAME 62
#define MAX_SPELLORDER 32
#define NO_SPELL 99
#define MAXLEN_STOREOWNERNAME 22
// mostly-const strings (non-compressed concatenated) of random names of things
#define MAXLEN_AMULETS 15
#define MAXLEN_ROCKS 13
#define MAXLEN_WOODS 11
#define MAXLEN_METALS 16
#define MAXLEN_COLORS 16
#define MAXLEN_MUSH 14
#define MAXLEN_SYLLABLES 5
#define MAXLEN_TITLES 10
#define MAXLEN_SPECNAME 19
// length of longest player-title string, including \0
#define MAXLEN_PTITLE 14



// These could be used for offsets in the 'recall' record
#define R_CMOVE    0
#define R_SPELLS   (sizeof(ULong))
#define R_KILLS    (2*sizeof(ULong))
#define R_DEATHS   (R_KILLS + sizeof(UShort))
#define R_CDEFENSE (R_KILLS + 2*sizeof(UShort))
#define R_WAKE     (R_KILLS + 3*sizeof(UShort))
#define R_IGNORE   (R_WAKE + sizeof(UChar))
#define R_ATTACKS  (R_WAKE + 2*sizeof(UChar))

// These could be used for offsets in the 'm_list' record
#define MLIST_hp 0
#define MLIST_csleep sizeof(Short)
#define MLIST_cspeed 2*sizeof(Short)
#define MLIST_fy        4*sizeof(Short)
#define MLIST_fx       (4*sizeof(Short) +   sizeof(Char))
#define MLIST_cdis     (4*sizeof(Short) + 2*sizeof(Char))
#define MLIST_ml       (4*sizeof(Short) + 3*sizeof(Char))
#define MLIST_stunned  (4*sizeof(Short) + 3*sizeof(Char) + sizeof(Boolean))
#define MLIST_confused (4*sizeof(Short) + 4*sizeof(Char) + sizeof(Boolean))

// These could be used for offsets in an 'inven_type' record
#define I_index    0
#define I_name2    sizeof(Short)
#define I_inscrip (sizeof(Short) + sizeof(Char))
#define I_flags   (sizeof(Short) + (1+INSCRIP_SIZE)*sizeof(Char))
#define I_tval  (sizeof(Long) +sizeof(Short) +(1+INSCRIP_SIZE)*sizeof(Char))
#define I_tchar (sizeof(Long) +sizeof(Short) +(2+INSCRIP_SIZE)*sizeof(Char))
#define I_p1    (sizeof(Long) +sizeof(Short) +(3+INSCRIP_SIZE)*sizeof(Char))
#define I_cost  (sizeof(Long)+2*sizeof(Short)+(3+INSCRIP_SIZE)*sizeof(Char))
#define I_subval (2*sizeof(Long)+2*sizeof(Short)+(3+INSCRIP_SIZE)*sizeof(Char))
#define I_number (2*sizeof(Long)+2*sizeof(Short)+(4+INSCRIP_SIZE)*sizeof(Char))
#define I_weight (2*sizeof(Long)+2*sizeof(Short)+(5+INSCRIP_SIZE)*sizeof(Char))
#define I_tohit (2*sizeof(Long)+3*sizeof(Short)+(5+INSCRIP_SIZE)*sizeof(Char))
#define I_todam (2*sizeof(Long)+3*sizeof(Short)+(6+INSCRIP_SIZE)*sizeof(Char))
#define I_ac    (2*sizeof(Long)+3*sizeof(Short)+(7+INSCRIP_SIZE)*sizeof(Char))
#define I_toac  (2*sizeof(Long)+3*sizeof(Short)+(8+INSCRIP_SIZE)*sizeof(Char))
#define I_toac  (2*sizeof(Long)+3*sizeof(Short)+(8+INSCRIP_SIZE)*sizeof(Char))
#define I_damage (2*sizeof(Long)+3*sizeof(Short)+(9+INSCRIP_SIZE)*sizeof(Char))
#define I_level (2*sizeof(Long)+3*sizeof(Short)+(11+INSCRIP_SIZE)*sizeof(Char))
#define I_ident (2*sizeof(Long)+3*sizeof(Short)+(12+INSCRIP_SIZE)*sizeof(Char))

