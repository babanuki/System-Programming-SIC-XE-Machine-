#include "20151602.h"
#include "hashtable.h"
#include "commandfunction_mem.h"
#include "assemble.h"

symbolInfo* symbolTable=NULL;
char regi[7]={'A', 'X', 'L', 'B', 'S', 'T', 'F'};
char directives[7][6]={"START", "END", "BYTE", "WORD", "RESB", "RESW", "BASE"};
Assembly* assembly_list=NULL;
obj* objL=NULL;
record* recordL=NULL;
int MAX, LEN, line;
const int VAL0=1048576;
const int VAL1=65536;
const int VAL2=4096;
const int VAL3=2048;

int comma_remover(char* str){  // comma_remover( ) 함수 : 넘겨받은 문자열(str)의 끝에 comma(,)가 있는지 확인하고, 이를 '\0'으로 바꿔주는 함수입니다. 만약 끝이 comma이라면 1을, 아니라면 0을 반환합니다.
	if(str[strlen(str)-1]==','){
		str[strlen(str)-1]='\0';

		return 1;
	}

	return 0;
}

int char_chk(char c){  // char_chk( ) 함수 : 넘겨받은 문자가 숫자 혹은 알파벳에 속하는지 확인하는 함수입니다. 만약 해당한다면 1을, 아니라면 0을 반환합니다.
	if(c>64 && c<91)
		return 1;
	
	if(c>96 && c<123)
		return 1;

	if(c>47 && c<58)
		return 1;

	return 0;
}

