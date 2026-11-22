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


static void prt_comment1(void) SEC_4;
static void prt_comment2(Long, Long, Short) SEC_4;
static void prt_comment3(Long, Long, Short) SEC_4;
static void prt_comment4(void) SEC_4;
static void prt_comment5(void) SEC_4;
static void prt_comment6(void) SEC_4;
//static void display_commands(void);
//static void haggle_commands(Short);
//static void display_inventory(Short, Short);
//static void display_cost(Short, Short);
//static void store_prt_gold(void);
//static void display_store(Short, Short);
//static Short get_store_item(Short *, Char *, Short, Short);
static Boolean increase_insults(Short) SEC_4;
static void decrease_insults(Short) SEC_4;
static Boolean haggle_insults(Short) SEC_4;
//static Short get_haggle(Char *, Long *, Short);
//static Short receive_offer(Short, Char *, Long *, Long, Short, Short);
//static Short purchase_haggle(Short, Long *, struct inven_type *);
static void purchase_haggle_startturn() SEC_4;
static void sell_haggle_startturn() SEC_4;
// Not implemented yet:
//static Short sell_haggle(Short, Long *, struct inven_type *);
//static Short store_purchase(Short, Short *);
//static Short store_sell(Short, Short *);

// Unfortunately I think my data section is full and these will have to
// be read into the database by my utility app.
/*
static Char comment1[14][44] = {
  "Done!",  "Accepted!",  "Fine.",  "Agreed!",  "Ok.",  "Taken!",
  "You drive a hard bargain, but taken.",
  "You'll force me bankrupt, but it's a deal.",  "Sigh.  I'll take it.",
  "My poor sick children may starve, but done!",  "Finally!  I accept.",
  "Robbed again.",  "A pleasure to do business with you!",
  "My spouse will skin me, but accepted."
};

static Char comment2a[3][44] = {
  "%A2 is my final offer; take it or leave it.",
  "I'll give you no more than %A2.",
  "My patience grows thin.  %A2 is final."
};

static Char comment2b[16][52] = {
  "%A1 for such a fine item?  HA!  No less than %A2.",
  "%A1 is an insult!  Try %A2 gold pieces.",
  "%A1?!?  You would rob my poor starving children?",
  "Why, I'll take no less than %A2 gold pieces.",
  "Ha!  No less than %A2 gold pieces.",
  "Thou knave!  No less than %A2 gold pieces.",
  "%A1 is far too little, how about %A2?",
  "I paid more than %A1 for it myself, try %A2.",
  "%A1?  Are you mad?!?  How about %A2 gold pieces?",
  "As scrap this would bring %A1.  Try %A2 in gold.",
  "May the fleas of 1000 orcs molest you.  I want %A2.",
  "My mother you can get for %A1, this costs %A2.",
  "May your chickens grow lips.  I want %A2 in gold!",
  "Sell this for such a pittance?  Give me %A2 gold.",
  "May the Balrog find you tasty!  %A2 gold pieces?",
  "Your mother was a Troll!  %A2 or I'll tell."
};

static Char comment3a[3][48] = {
  "I'll pay no more than %A1; take it or leave it.",
  "You'll get no more than %A1 from me.",
  "%A1 and that's final."
};

static Char comment3b[15][48] = {
  "%A2 for that piece of junk?  No more than %A1.",
  "For %A2 I could own ten of those.  Try %A1.",
  "%A2?  NEVER!  %A1 is more like it.",
  "Let's be reasonable. How about %A1 gold pieces?",
  "%A1 gold for that junk, no more.",
  "%A1 gold pieces and be thankful for it!",
  "%A1 gold pieces and not a copper more.",
  "%A2 gold?  HA!  %A1 is more like it.",  "Try about %A1 gold.",
  "I wouldn't pay %A2 for your children, try %A1.",
  "*CHOKE* For that!?  Let's say %A1.",  "How about %A1?",
  "That looks war surplus!  Say %A1 gold.",
  "I'll buy it as scrap for %A1.",
  "%A2 is too much, let us say %A1 gold."
};

static Char comment4a[5][48] = {
  "ENOUGH!  You have abused me once too often!",
  "THAT DOES IT!  You shall waste my time no more!",
  "This is getting nowhere.  I'm going home!",
  "BAH!  No more shall you insult me!",
  "Begone!  I have had enough abuse for one day."
};

static Char comment4b[5][32] = {
  "Out of my place!",  "out... Out... OUT!!!",  "Come back tomorrow.",
  "Leave my place.  Begone!",  "Come back when thou art richer."
};

static Char comment5[10][38] = {
  "You will have to do better than that!",  "That's an insult!",
  "Do you wish to do business or not?",  "Hah!  Try again.",
  "Ridiculous!",  "You've got to be kidding!",  "You'd better be kidding!",
  "You try my patience.",  "I don't hear you.",
  "Hmmm, nice weather we're having."
};

static Char comment6[5][29] = {
  "I must have heard you wrong.",  "What was that?",
  "I'm sorry, say that again.",  "What did you say?",
  "Sorry, what was that again?"
};
*/
// number of comments of each type: 14,  3, 16,  3, 15,  5,  5, 10,  5
static Short comment_N_start[9] = {  0, 14, 17, 33, 36, 51, 56, 61, 71 };

