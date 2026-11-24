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
#define MainForm              1000
#define ItsyFont              1009
#define MainFormMenu          1010
//#define menu_mainMap          1011 --> no, 4077
#define menu_mainSettings     1012
#define menu_mainButtons      1013
#define menu_mainInstruct     1014
#define menu_mainAbout        1015
#define menu_mainFont         1016
//#define menu_mainRecall       1017
#define menu_mainStats        1018
#define menu_mainMsgs         1019
#define menu_mainScores       1020
#define menu_mainWiz       3980
#define menu_mainWiz2      3981
#define menu_mainRepeaters 3982
#define menu_mainSpellBtns 3983
#define menu_mainBeam      3984
#define menu_mainTime      3985
#define menu_mainUser      3986

#define bmp_cmd_map          3701
#define bmp_cmd_log          3702
#define bmp_cmd_you          3703
#define bmp_cmd_fnt          3704
#define bmp_cmd_compact      3711
#define bmp_cmd_wts          3712
#define bmp_cmd_wtsOff       3713
#define bmp_cmd_hgl          3721
#define bmp_cmd_hglOff       3722
#define bmp_cmd_amnesia      3731


#define AboutStr              1021
#define QuitP                 1022
#define MapForm               1023
#define LowMageP              1024
#define LowPriestP            1025
#define DropAllP              1026
#define ExceedP               1027
#define PickupP               1028
//#define DropWornP             1029
#define RingHandP             1030
#define CursedP               1031
#define RestP                 1032
#define RecallClearP          1033
#define RecallClear2P         1034
#define CrashP                1035
#define DigSlowlyP            1036
#define BeamNextP             1037
#define CrownForm             1040
#define RetireP               1041

#define InvHelpStr 1050
#define EquHelpStr 1051
#define StoreHelpStr 1052
//#define HaggleHelpStr 1053
//#define RecallHelpStr 1054
#define BookHelpStr 1055
#define SpellHelpStr 1056
#define GainSpellHelpStr 1057
#define PrefsHelpStr 1058
#define DeathStr 1059
#define ButtonsStr 1060
#define CreditStr 1061
#define InstructStr 1062

#define PrefsForm             1100
// keep 1101-1104 consecutive:
#define list_prf_1            1101
#define list_prf_2            1102
#define label_prf_1    1103
#define label_prf_2    1104
#define btn_prf_ok            1105
#define btn_prf_cancel        1106
#define pbtn_prf_0            1107
#define pbtn_prf_1            1108
// keep these checkboxes consecutive:
#define check_prf_1           1111
#define check_prf_2           1112
#define check_prf_3           1113
#define check_prf_4           1114
#define check_prf_5           1115
#define check_prf_6           1116
#define check_prf_7           1117
#define check_prf_8           1118
#define check_prf_9           1119
#define check_prf_10          1120
#define check_prf_11          1121
#define check_prf_12          1122
#define check_prf_13          1123
#define check_prf_14          1124
#define check_prf_15          1125
#define check_prf_16          1126
#define check_prf_17          1127


#define HwButtonsForm         1140
#define btn_hwb_ok            1141
#define btn_hwb_cancel        1142
//#define check_hwb_keyb        1143
#define check_hwb             1144
#define pbtn_hwb_0            1145
#define pbtn_hwb_1            1146
#define pbtn_hwb_2            1147
#define pbtn_hwb_3            1148
#define pbtn_hwb_4            1149
#define pbtn_hwb_5            1150
#define pbtn_hwb_6            1151
#define pbtn_hwb_7            1152
#define list_hwb_1            1153


#define InvForm         1200
#define list_if         1201
// do NOT change these three numbers (frob, drop, throw)
#define btn_if_frob     1202
#define btn_if_drop     1203
#define btn_if_throw    1204
#define btn_if_eq       1205
#define btn_if_cancel   1206
#define InvFormMenu     1207
#define menu_invCompact 1208
#define menu_invWeights 1209

#define InvSelectForm 1211
#define list_is       1212
#define btn_is_frob   1213
#define btn_is_cancel 1214

#define EquForm      1220
#define list_eq      1221
#define btn_eq_off   1202
#define btn_eq_drop  1203
//#define btn_eq_throw 1204
#define btn_eq_if    1205
#define btn_eq_cancel 1206

#define StoreForm    1230
#define list_st      1231
#define btn_st_buy   1232
#define btn_st_prev  1233
#define btn_st_next  1234
#define btn_st_sell  1235
#define btn_st_exit  1236
#define StoreFormMenu   1237
#define menu_stoCompact 1238
#define menu_stoHaggle  1239
#define HaggleForm   1240
#define field_hag    1241
#define btn_hag_all  1242
#define btn_hag_ok   1243
#define btn_hag_exit 1244
#define btn_hag_incr 1245
#define btn_hag_boot 1246

#define BookForm         1260
#define list_book        1261
#define btn_book_ok      1262
#define btn_book_cancel  1263
#define SpellForm        1264
#define list_spell	 1265
#define btn_spell_ok     1266  
#define btn_spell_cancel 1267
#define GainSpForm       1270
#define list_gain        1271
#define btn_gain_ok      1272
#define btn_gain_cancel  1273
#define btn_gain_tog     1274