Assembly* tokenize_assembly(char *str){  // tokenize_assembly( ) 함수 : 말 그대로, 넘겨받은 문자열의 tokenization을 진행합니다. 이 때, 해당 문자열의 이상 유무도 확인합니다.
	Assembly* ret=(Assembly*)malloc(sizeof(Assembly));
	Hash* bucket;
	char *tok;
	int i, comma_chk=0;

	strcpy(ret->origin, str);
	memset(ret->operand[0], '\0', sizeof(ret->operand[0]));  // 처음엔 따로 초기화 등을 하지 않았으나, 이후 머신을 작동시켜보니 assemble을 반복하면 점점 해당 line의 object code의 값이 커지거나, 주소의 값이 커지는 사태가 발생하여 모든 값을 초기화하기로 했습니다.
	memset(ret->operand[1], '\0', sizeof(ret->operand[1]));
	memset(ret->directive, '\0', sizeof(ret->directive));
	memset(ret->instruction, '\0', sizeof(ret->instruction));
	memset(ret->label, '\0', sizeof(ret->label));

	ret->obj_code=0;
	ret->next=NULL;
	ret->type=2;
	ret->form=0;
	ret->locctr=0;
	ret->error_type=0;

	tok=strtok(str, " ");
	comma_remover(tok);

	if(tok[0]=='.'){  // 해당 line이 주석인지 확인
		ret->type=0;
		ret->form=0;

		return ret;
	}

	if(tok[0]==','){  // comma가 제일 처음에 나왔는지 확인
		ret->error_type=2;

		return ret;
	}

	for(i=0; i<7; i++){  // directive인지 확인
		if(!strcmp(tok, directives[i])){
			ret->form=0;
			ret->type=1;
			strcpy(ret->directive, tok);
			strcpy(ret->instruction, tok);
			break;
		}
	}

	if(ret->type!=1){  // 만약 처음으로 나온 token이 directive가 아닐 경우, opcode를 담은 hash table에 해당 token이 있는지 확인. 만약 없다면, 이는 label로 분류함.
		if(tok[0]=='+')
			bucket=(Hash*)findBucket(tok+1);
		else
			bucket=(Hash*)findBucket(tok);

		if(bucket==NULL){
			if(findSymbol(tok)==NULL){
				strcpy(ret->label, tok);

				tok=strtok(NULL, " ");  // label을 저장했으므로, 다음 단계인 instruction으로 넘어감

				if(tok==NULL){
					ret->error_type=2;

					return ret;
				}

				if(tok[0]=='+')
					bucket=(Hash*)findBucket(tok+1);
				else
					bucket=(Hash*)findBucket(tok);
			}
			else{  // 만약 동일한 라벨이 이미 symbol table에 등록되어 있다면, 이는 중복된 label이 들어왔다는 뜻이므로 error로 취급
				ret->error_type=1;
				return ret;
			}
		}

		if(tok[strlen(tok)-1]==','){  // instruction 뒤에 comma가 나오는지 확인
			ret->error_type=2;

			return ret;
		}

		for(i=0; i<7; i++){  // label 이후에 나온 instruction이 directive인지 확인
			if(!strcmp(tok, directives[i])){
				ret->form=0;
				ret->type=1;
				strcpy(ret->directive, tok);
				strcpy(ret->instruction, tok);

				break;
			}
		}

		if(bucket!=NULL){  // 앞서 해당 token (instruction)에 대한 bucket의 탐색을 마쳤으므로, 해당 bucket의 유무를 확인
			ret->type=2;
			ret->form=bucket->mod+1;
			strcpy(ret->instruction, tok);
			
			if(tok[0]=='+'){
				if(bucket->mod!=2){
					ret->error_type=2;

					return ret;
				}

				ret->form=4;
			}
		}

		else if(ret->type!=1){
			ret->error_type=2;
		
			return ret;
		}
	}

	tok=strtok(NULL, " ");  // operand 단계로 넘어감

	if(bucket!=NULL){  // directive와 같은 예외가 존재하기에, 해당 조건문(bucket의 유무)이 필요함
		if(bucket->operand==0 && tok!=NULL){  // 해당 instruction의 필요한 operand는 0개인데, token이 있다면 이는 오류
			ret->error_type=3;
			return ret;
		}

		if(bucket->operand!=0 && tok==NULL){  // 해당 instruction의 필요한 operand는 0개가 아닌데, token이 없다면 이는 오류
			ret->error_type=3;
			return ret;
		}

		if(bucket->operand==0)  // 해당 instruction의 필요한 operand가 0개라면, 이미 조건을 충족한 것이므로 ret을 반환. 이 때, token이 있는 경우는 위에서 이미 걸러졌을 것이므로, 이곳에 오는 것은, token이 없는 경우이므로 문제가 없음
			return ret;
	}

	comma_chk=comma_remover(tok);  // comma의 여부 확인 및 comma를 제거
	strcpy(ret->operand[0], tok);
	ret->operand_cnt=1;
	
	tok=strtok(NULL, " ");  // 다음 operand로 넘어감

	if(tok!=NULL && !comma_chk){  // 만약 이전 operand의 끝에 comma가 없었는데, 이번 token이 존재한다면 이는 오류
		ret->error_type=3;
		return ret;
	}
	
	if(tok!=NULL){
		strcpy(ret->operand[1], tok);
		ret->operand_cnt=2;
	}

	if(tok!=NULL && comma_remover(tok)){  // 만약 2번째 operand의 뒤에 comma가 존재한다면, 이는 오류
		ret->error_type=3;

		return ret;
	}

	if(tok==NULL && comma_chk){  // 만약 2번째 operand가 존재하지 않는데, 이전 operand의 끝에 comma가 존재한다면, 이는 오류
		ret->error_type=3;

		return ret;
	}

	if(ret->type==2){  // 만약 이번 line이 directive instruction을 포함하지 않았다면, 다음의 과정을 수행
		if(ret->form>2){
			if(findRegister(ret->operand[0]) || (ret->operand[1][0]!='\0' && strcmp(ret->operand[1], "X"))){
				ret->error_type=3;
				return ret;
			}

			if(ret->operand[1][0]!='\0')
				ret->index=1;
		}
		else if(ret->form==2){
			if(!findRegister(ret->operand[0])){
				ret->error_type=3;
				return ret;
			}

			else if(bucket->operand!=1){
				if(!findRegister(ret->operand[1])){
					ret->error_type=3;
					return ret;
				}
			}
		}
	}

	return ret;
}

