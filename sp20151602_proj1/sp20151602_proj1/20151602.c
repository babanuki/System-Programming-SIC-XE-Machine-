#include "20151602.h"
#include "commandfunction_mem.h"
#include "command.h"
#include "commandfunction_basic.h"
#include "hashtable.h"
#include "linkedlist.h"

int main(){  // main 함수 : 기본 작동 담당 함수
	char strInp[256], tmpInp[256];
	char commandInp[256];
	int i=0, cmdTmp, chk;
	Token* token;

	CreateTable();	

	for(;;){
		chk=0;

		printf("sicsim> ");

		fgets(strInp, sizeof(strInp), stdin);
		
		strInp[strlen(strInp)-1]='\0';

		strcpy(tmpInp, strInp);

		for(i=0; strInp[i]>96 && strInp[i]<123; i++)  // command를 잘라냄
			commandInp[i]=strInp[i];

		commandInp[i]='\0';

		cmdTmp=cmdInp(commandInp);  // 해당 command가 유효한 command인지 확인함

		if(cmdTmp!=-1){  // 유효한 command라면, 이후에 따라오는 argument를 잘라내고 이를 보관함
			token=tokenize(strInp, 5);

		}
		else{
			printf("\nERROR\n\n");

			continue;
		}

		switch(cmdTmp){  // 입력된 command의 유효한 argument의 개수를 확인함
			case -1:
				chk=2;
				break;
			case 0:
			case 1:
			case 2:
			case 3:
			case 7:
			case 9:
				if(token->arg_cnt!=0)
					chk=1;
				break;
			case 4:
				if(token->arg_cnt>2)
					chk=1;
				break;
			case 5:
				if(token->arg_cnt!=2)
					chk=1;
				break;
			case 6:
				if(token->arg_cnt!=3)
					chk=1;
				break;
			case 8:
				if(token->arg_cnt!=1)
					chk=1;
				break;
		}

		if(chk==2){  // command가 잘못되었을 경우, 경고 문구를 출력함
			printf("\nError: 잘못된 명령어입니다.\n\n");

			free(token);

			continue;
		}
		else if(chk==1){  // 유효한 argument 개수에 부합하지 않을 경우, 경고 문구를 출력함
			printf("\nError: 필요한 인자의 갯수가 잘못되었습니다.\n\n");

			free(token);

			continue;
		}

		switch(cmdTmp){  // 입력받은 command별로 각자의 기능을 수행함
						 // 이 때, 함수 작동의 이상 유무의 확인이 필요한 경우, 결과를 넘겨받음
						 // 만약 작동에 문제가 있을 경우, 이는 history에 추가하지 않아야 하기 때문
			case -1:
				break;
			case 0:
				helpF();
				break;
			case 1:
				quitF();
				return 0;
			case 2:
				ADDHIST(tmpInp);
				historyF();
				break;
			case 3:
				dirF();
				break;
			case 4:
				chk=dumpF(token);
				break;
			case 5:
				chk=editF(token);
				break;
			case 6:
			    chk=fillF(token);
				break;
			case 7:
				resetF();
				break;
			case 8:
				chk=opcodeF(token);
				break;
			case 9:
				opcodelistF();
				break;
		}

		if(chk!=-1 && cmdTmp!=2)  // 만약 함수의 작동에도 이상이 없었다면, history에 추가함
			ADDHIST(tmpInp);
	}

	return 0;
}
