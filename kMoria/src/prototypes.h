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
// main.c
// default section
// forms
Boolean About_Form_HandleEvent(EventPtr e) SEC_3;
Boolean Intro_Form_HandleEvent(EventPtr e) SEC_3;
Boolean Inv_Form_HandleEvent(EventPtr e) SEC_7;
void invOrEqu_drop(Short item) SEC_7;
void refill_lamp() SEC_7;
void take_a_turn(Boolean mainform) SEC_5;
Boolean Equ_Form_HandleEvent(EventPtr e) SEC_7;
Boolean Recall_Form_HandleEvent(EventPtr e) SEC_7;
Boolean Look_Form_HandleEvent(EventPtr e) SEC_6;
Boolean look_btnsHandleEvent(Short btn_i) SEC_7;
Boolean CharRace_Form_HandleEvent(EventPtr e) SEC_3;
Boolean CharGender_Form_HandleEvent(EventPtr e) SEC_3;
Boolean CharClass_Form_HandleEvent(EventPtr e);
Boolean CharSummary_Form_HandleEvent(EventPtr e);
Boolean Store_Form_HandleEvent(EventPtr e) SEC_6;
Boolean Haggle_Form_HandleEvent(EventPtr e) SEC_6;
void haggle_form_setfield(Long amt) SEC_6;
void haggle_form_enable_autoinc(Short incr) SEC_4;
void store_form_boot() SEC_6;
void haggle_form_boot(Boolean kick_out, Char *str) SEC_4;
void store_list_tens(Short dir) SEC_6;
Boolean Book_Form_HandleEvent(EventPtr e) SEC_4;
Boolean Spell_Form_HandleEvent(EventPtr e) SEC_4;
Boolean book_btnsHandleEvent(Short btn_i) SEC_7;
Boolean spell_btnsHandleEvent(Short btn_i) SEC_7;
Boolean Dalek_Form_HandleEvent(EventPtr e) SEC_3;
Boolean GainSp_Form_HandleEvent(EventPtr e) SEC_4;
Boolean MsgLog_Form_HandleEvent(EventPtr e) SEC_5;
Boolean Rest_Form_HandleEvent(EventPtr e) SEC_7;
Boolean Prefs_Form_HandleEvent(EventPtr e) SEC_5;
void useMoriaPrefs() SEC_5;
void writeMoriaPrefs() SEC_5;
Boolean HwButtons_Form_HandleEvent(EventPtr e) SEC_6;
Boolean Map_Form_HandleEvent(EventPtr e) SEC_5;
Boolean Death_Form_HandleEvent(EventPtr e) SEC_5;
Boolean Crown_Form_HandleEvent(EventPtr e) SEC_5;
Boolean Scores_Form_HandleEvent(EventPtr e) SEC_5;
Boolean InvS_Form_HandleEvent(EventPtr e) SEC_6;
Boolean Scribe_Form_HandleEvent(EventPtr e) SEC_6;
Boolean SnowCrash_Form_HandleEvent(EventPtr e) SEC_6;
Boolean RepeatCmds_Form_HandleEvent(EventPtr e) SEC_6;
Boolean SpellHwBtns_Form_HandleEvent(EventPtr e) SEC_7;
Boolean Beam_Form_HandleEvent(EventPtr e) SEC_7;
Boolean BeamCash_Form_HandleEvent(EventPtr e) SEC_7;
Boolean User_Form_HandleEvent(EventPtr e) SEC_7;
void toggle_hotsync(LocalID dbid, Boolean backup) SEC_7;


// These are used only by their own form_*.c AND by handera.c.
// In alphabetical order of form_*.c in which they appear:
void beamcash_draw() SEC_7;
void charrace_draw() SEC_3;
void chargender_draw() SEC_3;
void charclass_draw(Boolean all, Boolean squeeze);
void charsummary_draw();
void dalek_draw() SEC_3;
void crown_draw() SEC_5;
void tombstone_draw() SEC_5;
void look_draw(Boolean first) SEC_6;
void map_draw() SEC_6;
void scribe_draw() SEC_6;
void show_mana_at_bottom() SEC_4; // for BookForm.
void spell_draw() SEC_4;
void gainsp_draw(Boolean first) SEC_4;
void show_spell_char(Short i) SEC_7; // for SpellHwBtns.
void free_store_list(FormPtr frm) SEC_6;
//void store_draw() SEC_6;
void haggle_draw() SEC_6; // doesn't refresh everything; can't; sigh.
Short draw_topscores(Boolean reset) SEC_6;
void show_user_info(FormPtr frm, Short i) SEC_7; // for User.

