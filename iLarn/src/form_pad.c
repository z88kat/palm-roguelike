#include "palm.h"
#include "iLarnRsc.h"
#include "Globals.h"
/***********************************************************************
 * This is the form for Dealer McDope's pad, a store where you can buy
 * four kinds of drugs.  Hey I don't make this stuff up, I just port it.
 ***********************************************************************/


Boolean Pad_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  ListPtr lst;
  //  CharPtr label;
  Short store_item;

  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    /* ...init form... */
    FrmDrawForm(frm);
    pad_gold();
    handled = true;
    break;

  case lstSelectEvent:
    handled = true;
    break;

  case keyDownEvent:
    // 5-way: up / down pick a drug, the centre buys it, left leaves.
    frm = FrmGetActiveForm();
    switch (fiveway_key(e)) {
    case NAV_UP: case NAV_DOWN:
      fiveway_list(FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_pad)),
		   fiveway_key(e));
      handled = true;
      break;
    case NAV_SELECT:
      fiveway_hit(frm, btn_pad_buy);
      handled = true;
      break;
    case NAV_LEFT:
      fiveway_hit(frm, btn_pad_split);
      handled = true;
      break;
    case NAV_NONE:
      break;
    default:
      handled = true;
      break;
    }
    break;

  case ctlSelectEvent:
    frm = FrmGetActiveForm();
    switch(e->data.ctlSelect.controlID) {
    case btn_pad_buy:
      lst = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, list_pad));
      store_item = LstGetSelection(lst);
      if (store_item >= 0 && store_item <= 4)
	pad_buy(frm, store_item);
      handled = true;
      break;
    case btn_pad_split:
      LeaveForm(); // LeaveForm(true);
      //      message_clear(); // do I need this?
      print_stats(); // in case you spent money
      handled = true;
      break;
    }
    break;

  default:
    break;
  }

  return handled;
}