//Short last_store_inc;

Short comment_lines = 0;
static void clear_comment()
{
  RectangleType r;
  RctSetRectangle(&r, 0, 2+3*LINE_HEIGHT, SCR_WIDTH-4/*156*/, 4*LINE_HEIGHT);
  WinEraseRectangle(&r, 0);
  comment_lines = 0;
}
static void prt_comment(Char *p)
{
  Short wwlen, wwpixels=SCR_WIDTH-8 /*152*/, x=2, y;
  y = 2 + 3*LINE_HEIGHT + (LINE_HEIGHT * comment_lines);
  while (comment_lines < 4 && StrLen(p) > 0) {
    wwlen = FntWordWrap(p, wwpixels);
    WinDrawChars(p, wwlen, x, y);
    p += wwlen;
    y += LINE_HEIGHT;
    comment_lines++;
  }
}

/* Comments vary.                                       -RAK-   */
/* Comment one : Finished haggling                              */
static void prt_comment1()
{
  Short index = rund(14) + comment_N_start[0];
  prt_comment( get_haggle_string(index) ); // comment1
}

/* %A1 is offer, %A2 is asking.         */
static void prt_comment2(Long offer, Long asking, Short final)
{
  Char comment[80];
  Short index;

  if (final > 0)
    index = rund(3) + comment_N_start[1]; // comment2a
  else
    index = rund(16) + comment_N_start[2]; // comment2b
  StrCopy(comment, get_haggle_string(index) );

  // So, like, this isn't exactly working yet.
  insert_lnum(comment, "%A1", offer, false);
  insert_lnum(comment, "%A2", asking, false);
  prt_comment(comment);
}

static void prt_comment3(Long offer, Long asking, Short final)
{
  Char comment[80];
  Short index;

  if (final > 0)
    index = rund(3) + comment_N_start[3]; // comment3a
  else
    index = rund(15) + comment_N_start[4]; // comment3b
  StrCopy(comment, get_haggle_string(index) );

  insert_lnum(comment, "%A1", offer, false);
  insert_lnum(comment, "%A2", asking, false);
  prt_comment(comment);
}

/* Kick 'da bum out.                                    -RAK-   */
static void prt_comment4()
{
  Short tmp, x=2, y=2+3*LINE_HEIGHT, index;
  Char *p;
  tmp = rund(5);
  index = tmp + comment_N_start[5]; // comment4a
  prt_comment( get_haggle_string(index) );
  index = tmp + comment_N_start[6]; // comment4b
  p = get_haggle_string(index);
  y += (tmp == 3 ? 1 : 2) * LINE_HEIGHT;
  WinDrawChars(p, StrLen(p), x, y);
}

static void prt_comment5()
{
  Short index;
  index = rund(10) + comment_N_start[7]; // comment5
  prt_comment( get_haggle_string(index) );
}

static void prt_comment6()
{
  Short index;
  index = rund(5) + comment_N_start[8]; // comment6
  prt_comment( get_haggle_string(index) );
}

// display_commands: not relevant to me

// haggle_commands: not relevant to me

// display_inventory: equivalent in form_store (make sure I have it all...
// display_cost: do I need that?  I just redraw EVERYthing after a haggle.

// store_prt_gold: equivalent in form_store
// display_store: equivalent in form_store (calls those various other things)
// get_store_item: equivalent in form_store handler ?



