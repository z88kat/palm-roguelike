/*
 *  Ok.  This is a utility to get your character back FROM umoria 5.5.2.
 *  You MUST have generated this character in kMoria.
 *  You MUST have the kMoriaDB.pdb that you used to get this character out.
 *  This file will be clobbered, so better make a copy of it.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "xor.h"
#include "swap.h"
#include "rec.h"
extern int Noisy; // make xor.c be noisy


// Read version info
void read_version_info(FILE *fin)
{
  // We are writing for (major.minor.patchlevel) 5.5.2.
  unsigned char maj, min, pch, xorb;
  fread(&maj, 1, 1, fin);
  fread(&min, 1, 1, fin);
  fread(&pch, 1, 1, fin);
  if (maj != 5 || min != 5 || pch != 2) {
    fprintf(stderr, "Hey, this save file did not come from umoria 5.5.2!\n");
    fprintf(stderr, "It came from %d %d %d\n", maj, min, pch);
    exit(-1);
  }
  // now we read a byte with which umoria has xor'ed everything else..
  fread(&xorb, 1, 1, fin);
  set_xor_byte(xorb);
  // Ok done with the version header!
#ifdef NOISY
  printf("xor byte is %d\n", xorb);
#endif
}

#define MAX_CREATURES 279
#define RECALL_TYPE__SIZE 20
void read_recall(FILE *fout, FILE *fin)
{
  //  int j;
  unsigned short i, i_in, stop_here = 0xffff;
  // for each creature in this section of the file,
  // keep track of what creature we are on.
  // read in an index.  if stop_here then terminate.
  // if our counter is not up to that index yet, write 0s til we get there.
  // then read in the stuff and copy it over.
  fseek(fout, offsets[O_RECALL], SEEK_SET);
/*  #ifdef NOISY */
/*    printf("offset %ld\n", offsets[O_RECALL]); */
/*  #endif */
  for (i = 0 ; i < MAX_CREATURES ; i++) {
    xread_sh(fin, &i_in, 1);
    if (i_in == stop_here) return;
#ifdef VERYNOISY
    printf("reading monster %d", i_in);
#endif
    if (i < i_in) {
      // skip ahead by (i_in - i) * size of recall struct
      fseek(fout, ((i_in - i) * RECALL_TYPE__SIZE), SEEK_CUR);
      i = i_in;
    }
    // ok we are up to this index.  de-xorify it, and swap-copy it over.
    xuncopy_lo(fout, fin, 2);
    xuncopy_sh(fout, fin, 3);
    xuncopy_by(fout, fin, 6);
#ifdef VERYNOISY
    printf(" - done with %d\n", i);
#endif
  }
}

long flags_1;
void read_flags_1(FILE *fin)
{
  // We gotta read this long, but we don't gotta do anything with it yet.
  xread_lo(fin, &flags_1, 1);
  // sign bit is death, needs to go into my_save.uc[1]
  // next-to-sign bit is total_winner, go into my_save.uc[0]
  // this will be done just before we write my_save.
}

#define PLAYER_NAME_SIZE 27
// Called by read_py_etc:
void read_player_misc(FILE *fout, FILE *fin)
{
  //  unsigned char name[PLAYER_NAME_SIZE];
  // we are seeked within fout, thanks to caller.  at pymisc.
  // need to write: everything, in the same order as our inputfile.
  // except without the xors, and maybe with byte swapping.
  long position = ftell(fout);
  xuncopy_str(fout, fin, PLAYER_NAME_SIZE); // reading in variable-length str..
  fseek(fout, position+PLAYER_NAME_SIZE, SEEK_SET); //.. to fixed length string
  // write one boolean (unsigned char)
/*    Noisy = 1; */
  xuncopy_by(fout, fin, 1);
/*    Noisy = 0; */
  // write 3 (signed) longs
  xuncopy_lo(fout, fin, 3);
  // write 24 shorts (first 6 are unsigned)
  xuncopy_sh(fout, fin, 24);
  // write 4 unsigned chars, and then 4 shorts.
  xuncopy_by(fout, fin, 4);
  xuncopy_sh(fout, fin, 4);
  // whew, we are done.
}

