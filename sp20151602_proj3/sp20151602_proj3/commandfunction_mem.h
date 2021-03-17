#define lmt 1048575
short virtual_mems[1048576];

int translate(char*);
Token* tokenize(char*, int);
int dumpF(Token*);
int resetF();
int editF(Token*);
int fillF(Token*);
