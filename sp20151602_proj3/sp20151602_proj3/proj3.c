#include "20151602.h"
#include "linkedlist.h"
#include "commandfunction_mem.h"
#include "proj3.h"

int findControlSection(char* str){  // findControlSection 함수 : 넘겨받은 문자열과 같은 이름의 control section이 존재하는지 확인하는 함수
	Node* now=externalSymbolTable;
	controlSection* tmp;

	while(now){
		tmp=(controlSection*)(now->data);
		
		if(!strcmp(str, tmp->name))
			return 0;

		now=now->next;
	}

	return -1;
}

int findExternalSymbol(char* str){  // findExternalSymbol 함수 : 넘겨받은 문자열과 같은 이름의 external symbol이 존재하는지 확인하는 함수
	Node* now=externalSymbolTable;
	Node* tmp;

	while(now){
		tmp=((controlSection*)(now->data))->external_;
	
		while(tmp){
			if(!strcmp(str, ((externalSymbol*)(tmp->data))->name))
				return ((externalSymbol*)(tmp->data))->addr;

			tmp=tmp->next;
		}

		now=now->next;
	}

	return -1;
}

void freeExternalSymbolTable(){  // freeExternalSymbolTable 함수 : 저장했던 external symbol table을 지우는 함수
	Node* now=externalSymbolTable;

	while(now){
		FREELL(&(((controlSection*)(now->data))->external_));
		
		now=now->next;
	}

	FREELL(&externalSymbolTable);

	return;
}

void printSection0(LL section){  // printSection0 함수 : 저장했던 control section의 출력을 총담당하는 함수
	Node* now=section;

	while(now){
		printSection1(now->data);

		printSection2((((controlSection*)(now->data))->external_));

		now=now->next;
	}
}

void printSection1(controlSection* section){  // printSection1 함수 : control section의 header를 출력하는 함수
	if(section==externalSymbolTable->data){
		printf("control\tsymbol\taddress\tlength\n");
		printf("section\tname\n");
		printf("-------------------------------------------------------------------\n");
	}

	printf("%s\t\t%04X\t%04X\n", section->name, section->addr, section->length);

	return;
}

void printSection2(LL section){  // printSection2 함수 : 해당 control section에 저장된 external symbol을 출력하는 함수
	Node* now=section;

	while(now){
		printf("\t%s\t%04X\n", ((externalSymbol*)(now->data))->name, ((externalSymbol*)(now->data))->addr);

		now=now->next;
	}

	return;
}

void progaddrF(Token* token){  // progaddrF 함수 : program address를 넘겨받은 address로 갱신하는 함수
	int tmp=translate(token->token[0]);

	if(tmp<0 || tmp>1048575){
		printf("\n\nError : out of range\n\n\n");

		return;
	}

	Paddr=startAddr=tmp;	

	return;
}

int loaderF(Token* token){  // loaderF 함수 : 넘겨받은 obj 파일들에 대하여 loader 기능을 수행토록 하는 함수
	FILE* fp[token->arg_cnt];
	int i=0;
	int length;

	freeExternalSymbolTable();  // loader 기능을 수행하기에 앞서, 이전 loader로 생성했던 external symbol table (control section)을 초기화합니다.

	for(; i<token->arg_cnt; i++){  // 넘겨받은 파일들이 obj 파일이 맞는지, open이 되는지 확인합니다.
		if(strcmp(token->token[i]+strlen(token->token[i])-4, ".obj")){
			printf("\n\nError : not '.obj' file\n\n\n");

			return -1;
		}

		fp[i]=fopen(token->token[i], "r");

		if(!fp[i]){
			printf("\n\nError : cannot open the file\n\n\n");

			return -1;
		}
	}

	length=proj3_pass1(fp, token->arg_cnt);  // pass1을 진행합니다.
	
	if(length==-1)
		return -1;

	for(i=0; i<token->arg_cnt; i++){  // pass1을 진행하면서 각 파일을 끝까지 읽었으니, 한 번 close 한 뒤에 다시 open합니다.
		fclose(fp[i]);
		fp[i]=fopen(token->token[i], "r");
	}

	startAddr=proj3_pass2(fp, token->arg_cnt);  // pass2를 진행합니다.
	
	if(startAddr==-1)
		return -1;

	printSection0(externalSymbolTable);  // control section을 출력합니다.
	
	printf("-------------------------------------------------------------------\n");
	printf("\ttotal length %X\n", length);

	for(i=0; i<token->arg_cnt; i++)  // 열었던 파일들을 모두 닫아줍니다.
		fclose(fp[i]);

	for(i=0; i<9; i++)  // 모든 레지스터의 값을 0으로 초기화합니다.
		regi_value[i]=0;

	regi_value[2]=length;  // L 레지스터의 값을 갱신합니다.

	return 0;
}

