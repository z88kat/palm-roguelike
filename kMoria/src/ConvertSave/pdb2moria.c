/*
 * cryptic converter, (c) 2001 Bridget Spitznagel
 * Reads in a kMoriaSaveDB.pdb type of file, and writes a moria.save type of
 * file (for umoria 5.5.2).
 * Tested on one Linux machine.  It works for me.  Your mileage may vary.
 *
 * Because this program uses a little code from the pilot-link utils,
 * it inherits the GPL - you can get a copy of the GPL from
 * http://www.gnu.org/copyleft/gpl.html
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "swap.h"
#include "header.h"
#include "rec.h"
#include "to-umoria.h"

///////////////////////////////////////////////////////////////////////

int TotalRecords;
// record entry descriptors

rec_entry_descr *reds;


int
main(int argc, char** argv)
{
  FILE *fin, *fout;
  //  DWORD numRecs;
  //  int recNum = 0;
  //  struct rec0 r0;
  //  long start_position;

  printf("%s (for kMoria savefile-version 0x01 / Umoria 5.5.2)\n", argv[0]);
  if (argc < 3) {
    fprintf(stderr,"save conversion from kMoriaSaveDB.pdb to moria.save\n");
    fprintf(stderr,"usage: %s in.pdb out.save\n",argv[0]);
    exit(2);
  }

  if ( ! SwapChoose()) {
    printf("\nfailed to select proper byte swapping algorithm");
#if UNIX
    printf("\n");
#endif
    exit(1);
  }

  fin = fopen(argv[1], "rb");
  if (fin==0) {
    printf("can't open %s for read\n", argv[1]);
    exit(2);
  }

  // we read in all the offsets and record uids.
  read_preamble(fin);

  sort_offsets(fin);

  // now, we consult umoria/source/save.c
  fout = fopen(argv[2], "wb");
  if (fout==0) {
    printf("can't open %s for write\n", argv[2]);
    exit(2);
  }
  write_version_info(fout);
  write_character(fout, fin);
  write_dungeon(fout, fin);

  fclose(fin);
  fclose(fout);
  exit(0);
}