// dungeon.c
void enter_new_level() SEC_5;
void start_of_turn() SEC_5;
void end_of_turn() SEC_5;
Boolean do_command(Char com_val) SEC_5;
Boolean do_parsed_command(previous_state *ps) SEC_7;
Boolean do_xy_command(Short x, Short y) SEC_5;
Boolean do_dir_command(Short dir, Short XXX_item) SEC_5;
Char map_roguedir(Char *comval) SEC_5;
Char unmap_roguedir(Short dir) SEC_5;
void ident_monster(Char c) SEC_6;
void do_again() SEC_7;

// display.c
// default section
Boolean toggle_itsy();
void set_statsbar_loc(Boolean stats_at_top);
void where_in_dungeon(Short scr_x, Short scr_y, Short *dun_x, Short *dun_y);
void clear_screen();
void print(Short ch, Short col, Short row);
//void setscreen(Short col, Short row, Short ch);
void recalc_screen();
void animate_char(Short row, Short col, Char c, Boolean bold) SEC_6;
void refresh(Boolean clearall);
void recalc_and_refresh();
void nuke_display() SEC_5;;
void relativize_move(Short *x, Short *y) SEC_5;
void change_visible_xy(Short x, Short y);
void slide_dir(Short dir);
//
void count_message(Char *buf);
void level_message(Char *buf);
void clear_brief_comment();
void show_brief_comment(Char *buf);
Boolean in_brief_comment(Short x, Short y);
void message(Char *buf);
//#define unmessage() message(0)
Boolean message_clear(Boolean really);
void alert_message(Char *buf);
void preempt_messages();
void show_messages();
void show_a_more(Short lines_used) SEC_6;
void alloc_message_log();
void print_stats(UInt statsmode);
void update_status_words() SEC_5;
Boolean panel_contains(Short a, Short b); // FAKE!
Boolean get_panel(Short row, Short col, Boolean foo); // FAKE!
void show_time_and_volts() SEC_7;


// misc1.c
// all section1
void seed_random() SEC_1;
void set_seed(ULong seed) SEC_1;
void reset_seed() SEC_1;
Int randint(Int y) SEC_1;
Int rund(Int y) SEC_1;
Short randnor(Short mean, Short stand) SEC_1;// not 'correctly' implemented at presnt
Short bit_pos(ULong *test) SEC_1;
Boolean in_bounds(Short y, Short x) SEC_1;
Short distance(Short y1, Short x1, Short y2, Short x2) SEC_1;
Short damroll(Short num, Short sides) SEC_1;
Short pdamroll(UChar *array) SEC_1;
Boolean los(Short fromY, Short fromX, Short toY, Short toX) SEC_1;
UChar loc_symbol(Short y, Short x) SEC_1;
Boolean test_light(Short y, Short x) SEC_1;
Boolean compact_monsters() SEC_2;
void add_food(Short num) SEC_1;
Short popm() SEC_1;
Short max_hp(UChar *array) SEC_1;
Boolean place_monster(Short y, Short x, Short z, Boolean slp) SEC_1;
void place_win_monster() SEC_1;
Short get_mons_num(Short level) SEC_1;
void alloc_monster(Short num, Short dist, Boolean sleep) SEC_1;
Boolean summon_monster(Short *y, Short *x, Boolean slp) SEC_1;
Boolean summon_undead(Short *y, Short *x) SEC_1;
Short popt() SEC_1;
void pusht(UChar x) SEC_1;
Boolean magik(Short chance) SEC_1;
Short m_bonus(Short base, Short max_std, Short level) SEC_1;

// generate.c
// all section1
void generate_cave() SEC_1;