int proj3_pass1(FILE** fp, int arg_cnt){  // proj3_pass1 함수 : linking loader의 pass1을 실행함
	int i=0, j, sectionAddr=0;
	char str[128];
	char cs[14], es[14], addr[14];
	
	controlSection* tmpSection;
	externalSymbol* tmpSymbol;

	while(1){
		if(i==arg_cnt)  // 넘겨받은 파일을 모두 읽으면 종료합니다.
			break;

		tmpSection=(controlSection*)malloc(sizeof(controlSection));  // 현재의 section을 할당합니다.
		tmpSection->external_=NULL;  // 현재 해당 section에 아무 symbol도 없으니, NULL을 표시합니다.

		if(i)
			tmpSection->addr=sectionAddr;
		else
			tmpSection->addr=sectionAddr=Paddr;  // 만약 이게 첫 section이라면, section address를 program address로 바꿔줍니다.

		fgets(str, 128, fp[i]);

		if(str[strlen(str)-1]=='\n')
			str[strlen(str)-1]='\0';

		tmpSection->length=translate(str+strlen(str)-6);
		strncpy(cs, str+1, 6);
		
		if(str[0]!='H'){  // header가 없으면 종료합니다.
			printf("\n\nError : header is missing\n\n\n");

			return -1;
		}

		if(!findControlSection(cs)){  // 이미 같은 이름의 section이 있다면 종료합니다.
			printf("\n\nError : multiple control section name\n\n\n");

			return -1;
		}
		else{  // 없다면 external symbol table에 추가합니다.
			strcpy(tmpSection->name, cs);
			
			ADDLL(&externalSymbolTable, (void*)tmpSection);	
		}

		while(str[0]!='E'){  // end가 나올 때까지, 모든 line에 대하여 다음 과정을 수행합니다.
			if(str[0]=='D'){  // 만약 data라면, symbol의 중복 여부를 확인한 뒤, 해당 symbol의 이름과 주소를 external symbol table의, 현재의 section에 추가합니다.
				for(j=1; j<(int)strlen(str); j+=12){
					strncpy(addr, str+j+6, 6);
					strncpy(es, str+j, 6);
					
					if(findExternalSymbol(es)!=-1){
						printf("\n\nError : multiple external symbol name\n\n\n");

						return -1;
					}
					
					tmpSymbol=(externalSymbol*)malloc(sizeof(externalSymbol));

					tmpSymbol->addr=sectionAddr+translate(addr);
					strncpy(tmpSymbol->name, es, 6);
					
					ADDLL(&(tmpSection->external_), (void*)tmpSymbol);
				}
			}

			fgets(str, 128, fp[i]);

			if(str[strlen(str)-1]=='\n')
				str[strlen(str)-1]='\0';
		}

		sectionAddr+=tmpSection->length;

		i+=1;
	}

	endAddr=sectionAddr;

	return sectionAddr-Paddr;
}

int proj3_pass2(FILE** fp, int arg_cnt){  // proj3_pass2 함수 : linking loader의 pass2를 실행함
	
	int i=0, j, k, sectionAddr=Paddr, addrMod, ofs, length, len_text, *ref=NULL, symbolIdx=-24, cnt_modify;
	char str[128], es[14], s[14];
	Node* nowSection=externalSymbolTable;
	
	while(1){
		if(i==arg_cnt)  // 넘겨받은 파일을 모두 읽으면 종료합니다.
			break;
		
		length=((controlSection*)nowSection->data)->length;
		
		fgets(str, 128, fp[i]);

		while(str[0]!='E'){  // end가 나올 때까지 다음의 과정을 반복합니다.
			if(str[0]=='R'){  // reference가 나올 경우, 해당 정보들을 저장해둡니다.
				if(ref)
					free(ref);

				ref=(int*)malloc(((strlen(str)-1)/8+3)*sizeof(int));

				for(j=2; j<(int)(strlen(str)-1)/8+3; j++){
					strncpy(es, str+3+8*(j-2), 6);

					for(k=strlen(es); k<6; k++)
						es[k]=' ';
					
					ref[j]=findExternalSymbol(es);
				}

				ref[1]=sectionAddr;
				symbolIdx=j-1;
			}
			else if(str[0]=='T'){  // text가 나올 경우, 지정한 위치로 이동한 뒤, 해당하는 값을 넣습니다.
				memset(s, '\0', 7);
				strncpy(s, str+1, 6);
				ofs=translate(s);

				memset(s, '\0', 7);
				strncpy(s, str+7, 2);
				len_text=translate(s);

				for(j=1; j<len_text+1; j++){
					memset(s, '\0', 7);
					strncpy(s, str+2*j+7, 2);
					virtual_mems[j+ofs+sectionAddr-1]=translate(s);
				}
			}
			else if(str[0]=='M'){  // modification이 나올 경우, 지정한 symbol을 확인하고, 만약 존재할 경우, symbol value를 해당하는 위치에 가감산 시킵니다.
				memset(s, '\0', 7);
				strncpy(s, str+1, 6);
				ofs=translate(s);

				memset(s, '\0', 7);
				strncpy(s, str+7, 2);
				cnt_modify=translate(s);

				memset(s, '\0', 7);
				strcpy(s, str+10);

				if(translate(s)>symbolIdx){
					printf("\n\nError : reference number error\n\n\n");

					return -1;
				}

				addrMod=virtual_mems[sectionAddr+ofs]%(cnt_modify%2?16:256);

				for(j=0; j<(cnt_modify-1)/2; j++){
					addrMod*=256;
					addrMod+=virtual_mems[sectionAddr+ofs+j+1];
				}

				addrMod+=ref[translate(s)]*(str[9]=='+'?1:-1);

				for(j=(cnt_modify-1)/2; j>0; j--){
					virtual_mems[sectionAddr+ofs+j]=addrMod%256;
					addrMod/=256;
				}

				virtual_mems[sectionAddr+ofs]=(cnt_modify%2?virtual_mems[sectionAddr+ofs]/16*16:0)+addrMod;
			}

			fgets(str, 128, fp[i]);

			if(str[strlen(str)-1]=='\n')
				str[strlen(str)-1]='\0';
		}

		sectionAddr+=length;

		i+=1;

		nowSection=nowSection->next;
	}

	free(ref);

	return Paddr;
}
