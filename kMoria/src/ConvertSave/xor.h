// this file is public domain

void set_xor_byte(unsigned char uc);
void xwrite_by(FILE *fout, unsigned char *uc, int n);
void xcopy_by(FILE *fout, FILE *fin, int n);
void xwrite_str(FILE *fout, char *str, int n);
void xwrite_sh(FILE *fout, unsigned short *us, int n);
void xcopy_sh(FILE *fout, FILE *fin, int n);
void xwrite_lo(FILE *fout, unsigned long *ul, int n);
void xcopy_lo(FILE *fout, FILE *fin, int n);

void xread_by(FILE *fin, unsigned char *uc, int n);
void xread_str(FILE *fin, char *buf, int n);
void xuncopy_str(FILE *fout, FILE *fin, int n);
void xuncopy_by(FILE *fout, FILE *fin, int n);
void xuncopy_sh(FILE *fout, FILE *fin, int n);
void xread_sh(FILE *fin, unsigned short *us, int n);
void xuncopy_lo(FILE *fout, FILE *fin, int n);
void xread_lo(FILE *fin, unsigned long *ul, int n);