// lock.c
// default section
void re_init() SEC_5;
void data_record_create();
void data_record_lock(Boolean unlocked); // call at startup and shutdown
//void create_cave_rec();
void blank_cave();
void cave_set(Short row, Short col, Short element, UChar val);
//void create_tlist_rec();
void blank_tlist();
// mess with objects-on-level
#define tlist_set_p1 invy_set_p1
#define tlist_set_cost invy_set_cost
#define tlist_set_flags invy_set_flags
#define tlist_set_name2 invy_set_name2
//void tlist_set_p1(Short index, Short p1);
//void tlist_set_flags(Short index, ULong flags);
void tlist_mv(Short from, Short to);
void invcopy(inven_type *to_rec, Short to_index, Short from_index);//was desc.c
void storeinv_nullify(inven_record *to_rec, Short to_index) SEC_5;
//void storeinvcopy(inven_record *to_rec, Short to_index, Short from_index);
void blank_mlist();
void blank_mlist();
void mlist_set_short(Short mptr, Short offset, Short val);
void mlist_set_uchar(Short mptr, Short offset, UChar val);
void mlist_set_ml(Short mptr, Boolean ml);
void mlist_mv(Short from, Short to);
// mess with inventory
void invy_set_number(inven_type *iptr, Short index, UChar n) SEC_5; // fixed
void invy_set_flags(inven_type *iptr, Short index, ULong n) SEC_5;
void invy_set_p1(inven_type *iptr, Short index, Short n) SEC_5;
void invy_set_cost(inven_type *iptr, Short index, Long cost) SEC_5;
void invy_set_hitdamac(inven_type *iptr, Short index,
			    Char tohit, Char todam, Char toac) SEC_5;
void invy_set_index(inven_type *iptr, Short index, UShort val) SEC_5;
void invy_delete(Short item_val) SEC_5;
void invy_set_name2(inven_type *iptr, Short index, UChar val) SEC_5;
void invy_insert(Short locn, inven_type *copyme) SEC_5;
void invy_swap(Short item_a, Short item_b) SEC_5;
void invy_set_ident(inven_type *iptr, Short index, UChar ident) SEC_5;
void blank_objident() SEC_5;
// mess with identification status
void object_ident_or(Short index, UChar or_me) SEC_5;
void object_ident_nand(Short index, UChar nand_me) SEC_5;
// mess with the monster memory (c_recall)
void blank_recall();
void recall_cmove_or(Short index, ULong or_me) SEC_5;
void recall_set_cmove(Short index, ULong val) SEC_5;
void recall_spells_or(Short index, ULong val) SEC_5;
void recall_set_spells(Short index, ULong val) SEC_5;
void recall_increment_kills(Short index) SEC_5;
void recall_increment_deaths(Short index) SEC_5;
void recall_cdefense_or(Short index, UShort or_me) SEC_5;
void recall_set_wake(Short index, UChar val) SEC_5;
void recall_set_ignore(Short index, UChar val) SEC_5;
void recall_increment_attacks(Short index, Short attindex) SEC_5;
// scramble/create the arrays of names of magic stuff.
void magic_init() SEC_5;
// store foo
void store_set_ctr(Short index, UChar ctr) SEC_5;
Char *get_haggle_string(Short i) SEC_4;
void write_history_block(Char *history_block) SEC_5;
Short get_savefile_version() SEC_7;
void perform_conversion(Short from_version, Boolean pyrec_found) SEC_7;

// sets.c
// these could practically be macros, or something
// all section1
Boolean is_room(UChar element) SEC_1;
Boolean is_corr(UChar element) SEC_1;
Boolean is_floor(UChar element) SEC_1;
Boolean is_corrodes(inven_type *item) SEC_1;
Boolean is_flammable(inven_type *item) SEC_1;
Boolean is_frost_destroy(inven_type *item) SEC_1;
Boolean is_acid_affect(inven_type *item) SEC_1;
Boolean is_lightning_destroy(inven_type *item) SEC_1;
Boolean is_null(inven_type *item) SEC_1;
Boolean is_acid_destroy(inven_type *item) SEC_1;
Boolean is_fire_destroy(inven_type *item) SEC_1;
Boolean is_large(treasure_type *item) SEC_1;
Boolean is_general_store(UChar element) SEC_1;
Boolean is_armory(UChar element) SEC_1;
Boolean is_weaponsmith(UChar element) SEC_1;
Boolean is_temple(UChar element) SEC_1;
Boolean is_alchemist(UChar element) SEC_1;
Boolean is_magic_shop(UChar element) SEC_1;
Boolean is_store_buy(Short storeno, UChar element) SEC_1;

