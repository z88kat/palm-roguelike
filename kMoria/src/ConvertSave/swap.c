// This file is GPL stuff borrowed from pilot-link -ish utilities

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "swap.h"

////////////// utilities //////////////////////////////////////

WORD SwapWord21(WORD r)
{
  return (r>>8) + (r<<8);
}
WORD SwapWord12(WORD r)
{
  return r;  
}
DWORD SwapLong4321(DWORD r)
{
  return  ((r>>24) & 0xFF) + (r<<24) + ((r>>8) & 0xFF00) + ((r<<8) & 0xFF0000);
}
DWORD SwapLong1234(DWORD r)
{
  return r;
}

WORD (*SwapWord)(WORD r) = NULL;
DWORD (*SwapLong)(DWORD r) = NULL;


// copy bytes into a word and double word and see how they fall,
// then choose the appropriate swappers to make things come out
// in the right order.
int SwapChoose()
{
  union { char b[2]; WORD w; } w;
  union { char b[4]; DWORD d; } d;

  strncpy(w.b, "\1\2", 2);
  strncpy(d.b, "\1\2\3\4", 4);

  if (w.w == 0x0201)
    SwapWord = SwapWord21;
  else if (w.w == 0x0102)
    SwapWord = SwapWord12;
  else
    return 0;

  if (d.d == 0x04030201)
    SwapLong = SwapLong4321;
  else if (d.d == 0x01020304)
    SwapLong = SwapLong1234;
  else
    return 0;
  
  return 1;
}  



void out_word(FILE* fout, short w)
{
  short m = SwapWord(w);
  fwrite(&m, 2, 1, fout);
}
void out_long(FILE* fout, long d)
{
  long d1 = SwapLong(d);
  fwrite(&d1, 4, 1, fout);
}

short in_word(FILE* fin)
{
  long m;
  fread(&m, 2, 1, fin);
  return SwapWord(m);
}
long in_long(FILE* fin)
{
  long d1;
  fread(&d1, 4, 1, fin);
  return SwapLong(d1);
}


