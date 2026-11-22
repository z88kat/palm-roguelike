typedef unsigned char byte;
typedef unsigned long DWORD;
typedef unsigned short WORD;

int SwapChoose();
WORD (*SwapWord)(WORD r);
DWORD (*SwapLong)(DWORD r);

void out_word(FILE* fin, short w);
void out_long(FILE* fin, long d);
short in_word(FILE* fin);
long in_long(FILE* fin);