// misc2.c
// all section2
void magic_treasure(Short x, Short level) SEC_6;
void set_options() SEC_2;

// misc3.c
// all section1 (so far)
void place_trap(Short y, Short x, Short subval) SEC_1;
void place_rubble(Short y, Short x) SEC_1;
void place_gold(Short y, Short x) SEC_1;
Short get_obj_num(Short level, Boolean must_be_small) SEC_1;
void place_object(Short y, Short x, Boolean must_be_small) SEC_1;
void alloc_object(UChar alloc_set, Short typ, Short num) SEC_1;
void random_object(Short y, Short x, Short num) SEC_1;
Short stat_adj(Short stat) SEC_1;
Short chr_adj() SEC_1;
Short con_adj() SEC_1;
UChar modify_stat (Short stat, Short amount) SEC_1;
void set_use_stat(Short stat) SEC_1;
Boolean inc_stat(Short stat) SEC_1;
Boolean dec_stat(Short stat) SEC_1;
Boolean res_stat (Short stat) SEC_1;
void bst_stat (Short stat, Short amount) SEC_1;
Short tohit_adj() SEC_1;
Short toac_adj() SEC_1;
Short todis_adj() SEC_1;
Short todam_adj() SEC_1;
void draw_cave() SEC_1;
void inven_destroy(Short item_val) SEC_1;
void take_one_item(inven_type *s_ptr, inven_type *i_ptr) SEC_1;
void inven_drop(Short item_val, Boolean drop_all) SEC_1;
//Short inven_damage(Boolean (*typ)(), Short perc) SEC_1;
Short inven_damage(UChar typ, Short perc) SEC_1;
Short weight_limit() SEC_1;
Boolean inven_similar(inven_type *a_ptr, inven_type *b_ptr) SEC_1;
Boolean inven_check_num (inven_type *t_ptr) SEC_1;
Boolean inven_check_weight(inven_type *i_ptr) SEC_1;
void check_strength() SEC_1;
Short inven_carry(inven_type *i_ptr) SEC_1;
Short spell_chance(Short spell) SEC_4;
     //...
void calc_spells(Short stat) SEC_4;
void gain_spells() SEC_4;
void gain_spell_helper() SEC_4;
void calc_mana(Short stat) SEC_4;
void calc_hitpoints() SEC_4;
void prt_experience() SEC_5;
     //...
void insert_lnum(Char *object_str, const Char *mtc_str,
		 Long number, Boolean signp) SEC_1;
void insert_str(Char *object_str, Char *mtc_str, Char *insert) SEC_1;
Short attack_blows(Short weight, Short *wtohit) SEC_1;
Short tot_dam(inven_type *i_ptr, Short tdam, Short monster) SEC_1;
Short critical_blow(Short weight, Short plus, Short dam,
		    Short attack_type) SEC_1;
Boolean mmove(Short dir, Short *y, Short *x) SEC_1;
Boolean player_saves() SEC_1;
Boolean find_range(Short item1, Short item2, Short *j, Short *k) SEC_1;
void teleport(Short dis) SEC_1;

// misc4.c
//void scribe_object() SEC_3;
void add_inscribe(inven_type *i_ptr, Short index, UChar type) SEC_3;
//void inscribe(Short iptr, Char *str) SEC_3;
Boolean check_view() SEC_3;