void symbolF(){  // symbolF 함수 : 이전 assemble 과정에서 만들어진 symbol table의 내용물을, 정렬된 상태로 출력합니다.
	symbolInfo* now=symbolTable;

	if(!now){
		printf("\nsymbol table is empty\n\n");

		return;
	}
	else{
		while(now!=NULL){
			printf("\t%s\t%04X\n", now->symbol_name, now->addr);

			now=now->next;
		}
	}

	return;
}

symbolInfo* findSymbol(char* str){  // findSymbol( ) 함수 : 넘겨받은 str과 일치하는 label명을 가진 symbol이 symbol table에 존재하는지 확인하는 함수입니다. 만약 있다면 해당 symbol을, 없다면 NULL을 반환합니다.
	symbolInfo* ret=symbolTable;

	while(ret!=NULL){
		if(!strcmp(str, ret->symbol_name))
			return ret;

		ret=ret->next;
	}

	return NULL;
}

int findRegister(char* str){  // findRegister( ) 함수 : 넘겨받은 문자열(str)이 유효한 register인지 확인하는 함수입니다. 유효한 register라면 1을, 아니라면 0을 반환합니다.
	int i=0;

	if(strlen(str)!=1)
		return 0;

	for(; i<7; i++){
		if(str[0]==regi[i])
			return 1;
	}

	return 0;
}

void symbolAdd(char* str, int addr){  // symbolAdd( ) 함수 : 넘겨받은 문자열과 주소를 통해 새로운 symbolInfo 객체를 생성하고, 이를 symbol table에 추가하는 함수입니다.
	symbolInfo* now=symbolTable;
	symbolInfo* putNode=(symbolInfo*)malloc(sizeof(symbolInfo));

	strcpy(putNode->symbol_name, str);
	putNode->addr=addr;
	putNode->next=NULL;

	if(symbolTable==NULL){
		symbolTable=putNode;

		return;
	}
	else if(strcmp(symbolTable->symbol_name, putNode->symbol_name)>0){  // 정렬 상태를 위해 제일 앞에 putNode를 삽입해야 하는 경우
		putNode->next=symbolTable;
		symbolTable=putNode;

		return;
	}

	while(now->next!=NULL && strcmp((now->next)->symbol_name, str)<0)  // 정렬 상태를 위해 적절한 위치를 찾아야 하는 경우
		now=now->next;

	putNode->next=now->next;
	now->next=putNode;

	return;
}

int assembleF(Token* token){  // assembleF( ) 함수 : 'assemble' 명령어를 수행하는 함수입니다. 초기화와 파일의 open/close, 그리고 assembler의 two passes를 감독합니다.
	FILE* origin_fp=fopen(token->token[0], "r"), *lst_fp, *obj_fp;
	char lst_name[256], obj_name[256];
	int len=strlen(token->token[0]);


	line=MAX=LEN=0;  //

	if(!origin_fp){
		printf("\nERROR\n\n");

		return -1;
	}

	freeAssembly();  // 초기화를 진행
	free_objL();
	freeSymbol();
	freeRecord();
	strcpy(lst_name, token->token[0]);
	strcpy(obj_name, lst_name);

	lst_name[len-3]='l';
	lst_name[len-2]='s';
    lst_name[len-1]='t';

	obj_name[len-3]='o';
	obj_name[len-2]='b';
	obj_name[len-1]='j';

	lst_fp=fopen(lst_name, "w");
	obj_fp=fopen(obj_name, "w");

	if(!(lst_fp && obj_fp)){
		printf("\nERROR\n\n");

		fclose(origin_fp);

		if(lst_fp)
			fclose(lst_fp);
		if(obj_fp)
			fclose(obj_fp);

		return -1;
	}

	if(!Assembly_pass1(origin_fp) && !Assembly_pass2(lst_fp, obj_fp))  // pass1과 pass2를 호출
		printf("Successfully assemble %s.\n", token->token[0]);
	else{
		printf("\nERROR\n\n");

		remove(lst_name);  // 만약 pass1과 pass2를 진행하는 도중에 문제가 발생할 경우, lst파일과 obj파일을 삭제
		remove(obj_name);

		fclose(origin_fp);
		fclose(lst_fp);
		fclose(obj_fp);

		return -1;
	}

//	freeASM();
//	free_objL();
//	freeSymbol();
//	freeRecord();

	fclose(obj_fp);
	fclose(lst_fp);
	fclose(origin_fp);

	return 0;
}

