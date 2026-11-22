// this file is public domain

#define NOISY 1
#undef VERYNOISY
// #define VERYNOISY NOISY

#define MYRECS 20
// these defines are from kmoria's records.h
#define REC_CAVE_CPTR   200
#define REC_CAVE_TPTR   201
#define REC_CAVE_FVAL   202
#define REC_CAVE_LIGHT  203
#define REC_TLIST       204
#define REC_MLIST       205
#define REC_INVENTORY   209
#define REC_OBJIDENT    210
#define REC_RECALL      211
#define REC_STORES      170
#define REC_STOREINV_A  171
#define REC_MAGICNAMEIDX 212
#define REC_SCROLLTITLES 213
#define REC_PHISTORY    111
#define REC_PY           50
// end of from-records.h
#define REC_STOREINV_1 (REC_STOREINV_A)
#define REC_STOREINV_2 (REC_STOREINV_A+1)
#define REC_STOREINV_3 (REC_STOREINV_A+2)
#define REC_STOREINV_4 (REC_STOREINV_A+3)
#define REC_STOREINV_5 (REC_STOREINV_A+4)
#define REC_STOREINV_6 (REC_STOREINV_A+5)
#define NOREC 0
#define O_CPTR 0 
#define O_TPTR 1 
#define O_FVAL 2 
#define O_LIGHT 3 
#define O_TLIST 4
#define O_MLIST 5
#define O_INVENTORY 6
#define O_OBJIDENT 7
#define O_RECALL 8
#define O_STORES 9
#define O_STOREINV_1 10
#define O_PY 19



typedef struct {
  long offset;
  //  unsigned char attribs; // We don't actually care about these.
  long unique_id;
} rec_entry_descr;


// got to use open lseek read write for moria2pdb frobbing
typedef int FD;
