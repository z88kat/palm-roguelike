#ifdef I_AM_COLOR

extern Boolean found_color; // in lock.c: indicates existence of color array
extern IndexedColorType black, white;

// for these fns, caller must check: if (found_color && my_prefs.color_on)

#define start_color()     { WinPushDrawState(); if (my_prefs.black_bg) { WinSetBackColor(black); } }
#define start_color_all()    { WinPushDrawState(); if (my_prefs.black_bg) { WinSetBackColor(black); WinSetTextColor(white); WinSetForeColor(white); } }
#define stop_color()   WinPopDrawState()
IndexedColorType get_color(Char c, Short x, Short y);


//extern IndexedColorType *mon_colors;


#endif // I_AM_COLOR