/* Increase the insult counter and get angry if too many -RAK-  */
static Boolean increase_insults(Short store_num)
{
  Boolean angry;
  store_type *s_ptr;
  Short offset, ctr, bbuy;
  Long open;

  angry = false;
  s_ptr = &store[store_num];
  ctr = s_ptr->insult_cur + 1;
  if (ctr > owners[s_ptr->owner].insult_max) {
    // Long, Short, Uchar, Uchar, Ushort, Ushort
    // open, insult_cur, owner, ctr, good, bad.
    prt_comment4();
    ctr = 0;
    open = turn + 2500 + randint(2500);
    // first value in struct
    offset = store_num*sizeof(store_type);
    DmWrite(store, offset, &open, sizeof(Long)); // make store closed awhile
    bbuy = s_ptr->bad_buy + 1;
    // last value in struct... see store_type in types.h
    offset += sizeof(Long) + sizeof(Short) + 2*sizeof(UChar) + sizeof(Short);
    DmWrite(store, offset, &bbuy, sizeof(Short)); // "bad_buy++"
    angry = true;
  }
  // skip long
  offset = store_num * sizeof(store_type) + sizeof(Long);
  DmWrite(store, offset, &ctr, sizeof(Short)); // "insult_cur++"
  return angry;
}


/* Decrease insults                                     -RAK-   */
static void decrease_insults(Short store_num)
{
  store_type *s_ptr;
  Short offset, ctr;

  s_ptr = &store[store_num];
  if (s_ptr->insult_cur != 0) {
    offset = store_num * sizeof(store_type) + sizeof(Long);
    ctr = s_ptr->insult_cur - 1;
    DmWrite(store, offset, &ctr, sizeof(Short)); // "insult_cur--"
  }
}


/* Have insulted while haggling                         -RAK-   */
static Boolean haggle_insults(Short store_num)
{
  register int angry;

  angry = false;
  if (increase_insults(store_num))
    angry = true;
  else {
    prt_comment5();
    //   msg_print (CNIL); /* keep insult separate from rest of haggle */
    // hm, will I have to print it in a dedicated line or something?
  }
  return angry;
}


Boolean thrown_out = false;

/********************************************************************/
/*   Whew.  I have made purchase_haggle into an event model thingy. */
/********************************************************************/

/* Haggling routine					-RAK-	*/
// return 0, 1, or 2
typedef struct haggle_foo {
  Long price;
  Long cur_ask, final_ask;
  Long last_offer, new_offer;
  Long min_per, max_per;
  Short num_offer, final_flag;
  Short last_store_inc;
  Short item_val;
  //  Boolean flag, loop_flag, didnt_haggle;
  Boolean didnt_haggle;
  Short store_num;
  inven_type *item;
  Char comment[16];
} haggle_foo;

static haggle_foo h;

void reset_haggle_foo()
{
  h.price = 0;
  h.cur_ask = h.final_ask = 0;
  h.last_offer = h.new_offer = 0;
  h.min_per = h.max_per = 0;
  h.num_offer = h.final_flag = 0;
  h.last_store_inc = 0;
  h.item_val = 0;
  h.didnt_haggle = false;
  // skip store_num
  // skip item
  h.comment[0] = '\0';
}

Long haggle_get_ask()
{
  return h.cur_ask;
}
Long haggle_get_incr()
{
  return (h.last_offer + h.last_store_inc);
}

// Call this when you ENTER the purchase haggle form
// (store_purchase_init will always be called BEFORE entering that form.)
void purchase_haggle_init()
{
  Long cost;
  owner_type *o_ptr = &owners[store[h.store_num].owner];
  //  Char comment[80];

  //  h.flag = false;
  //  purchase = 0;
  // Price, store_num, item have already been set.
  //  h.price = 0;
  //  h.store_num = store_num;
  //  h.item = item;
  h.final_flag = 0;
  h.didnt_haggle = false;
  thrown_out = false;
  clear_comment();

  cost = sell_price(h.store_num, &(h.cur_ask), &(h.final_ask), h.item);

  // First price that the owner asks
  h.cur_ask = h.cur_ask * chr_adj() / 100;
  if (h.cur_ask <= 0)  h.cur_ask = 1;
  // Least price the owner is willing to accept
  h.final_ask = h.final_ask * chr_adj() / 100;
  if (h.final_ask <= 0)  h.final_ask = 1;
  //
  h.min_per  = o_ptr->haggle_per;
  h.max_per  = h.min_per * 3;
  //  haggle_commands(1); not relevant to me
  h.new_offer = 0;
  h.num_offer = 0; /* this prevents incremental haggling on first try */
  StrCopy(h.comment, "Asking");

  /* go right to final price if player has bargained well */
  h.didnt_haggle = noneedtobargain(h.store_num, h.final_ask);
  if (h.didnt_haggle || !my_prefs.store_is_haggling) {
    if (!h.didnt_haggle) // (we are here only because of turning off Haggle)
      h.final_ask = (h.final_ask * 120) / 100; // Spend extra to save time.
    prt_comment("After a long bargaining session, you agree upon the price.");
    h.cur_ask = h.final_ask;
    StrCopy(h.comment, "Final offer");
    /* Set up automatic increment, so a return will accept the final price.  */
    h.last_store_inc = h.final_ask;
    h.num_offer = 1;
    haggle_form_setfield(h.final_ask);
  }
  purchase_haggle_startturn();
}