// Called by read_py_etc:
void read_player_hist(FILE *fin)
{
  // we're not going to touch the history in fout, because
  // (a) the person running this program is supposed to assure us that
  // it's really the same anyway, and (b) if it's not the same,
  // the new history could be a different size and screw up EVERYTHING.
  char buf[60];
  xread_str(fin, buf, 60);
  xread_str(fin, buf, 60);
  xread_str(fin, buf, 60);
  xread_str(fin, buf, 60);
}
// Called by read_py_etc:
void read_player_stats(FILE *fout, FILE *fin)
{
  // we are seeked within a record in fout, thanks to caller.  at pystats.
  // need to write: everything, in the same order as our inputfile.
  xuncopy_by(fout, fin, 12);
  xuncopy_sh(fout, fin, 6);
  xuncopy_by(fout, fin, 6);
}
// Called by read_py_etc:
void read_player_flags(FILE *fout, FILE *fin)
{
  xuncopy_lo(fout, fin, 1);
  xuncopy_sh(fout, fin, 24);
  xuncopy_by(fout, fin, 19);
}
typedef struct {
  unsigned long ul[8];
  unsigned short us[6];
  unsigned char uc[2];
  char died_from[25];
} m_save;
typedef struct {
  short us[8];
} f_save;
m_save my_save;
f_save floor_save;
unsigned short inven_ctr;
void read_after_player_flags(FILE *fin)
{
  // missile_ctr, turn, inven_ctr
  xread_sh(fin, &my_save.us[0], 1);
  xread_lo(fin, &my_save.ul[6], 1);
  xread_sh(fin, &inven_ctr, 1);
  my_save.us[1] = inven_ctr;
}

static long player_hp_position; // we'll return to this point in REC_PY later!

void read_py_etc(FILE *fout, FILE *fin)
{
  //  long position;
  char trash = 0;
  fseek(fout, offsets[O_PY], SEEK_SET);
  read_player_misc(fout, fin);
  read_player_hist(fin);
  read_player_stats(fout, fin);
  read_player_flags(fout, fin);
  /* // now we must make up for its 'odd' size:
     write(fout, &trash, 1); // fwrite(&trash, 1, 1, fout); */
  //
  // ok... now we are at player_hp (in fout == kMoriaSaveDB).
  // but what we need to READ (fin == moria.save) is different random cruft.
  //
  // Let's save our output file position so we can jump back later!
  /* player_hp_position = ftell(fout); */
  //player_hp_position = lseek(fout, 1, SEEK_CUR); // THIS makes up for odd sz
  //  fseek(fout, 1, SEEK_CUR); // THIS makes up for odd size..
  fwrite(&trash, 1, 1, fout);
  player_hp_position = ftell(fout); // ..there.
  // think about what to do there.
}

#define INVEN_ARRAY_SIZE 34
#define INVEN_WIELD 22
#define INVEN_TYPE__SIZE 40
// used by read_inventory and read_store:
void read_invy_item(FILE *fout, FILE *fin)
{
  int j;
  unsigned char tmp;
  unsigned short tofoo[4];
  long position;
  // write out the inventory item
  xuncopy_sh(fout, fin, 1); // index
  xuncopy_by(fout, fin, 1); // name2
  position = ftell(fout);
  xuncopy_str(fout, fin, 13); // inscrip
  fseek(fout, position+13, SEEK_SET); //.. to fixed length string
  xuncopy_lo(fout, fin, 1); // flags
  xuncopy_by(fout, fin, 2); // t..
  xuncopy_sh(fout, fin, 1);
  xuncopy_lo(fout, fin, 1); // cost
  xuncopy_by(fout, fin, 2); // sv,#
  xuncopy_sh(fout, fin, 1); // w
  // note: tohit, todam, ac, toac must be squished back into uchars.
  xread_sh(fin, tofoo, 4);
  for (j=0 ; j<4 ; j++) {
    tmp = tofoo[j];
    /*write(fout, &tmp, 1);*/    fwrite(&tmp, 1, 1, fout);
  }
  xuncopy_by(fout, fin, 4); // dmg(2),lv,id
}
void read_inventory(FILE *fout, FILE *fin, int inven_ctr)
{
  int i;
  long position;
  position = fseek(fout, offsets[O_INVENTORY], SEEK_SET);
/*  #ifdef NOISY */
/*    printf("inv: inven_ctr is %d\n", inven_ctr); */
/*  #endif */
  for (i = 0 ; i < INVEN_ARRAY_SIZE ; i++) {
    if ((i >= inven_ctr) && (i < INVEN_WIELD)) {
      i = INVEN_WIELD; // skip blanks in the middle..
      // hm, maybe I should be zeroing them instead.  shrug.
      fseek(fout, position + INVEN_WIELD * INVEN_TYPE__SIZE, SEEK_SET);
    }
    read_invy_item(fout, fin);
  }
}

