#include "20151602.h"
#include "commandfunction_basic.h"
#include "linkedlist.h"
#include "commandfunction_mem.h"
#include "proj3.h"

void helpF(){  // helpF 함수 : 각 command를 사용하는 방법을 출력함
	printf("\n\nh[elp]\nd[ir]\nq[uit]\nhi[story]\ndu[mp] [start, end]\ne[dit] address, value\nf[ill] start, end, value\nreset\nopcode mnemonic\nopcodelist\nassemble filename\ntype filename\nsymbol\nprogaddr address\nloader filename\nbp\nrun\n\n");

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

void printBreakPoint(LL tmp){  // printBreakPoint 함수 : 현재까지 저장된 break point를 출력합니다.
	Node* now=tmp;

	while(now){
		printf("\t%X\n", ((breakPoint*)(now->data))->addr);

		now=now->next;
	}

	return;
}

int findBreakPoint(int str){  // findBreakPoint 함수 : 넘겨받은 값에 해당하는 break point가 이미 존재하는지 확인합니다.
	Node* now=breakPointTable;

	while(now){
		if(str==((breakPoint*)(now->data))->addr)
			return 1;

		now=now->next;
	}

	return 0;
}

int bpF(Token* token){  // bpF 함수 : bp 명령어의 기능을 수행합니다.
	breakPoint* now=(breakPoint*)malloc(sizeof(breakPoint));

	if(token->arg_cnt==0){  // 만약 bp의 뒤에 따라오는 인자가 없을 경우, bp를 출력합니다.
		printf("\tbreakpoint\n");
		printf("\t----------\n");
		printBreakPoint(breakPointTable);

		free(now);

		return 0;
	}

	if(strcmp("clear", token->token[0]) && findBreakPoint(translate(token->token[0]))){  // bp의 뒤에 clear가 아닌 다른 값이 따라오면서, 해당 값이 이미 bp로 지정되어 있다면, 넘어갑니다.
		printf("\n\nNOTICE : this address is already a break point\n\n\n");

		free(now);

		return 0;
	}

	if(!strcmp("clear", token->token[0])){  // bp의 뒤에 clear가 따라오면, 지금까지 저장한 break point를 모두 없앱니다.
		FREELL(&breakPointTable);

		free(now);

		return 0;
	}

	// 위의 모든 과정을 겪고도 종료되지 않았다면, bp의 뒤에 따라온 값을 break point로 추가합니다.

	now->addr=translate(token->token[0]);

	ADDLL(&breakPointTable, (void*)now);

	return 0;
}

void printRegis(){  // printRegis 함수 : register들의 값을 출력합니다.
	printf("\tA : %06X X : %06X\n", regi_value[0], regi_value[1]);
	printf("\tL : %06X PC : %06X\n", regi_value[2], regi_value[7]);
	printf("\tB : %06X S : %06X\n", regi_value[3], regi_value[4]);

	return;
}

int calculateAddr(int addr, int h){  // calculateAddr 함수 : 주어진 주소에서부터, half-byte만큼의 공간에 있는 값을 반환합니다.
	int ret, i=0;

	if(addr>1048575)
		return 0;

	if(h%2)
		ret=virtual_mems[addr]%16;
	else
		ret=virtual_mems[addr]%256;

	for(; i<(h-1)/2; i++){
		ret*=256;
		ret+=virtual_mems[addr+i+1];
	}

	return ret;
}

int simpleAddr(int addr, int format){  // simpleAddr 함수 : simple address일 경우의 주소를 계산합니다.
	int tmp=(virtual_mems[addr+1]/16)&6;
	int flag;
	int ret;

	if(format==3){
		flag=0;
		ret=calculateAddr(addr+1, 3);
	}
	else{
		flag=1;
		ret=calculateAddr(addr+1, 5);
	}

	if(tmp==4)
		ret+=regi_value[3];
	else if(tmp==2){
		if(flag){
			if(ret & 0x80000)
				ret|=0xFFF00000;
		}
		else{
			if(ret&0x800)
				ret|=0xFFFFF000;
		}

		ret+=regi_value[7];
	}

	return ret;
}

int calculateTarget(int addr, int format){  // calculateTarget 함수 : 넘겨받은 주소의 address mode를 판별한 뒤, 이에 대응하는 방식으로 target address를 계산합니다.
	int mod=virtual_mems[addr]&3;
	int ret;

	switch(mod){
		case 0:  // SIC
			ret=calculateAddr(addr, 5)&0x7FFF;
			break;
		case 1:  // immediate
			ret=simpleAddr(addr, format);
			break;
		case 2:  // indirect
			ret=simpleAddr(addr, format);
			ret=calculateAddr(ret, 6);
			break;
		case 3:  // simple
			ret=simpleAddr(addr, format);
	}

	if(virtual_mems[addr+1]&128)
		ret+=regi_value[1];

	return ret;
}

void insert(int addr, int byte, int value){  // insert 함수 : 주어진 address에, 넘겨받은 값을 넣습니다.
	int i=addr+byte;

	if(addr>1048575)
		return;

	for(; i>addr; i--){
		virtual_mems[i-1]=0xFF&value;
		value/=256;
	}

	return;
}

int runF(){  // runF 함수 : run 명령어의 기능을 수행합니다.
	static int nowAddr=-24;
	static int nowBreakPoint=-24;
	int i, targetAddr, targetValue, equal=-1;
	run_* now=(run_*)malloc(sizeof(run_));
	
	if(nowAddr==-24)
		nowAddr=startAddr;

	regi_value[7]=nowAddr;
	
	for(; nowAddr<endAddr; ){  // nowAddr가 endAddr보다 작으면 다음의 과정을 수행합니다. (이 때, 다음 nowAddr은, PC register에 들어있는 값입니다.)
		now->op=virtual_mems[nowAddr]&0xFC;  // nowAddr에 담긴 명령어를 저장합니다.

		if(!virtual_mems[nowAddr]){  // 만약 nowAddr에 담긴 게 아무것도 없다면, PC를 1 증가시키고 continue를 합니다.
			now->format=1;

			regi_value[7]++;
			nowAddr=regi_value[7];

			continue;
		}
		else{  // 만약 그렇지 않다면, 해당 address에 담긴 값을 통해 format을 확인하고, 그에 대해 명령어의 값을 갱신합니다.
			switch(virtual_mems[nowAddr]/16){
				case 15:
				case 12:
					now->op+=virtual_mems[nowAddr]&3;
					now->format=1;
					break;
	
				case 11:
				case 10:
				case 9:
					now->op+=virtual_mems[nowAddr]&3;
					now->format=2;
					break;
	
				default:
					if(virtual_mems[nowAddr+1]&16)
						now->format=4;
					else
						now->format=3;
			}
		}

		for(i=nowAddr; i<nowAddr+now->format; i++){  // break point에 걸리는지 확인합니다.
			if(nowBreakPoint!=i && findBreakPoint(i)){
				nowBreakPoint=i;
				
				printRegis();

				return 0;
			}
		}

		regi_value[7]+=now->format;
		now->addr=3;

		switch(now->format){  // format에 대응하는 처리를 합니다.
			case 2:
				now->regi1=virtual_mems[nowAddr+1]/16;
				now->regi2=virtual_mems[nowAddr+1]%16;
				
				break;

			case 3:
			case 4:
				now->targetAddr=calculateTarget(nowAddr, now->format);
				now->addr=virtual_mems[nowAddr]&3;

				if(virtual_mems[nowAddr]&128)
					now->targetAddr+=regi_value[1];

				if(now->addr==1)
					now->immediateAddr=now->targetAddr;
		}

		targetAddr=now->targetAddr;

		if(now->targetAddr<1048576){
			if(now->addr==1)
				targetValue=now->immediateAddr;
			else
				targetValue=calculateAddr(now->targetAddr, 6);
		}

		switch(now->op){  // 명령어에 대응하는 처리를 합니다.
			case 0x18:
				regi_value[0]+=targetValue;
				break;
			case 0x58:
				regi_value[6]+=targetValue;
				break;
			case 0x90:
				regi_value[now->regi2]+=regi_value[now->regi1];
				break;
			case 0x40:
				regi_value[0]&=targetValue;
				break;
			case 0xB4:
				regi_value[now->regi1]=0;
				break;
			case 0x28:
				if(regi_value[0]<targetValue)
					equal=0;
				else if(regi_value[0]==targetValue)
					equal=1;
				else
					equal=2;
				break;
			case 0x88:
				if(regi_value[6]<targetValue)
					equal=0;
				else if(regi_value[6]==targetValue)
					equal=1;
				else
					equal=2;
				break;
			case 0xA0:
				if(regi_value[now->regi1]<regi_value[now->regi2])
					equal=0;
				else if(regi_value[now->regi1]==regi_value[now->regi2])
					equal=1;
				else
					equal=2;
				break;
			case 0x24:
				if(targetValue==0){
					printf("\n\nERROR : 0 division\n\n\n");

					nowAddr=endAddr;

					continue;
				}

				regi_value[0]/=targetValue;
				break;
			case 0x64:
				if(targetValue==0){
					printf("\n\nERROR : 0 division\n\n\n");

					nowAddr=endAddr;

					continue;
				}

				regi_value[6]/=targetValue;
				break;
			case 0x9C:
				if(regi_value[now->regi1]==0){
					printf("\n\nERROR : 0 division\n\n\n");

					nowAddr=endAddr;

					continue;
				}

				regi_value[now->regi2]/=regi_value[now->regi1];
				break;
			case 0xC4:
				regi_value[0]=regi_value[6];
				break;
			case 0xC0:
				regi_value[6]=regi_value[0];
				break;
			case 0x3C:
				regi_value[7]=targetAddr;
				break;
			case 0x30:
				if(equal==1)
					regi_value[7]=targetAddr;
				break;
			case 0x34:
				if(equal==2)
					regi_value[7]=targetAddr;
				break;
			case 0x38:
				if(equal==0)
					regi_value[7]=targetAddr;
				break;
			case 0x48:
				regi_value[2]=regi_value[7];
				regi_value[7]=targetAddr;
				break;
			case 0x00:
				regi_value[0]=targetValue;
				break;
			case 0x68:
				regi_value[3]=targetValue;
				break;
			case 0x50:
				regi_value[0]=targetValue/0x10000+(0xFFFFFF00&regi_value[0]);
				break;
			case 0x70:
				regi_value[6]=targetValue;
				break;
			case 0x08:
				regi_value[2]=targetValue;
				break;
			case 0x6C:
				regi_value[4]=targetValue;
				break;
			case 0x74:
				regi_value[5]=targetValue;
				break;
			case 0x04:
				regi_value[1]=targetValue;
				break;
			case 0x20:
				regi_value[0]*=targetValue;
				break;
			case 0x60:
				regi_value[6]*=targetValue;
				break;
			case 0x98:
				regi_value[now->regi2]*=regi_value[now->regi1];
				break;
			case 0x44:
				regi_value[0]|=targetValue;
				break;
			case 0xD8:
				equal=1;
				break;
			case 0xAC:
				regi_value[now->regi2]=regi_value[now->regi1];
				break;
			case 0x4C:
				regi_value[7]=regi_value[2];
				break;
			case 0xA4:
				regi_value[now->regi1]<<=regi_value[now->regi2];
				break;
			case 0x0C:
				insert(targetAddr, 3, regi_value[0]);
				break;
			case 0x78:
				insert(targetAddr, 3, regi_value[3]);
				break;
			case 0x54:
				virtual_mems[targetAddr]=0xFF&regi_value[0];
				break;
			case 0x80:
				insert(targetAddr, 6, regi_value[6]);
				break;
			case 0x14:
				insert(targetAddr, 3, regi_value[2]);
				break;
			case 0x7C:
				insert(targetAddr, 3, regi_value[4]);
				break;
			case 0xE8:
				insert(targetAddr, 3, regi_value[8]);
				break;
			case 0x84:
				insert(targetAddr, 3, regi_value[5]);
				break;
			case 0x10:
				insert(targetAddr, 3, regi_value[1]);
				break;
			case 0x1C:
				regi_value[0]-=targetValue;
				break;
			case 0x5C:
				regi_value[6]-=targetValue;
				break;
			case 0x94:
				regi_value[now->regi2]-=regi_value[now->regi1];
				break;
			case 0xE0:
			case 0xF8:
				equal=0;
				break;
			case 0x2C:
				regi_value[1]++;

				if(regi_value[1]<targetValue)
					equal=0;
				else if(regi_value[1]==targetValue)
					equal=1;
				else
					equal=2;
				
				break;
			case 0xB8:
				regi_value[1]++;

				if(regi_value[1]<regi_value[now->regi1])
					equal=0;
				else if(regi_value[1]==regi_value[now->regi1])
					equal=1;
				else
					equal=2;

				break;
			case 0xDC:
				break;
			case 0xF4:
			case 0xD0:
			case 0xC8:
			case 0xF0:
			case 0xEC:
			case 0xD4:
				break;
			default:
				regi_value[7]-=now->format-1;
				nowAddr=regi_value[7];

				continue;
		}
		
		nowAddr=regi_value[7];  // nowAddr를 PC register의 값으로 갱신합니다.
	}

	regi_value[7]=endAddr;  // PC register에 종료 주소를 넣습니다.

	printRegis();  // 정해진 register의 값들을 출력합니다.

	regi_value[2]=endAddr-startAddr;  // L register의 값을 갱신합니다.

	printf("the end\n");

	nowAddr=nowBreakPoint=-24;
	equal=-1;

	return 0;
}