static void purchase_haggle_startturn()
{
  Char out_val[80];
  StrPrintF(out_val, "%s :  %ld", h.comment, h.cur_ask);
  WinDrawChars(out_val, StrLen(out_val), 2, HAG_Y-LINE_HEIGHT);// above field.
  if (h.num_offer > 1)
    //     && h.last_store_inc != 0)
    haggle_form_enable_autoinc(h.last_store_inc);

}

// Call this when we receive a syntactically correct offer.
// returns FALSE if we should then restrict buttons to "Exit" only.
Short purchase_haggle_endturn(Long new_offer)
{
  Short still_haggling = HAG_ONGOING;
  Char out_val[80];
  Long x1, x2, x3; // temp variables

  clear_comment();
  h.new_offer = new_offer;
  // "receive_offer()" makes sure that your offer is higher than the previous
  if (! (new_offer >= h.last_offer) ) {
    // Your offers are DEcreasing - store owner insults you
    if (haggle_insults(h.store_num)) {
      thrown_out = true;
      return HAG_BOOTED; // owner is really mad, you are thrown out
    } else {
      // in any case, your absurd offer is rejected
      h.new_offer = h.last_offer;
      purchase_haggle_startturn();
      return HAG_ONGOING;
    }
  }
  // Ok, your offer is at least monotonically increasing, sort of thing.

  // Compare your offer to the asking price.
  // If you offered MORE than was asked, something is screwy;
  //    reject it and perhaps reset the autoincrement value.
  // If you offered the asking price, your offer is accepted.
  if (new_offer > h.cur_ask) {
    prt_comment6();
    h.new_offer = h.last_offer;
    if (h.last_offer + h.last_store_inc > h.cur_ask)
      h.last_store_inc = 0;
    purchase_haggle_startturn();
    return HAG_ONGOING;
  } else if (new_offer == h.cur_ask) {
    h.price = new_offer;
    still_haggling = HAG_BOUGHT;
  } // Else new_offer < cur_ask.

  // Ok - if you are still haggling, your offer is less than the asking price
  // (but at least it is >= your previous offer.)
  if (still_haggling == HAG_ONGOING) {
    x1 = (new_offer - h.last_offer) * 100 / (h.cur_ask - h.last_offer);
    if (x1 < h.min_per) {
      // Offer is less of an increment than the owner considers reasonable.
      if (haggle_insults(h.store_num)) {
	thrown_out = true;
	return HAG_BOOTED; // thrown out!
      }
    } else if (x1 > h.max_per) {
      // More of an increment than the owner was expecting.  Decrease it a bit.
      x1 = x1 * 75 / 100;
      if (x1 < h.max_per)  x1 = h.max_per;
    }

    // Calculate the next asking price.
    x2 = x1 + randint(5) - 3;
    x3 = ((h.cur_ask - new_offer) * x2 / 100) + 1;
    if (x3 < 0)  x3 = 0;  // don't let the price go UP
    h.cur_ask -= x3;

    if (h.cur_ask < h.final_ask) {
      h.cur_ask = h.final_ask;
      StrCopy(h.comment, "Final Offer");
      haggle_form_setfield(h.final_ask);
      // Set the automatic haggle increment so that RET will give
      // a new_offer equal to the final_ask price.
      h.last_store_inc = h.final_ask - new_offer;
      h.final_flag++;
      if (h.final_flag > 3) {
	if (increase_insults(h.store_num)) {
	  thrown_out = true;
	  return HAG_BOOTED;
	} else
	  return HAG_REJECTED; // maybe not thrown out, but bargaining is over.
      }
    } else if (new_offer >= h.cur_ask) {
      // accept offer
      h.price = new_offer;
      still_haggling = HAG_BOUGHT;
    }

    if (still_haggling == HAG_ONGOING) {
      h.last_store_inc = new_offer - h.last_offer; // I added this line.
      h.last_offer = new_offer;
      h.num_offer++; // to enable incremental haggling
      StrPrintF(out_val, "Your last : %ld", h.last_offer);
      WinDrawChars(out_val, StrLen(out_val), SCR_WIDTH/2+2, HAG_Y-LINE_HEIGHT); // 1 line above field.
      prt_comment2(h.last_offer, h.cur_ask, h.final_flag);

      // If the current increment would take you over the store's
      // price, then decrease it to an exact match.
      if (h.cur_ask - h.last_offer < h.last_store_inc)
	h.last_store_inc = h.cur_ask - h.last_offer;
    }
  }
  
  //note: anyone who didn't return already and isn't still haggling has BOUGHT.
  if (still_haggling == HAG_ONGOING)
    purchase_haggle_startturn();
  else if (still_haggling == HAG_BOUGHT)
    // record how good a job they did, if they haggled, that is.
    if (!h.didnt_haggle && my_prefs.store_is_haggling)
      updatebargain(h.store_num, h.price, h.final_ask);

  return still_haggling;
}