// moria1.c
void change_speed(Short num) SEC_1;
void py_bonuses(inven_type *t_ptr, Short factor) SEC_1;
void calc_bonuses() SEC_1;
Char *describe_use(Short i) SEC_1;
void takeoff(Short item_val, Short posn) SEC_1;
Boolean no_light() SEC_1;
void move_rec(Short y1, Short x1, Short y2, Short x2) SEC_1;
void light_room(Short y, Short x) SEC_1;
void lite_spot(Short y, Short x) SEC_1;
//static void sub1_move_light(Short y1, Short x1, Short y2, Short x2);
//static void sub3_move_light(Short y1, Short x1, Short y2, Short x2);
void move_light(Short y1, Short x1, Short y2, Short x2) SEC_1;
void disturb(Boolean se, Boolean li) SEC_1;
void search_on() SEC_1;
void search_off() SEC_1;
void rest() SEC_5;
void rest_off() SEC_1;
Boolean test_hit(Short bth, Short level, Short pth, Short ac,
		 Short attack_typ) SEC_1;
void take_hit(Short damage, Char *hit_from) SEC_1;

// moria2.c
void change_trap(Short y, Short x) SEC_2;
void search(Short y, Short x, Short chance) SEC_2;
void find_init(Short dir) SEC_2;
Boolean hit_dont_run(Short dir) SEC_5;
void find_run() SEC_2;
void end_find() SEC_2;
//static Boolean see_wall(Short dir, Short y, Short x) SEC_2;
//static Boolean see_nothing(Short dir, Short y, Short x) SEC_2;
void area_affect(Short dir, Short y, Short x) SEC_2;
Boolean minus_ac(ULong typ_dam) SEC_2;
void corrode_gas(Char *kb_str) SEC_2;
void poison_gas(Short dam, Char *kb_str) SEC_2;
void fire_dam(Short dam, Char *kb_str) SEC_2;
void cold_dam(Short dam, Char *kb_str) SEC_2;
void light_dam(Short dam, Char *kb_str) SEC_2;
void acid_dam(Short dam, Char *kb_str) SEC_2;

// moria3.c  (moria3a.c)  There is more yet to come.
void delete_monster(Short j) SEC_3;
Short cast_spell(Char *prompt, Short item_val, Short *sn, Short *sc) SEC_3;//xxx not implemented yet
void fix1_delete_monster(Short j) SEC_3;
void fix2_delete_monster(Short j) SEC_3;
Boolean delete_object(Short y, Short x) SEC_3;
ULong monster_death(Short y, Short x, ULong flags) SEC_3;
Short mon_take_hit(Short monptr, Short dam) SEC_3;
void py_attack(Short y, Short x) SEC_3;
void move_char(Short dir, Boolean do_pickup) SEC_3;// xxx
void chest_trap(Short y, Short x) SEC_3;
void openobject(Short dir) SEC_3; // xxx
void closeobject(Short dir) SEC_3;// xxx
Boolean twall(Short y, Short x, Short t1, Short t2) SEC_3;


// moria4.c
void tunnel(Short dir) SEC_3;
void disarm_trap(Short dir) SEC_3;
void look(Short dir) SEC_3;
void throw_object(Short item_val, Short dir) SEC_3;
void bash(Short dir) SEC_3;



// desc.c
Boolean is_a_vowel(Char ch) SEC_1;
Short object_offset(inven_type *t_ptr) SEC_1;
void known1(inven_type *i_ptr) SEC_1;
Boolean known1_p(inven_type *i_ptr) SEC_1;
void known2(inven_type *i_ptr, Short index) SEC_1;
void known2_local(inven_type *i_ptr) SEC_1;
Boolean known2_p(inven_type *i_ptr) SEC_1;
void clear_known2(inven_type *i_ptr, Short index) SEC_1;
void clear_empty(inven_type *i_ptr, Short index) SEC_1;
void store_bought(inven_type *i_ptr, Short index) SEC_1;
Boolean store_bought_p(inven_type *i_ptr) SEC_1;
void sample (inven_type *i_ptr) SEC_1;
void identify(Short *item) SEC_1;
void unmagic_name(Short iptr) SEC_1;
void objdes(Char *out_val, inven_type *i_ptr, Boolean pref) SEC_1;
void desc_charges(Short item_val) SEC_1;
void desc_remain(Short item_val) SEC_1;


// recall.c
Boolean bool_roff_recall(Short mon_num) SEC_7;
void roff_recall(Short mon_num, Char *buf) SEC_7;

// util.c
Boolean hit_button_if_usable(FormPtr frm, Word btn_index);
void update_field_scrollers(FormPtr frm, FieldPtr fld,
			    Word up_scroller, Word down_scroller);
