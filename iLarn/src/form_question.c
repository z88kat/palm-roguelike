#include "palm.h"
#include "iLarnRsc.h"
#include "Globals.h"

/***********************************************************************
 * For most kinds of items, when you walk on them, you get to choose
 * up to 4 possible actions.  This is the form that lets you select
 * which action to perform.  The buttons are relabeled or disabled
 * according to the kind of item.
 ***********************************************************************/

Boolean dspchQuestionForm(Short button) SEC_2;
extern Char question_hotkeys[4];
extern Short in_store;

// 5-way support: the navigator moves a frame between the buttons (in the
// order One, Two, Three, Ignore) and the centre button presses the framed
// one.  Nothing is framed until the navigator is first used; then the
// centre button presses the first choice.
static Short qst_focus = -1; // 0..3 = btn_qst_1 + qst_focus, -1 = none

static Boolean qst_usable(FormPtr frm, Short i)
{
  ControlPtr btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_qst_1+i));
  return btn->attr.usable;
}

static void qst_toggle_frame(FormPtr frm)
{
  RectangleType r;
  if (qst_focus < 0) return;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, btn_qst_1 + qst_focus), &r);
  r.topLeft.x -= 1;  r.topLeft.y -= 1;
  r.extent.x += 2;   r.extent.y += 2;
  WinInvertRectangleFrame(simpleFrame, &r); // inverting twice erases it
}

// Move the frame to the next (step 1) or previous (step -1) usable button.
static void qst_move_focus(FormPtr frm, Short step)
{
  Short i, f = qst_focus;
  if (f < 0) f = (step > 0) ? -1 : 4;
  for (i = 0 ; i < 4 ; i++) {
    f += step;
    if (f < 0 || f > 3) return; // no more buttons that way
    if (qst_usable(frm, f)) break;
  }
  if (i == 4) return;
  qst_toggle_frame(frm);
  qst_focus = f;
  qst_toggle_frame(frm);
}
Boolean Question_Form_HandleEvent(EventPtr e)
{
  Boolean handled = false;
  FormPtr frm;
  Boolean took_time = false;
  Short i, j;
  //  CharPtr tmpstr;
  //  ControlPtr btn; 
  switch (e->eType) {

  case frmOpenEvent:
    frm = FrmGetActiveForm();
    FrmDrawForm(frm);
    setupQuestionForm(frm);
    qst_focus = -1;
    qst_move_focus(frm, 1); // frame the default button for the 5-way
    /* // for testing interesting api calls, when I was experimenting
       // with HandEra support in iRogue.
    {
      RectangleType r;
      Char buf[40];
      //      FrmGetFormBounds(frm, &r);
      WinGetWindowBounds(&r);
      StrPrintF(buf, "%d %d, %d %d", r.topLeft.x, r.topLeft.y,
		r.extent.x, r.extent.y);
      WinDrawChars(buf, StrLen(buf), 5, 15);
    }
    */
    handled = true;
    break;

  case keyDownEvent:
    // Act like we pressed a button.  For keyboard support.
    frm = FrmGetActiveForm();
    switch (fiveway_key(e)) {
    case NAV_NONE:
      break;
    case NAV_UP: case NAV_LEFT:
      qst_move_focus(frm, -1);
      return true;
    case NAV_DOWN: case NAV_RIGHT:
      qst_move_focus(frm, 1);
      return true;
    case NAV_SELECT:
      if (qst_focus < 0)
	qst_move_focus(frm, 1);
      if (qst_focus >= 0)
	CtlHitControl(FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, btn_qst_1 + qst_focus)));
      return true;
    default:
      return true;
    }
    i = -1;
    for (j = 0 ; j < 4 ; j++) {
      if ((e->data.keyDown.chr == '1' + j) ||
	  (e->data.keyDown.chr == question_hotkeys[j]))
	i = btn_qst_1 + j;
    }
    if (i == -1) return handled;
    else {
      ControlPtr btn = FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, i));
      if (btn->attr.usable) 
	CtlHitControl(btn);
      handled = true;
    }
    break;

  case ctlSelectEvent:
    LeaveForm(); // LeaveForm(false);
    handled = true;
 //  print_stats(env); // why is this here? because the form was not redrawing?
    switch(e->data.ctlSelect.controlID) {
    case btn_qst_1:
      took_time = dspchQuestionForm(btn_qst_1 - btn_qst_1);
      break;
    case btn_qst_2:
      took_time = dspchQuestionForm(btn_qst_2 - btn_qst_1);
      break;
    case btn_qst_3:
      took_time = dspchQuestionForm(btn_qst_3 - btn_qst_1);
      break;
    case btn_qst_ign:
      took_time = dspchQuestionForm(btn_qst_ign - btn_qst_1);
      break;
    }
    env->dropflag = true;
    start_of_Play_Level_loop(); // make it ALWAYS take time!
    // [Errr.  There's just one problem with this; in_store gets reset. Fixed]
    // ... because really we jumped to the popup while IN the middle of
    // that.. and setting dropflag will make us pick up where we left off....
    //
    // this is how i used to be, but really Ularn makes them ALL take time! :
    //    if (false && took_time) {
    //      env->dropflag = true;
    //      start_of_Play_Level_loop();
    //    }
    //    // else if (env->pending_splash)
    //    //      show_messages();  // FrmPopupForm(SplashForm);
    break;

  default:
    break;
  }
  return handled;
}