#define MAX_PLAYER_LEVEL 40
#define MAX_SPELLS 31
void read_spell_order(FILE *fout, FILE *fin)
{
  // We saved this position in REC_PY while we were in read_py_etc()
  long position = player_hp_position;
  position += sizeof(short) * MAX_PLAYER_LEVEL; // skip over player_hp.
  fseek(fout, position, SEEK_SET);
  // write MAX_SPELLS+1 uchars.
/*    Noisy = 1; */
  xuncopy_by(fout, fin, MAX_SPELLS+1);
/*    Noisy = 0; */
}

#define OBJECT_IDENT_SIZE 448
void read_objident(FILE *fout, FILE *fin)
{
  fseek(fout, offsets[O_OBJIDENT], SEEK_SET);
  xuncopy_by(fout, fin, OBJECT_IDENT_SIZE);
}
#define MAX_SAVE_MSG 22
void read_old_msgs(FILE *fin)
{
  char buf[80];
  int i;
  short lastm;
  xread_sh(fin, &lastm, 1);
  for (i=0 ; i<MAX_SAVE_MSG ; i++)
    xread_str(fin, buf, 80);
}

void read_player_hp(FILE *fout, FILE *fin)
{
  // We saved this position in REC_PY while we were in read_py_etc()
  fseek(fout, player_hp_position, SEEK_SET);
  // write MAX_PL_LEV shorts.
  xuncopy_sh(fout, fin, MAX_PLAYER_LEVEL);
}
void read_inventory_thru_php(FILE *fout, FILE *fin)
{
  unsigned short us = 0;
  long trash;
  read_after_player_flags(fin);
/*  #ifdef NOISY */
/*    printf("inv: .save position is %ld\n", ftell(fin)); */
/*  #endif */
  read_inventory(fout, fin, inven_ctr);
/*  #ifdef NOISY */
/*    printf("post inv: .save position is %ld\n", ftell(fin)); */
/*  #endif */
  xread_sh(fin, &my_save.us[2], 2); // inven_weight, equip_ctr
  xread_lo(fin, &my_save.ul[0], 3); // spell{_learned, _worked, _forgotten}
  // The rec_py is ok up to here.
  read_spell_order(fout, fin);
  read_objident(fout, fin);
  // kmoria does not use randes_seed, throw it away.
  xread_lo(fin, &trash, 1);
  xread_lo(fin, &my_save.ul[3], 1); // copy town_seed into town_seed
  read_old_msgs(fin); // and throw them away
  // panic save, total winner, noscore...
  xread_sh(fin, &us, 1); // throw away panic save
  xread_sh(fin, &us, 1); // total winner
  my_save.uc[0] = ((us != 0) ? 1 : 0);
  xread_sh(fin, &us, 1); // throw away noscore
  read_player_hp(fout, fin);
}