void page_scroll_field(FormPtr frm, FieldPtr fld,
#ifndef I_AM_COLOR
                       DirectionType
#else /* I_AM_COLOR */
                       WinDirectionType
#endif /* I_AM_COLOR */
		       dir);
void LeaveForm();
Char * md_malloc(Int n);
void seconds_to_string(ULong seconds, Char *buf, Short start);// SEC_7;

// spells.c  (spells-a.c, but not missing any functions)
void monster_name(Char *m_name, monster_type *m_ptr,
		  creature_type *r_ptr) SEC_2;
void lower_monster_name(Char *m_name, monster_type *m_ptr,
			creature_type *r_ptr) SEC_2;
Boolean sleep_monsters1(Short y, Short x) SEC_2;
Boolean detect_treasure() SEC_2;
Boolean detect_object() SEC_2;
Boolean detect_trap() SEC_2;
Boolean detect_sdoor() SEC_2;
Boolean detect_invisible() SEC_2;
Boolean light_area(Short y, Short x) SEC_2;
Boolean unlight_area(Short y, Short x) SEC_2;
void map_area() SEC_2;
Short ident_spell(Short item_val) SEC_2;
Boolean aggravate_monster(Short dis_affect) SEC_2;
Boolean trap_creation() SEC_2;
Boolean door_creation() SEC_2;
Short td_destroy() SEC_2;
Boolean detect_monsters() SEC_2;
void light_line(Short dir, Short y, Short x) SEC_2;
void starlite(Short y, Short x) SEC_2;
Boolean disarm_all(Short dir, Short y, Short x) SEC_2;
//static void get_flags(Short typ, ULong *weapon_type, Short *harm_type, 
//		      Short (**destroy)() ) SEC_2;
void fire_bolt(Short typ, Short dir, Short y, Short x, Short dam,
	       Char *bolt_typ) SEC_2;
void fire_ball(Short typ, Short dir, Short y, Short x, Short dam_hp,
	       Char *descrip) SEC_2;
void breath(Short typ, Short y, Short x, Short dam_hp, Char *ddesc,
	    Short monptr) SEC_2;
Boolean recharge(Short item_val, Short num) SEC_2;
Boolean hp_monster(Short dir, Short y, Short x, Short dam) SEC_2;
Boolean drain_life(Short dir, Short y, Short x) SEC_2;
Boolean speed_monster(Short dir, Short y, Short x, Short spd) SEC_2;
Boolean confuse_monster(Short dir, Short y, Short x) SEC_2;
Boolean sleep_monster(Short dir, Short y, Short x) SEC_2;
Boolean wall_to_mud(Short dir, Short y, Short x) SEC_2;
Boolean td_destroy2(Short dir, Short y, Short x) SEC_2;
Boolean poly_monster(Short dir, Short y, Short x) SEC_2;
Boolean build_wall(Short dir, Short y, Short x) SEC_2;
Boolean clone_monster(Short dir, Short y, Short x) SEC_2;
void teleport_away(Short monptr, Short dis) SEC_2;
void teleport_to(Short ny, Short nx) SEC_2;
Boolean teleport_monster(Short dir, Short y, Short x) SEC_2;
Boolean mass_genocide() SEC_2;
Boolean genocide(Char c) SEC_2;
Boolean speed_monsters(Short spd) SEC_2;
Boolean sleep_monsters2() SEC_2;
Boolean mass_poly() SEC_2;
Boolean detect_evil() SEC_2;
Boolean hp_player(Short num) SEC_2;
Boolean cure_confusion() SEC_2;
Boolean cure_blindness() SEC_2;
void cause_blindness(Short duration) SEC_5;
Boolean cure_poison() SEC_2;
Boolean remove_fear() SEC_2;
void earthquake() SEC_2;
Boolean protect_evil() SEC_2;
void create_food() SEC_2;
Boolean dispel_creature(Short cflag, Short damage) SEC_2;
Boolean turn_undead() SEC_2;
void warding_glyph() SEC_2;
void lose_str() SEC_2;
void lose_int() SEC_2;
void lose_wis() SEC_2;
void lose_dex() SEC_2;
void lose_con() SEC_2;
void lose_chr() SEC_2;
void lose_exp(Long amount) SEC_2;
Boolean slow_poison() SEC_2;
void bless(Short amount) SEC_2;
void detect_inv2(Short amount) SEC_2;
//static void replace_spot(Short y, Short x, Short typ) SEC_2;
void destroy_area(Short y, Short x) SEC_2;
Boolean enchant(Short *plusses, Short limit) SEC_2;
Boolean remove_curse() SEC_2;
Boolean restore_level() SEC_2;