int Assembly_pass1(FILE* origin_fp){  // Assembly_pass1 함수 : pass1을 수행하는 함수입니다.
	Assembly *now, *prev=NULL;
	char tmp[256];
	int start_loc=-24, line5x=0, digit;
	int isError;

	while(fgets(tmp, 256, origin_fp)!=NULL){  // origin_fp의 모든 line을 읽으면 종료
		isError=0;

		line5x+=5;

		if(tmp[strlen(tmp)-1]=='\n')
			tmp[strlen(tmp)-1]='\0';

		MAX=(MAX<(int)strlen(tmp)?(int)strlen(tmp):MAX);  // origin_fp의 line들 중에서 가장 긴 line의 길이를 저장. 이는 이후에 lst 파일을 제작할 때 쓰임

		now=tokenize_assembly(tmp);  // 현재의 line을 tokenize함
		
		if(!strcmp(now->directive, "START")){  // 만약 현재의 line에 START가 있다면, 이하의 과정을 수행
			if(start_loc!=-24){  // 만약 START가 나오기 전에 이미 다른 시작 지점이 갱신되어 있다면, 이는 오류
				printf("\nERROR\n\n");

				isError=1;

				break;
			}

			start_loc=translate(now->operand[0]);  // 시작 지점을 START의 operand로 갱신
			assembly_list=now;  // assembly_list의 시발점을 now로 갱신
			prev=NULL;  // now가 시발점이므로, prev는 NULL
			now->locctr=start_loc;  // now의 주소를 시작 주소로 갱신
		}

		if(start_loc==-24){  // 만약 START를 가진 line이 나오지 않았다면,
			start_loc=0;  // 시작 지점을 0으로 결정, 이후의 과정은 위와 동일
			assembly_list=now;
			prev=NULL;
			now->locctr=start_loc;
		}
		else if(prev!=NULL){
			prev->next=now;
		}

		now->locctr=start_loc;  // now의 주소를 현재의 지점으로 갱신
		now->line=line5x;  // now의 line 또한 갱신
		if(now->type==0){  // 만약 now가 주석이라면, 따로 이후의 작업은 진행하지 않고, 현재 위치를 주석의 size만큼 증가시키고 새로운 line에 대한 작업을 시작
			start_loc+=now->sz;
			prev=now;
			continue;
		}

		if(strcmp("START", now->directive) && now->label[0]!='\0'){  // 만약 label이 존재한다면,
			if(findSymbol(now->label)){  // 아직 등록되어 있지 않아야 하므로, 만약 해당 label의 symbol이 symbol table에 존재한다면 이는 오류
				now->error_type=1;
				isError=1;

				break;
			}
			
			symbolAdd(now->label, start_loc);  // symbol table에 해당 symbol을 등록
		}
		
		if(now->type==1){  // 만약 현재의 line이 directive를 가지고 있다면, 이하의 과정을 수행. size를 계산하는 부분임
			if(!strcmp(now->directive, "BYTE")){
				switch(now->operand[0][0]){
					case 'C':
						now->sz=strlen(now->operand[0])-3;
						break;
					case 'X':
						now->sz=strlen(now->operand[0])/2-1;
						break;
					default:
						now->sz=1;

						for(digit=256; digit<=atoi(now->operand[0]); digit*=256)
							now->sz++;
				}
			}
			else if(!strcmp(now->directive, "WORD"))
				now->sz=3;
			else if(!strcmp(now->directive, "RESB"))
				now->sz=atoi(now->operand[0]);
			else if(!strcmp(now->directive, "RESW"))
				now->sz=3*atoi(now->operand[0]);
		}
		else if(now->type==2)
			now->sz=now->form;

		start_loc+=now->sz;  // 현재의 line의 size만큼, 현재 위치를 이동
		prev=now;  // 또한, prev를 now로 변경

		if(!strcmp(now->instruction, "END"))  // 만약 END가 들어왔다면, 반복을 종료
			break;

		if(now->error_type!=0)  // 만약 현재의 line이 error를 가지고 있다면, isError (error 여부를 확인)의 값을 변경
			isError=1;

		if(isError){  // error 발생 시, 반복을 종료
			break;
		}
	}
	
	LEN=start_loc;  // 생성될 파일의 총 길이를 저장
	
	if(isError){  // 만약 위의 과정에서 error가 검출되었다면, 각 error 타입에 맞게 출력 문구를 변경하고, 에러가 생겼다고 assembleF( ) 함수에게 알림 (return -1)
		switch(now->error_type){
			case 1:
				strcpy(tmp, "PSEUDO");
				break;
			case 2:
				strcpy(tmp, "INSTRUCTION");
				break;
			case 3:
				strcpy(tmp, "OPERAND");
				break;
		}

		printf("\nERROR : %s error occurs\nline : %d\n", tmp, line5x);

		return -1;
	}

	return 0;  // 정상적으로 일을 끝마쳤다면, 0을 반환
}