/********************************************************************/
/*   Whew.  End of purchase_haggle as event model thingy.           */
/********************************************************************/

/********************************************************************/
/*   Ok.  On to sell_haggle foo.                                    */
/********************************************************************/
extern UChar rgold_adj[MAX_RACES][MAX_RACES];
void sell_haggle_init()
{
  Long cost, max_gold;
  owner_type *o_ptr = &owners[store[h.store_num].owner];
  // store_num, price, item are all in h.

  h.price = 0;
  h.final_flag = 0;
  h.didnt_haggle = false;
  thrown_out = false;
  clear_comment();
  // also, here the original code set return value == 0.

  cost = item_value(h.item);
  //  if (cost < 1)
  //    return 3;

  cost = cost * (200 - chr_adj()) / 100;
  cost = cost * (200 - rgold_adj[o_ptr->owner_race][pymisc.prace]) / 100;
  if (cost < 1)  cost = 1;

  // max sell is AKA {min,last}_offer (the first offer owner will make)
  // max buy is AKA cur_ask
  // min buy is AKA final_ask
  h.last_offer = cost * o_ptr->max_inflate / 100;
  /* cast max_inflate to signed so that subtraction works correctly */
  h.cur_ask  = cost * (200 - (Short) o_ptr->max_inflate) / 100;
  h.final_ask  = cost * (200 - o_ptr->min_inflate) / 100;
  if (h.final_ask < 1) h.final_ask = 1;
  if (h.cur_ask < 1) h.cur_ask = 1;
  if (h.final_ask < h.cur_ask)  h.final_ask = h.cur_ask;
  h.min_per  = o_ptr->haggle_per;
  h.max_per  = h.min_per * 3;
  max_gold = o_ptr->max_cost;
  if (max_gold < 1) max_gold = 1; // just in case!!
  h.num_offer = 0;
  h.new_offer = 0;

  if (h.cur_ask > max_gold) {
    h.final_flag = 1;
    StrCopy(h.comment, "Final Offer");
    /* Disable the automatic haggle increment on RET.  */
    h.last_store_inc = 0;
    h.cur_ask   = max_gold;
    h.final_ask = max_gold;
    haggle_form_setfield(h.final_ask);
    prt_comment("I am sorry, but I have not the money to afford such a fine item.");
    h.didnt_haggle = true;
  } else {
    if (h.final_ask > max_gold)
      h.final_ask = max_gold;
    StrCopy(h.comment, "Offer");

    /* go right to final price if player has bargained well */
    h.didnt_haggle = noneedtobargain(h.store_num, h.final_ask);
    if (h.didnt_haggle || !my_prefs.store_is_haggling) {
      if (!h.didnt_haggle) { //(we are here only because of turning off Haggle)
	h.final_ask = (h.final_ask * 80) / 100; // Lose money to save time.
	if (h.final_ask <= 0) h.final_ask = 1;
      }
      prt_comment("After a long bargaining session, you agree upon the price.");
      h.cur_ask = h.final_ask;
      StrCopy(h.comment, "Final offer");
      h.didnt_haggle = true;
      /* Set up automatic increment, so that a return will accept the
	 final price.  */
      h.last_store_inc = h.final_ask;
      h.num_offer = 1;
      // Note - I should also act like the user hit the "[=]" button.
      haggle_form_setfield(h.final_ask);
    }
  }

  sell_haggle_startturn();
}

