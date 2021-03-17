#include "20151602.h"
#include "command.h"

cmd cmd_map[17]={  // 각 command를 구분하는 데에 쓰이는 문자열과 구분자
	{"help", "h", help},
	{"quit", "q", quit},
	{"history", "hi", history},
	{"dir", "d", dir},
	{"dump", "du", dump},
	{"edit", "e", edit},
	{"fill", "f", fill},
	{"reset", "reset", reset},
	{"opcode", "opcode", opcode},
	{"opcodelist", "opcodelist", opcodelist},
	{"assemble", "assemble", assemble},
	{"type", "type", type},
	{"symbol", "symbol", symbol},
	{"progaddr", "progaddr", progaddr},
	{"loader", "loader", loader},
	{"bp", "bp", bp},
	{"run", "run", run}
};

int cmdInp(char* str){  // cmdInp 함수 : command를 구분해내는 함수
	int i=0;

	if(strlen(str)==0)
		return -1;

	for(i=0; i<17; i++){
		if(!strcmp(cmd_map[i].str1, str) || !strcmp(cmd_map[i].str2, str)){
			return cmd_map[i].cmdl;
		}
	}

	return -1;
}