int Assembly_pass2(FILE* lst_fp, FILE* obj_fp){  // Assembly_pass2 함수 : pass2를 수행하는 함수입니다.
	Assembly* now;
	symbolInfo* sym;
	int regi_base=0, regi_pc=assembly_list->locctr;
	int start_loc, flagA, flagB, i, j;
	int b_bit, p_bit, x_bit;
	int i_bit, n_bit;
	char tmp[4];
	
	now=assembly_list;  // assembly_list의 시작지점을, now의 초기값으로 정함

	while(now!=NULL){
		b_bit=p_bit=x_bit=0;
		i_bit=n_bit=flagA=flagB=1;
		
		sym=NULL;  // 현재의 symbol을 NULL로 갱신

		if(now->next!=NULL)  // 만약 now의 다음 code가 존재한다면, pc register의 값을 다음 code의 location으로 설정
			regi_pc=(now->next)->locctr;

		switch(now->type){  // now의 type에 따라 이하의 과정을 수행
			case 2:  // now의 type이 instruction이라면, 
				x_bit=p_bit=b_bit=0;
				i_bit=n_bit=1;

				if(now->instruction[0]=='+')  // now의 instruction이 담긴 bucket을 탐색하고, 해당 bucket에 담긴 opcode의 값을 now의 object code값으로 저장
					strcpy(tmp, ((Hash*)findBucket((now->instruction)+1))->opCode);
				else
					strcpy(tmp, ((Hash*)findBucket(now->instruction))->opCode);

				now->obj_code=translate(tmp);

				switch(now->form){  // now의 format에 따라 각자의 정해진 과정을 수행
									// 그냥 object code값을 계산하고, 이를 저장하는 게 끝이라 뭐라 덧붙일 말이 없음
					case 4:
						sym=findSymbol((now->operand[0])+(char_chk(now->operand[0][0])?0:1));
						
						if(sym==NULL){
							now->obj_code=16*(now->obj_code)+16;
							now->obj_code++;
							now->obj_code*=VAL0;
							now->obj_code+=atoi(now->operand[0]+1)&1048575;
						}
						else{
							now->obj_code=16*now->obj_code+48;
							now->obj_code++;
							now->obj_code*=VAL0;
							now->obj_code+=sym->addr&1048575;
						}
						break;

					case 3:
						if(now->operand_cnt==0){
							now->obj_code=VAL1*(now->obj_code)+3*VAL1;

							break;
						}

						if(now->index)
							x_bit=1;
						if(now->operand[0][0]=='#')
							n_bit=0;
						if(now->operand[0][0]=='@')
							i_bit=0;

						sym=findSymbol(now->operand[0]+(n_bit+i_bit<2?1:0));

						if(sym==NULL && VAL3+regi_pc-atoi(now->operand[0]+1)>-1){
							now->obj_code++;

							if(n_bit)
								now->obj_code++;

							now->obj_code*=VAL1;
							now->obj_code+=atoi(now->operand[0]+1);

							break;
						}

						else if(sym!=NULL){
							if(VAL3+(sym->addr)>=regi_pc){
								if(n_bit==1){
									if(i_bit==1)
										now->obj_code+=3;
									else
										now->obj_code+=2;
								}
								else
									now->obj_code++;

								now->obj_code*=16;

								now->obj_code+=(x_bit==0?2:10);
								now->obj_code*=VAL2;
								now->obj_code+=(sym->addr-regi_pc)&4095;
							}
							else if(sym->addr-VAL2<regi_base){
								if(n_bit==1){
									if(i_bit==1)
										now->obj_code+=3;
									else
										now->obj_code+=2;
								}
								else
									now->obj_code++;

								now->obj_code*=16;

								now->obj_code+=(x_bit==0?4:12);
								now->obj_code*=VAL2;
								now->obj_code+=(sym->addr-regi_base)&4095;
							}
							else if(sym->addr<32768){
								now->obj_code*=16;
								
								if(x_bit==1)
									now->obj_code+=8;

								now->obj_code*=VAL2;
								now->obj_code+=sym->addr;
							}
							else{
								now->error_type=2;

								return -1;
							}
						}

						break;

					case 2:
						for(i=0; i<2; i++){
							now->obj_code*=16;

							for(j=0; j<7; j++)
								if(regi[j]==now->operand[i][0])
									now->obj_code+=j;
						}

						break;
				}

				break;

			case 1:  // now의 type이 pseudo (directive instruction)이라면, 아래의 과정을 수행
					 // 이 때, 각 directive instruction은 각자의 역할(영향력)이 다르기에, 각 directive마다 따로 설정해줘야 함
				if(!strcmp(now->directive, "START")){  // START가 나왔으므로, print_obj( ) 함수를 호출함
					flagB=0;
					start_loc=regi_base=now->locctr;
					print_obj(obj_fp, now, start_loc);
				}
				else if(!strcmp(now->directive, "END")){  // END가 나왔으므로, print_obj( ) 함수를 호출함
					flagA=flagB=0;
					print_obj(obj_fp, now, start_loc);
				}
				else if(!strcmp(now->directive, "WORD"))
					now->obj_code=atoi(now->operand[0]);
				else if(!strcmp(now->directive, "BASE")){
					flagA=flagB=0;
					regi_base=findSymbol(now->operand[0])->addr;
				}
				else if(!strncmp(now->directive, "RES", 3))
					flagB=0;
				else if(!strcmp(now->directive, "BYTE")){
					if(now->operand[0][0]=='X')
						now->obj_code+=strtol(now->operand[0]+2, NULL, 16);
					else if(now->operand[0][0]=='C')
						for(i=2; now->operand[0][i]!='\''; i++){
							now->obj_code*=256;
							now->obj_code+=now->operand[0][i];
						}
					else
						now->obj_code=atoi(now->operand[0]);
				}

				break;

			case 0:  // 만약 now의 type이 주석이라면, lst에 따로 다른 정보를 출력할 필요가 없으므로, flag는 모두 0으로 갱신
				flagA=flagB=0;
		}

		if(now->obj_code)  // 만약 now의 object code값이 0이 아니라면, print_obj( ) 함수를 호출
			print_obj(obj_fp, now, start_loc);

		print_lst(lst_fp, now, flagA, flagB);  // print_lst( ) 함수를 호출
		
		now=now->next;  // now를 다음 객체로 변경
	}

	return 0;
}

