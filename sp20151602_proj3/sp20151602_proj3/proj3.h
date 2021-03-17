int Paddr;
int startAddr, endAddr;

void printSection0(LL);
void printSection1(controlSection*);
void printSection2(LL);
void progaddrF(Token*);
int loaderF(Token*);
int proj3_pass1(FILE**, int);
int proj3_pass2(FILE**, int);
int findControlSection(char*);
int findExternalSymbol(char*);