// eat.c
void eat(Short item_val) SEC_2;
Boolean get_item(Short *, Char *, Short, Short, Char *, Char *); // this is a dummy function and should be replaced by form type stuff.

// potions.c
void quaff(Short item_val) SEC_2;

// staffs.c
void use(Short item_val) SEC_3;

// wands.c
void aim(Short item_val, Short dir) SEC_3;

// magic.c
void cast() SEC_3;
void cast_helper(Short choice, Short dir_or_minus_one) SEC_3;// SEC_3; // just debugging...
// form_spell.c
Boolean confirmed_spell(Short spell) SEC_4;
Boolean has_book(Short spell) SEC_7;

// prayer.c
void pray() SEC_3;
void pray_helper(Short choice, Short dir_or_minus_one) SEC_3;

// misc5.c
void draw_directional() SEC_3;
void undraw_directional() SEC_3;
Short convert_to_dir(Short x, Short y, Short ignore_center) SEC_3;
void dispatch_directional(Short dir) SEC_3;
void get_default_username() SEC_5;
void do_feep(Long frq, UInt dur) SEC_6;

// store1.c
Long item_value(inven_type *i_ptr) SEC_4;
Long sell_price(Short snum, Long *max_sell, Long *min_sell,
		inven_type *item) SEC_4;
Boolean store_check_num(inven_type *t_ptr, Short store_num) SEC_4;
void store_carry(Short store_num, Short *ipos, inven_type *t_ptr) SEC_4;
void store_destroy(Short store_num, Short item_val, Boolean one_of) SEC_4;
void store_init() SEC_4;
void store_maint() SEC_4;
Boolean noneedtobargain(Short store_num, Long minprice) SEC_4;
void updatebargain(Short store_num, Long price, Long minprice) SEC_4;

// store2.c
void reset_haggle_foo() SEC_6;
Long haggle_get_ask() SEC_4;
Long haggle_get_incr() SEC_4;
void purchase_haggle_init() SEC_4;
Short purchase_haggle_endturn(Long new_offer) SEC_4;
void sell_haggle_init() SEC_4;
Short sell_haggle_endturn(Long new_offer) SEC_4;
Boolean store_purchase_init(Short store_num, Short item_val) SEC_4;
void store_purchase_end(Boolean in_haggle) SEC_4;
Boolean store_sell_init(Short store_num, Short item_val) SEC_4;
void store_sell_end(Boolean in_haggle) SEC_4;
Boolean enter_store(Short store_num) SEC_4;

// creature.c
void update_mon(Short monptr) SEC_4;
Boolean multiply_monster(Short y, Short x, Short cr_index, Short monptr) SEC_4;
void creatures(Short attack) SEC_5;

// save.c
Boolean recover_crash() SEC_5;
void didnt_crash() SEC_5;
void maybe_checkpoint() SEC_5;
Boolean save_char() SEC_5;
Boolean load_char() SEC_5;
void antisave_char() SEC_5;

// scrolls.c
void read_scroll(Short item_val) SEC_6;

// death.c, or form_topscores.c anyway
Long total_points() SEC_6;

// win.c
void WinDrawChars_ctr(Char *buf, Short buflen, Short x_mid, Short y) SEC_6;
void WinDrawChars_rj(Char *buf, Short buflen, Short x_right, Short y) SEC_6;


// abs seemed to be acting up but I think it was just %c problems after all.
//#define abs(a) (((a) >= 0) ? (a) : (-(a)))
// Because desc.c is cranky about %c ... this returns a char + or - for sign.
#define sgnchar(a) (((a) >= 0) ? ((Char) '+') : ((Char) '-'))

#define IsColor (found_color && my_prefs.color_on)