void print_obj(FILE* obj_fp, Assembly* now, int start_loc){  // print_obj( ) 함수 : object 파일에 자료를 write하는 함수입니다. 넘겨받은 now가 어떤지 확인하고 출력을 할지, 혹은 objL에 추가할지를 결정합니다. 만약 정해진 길이를 넘어설 경우, 현재 objL에 쌓인 자료들을 출력하고 길이를 0으로 갱신해줍니다.
	char* tmp;
	
	if(!strcmp(now->directive, "START")){  // now의 directive가 START일 경우, 시작지점 표시(Header)를 출력하고, 함수를 종료
		fprintf(obj_fp, "H%-6s%06X%06X\n", now->label, now->locctr, LEN);
		
		return;
	}

	if(!strcmp(now->directive, "END")){  // now의 directive가 END일 경우, 종료지점 표시(End)를 출력하고, 함수를 종료, 그리고 현재까지 기록된 midified recording을 출력하도록 함(printRecord( ) 함수를 호출)
		printRecord(obj_fp);

		fprintf(obj_fp, "E%06X\n", start_loc);

		return;
	}

	tmp=now->operand[0];

	if(!char_chk(now->operand[0][0]))
		tmp+=1;

	if(findSymbol(tmp) && now->form==4)  // 만약 now의 format이 4라면, modified recording list에 추가
		addRecord(now->locctr+1-start_loc, 5);

	if(now->sz+line>30){  // 만약 objL의 길이가 30을 넘긴다면, 현재 지점을 텍스트 표시(Text)와 함께 출력하고, 현재까지 저장된 objL을 출력(print_objL( ) 함수를 호출)
		fprintf(obj_fp, "T%06X%02X", (objL==NULL?now->locctr:objL->loc), line);

		print_objL(obj_fp);

		line=0;  // 다 출력한 뒤에는 저장 길이를 0으로 한 뒤,

		free_objL();  // objL을 free 시킴
	}

	add_objL(now);  // now를 objL에 추가하고,
	line+=now->sz;  // now의 size만큼 길이를 늘림

	if(strcmp(now->label, "INPUT") && (!strcmp(now->directive, "BYTE") || !strcmp(now->directive, "WORD"))){  // 만약 now가 Text라면, 현재까지 저장된 objL을 출력(과정은 위와 마찬가지)
		fprintf(obj_fp, "T%06X%02X", (objL==NULL?now->locctr:objL->loc), line);

		print_objL(obj_fp);

		line=0;

		free_objL();
	}

	return;
}