static void sell_haggle_startturn()
{
  Char out_val[80];
  StrPrintF(out_val, "%s :  %ld", h.comment, h.cur_ask);
  WinDrawChars(out_val, StrLen(out_val), 2, HAG_Y-LINE_HEIGHT); // 1 lines above the field.
  if (h.num_offer > 1)
    //     && h.last_store_inc != 0)
    haggle_form_enable_autoinc(h.last_store_inc);
}

Short sell_haggle_endturn(Long new_offer)
{
  Short still_haggling = HAG_ONGOING;
  Char out_val[80];
  Long x1, x2, x3; // temp variables

  clear_comment();
  h.new_offer = new_offer;
  // "receive_offer()" makes sure that asking price is lower than the previous
  if (! (new_offer <= h.last_offer) ) {
    // Your asking prices are DEcreasing - store owner insults you
    if (haggle_insults(h.store_num)) {
      thrown_out = true;
      return HAG_BOOTED; // owner is really mad, you are thrown out
    } else {
      // in any case, your absurd price is rejected
      h.new_offer = h.last_offer;
      purchase_haggle_startturn();
      return HAG_ONGOING;
    }
  }
  // Ok, your asking price is at least monotonically decreasing, more or less

  // Compare your asking price to the owner's offer
  // If you ask LESS than was offered, something is screwy;
  //    reject it and perhaps reset the autoincrement value.
  // If you ask for the offered price, it is accepted.
  if (new_offer < h.cur_ask) {
    prt_comment6();
    h.new_offer = h.last_offer;
    if (h.last_offer + h.last_store_inc < h.cur_ask)
      h.last_store_inc = 0;
    purchase_haggle_startturn();
    return HAG_ONGOING;
  } else if (new_offer == h.cur_ask) {
    h.price = new_offer;
    still_haggling = HAG_BOUGHT;
  } // Else new_offer > cur_ask.

  // Ok - if you are still haggling, your asking price is less than the offer
  // (but at least it is >= your previous asking price.)
  if (still_haggling == HAG_ONGOING) {
    x1 = (h.last_offer - new_offer) * 100 / (h.last_offer - h.cur_ask);
    if (x1 < h.min_per) {
      // Asking price has dropped less than the owner considers reasonable
      if (haggle_insults(h.store_num)) {
	thrown_out = true;
	return HAG_BOOTED; // thrown out!
      }
    } else if (x1 > h.max_per) {
      // More of a increment than the owner was expecting.  Decrease it a bit.
      x1 = x1 * 75 / 100;
      if (x1 < h.max_per)  x1 = h.max_per;
    }

    // Calculate the next asking price.
    x2 = x1 + randint(5) - 3;
    x3 = ((new_offer - h.cur_ask) * x2 / 100) + 1;
    if (x3 < 0) x3 = 0;    // don't let the price go DOWN
    h.cur_ask += x3;

    if (h.cur_ask > h.final_ask) {
      h.cur_ask = h.final_ask;
      StrCopy(h.comment, "Final Offer");
      haggle_form_setfield(h.final_ask);
      // Set the automatic haggle increment so that RET will give
      // a new_offer equal to the final_ask price.
      h.last_store_inc = h.final_ask - h.new_offer;
      h.final_flag++;
      if (h.final_flag > 3) {
	if (increase_insults(h.store_num)) {
	  thrown_out = true;
	  return HAG_BOOTED;
	} else
	  return HAG_REJECTED; // maybe not thrown out, but bargaining is over.
      }
    } else if (new_offer <= h.cur_ask) {
      // accept offer
      h.price = new_offer;
      still_haggling = HAG_BOUGHT;
    }
    if (still_haggling == HAG_ONGOING) {
      h.last_store_inc = new_offer - h.last_offer; // I added this line.
      h.last_offer = new_offer;
      h.num_offer++; // to enable incremental haggling
      StrPrintF(out_val, "Your last : %ld", h.last_offer);
      WinDrawChars(out_val, StrLen(out_val), SCR_WIDTH/2+2, HAG_Y-LINE_HEIGHT); // 1 line above field.
      prt_comment3(h.cur_ask, h.last_offer, h.final_flag);

      // If the current decrement would take you under the store's
      // price, then increase it to an exact match.
      if (h.cur_ask - h.last_offer > h.last_store_inc)
	h.last_store_inc = h.cur_ask - h.last_offer;
    }
  }
  //note: anyone who didn't return already and isn't still haggling has BOUGHT.
  if (still_haggling == HAG_ONGOING)
    sell_haggle_startturn();
  else if (still_haggling == HAG_BOUGHT)
    // record how good a job they did, if they haggled, that is.
    if (!h.didnt_haggle && my_prefs.store_is_haggling)
      updatebargain(h.store_num, h.price, h.final_ask);

  return still_haggling;

}

