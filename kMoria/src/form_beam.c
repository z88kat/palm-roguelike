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

// chocolatey beaming GOODNESS... dang, once I get this figured out I
// should put it in robotfindskitten just for the halibut...
// (er but what would you beam?)
#ifdef I_AM_COLOR

static Err beam_myself() SEC_7;
static Err beam_constdata() SEC_7;
//static Err beam_uids() SEC_7;
//static Err beam_message() SEC_7;
static Boolean get_my_best_score(high_scores *score) SEC_7;
static Err beam_scores() SEC_7;
static Err beam_amount(Long gold) SEC_7;
Err beam_object(Short item) SEC_7; // used in form_invselect.c
extern UChar invequ_mode; // for beaming inventory item
Boolean Beam_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ControlPtr btn;
    
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    // if we are in an old and crufty OS that does not have IR,
    // we should never have gotten to this form. (same as changing font)
    // NOTE - I should disable the 'object' button iff inven_ctr <= 0.
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_beam_item));
    if (inven_ctr <= 0)
      CtlHideControl(btn);
    btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_beam_cash));
    if (pymisc.lev > NEWBIE_LEVEL) // discourage the use of veal fattening pens
      CtlShowControl(btn);
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case ctlSelectEvent:
    switch(e->data.ctlSelect.controlID) {
    case btn_beam_myself:
      if (0 == beam_myself())
      	if (0 == FrmAlert(BeamNextP))
	  if (0 == beam_constdata())
	    ; // success!
      handled = true;
      break;
    case btn_beam_scores:
      beam_scores();
      handled = true;
      break;
    case btn_beam_item:
      invequ_mode = ACT_BEAM;
      FrmPopupForm(InvSelectForm);
      handled = true;
      break;
    case btn_beam_cash:
      LeaveForm(); // I should make this btn more consistent with the others..
      // If I take out THIS LeaveForm, remember to have BeamCash_Form_HE,
      // after _it_ calls LeaveForm, do a 'if (IsVGA) PrvMoveForm(true)'.
      FrmPopupForm(BeamCashForm);
      handled = true;
      break;
    case btn_beam_done:
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

// can this be in SEC_7 or will that make it grumpy?  hmmm
// Args: data to send; size of the data; protocol to use
// Returns: 0 on success, nonzero on failure
// Also sets 'size' to the size that was actually sent... in normal
// conditions this will be the same as the size we told it to send..
// (I think the z'catalog thunk is essentially the same as this one.)
Err beam_myself_thunk(const void *data, ULong *size, void *userData)
{
  Err err = 0;

  // send it over!
  *size = ExgSend((ExgSocketType *)userData, (void *)data, *size, &err);

  return err;
}

static Err beam_myself()
{
  // This is borrowed from example by Aaron Ardiri in palm-dev forum
  UShort        card;
  LocalID       dbID;
  Char          strName[32];
  Char          strDescription[32];
  ExgSocketType socket;
  Err           err, e;

  // let's get the current application details
  SysCurAppDatabase(&card, &dbID);

  // define the strings
  StrCopy(strName,        "kMoria.prc");
  StrCopy(strDescription, "kMoria");

  // configure
  MemSet(&socket, sizeof(ExgSocketType), 0);
  socket.target      = sysFileCLauncher;         // who will receive it
  socket.description = strDescription;           // what to show user :)
  socket.name        = strName;                  // filename for target

  // send it over!
  err = errNone;
  e = ExgPut(&socket);
  err |= e;
  if (err == errNone) {
    e = ExgDBWrite(beam_myself_thunk, &socket, NULL, dbID, card);
    err |= e;
    e = ExgDisconnect(&socket, err);
    err |= e;
  }

  // did everything go ok?
  if (err != errNone)
    SndPlaySystemSound(sndError);
  return err;
}

  // We could just beam kMoriaDB.pdb to the launcher too, but,
  // let's send it to the newly beamed kMoria app so that it can
  // tidy up the record ids.
  // Or... hm... blah... I will test and see if the .prc uids are changed.
  // 0. I installed .prc on iiic
  // 1. I beamed to visor
  // 2. I fetched .prc from iiic (uids are still the same)
  // 3. I beamed from visor to iiic
  // 4. I fetched .prc from iiic (uids are still the same)
  // wait, so maybe if I send the pdb to the launcher it will be happy too?
  // let's try.
  // nope seems to be screwed.  are apps "special" or something?
  // hm then here's what I can do:
  // send a list of the unique ids in the order the records are in, and
  // have the receiving kmoria frob its own database records to match.

static Err beam_constdata()
{
  // This is borrowed from example by Aaron Ardiri in palm-dev forum
  UShort        card = 0;
  LocalID       dbID;
  Char          strName[32];
  Char          strDescription[32];
  ExgSocketType socket;
  Err           err, e;

  // let's get the current application details
  //  SysCurAppDatabase(&card, &dbID);
  dbID = DmFindDatabase(card, MoriaDBName);
  if (!dbID) { // this is impossible, but let's check for missing-db anyway.
    // hm I have no handy little error-popup do I? eh who cares: unreachable!
    SndPlaySystemSound(sndError);
    return exgErrNotFound;
  }

  // define the strings
  StrCopy(strName,        "kMoriaDB.pdb");
  StrCopy(strDescription, "kMoriaDB");

  // configure
  MemSet(&socket, sizeof(ExgSocketType), 0);
  socket.target      = sysFileCLauncher;         // who will receive it
  //socket.target      = MoriaAppID;               // [kMoria will receive it!]
  socket.description = strDescription;           // what to show user :)
  socket.name        = strName;                  // filename for target

  // send it over!
  err = errNone;
  e = ExgPut(&socket);
  err |= e;
  if (err == errNone) {
    e = ExgDBWrite(beam_myself_thunk, &socket, NULL, dbID, card);
    err |= e;
    e = ExgDisconnect(&socket, err);
    err |= e;
  }

  // did everything go ok?
  if (err != errNone)
    SndPlaySystemSound(sndError);

  //  Well!  Did it work?  let's find out!
  //  No it is hosed (actually I could launch kMoria, but when I entered
  // a store I was fscked.  visor is low on batts so I won't try
  // beaming back and inspecting right now.)

  // if !err, tell the new kMoria to tidy things up.   set the UIDs to: 
  // 0x96, 0x97, 0xa0, 0xa1, 0xce, 0xcf, 0xd0, 0xb4, 0xb5, 0xd6, 0x6e, 0x70
  return err;
}

#define HELLO_BUF 40
/*
static Err beam_message()
{
  Err err, e;
  ExgSocketType socket;
  Char buf[HELLO_BUF];
  Char strDescription[32];
  Char mimetype[32];
  StrCopy(strDescription, "message");
  StrCopy(mimetype, MY_MIMETYPE);
  MemSet(&socket, sizeof(socket), 0);
  socket.target = MoriaAppID;
  socket.type = mimetype; // how come this is not working?
  socket.description = strDescription;
  StrPrintF(buf, "hello world");
  err = ExgPut(&socket);
  if (!err) {
    ExgSend(&socket, buf, StrLen(buf)+1, &err); // returns #bytes sent?
  }
  e = ExgDisconnect(&socket, err); // don't you want to be in the 'if' too?
  err |= e;
  return err;
}
*/

static Err beam_scores()
{
  // send the top one of "my" scores... to kMoria...
  high_scores score;
  Boolean found;
  Err err, e;
  ExgSocketType socket;
  Char strDescription[32];
  Char mimetype[32];
  Char *buf;

  found = get_my_best_score(&score);
  if (!found)
    return exgErrNotFound;//there isn't any score that's yours!  fail silently?

  StrCopy(strDescription, "scores");
  StrCopy(mimetype, MY_MIMETYPE);
  MemSet(&socket, sizeof(socket), 0);
  socket.target = MoriaAppID;
  socket.type = mimetype;
  socket.description = strDescription;
  err = ExgPut(&socket);
  if (!err) {
    buf = (Char *) (&score);
    ExgSend(&socket, buf, sizeof(score), &err);
  }
  e = ExgDisconnect(&socket, err);
  err |= e;
  return err;
}

static Err beam_amount(Long gold)
{
  // send some cash... to kMoria...
  Err err, e;
  ExgSocketType socket;
  Char strDescription[32];
  Char mimetype[32];
  Char *buf;
  Char tmp[80];

  if (pymisc.au < gold || gold <= 0) {
    StrPrintF(tmp, "You can't send %ld gold pieces!", gold);
    message(tmp);
    return 0;
  }
  StrCopy(strDescription, "cash");
  StrCopy(mimetype, MY_MIMETYPE);
  MemSet(&socket, sizeof(socket), 0);
  socket.target = MoriaAppID;
  socket.type = mimetype;
  socket.description = strDescription;
  err = ExgPut(&socket);
  if (!err) {
    buf = (Char *) (&gold);
    ExgSend(&socket, buf, sizeof(Long), &err);
    //
    pymisc.au -= gold;
    StrPrintF(tmp, "You send %ld gold pieces to a friend.", gold);
    message(tmp); // I'm not convinced this will work, though.
    print_stats(STATS_GOLD);
  }
  e = ExgDisconnect(&socket, err);
  err |= e;
  return err;
}

Err beam_object(Short item)
{
  // beam the i'th inventory item to another kMoria, and DELETE it from here
  /*  Boolean found; */
  Err err, e;
  ExgSocketType socket;
  Char strDescription[32];
  Char mimetype[32];
  Char *buf;
  inven_type thing;

  if ((item < 0) || (item >= inven_ctr))
    return exgErrBadData; // out of bounds!
  thing = inventory[item];
  if (thing.subval <= ITEM_SINGLE_STACK_MAX)
    thing.number = 1; // so that  the qty beamed == the qty destroyed.
  StrCopy(strDescription, "item");
  StrCopy(mimetype, MY_MIMETYPE);
  MemSet(&socket, sizeof(socket), 0);
  socket.target = MoriaAppID;
  socket.type = mimetype;
  socket.description = strDescription;
  err = ExgPut(&socket);
  if (!err) {
    buf = (Char *) (&thing);
    ExgSend(&socket, buf, sizeof(inven_type), &err);
    //
    inven_destroy(item);
  }
  e = ExgDisconnect(&socket, err);
  err |= e;
  return err;
}

#define MY_SCORES 0
static Boolean get_my_best_score(high_scores *score)
{
  Short i, scores_ctr;
  VoidHand vh;
  VoidPtr p = NULL;
  high_scores *score_p;
  scores_ctr = DmNumRecords(MoriaScoreDB);
  if (scores_ctr <= 0)
    return false;
  for (i = 0 ; i < scores_ctr ; i++) {
    vh = DmQueryRecord(MoriaScoreDB, i);
    p = MemHandleLock(vh);
    score_p = (high_scores *) p;
    if (score_p->source == MY_SCORES) {
      *score = *score_p; // this _should_ work...
      MemHandleUnlock(vh);
      return true;
    } else
      MemHandleUnlock(vh);
  }
  return false;
}

//////////////////////////////////////////////////////////////////////
// Ok, that was the sending stuff.
// Now for the receiving stuff.
//
// The default Launcher will receive kMoria.prc for us.
//
// WE will receive, and fix, kMoriaDB.pdb
//
// We might also receive scores to merge into our existing scores
// but this must wait until I have put scores in their own db,
// as separate records, and with a boolean for 'created by me' vs
// 'received from a friend' (we beam the former and receive as the latter).

// basically just like the sending thunk
Err beam_receive_thunk(const void *data, ULong *size, void *userData)
{
  Err err = 0;
  *size = ExgReceive((ExgSocketType *)userData, (void *)data, *size, &err);
  return err;
}

// ok, so, we have refused to answer unless we have globals...
// I dunno if we can call stuff that isn't in section 0, or not.
extern Boolean in_beaming;
void insert_score(high_scores *score) SEC_6; // in form_death.c
void drop_throw(Short y, Short x, inven_type *t_ptr) SEC_7; // in moria4.c
Err beam_receive(ExgSocketPtr socketPtr)
{
  Char buf[80];// it's greater than sizeof(inven_type) or sizeof(high_scores).
  ULong read_bytes;
  Err err = 0;
  in_beaming = true;
  // gaaah.  when I have .t shortcut on, I have a happy socketPtr->type,
  // but when I am beaming for real, it is like null.
  //message(socketPtr->type); // for debugging...
  //  message(socketPtr->description); // for debugging...
  // Wow, palmos really doesn't love me.  I **do** get the description ok
  // but not the mimetype.  well, fsck.
  if (0 == StrCompare(socketPtr->description, "item")) {
    inven_type thing, *tmp;
    if (!ExgAccept(socketPtr)) {
      read_bytes = ExgReceive(socketPtr, buf, sizeof(buf), &err);
      if (read_bytes > 0) {
	tmp = (inven_type *) buf;
	thing = *tmp;
	// hm... what to do with it... this SHOULD drop it "near" the player.
	drop_throw(char_row, char_col, &thing);
	//message("non-empty item"); // DEBUGGING
      } else {
	message("empty item!");
      }
    }
    ExgDisconnect(socketPtr, err);
  } else if (0 == StrCompare(socketPtr->description, "cash")) {
    Long gold, *tmp;
    if (!ExgAccept(socketPtr)) {
      read_bytes = ExgReceive(socketPtr, buf, sizeof(buf), &err);
      if (read_bytes > 0) {
	tmp = (Long *) buf;
	gold = *tmp;
	// make sure to avoid overflow and negatives
	if (gold <= 0) {
	  StrPrintF(buf, "You reject the offer of %ld.", gold);
	  message(buf);
	} else {
	  if (gold > MAX_LONG - (pymisc.au+1)) // probably cheating anyway..
	    gold = MAX_LONG - (pymisc.au+1);
	  StrPrintF(buf, "You receive %ld from a friend.", gold);
	  pymisc.au += gold;
	  message(buf);
	}
      } else {
	message("empty cash!");
      }
    }
    ExgDisconnect(socketPtr, err);
  } else if (0 == StrCompare(socketPtr->description, "scores")) {
    high_scores score, *tmp;
    if (!ExgAccept(socketPtr)) {
      read_bytes = ExgReceive(socketPtr, buf, sizeof(buf), &err);
      if (read_bytes > 0) {
	tmp = (high_scores *) buf;
	score = *tmp;
	score.source = MAX_UCHAR;
	insert_score(&score); // surprise, it seems to work
	//message("non-empty score"); // DEBUGGING
      } else {
	message("empty score!");
      }
    }
    ExgDisconnect(socketPtr, err);
  }
  /*
  else if (0 == StrCompare(socketPtr->description, "message")) {
    if (!ExgAccept(socketPtr)) {
      read_bytes = ExgReceive(socketPtr, buf, sizeof(buf), &err);
      if (read_bytes > 0) {
	buf[HELLO_BUF-1] = '\0'; // make sure of null terminator
	message(buf);
      } else {
	message("empty message!");
      }
    }
    ExgDisconnect(socketPtr, err);
  }
  */
  else {
    message("screwy socketPtr->type");
    //err = exgErrUnknown;
  }
  in_beaming = false;
  return err;
}




Boolean BeamCash_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  FieldPtr fld;
  FieldAttrType fieldattr;
  Long gold;
  Word chr;
  Char *textp;
  VoidHand h;
  VoidPtr p;
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
#ifndef I_AM_OS_2
    if (IsVGA) PrvMoveForm(false);
#endif
    FrmDrawForm(frm);
    beamcash_draw();
    // allocate memory for field
    h = MemHandleNew(sizeof(Char) * 12);
    p = MemHandleLock(h);
    ((Char *)p)[0] = '\0';
    MemPtrUnlock(p);
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_bc));
    FldSetTextHandle(fld, (Handle) h);
    FrmSetFocus(frm, FrmGetObjectIndex(frm, field_bc));
    handled = true;
    break;