void print_objL(FILE* obj_fp){  // print_objL( ) 함수 : 현재까지 저장된 object code들(objL의 내용물)을 obj_fp에 작성하는 함수입니다.
	obj* now=objL;

	while(now){  // objL을 탐색하면서 각각의 object code값을 obj_fp에 작성
		switch(now->sz){
			case 1:
				fprintf(obj_fp, "%02X", now->code);
				break;
			case 2:
				fprintf(obj_fp, "%04X", now->code);
				break;
			case 3:
				fprintf(obj_fp, "%06X", now->code);
				break;
			case 4:
				fprintf(obj_fp, "%08X", now->code);
		}

		now=now->next;
	}

	fprintf(obj_fp, "\n");

	return;
}

void add_objL(Assembly* tmp){
	obj* now=objL, *putNode=NULL;

	putNode=(obj*)malloc(sizeof(obj));

	putNode->code=tmp->obj_code;
	putNode->sz=tmp->sz;
	putNode->next=NULL;
	putNode->loc=tmp->locctr;

	if(objL==NULL){
		objL=putNode;

		return;
	}

	while(now->next)
		now=now->next;

	now->next=putNode;

	return;
}

void printRecord(FILE* obj_fp){  // printRecord( ) 함수 : 현재까지 저장된 modified recording list (recordL)에 담긴 component들을 object 파일에 작성하는 함수입니다.
	record* now=recordL;

	while(now!=NULL){
		fprintf(obj_fp, "M%06X%02d\n", now->loc, now->sz);

		now=now->next;
	}

	return;
}