/************************
 * Still need Store Sell
 * Still need Enter Store
 ************************/

static inven_type sell_obj;
Boolean store_purchase_init(Short store_num, Short item_val)
{
  //  store_type *s_ptr = &store[store_num];
  inven_record *storeinv = store_inven[store_num];
  take_one_item(&sell_obj, &storeinv[item_val].sitem);
  h.price = 0;
  h.store_num = store_num;
  h.item = &sell_obj;
  h.item_val = item_val;
  thrown_out = false;
  if (inven_check_num(&sell_obj)) {
    // Some things (such as a hard biscuit) have a fixed cost after you
    // have bought one of them.  If this is the case, skip the haggling!
    // Heck don't even pop up the form.
    if (storeinv[item_val].scost > 0) {
      h.price = storeinv[item_val].scost;
      // Hey, call the "I bought an object" stuff here too.
      //      show_brief_comment("Done!");
      // You have a Hard Biscuit. (c) // You have 3 Hard Biscuits. (c)
      store_purchase_end(false); // hope this works!!!!
      return false; // do NOT pop up a reg'lar purchase haggle form.
    } else
      return true; // DO pop up a form
  } else {
    //    show_brief_comment("You cannot carry that many different items.");
    show_brief_comment("You cannot carry that.");
    return false;
  }
}

// theoretically this should work whether youa re in the haggle form
// or the store form.  thus in_haggle.
void store_purchase_end(Boolean in_haggle)
{
  Char tmp_str[160], out_val[160];
  Short item_new, i;
  store_type *s_ptr = &store[h.store_num];
  // First, set the buttons so that you can only Leave the form.
  if (!thrown_out) {
    if (pymisc.au >= h.price) {
      if (in_haggle) prt_comment1(); // we'll skip it for fixed price items
      decrease_insults(h.store_num);
      pymisc.au -= h.price;
      item_new = inven_carry(h.item);
      i = s_ptr->store_ctr;
      store_destroy(h.store_num, h.item_val, true); // Is this working??????
      objdes(tmp_str, &inventory[item_new], true);
      StrPrintF(out_val, "You have %s (%c)", tmp_str, (Char)(item_new + 'a'));
      if (in_haggle) {
	// it would be nice to use the two lines of space we have
	Short len = StrLen(out_val);
	Short wwlen = FntWordWrap(out_val, SCR_WIDTH-8/*156-2-2*/);
	if (wwlen >= len) // it can fit on one line
	  WinDrawChars(out_val, len, 2, HAG_Y-LINE_HEIGHT);
	else {
	  WinDrawChars(out_val, wwlen, 2, HAG_Y-2*LINE_HEIGHT);
	  WinDrawChars(out_val+wwlen, len-wwlen, 2, HAG_Y-LINE_HEIGHT);
	}
      }
      else
	show_brief_comment(out_val);
      check_strength();
      //      if (*cur_top >= s_ptr->store_ctr)	{
      //	*cur_top = 0;
      //	display_inventory(store_num, *cur_top);
      //      } else {
      //	  r_ptr = &s_ptr->store_inven[item_val];
      if (i == s_ptr->store_ctr) {
	// make price fixed
	inven_record *storeinv, *r_ptr;
	storeinv = store_inven[h.store_num];
	r_ptr = &storeinv[h.item_val];
	if (r_ptr->scost < 0) {
	  // Hey, will this work?
	  Short offset = h.item_val * sizeof(inven_record);
	  DmWrite(storeinv, offset, &h.price, sizeof(Long));
	  //	  display_cost(store_num, item_val);
	}
      }
      if (!in_haggle)
	store_list_tens(0);
      //	  if (i == s_ptr->store_ctr)
      //	    {
      //	      if (r_ptr->scost < 0)
      //		{
      //		  r_ptr->scost = price;
      //		  display_cost(store_num, item_val);
      //		}
      //	    }
      //	  else
      //	    display_inventory(store_num, item_val);
      //	}
      //      store_prt_gold();
    } else {
      if (increase_insults(h.store_num))
	thrown_out = true;
      else {
	prt_comment1();
	StrCopy(out_val, "Liar!  You have not the gold!");
	if (in_haggle)
	  WinDrawChars(out_val, StrLen(out_val), 2, HAG_Y-LINE_HEIGHT);
	else show_brief_comment(out_val);
      }
    }
  }
  // change some buttons so that you can only exit.
  if (in_haggle)
    haggle_form_boot(thrown_out, "Ok");
}