#ifndef I_AM_OS_2
  case displayExtentChangedEvent:
    if (IsVGA) PrvMoveForm(true);
    return true;
#endif

  case keyDownEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_bc));
    FldGetAttributes(fld, &fieldattr);
    chr = e->data.keyDown.chr;
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
	FrmSetFocus(frm, FrmGetObjectIndex(frm, field_bc));
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    fld = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, field_bc));
    switch(e->data.ctlSelect.controlID) {
    case btn_bc_ok:
      // beam the cash....
      textp = FldGetTextPtr(fld); // if (textP) ...
      gold = StrAToI(textp);
      // make sure you can afford it, too..
      FldReleaseFocus(fld);
      FldSetSelection(fld, 0, 0);
      FldFreeMemory(fld);
      LeaveForm();
      handled = true;
      beam_amount(gold); // beam_amount will check whether we have enough cash!
      // Need to REDRAW our gold!
      show_messages(); // to redraw stats.
      break;
    case btn_bc_cancel:
      FldReleaseFocus(fld);
      FldSetSelection(fld, 0, 0);
      FldFreeMemory(fld);
      LeaveForm();
      handled = true;
      break;
    }

  default:
    break;
  }

  return handled;
}

void beamcash_draw()
{
  Char buf[40];
  StrPrintF(buf, "(You have: %ld gold pieces)", pymisc.au);
  WinDrawChars(buf, StrLen(buf), visible_char_w, 2*visible_char_h); //5,20
}

#endif
