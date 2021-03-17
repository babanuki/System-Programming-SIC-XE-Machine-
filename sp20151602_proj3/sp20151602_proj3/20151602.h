#include<stdio.h>
#include<math.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<ctype.h>

typedef enum commandlist{
	help, quit, history, dir, dump, edit, fill, reset, opcode, opcodelist, assemble, type, symbol, progaddr, loader, bp, run
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
	int operand;
	struct Hash* next;
	enum{mod1, mod2, mod34} mod;
}Hash;

typedef struct Token{
	char token[5][256];
	char valid;
	int arg_cnt;
}Token;

typedef Node* LL;

typedef struct symbolInfo{
	char symbol_name[256];
	struct symbolInfo* next;
	int addr;
}symbolInfo;

typedef struct Assembly{
	struct Assembly* next;
	char origin[256];
	char label[256];
	char instruction[256];
	char operand[2][256];
	char directive[6];
	int index;
	int line;
	int locctr;
	int operand_cnt;
	int sz;
	int obj_code;
	int type;		 // 1: pseudo type
					 // 2: instruction
	int form;
	int error_type;  // 1: about symbol
					 // 2: about instruction
					 // 3: about operand
}Assembly;

typedef struct obj{
	struct obj* next;
	int loc;
	int sz;
	int code;
}obj;

typedef struct record{
	struct record* next;
	int sz;
	int loc;
}record;

typedef struct controlSection{
	char name[14];
	LL external_;
	int addr;
	int length;
}controlSection;

typedef struct externalSymbol{
	char name[14];
	int addr;
}externalSymbol;

typedef struct breakPoint{
	int addr;
}breakPoint;

typedef struct run_{
	int op;
	int format;
	int addr;
	int targetAddr;
	int immediateAddr;
	int index;
	int regi1;
	int regi2;
}run_;

int regi_value[9];