#define MAX_STORES 6
#define STORE_TYPE__SIZE 12
void read_stores(FILE *fout, FILE *fin)
{
  // For each store,
  //   print the store struct,
  //   and then print its items
  int i, j;
  unsigned char inv_ctr;
  for (i=0 ; i<MAX_STORES ; i++) {
    // goto store
    fseek(fout, offsets[O_STORES] + i * STORE_TYPE__SIZE, SEEK_SET);
    xuncopy_lo(fout, fin, 1);
    xuncopy_sh(fout, fin, 1);
    xuncopy_by(fout, fin, 1);
    xread_by(fin, &inv_ctr, 1); // needed, j for loop
    /*write(fout, &inv_ctr, 1);*/    fwrite(&inv_ctr, 1, 1, fout);
/*  #ifdef NOISY */
/*      printf("store %d at %ld has %d (%x) inv\n",  */
/*  	   i, offsets[O_STORES] + i * STORE_TYPE__SIZE, */
/*  	   inv_ctr, inv_ctr); */
/*  #endif */
    xuncopy_sh(fout, fin, 2);
    // goto store inventory
    fseek(fout, offsets[O_STOREINV_1 + i], SEEK_SET);
    for (j=0 ; j<inv_ctr ; j++) {
      xuncopy_lo(fout, fin, 1); // scost
      read_invy_item(fout, fin);
    }
  }
}

void read_character_tail(FILE *fin)
{
  xread_lo(fin, &my_save.ul[4], 1); // save timestamp
  xread_str(fin, my_save.died_from, 25);
  xread_lo(fin, &my_save.ul[7], 1); // max points
  xread_lo(fin, &my_save.ul[5], 1); // birth timestamp
  // Then we are done with CHARACTER but still need DUNGEON
}

void read_character(FILE *fout, FILE *fin)
{
  // these are tentative divisions
  read_recall(fout, fin);
  read_flags_1(fin);
  read_py_etc(fout, fin);
  // ahhh someone is coming back later and stomping all over REC_PY
  read_inventory_thru_php(fout, fin);
  // size-wise, we are ok up to here - but read_stores is not ok.
  read_stores(fout, fin);
  read_character_tail(fin);
  // can we write my_save stuff now?  have we read enough to do that?
  // no some of it is not til tlist and mlist.
}

void read_floor(FILE *fin)
{
  xread_sh(fin, floor_save.us, 8); // the whole thing
}
#define MAX_HEIGHT 66
#define MAX_WIDTH 198
// potential for problems here if these are not #defined
// the same in umoria+kmoria.  fortunately they are.
// Actually, perhaps max_panel_rows and max_panel_cols will be unhappy?
void read_packed_cptr_tptr(FILE *fout, FILE *fin)
{
  unsigned char i, j, xptr;
  int k, k_in, o_index;
  for (o_index = O_CPTR ; o_index <= O_TPTR ; o_index++) {
    fseek(fout, offsets[o_index], SEEK_SET);
    // ahh crud have to undo the encoding
    for (k = 0 ; k < MAX_HEIGHT*MAX_WIDTH ; k++) {
      xptr = 0;
      xread_by(fin, &i, 1);
      if (i == 0xff) {
	// must finish the array
	for ( ; k < MAX_HEIGHT*MAX_WIDTH ; k++)
	  /*write(fout, &xptr, 1);*/  fwrite(&xptr, 1, 1, fout);
	break;
      }
      xread_by(fin, &j, 1);
      k_in = i*MAX_WIDTH+j;
      // must catch up on any empty space
      for ( ; k < k_in ; k++)
	/*write(fout, &xptr, 1);*/  fwrite(&xptr, 1, 1, fout);
      xread_by(fin, &xptr, 1);
      /*write(fout, &xptr, 1);*/  fwrite(&xptr, 1, 1, fout);
    }
  }
}