void addRecord(int loc, int len){  // addRecord( ) 함수 : 넘겨받은 주소와 길이를 이용해 새로운 record 객체를 생성하고, 이를 기존의 modified recording list (recordL)에 추가하는 함수입니다.
	record* now=recordL, *putNode=NULL;

	putNode=(record*)malloc(sizeof(record));

	putNode->next=NULL;
	putNode->sz=len;
	putNode->loc=loc;

	if(recordL==NULL){
		recordL=putNode;

		return;
	}

	while(now->next)
		now=now->next;

	now->next=putNode;

	return;
}

void print_lst(FILE* lst_fp, Assembly* tmp, int flagA, int flagB){  // print_lst( ) 함수 : 넘겨받은 line의 정보를 listing 파일에 작성하는 함수입니다.
	int i=0;

	fprintf(lst_fp, "%d\t", tmp->line);  // 해당 code의 line을 출력

	if(flagA)  // 해당 line의 위치값을 출력해야 한다면, 출력
		fprintf(lst_fp, "%04X\t", tmp->locctr);
	else  // 그게 아니라면 그냥 넘김
		fprintf(lst_fp, "\t");

	for(; i<(int)strlen(tmp->origin); i++){  // 해당 code의 기본 정보들을 출력
		fprintf(lst_fp, "%c", tmp->origin[i]);
	}

	for(; i<MAX; i++)  // listing 파일의 정렬을 위해, code의 최대 길이까지 공백을 마련
		fprintf(lst_fp, " ");

	if(flagB){  // 해당 line의 object code값을 출력해야 한다면, 출력
//		fprintf(lst_fp, "\t");

		switch(tmp->sz){
			case 1:
				fprintf(lst_fp, "%02X", tmp->obj_code);
				break;
			case 2:
				fprintf(lst_fp, "%04X", tmp->obj_code);
				break;
			case 3:
				fprintf(lst_fp, "%06X", tmp->obj_code);
				break;
			case 4:
				fprintf(lst_fp, "%08X", tmp->obj_code);
				break;
		}
	}

	fprintf(lst_fp, "\n");

	return;
}

void free_objL(){  // free_objL( ) 함수 : 현재까지 구성된 object code의 list를 free하는 함수입니다.
	obj* now=objL, *next;

	while(now!=NULL){
		next=now->next;

		free(now);

		now=next;
	}

	objL=NULL;

	return;
}

void freeRecord(){  // freeRecord( ) 함수 : 현재까지 구성된 modified record의 list를 free하는 함수입니다.
	record* now=recordL, *next;

	while(now){
		next=now->next;

		free(now);

		now=next;
	}

	recordL=NULL;

	return;
}

void freeSymbol(){  // freeSymbol( ) 함수 : 현재까지 구성된 symbol table을 free하는 함수입니다.
	symbolInfo* now=symbolTable, *next;

	while(now){
		next=now->next;

		free(now);

		now=next;
	}

	symbolTable=NULL;

	return;
}

void freeAssembly(){  // freeAssembly( ) 함수 : 현재까지 구성된 source code의 line으로 구성된 list를 free하는 함수입니다.
	Assembly* now=assembly_list, *next;

	while(now){
		next=now->next;

		free(now);

		now=next;
	}

	assembly_list=NULL;

	return;
}
