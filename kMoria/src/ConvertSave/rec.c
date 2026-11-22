// this file is public domain
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defs.h"
#include "xor.h"
#include "swap.h"

extern rec_entry_descr *reds;
extern int TotalRecords;

long offsets[MYRECS];

void sort_offsets(FILE* fin)
{
  int i, j;
  for (j = 0 ; j < MYRECS ; j++)
    offsets[j] = NOREC;

  for (i = 0 ; i < TotalRecords ; i++) {
    switch(reds[i].unique_id) {
    case REC_CAVE_CPTR:    offsets[O_CPTR] = reds[i].offset; break;
    case REC_CAVE_TPTR:    offsets[O_TPTR] = reds[i].offset; break;
    case REC_CAVE_FVAL:    offsets[O_FVAL] = reds[i].offset; break;
    case REC_CAVE_LIGHT:   offsets[O_LIGHT] = reds[i].offset; break;
    case REC_TLIST:        offsets[O_TLIST] = reds[i].offset; break;
    case REC_MLIST:        offsets[O_MLIST] = reds[i].offset; break;
    case REC_INVENTORY:    offsets[O_INVENTORY] = reds[i].offset; break;
    case REC_OBJIDENT:     offsets[O_OBJIDENT] = reds[i].offset; break;
    case REC_RECALL:       offsets[O_RECALL] = reds[i].offset; break;
    case REC_STORES:       offsets[O_STORES] = reds[i].offset; break;
    case REC_STOREINV_1:   offsets[O_STOREINV_1] = reds[i].offset; break;
    case REC_STOREINV_2:   offsets[11] = reds[i].offset; break;
    case REC_STOREINV_3:   offsets[12] = reds[i].offset; break;
    case REC_STOREINV_4:   offsets[13] = reds[i].offset; break;
    case REC_STOREINV_5:   offsets[14] = reds[i].offset; break;
    case REC_STOREINV_6:   offsets[15] = reds[i].offset; break;
    case REC_MAGICNAMEIDX: offsets[16] = reds[i].offset; break;
    case REC_SCROLLTITLES: offsets[17] = reds[i].offset; break;
    case REC_PHISTORY:     offsets[18] = reds[i].offset; break;
    case REC_PY:           offsets[O_PY] = reds[i].offset; break;
    }
  }

  for (j = 0 ; j < MYRECS ; j++) {
    if (offsets[j] == NOREC) {
      fprintf(stderr, "Missing record (%d).  Aborting...\n", j);
      exit(-1);
    }
  }

  // Got this far, guess we have them all
#ifdef NOISY
  printf("all records present and accounted for\n");
#endif

}


