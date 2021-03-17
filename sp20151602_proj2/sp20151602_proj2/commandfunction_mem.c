#include "20151602.h"
#include "commandfunction_mem.h"

// lmt=1048575;
// short virtual_mems[1048576];

int translate(char* str){  // translate 함수 : 넘겨받은 문자열 배열을 16진수 값으로 변환함
	int ret=0, i=strlen(str)-1, digit=1;

	for(; i>-1; i--){
		if(!isxdigit(str[i]))  // 16진수로 이뤄졌는지 확인
			return -1;
		if(str[i]>47 && str[i]<58)  // 숫자로 이뤄졌는지 확인
			ret+=digit*(str[i]-48);  // 만약 숫자로 이뤄졌다면 '0'을 뺀 값을 최종 결과에 더해줌
		else
			ret+=digit*(tolower(str[i])-87);  // 만약 알파벳으로 이뤄졌다면 ('a'-10)을 뺀 값을 최종 결과에 더해줌

		digit*=16;
	}

	return ret;
}

Token* tokenize(char* strInp, int val_cnt){  // tokenize 함수 : 넘겨받은 문자열을 띄어쓰기를 기준으로 잘라냄
	Token *ret=malloc(sizeof(Token));
	char *tmp, *str[5];
	int i=0, n;

	for(; i<val_cnt; i++){  // 전처리
		ret->token[i][0]='\0';
	}
	
	ret->valid=1;

	tmp=strtok(strInp, " ");  // command에 해당하는 부분은 불필요하므로 잘라냄 (이는 20151602.c에서 구분함)
	
	i=0;

	while((tmp=strtok(NULL, " "))!=NULL){  // command의 뒷부분을 띄어쓰기를 기준으로 하나씩 잘라냄
		if(i>=val_cnt){
			ret->valid='\0';

			break;
		}

		str[i]=(char*)malloc(sizeof(char)*(strlen(tmp)+1));

		strcpy(str[i], tmp);  // 잘라낸 정보들을 복사해 놓음

		i++;
	}

	n=i;

	ret->arg_cnt=n;  // 총 몇 개의 argument를 받았는지 저장함

	if(ret->valid=='\0')
		return ret;

	for(i=0; i<n; i++){  // 잘라낸 argument들을 Token 구조체 object에 저장함
		if(strlen(str[i])<1)
			continue;

		if(str[i][strlen(str[i])-1]==',')
			str[i][strlen(str[i])-1]='\0';

		strcpy(ret->token[i], str[i]);
	}

	return ret;
}

int dumpF(Token* token){  // dumpF 함수 : 넘겨받은 argument들을 바탕으로 원하는 정보를 출력함
	static int start_num=0;
	int end_num;
	int i=0, j;
	int args[2];

	for(; i<2; i++){  // 넘겨받은 argument의 16진수 변환
		if(token->token[i][0]!='\0')
			args[i]=translate(token->token[i]);
		else
			args[i]=-1;
	}

	if(args[0]!=-1)  // 출력할 시작지점과 끝지점을 설정
		start_num=args[0];

	end_num=args[1];

	if(end_num==-1)
		end_num=start_num+159;

	if(end_num>lmt)
		end_num=lmt;

	if(start_num>end_num || start_num<0 || start_num>lmt || end_num<0 || end_num>lmt){
		printf("\nError.\n\n");

		free(token);

		return -1;
	}

	for(i=(start_num/16)*16; i<(end_num/16)*16+16; i++){  // 출력할 구간의 정보를 출력
		if(i%16==0)
			printf("%05X ", i);

		if(i<start_num || i>end_num)
			printf("   ");
		else
			printf("%02X ", virtual_mems[i]);
		
		if(i%16==15){
			printf("; ");

			for(j=i/16*16; j<i+1; j++){
				if(j>start_num-1 && j<end_num+1 && virtual_mems[j]>31 && virtual_mems[j]<127)
					printf("%c", virtual_mems[j]);
				else printf(".");
			}

			printf("\n");
		}
	}

	start_num=end_num+1;  // 다음 시작 지점을 미리 예측

	if(start_num>lmt)
		start_num=0;

	free(token);

	return 0;
}

int resetF(){  // resetF 함수 : 모든 구간의 정보를 0으로 설정함
	int i=0;

	for(; i<lmt+1; i++)
		virtual_mems[i]=0;

	return 0;
}

int editF(Token* token){  // editF 함수 : 넘겨받은 argument들을 바탕으로 특정 위치의 정보를 변경
	int addr, value, i=0;
	int args[2];

	for(; i<2; i++){  // 넘겨받은 argument들을 16진수로 변환
		if(token->token[i][0]!='\0')
			args[i]=translate(token->token[i]);
		else
			args[i]=-1;
	}

	addr=args[0];
	value=args[1];

	if(addr==-1 || addr>lmt || addr<0 || value>255 || value<0){
		printf("\nError.\n\n");

		free(token);

		return -1;
	}

	virtual_mems[addr]=value;  // 정보를 변경

	free(token);

	return 0;
}

int fillF(Token* token){  // fillF 함수 : 넘겨받은 argument들을 바탕으로 특정 구간의 정보를 변경
	int start_addr, end_addr, value;

	start_addr=translate(token->token[0]);  // 넘겨받은 argument들을 16진수로 변환
	end_addr=translate(token->token[1]);
	value=translate(token->token[2]);

	if(start_addr>end_addr || value>255 || value<0 || start_addr<0 || start_addr>lmt || end_addr>lmt || end_addr<0){
		printf("\nError.\n\n");

		free(token);

		return -1;
	}

	for(; start_addr!=end_addr+1; start_addr++)  // 시작 지점부터 끝지점까지의 정보를 변경
		virtual_mems[start_addr]=value;

	free(token);

	return 0;
}