void read_packed_flval(FILE *fout, FILE *fin)
{
  // This one is run-length encoded.  grumble
  unsigned char lightfval, fval, light, run=0;
  long position_f, position_l;
  int k, m;
  position_f = offsets[O_FVAL];
  position_l = offsets[O_LIGHT];
  for (k = 0 ; k < MAX_HEIGHT*MAX_WIDTH ; k += run) {
    do {
      xread_by(fin, &run, 1);
      xread_by(fin, &lightfval, 1);
    } while (k==0 && run==0);  // i.e. it might start with a couple 0's.
    // write 'run' bytes of lightfval (split into two parts)
    fval = lightfval & 0x0f;
    light = (lightfval >> 4) & 0x0f;
    // put a bunch of fval
    fseek(fout, position_f, SEEK_SET);
    for (m=0 ; m<run ; m++)
      /*write(fout, &fval, 1);*/  fwrite(&fval, 1, 1, fout);
    /*position_f += run;*/    position_f = ftell(fout);
    // put a bunch of light
    fseek(fout, position_l, SEEK_SET);
    for (m=0 ; m<run ; m++)
      /*write(fout, &light, 1);*/  fwrite(&light, 1, 1, fout);
    /*position_l += run;*/    position_l = ftell(fout);
  }
  // there shouldn't be any unfinished business at the end.
}
void read_tlist(FILE *fout, FILE *fin)
{
  int i;
  short tcptr;
  xread_sh(fin, &tcptr, 1);
#ifdef NOISY
  printf("tcptr = %d\n", tcptr);
#endif
  fseek(fout, offsets[O_TLIST] + INVEN_TYPE__SIZE, SEEK_SET);
  // write each item in t_list ... this is yet-another inven type.
  // SKIP the first one, though: it's just 0
  for (i=1 ; i<tcptr ; i++) {
    read_invy_item(fout, fin);
  }
  my_save.us[4] = tcptr;
}
void read_mlist(FILE *fout, FILE *fin)
{
  int i;
  short mfptr;
  long monsize = sizeof(short)*4 + sizeof(unsigned char)*6;
  xread_sh(fin, &mfptr, 1);
#ifdef NOISY
  printf("mfptr = %d\n", mfptr);
#endif
  fseek(fout, offsets[O_MLIST] + 2*monsize, SEEK_SET);
  // write each monster in m_list  (skipping the first two!!)
  for (i=2 ; i<mfptr ; i++) {
    xuncopy_sh(fout, fin, 4);
    xuncopy_by(fout, fin, 6);
  }
  my_save.us[5] = mfptr;
}

// things we read in along the way that get put in a different spot.
void do_leftovers(FILE *fout)
{
  long position = player_hp_position;
  int i;
  // get total_winner and death into my_save... they were read in, long ago.
  if (flags_1 & 0x40000000L)
    my_save.uc[0] = 1;
  if (flags_1 & 0x80000000L)
    my_save.uc[1] = 1;
  position += sizeof(short) * MAX_PLAYER_LEVEL; // skip over player_hp.
  position += sizeof(char) * (MAX_SPELLS+1); // skip over spell_order
  fseek(fout, position, SEEK_SET);
  // we are at my_save -- yeah write it all out
  for (i=0 ; i<8 ; i++)
    out_long(fout, my_save.ul[i]);
  for (i=0 ; i<6 ; i++)
    out_word(fout, my_save.us[i]);
  /*write(fout, my_save.uc, 2);*/  fwrite(my_save.uc, 1, 2, fout);
  /*write(fout, my_save.died_from, 25);*/fwrite(my_save.died_from,1,25, fout);
  //
  position = player_hp_position;
  position += sizeof(short) * MAX_PLAYER_LEVEL; // skip over player_hp.
  position += sizeof(char) * (MAX_SPELLS+1); // skip over spell_order
  position += sizeof(my_save);
  fseek(fout, position, SEEK_SET);
  // we are at floor_save - write it all out
  for (i=0 ; i<8 ; i++)
    out_word(fout, floor_save.us[i]);
}

void read_dungeon(FILE *fout, FILE *fin)
{
  read_floor(fin);
  // ok, the only problem child left is you, cptr tptr FREAK.
  read_packed_cptr_tptr(fout, fin);
  read_packed_flval(fout, fin);
  read_tlist(fout, fin);
  read_mlist(fout, fin);
  do_leftovers(fout);
  // and then we are DONE!  I think --- I may have missed something.
}



// this file should contain no:
// xwrite_*   xcopy_*   in_*    fseek(..)   ftell(..)   write_
// xread_??(fout..)
