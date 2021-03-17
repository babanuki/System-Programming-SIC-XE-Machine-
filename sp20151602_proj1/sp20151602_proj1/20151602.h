#include<stdio.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<ctype.h>

typedef enum commandlist{
	help, quit, history, dir, dump, edit, fill, reset, opcode, opcodelist
}commandlist;

typedef struct Node{
	void* data;
	struct Node* next;
//	struct Node* prev;             <- 보류
}Node;

typedef struct historyNode{
	char str[256];
}historyNode;

typedef struct Hash{
	char opCode[4];
	char opInst[256];
	struct Hash* next;
	enum{mod1, mod2, mod34} mod;
}Hash;

typedef struct Token{
	char token[5][256];
	char valid;
	int arg_cnt;
}Token;

typedef Node* LL;