#define RecallForm         1300
#define field_recall       1301
#define repeat_recall_up   1302
#define repeat_recall_down 1303
#define btn_recall_ok      1304
#define btn_recall_next    1305
#define menu_recClear      1306
#define RecallFormMenu     1307

#define MsgLogForm         1310
#define field_ml           1311
#define repeat_ml_up       1312
#define repeat_ml_down     1313
#define btn_ml_ok          1314

#define ScribeForm         1320
#define field_sb           1321
#define btn_sb_ok          1322
#define btn_sb_cancel      1323

#define LookForm           1330
#define btn_look_exit      1331
#define btn_look_recall    1332

#define DalekForm          1340
#define popup_dal_1        1341
#define list_dal_1         1342
#define btn_dal_ok         1343
#define btn_dal_cancel     1344

#define RepeatCmdsForm     1350
#define btn_rptc_ok        1351
#define btn_rptc_cancel    1352
#define check_rptc_1       1353
#define check_rptc_2       1354
#define check_rptc_3       1355
#define check_rptc_4       1356
#define check_rptc_5       1357

#define SpellHwBtnsForm       1440
#define btn_shwb_ok           1441
#define list_shwb_1           1442
#define popup_shwb_2          1443
#define list_shwb_2           1444
#define btn_shwb_clear        1445
// keep these buttons "in order"
#define pbtn_shwb_0           1450
#define pbtn_shwb_1           1451
#define pbtn_shwb_2           1452
#define pbtn_shwb_3           1453
#define pbtn_shwb_4           1454
#define pbtn_shwb_5           1455
#define pbtn_shwb_6           1456
#define pbtn_shwb_7           1457

#define UserForm         1470
#define btn_user_ok      1471
#define btn_user_new     1472
#define btn_user_cancel  1474
#define check_user_1     1475
#define list_user        1477

#define IntroForm            3019
#define AboutForm            3020
#define btn_about_ok         3021
#define bitmap_moria         3022
#define btn_about_more       3024
#define btn_about_credits    3025
#define DeathForm            3030
#define btn_death_ok         3031
#define btn_death_wok        3032
#define ScoresForm           3040
#define btn_scores_ok        3041
#define btn_scores_next      3042

#define SnowCrashForm  3050
#define RestForm       3060
#define btn_rest_oops  3061
#define btn_rest_10    3062
#define btn_rest_20    3063
#define btn_rest_max   3064
#define BeamForm       3070
#define btn_beam_myself   3071
#define btn_beam_item     3072
#define btn_beam_scores   3073
#define btn_beam_done     3074
#define btn_beam_cash     3075
#define BeamCashForm      3080
#define field_bc          3081
#define btn_bc_ok         3082
#define btn_bc_cancel     3083

// Character generation.
#define CharGenStr       3090
#define CharRaceForm     3091
#define CharGenderForm   3092
#define CharClassForm    3093
#define CharSummaryForm  3094
#define pbtn_race_0      3100
#define pbtn_race_1      3101
#define pbtn_race_2      3102
#define pbtn_race_3      3103
#define pbtn_race_4      3104
#define pbtn_race_5      3105
#define pbtn_race_6      3106
#define pbtn_race_7      3107
#define btn_race_ok      3109
#define pbtn_gender_0    3110
#define pbtn_gender_1    3111
#define btn_gender_no    3112
#define btn_gender_ok    3119
#define pbtn_class_0     3120
#define pbtn_class_1     3121
#define pbtn_class_2     3122
#define pbtn_class_3     3123
#define pbtn_class_4     3124
#define pbtn_class_5     3125
#define btn_class_no     3126
#define btn_class_reroll 3127
#define btn_class_ok     3128
#define btn_class_ok2    3129
#define field_charsum    3130
#define btn_charsum_no   3138
#define btn_charsum_ok   3139

// Do not mess with these numbers.  They are an ascii-related hack.
#define MAGIC_MENU_NUMBER 4000
#define menu_cmd_space   4032
#define menu_cmd_bang    4033
#define menu_cmd_pound   4035
#define menu_cmd_dollar  4036
#define menu_cmd_minus   4045
#define menu_cmd_dot     4046
#define menu_mainRecall  4047
#define menu_cmd_lt      4060
#define menu_cmd_gt      4062
#define menu_cmd_look    4063
#define menu_cmd_G       4071
#define menu_mainMap     4077
#define menu_cmd_quit    4081
#define menu_cmd_R       4082
#define menu_cmd_S       4083
#define menu_cmd_disarm  4094
#define menu_cmd_a       4097
#define menu_cmd_c       4099
#define menu_cmd_d       4100
#define menu_cmd_e       4101
#define menu_cmd_f       4102
#define menu_cmd_g       4103
#define menu_cmd_i       4105
#define menu_cmd_m       4109
#define menu_cmd_o       4111
#define menu_cmd_p       4112
#define menu_cmd_r       4114
#define menu_cmd_s       4115
#define menu_cmd_t       4116
#define menu_cmd_x       4120
#define menu_cmd_z       4122
#define menu_cmd_scribe  4123