// Return false if you should NOT pop up the haggle form.
// Note: you CAN get thrown out WITHOUT entering the haggle form.
//
// ALSO, I will add a menu item [xyzzy] to avoid haggling but you
// will suffer an n% loss.  Have it return false in this case but
// you DO sell the item.  call store_sell_end or something (it redraws inv)
Boolean store_sell_init(Short store_num, Short item_val)
{
  Boolean flag;
  // make a 1-quantity COPY of the inventory item
  take_one_item(&sell_obj, &inventory[item_val]);
    h.price = 0;
    h.store_num = store_num;
    h.item = &sell_obj;
    h.item_val = item_val;
    thrown_out = false;

  // actually, we already checked this before enabling the Sell button,
  // but just in case.
  flag = is_store_buy(store_num, inventory[item_val].tval);
  if (!flag) {
    show_brief_comment("I do not buy such items.");
    return false;
  }
  if (item_value(h.item) < 1) {
    show_brief_comment("How dare you!  I will not buy that!");
    if (increase_insults(store_num))
      thrown_out = true; // Callee must check it, and disable all but [Exit].
    return false;
  }
  if (store_check_num(&sell_obj, store_num)) {
    return true;
  } else {
    show_brief_comment("I have not the room here to keep it.");
    return false;
  }
}

void store_sell_end(Boolean in_haggle)
{
  Char tmp_str[160], out_val[160];
  Short item_pos; // item_new, i, item_pos;
  //  store_type *s_ptr = &store[h.store_num];
  // change some buttons so that you can only exit.
  if (in_haggle)
    haggle_form_boot(thrown_out, "Ok");
  if (!thrown_out) {
    if (in_haggle) prt_comment1(); // we'll skip it for fixed price items
    decrease_insults(h.store_num);
    pymisc.au += h.price;
    /* identify object in inventory to set object_ident */
    identify(&h.item_val);
    /* retake sold_obj so that it will be identified */
    take_one_item(h.item, &inventory[h.item_val]);
    /* call known2 for store item, so charges/pluses are known */
    known2_local(h.item); // special version of known2 for object in heap. sigh
    inven_destroy(h.item_val);
    objdes(tmp_str, h.item, true);
    StrPrintF(out_val, "You've sold %s", tmp_str);
    if (in_haggle) {
      // it would be nice to use the two lines of space we have
      Short len = StrLen(out_val);
      Short wwlen = FntWordWrap(out_val, SCR_WIDTH-8/*156-2-2*/);
      if (wwlen >= len) // it can fit on one line
	WinDrawChars(out_val, len, 2, HAG_Y-LINE_HEIGHT);
      else {
	WinDrawChars(out_val, wwlen, 2, HAG_Y-2*LINE_HEIGHT);
	WinDrawChars(out_val+wwlen, len-wwlen, 2, HAG_Y-LINE_HEIGHT);
      }
    }
    else
      show_brief_comment(out_val);
    store_carry(h.store_num, &item_pos, h.item);
    check_strength(); // hey, this prints messages.  is that a good idea?...
    calc_bonuses(); // in case we sold an equipped armor/weapon
    if (!in_haggle)
      store_list_tens(0);
  }
}


extern Short in_store; // Need to know which store we are in!
extern Short store_item; // so that Haggle knows what we're bidding on
extern Short store_current_tens;
extern Boolean in_a_store; 

// return true if you can enter it
Boolean enter_store(Short store_num)
{
  store_type *s_ptr;
  s_ptr = &store[store_num];
  if (s_ptr->store_open >= turn) {
    message("The doors are locked.");
    return false;
  } else if (s_ptr->store_ctr <= 0) {
    message("(Bug!) The store has nothing to sell!");
    return false;
  }
  // elsewhere
  in_a_store = true;
  in_store = store_num;
  store_current_tens = 0;
  store_item = -1;
  // here
  comment_lines = 0;
  thrown_out = false;
  FrmPopupForm(StoreForm);
  return true;
}
