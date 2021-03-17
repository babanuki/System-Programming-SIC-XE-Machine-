#include "20151602.h"
#include "commandfunction_basic.h"
#include "linkedlist.h"

void helpF(){  // helpF 함수 : 각 command를 사용하는 방법을 출력함
	printf("\n\nh[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\ne[dit] address, value\nf[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\nassemble filename\ntype filename\nsymbol\n\n");

	return;
}

void quitF(){  // quitF 함수 : 시스템의 작동을 정지한다는 문구를 출력함
			   // 그리고 할당했던 리스트의 메모리를 해제함
	int i=0;

	printf("\n\nExit SIC\n\n\n");

	FREELL(&historyLL);

	for(; i<20; i++)
		FREELL(&opList[i]);

	return;
}

void dirF(){  // dirF 함수 : 해당 directory의 상태를 출력함
	DIR* dir=opendir(".");
	char* entrstr;
	char root[256]="./";
	struct stat stbuf;
	struct dirent* entr;

	if(!dir){
		printf("ERROR\n\n");

		return;
	}

	entr=readdir(dir);

	while(entr){
		root[2]='\0';

		entrstr=entr->d_name;
		stat(strcat(root, entrstr), &stbuf);

		printf("%-s", entrstr);

		if(S_ISDIR(stbuf.st_mode))
			printf("/");
		else if(stbuf.st_mode & S_IXUSR)
			printf("*");

		entr=readdir(dir);

		if(entr)
			printf("\n");
	}

	closedir(dir);

	printf("\n\n");

	return;
}

void ADDHIST(char* str){  // ADDHIST 함수 : 유효한 command line의 정보를 저장함
	void* putNode=malloc(sizeof(historyNode));

	strcpy(((historyNode*)putNode)->str, str);

	ADDLL(&historyLL, putNode);

	return;
}

void historyF(){  // historyF 함수 : 지금까지 기록된, 유효한command line의 정보를 출력함
	Node* now=historyLL;
	int i=1;

	printf("\n");

	while(now){
		printf("%3d   %s\n", i++, ((historyNode*)now->data)->str);

		now=now->next;
	}

	printf("\n");

	return;
}

int typeF(Token* token){  // typeF 함수 : 넘겨받은 문자열과 같은 이름을 가진 파일의 내용물을 출력함
	FILE* fp=fopen(token->token[0], "r");
	char c;

	if(!fp){
		printf("\nERROR : open failed\n\n");

		return -1;
	}

	while(1){
		c=fgetc(fp);

		if(c==EOF)
			break;

		printf("%c", c);
	}

	if(fclose(fp)){
		printf("\nERROR : close failed\n\n");

		return -1;
	}

	return 0;
}
