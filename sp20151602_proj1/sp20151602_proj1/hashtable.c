#include "20151602.h"
#include "linkedlist.h"
#include "hashtable.h"
#include "commandfunction_mem.h"

Hash* findBucket(char* str){  // findBucket 함수 : 넘겨받은 op instruction이 담긴 bucket을 찾아냄
	void* ret=opList[hashfunction(str)]->data;  // 해당 instruction이 속해야 할 list를 불러옴

	while(ret && strcmp(str, ((Hash*)ret)->opInst))  // 불러온 list를 하나하나 뒤져봄
		ret=((Hash*)ret)->next;

	return (Hash*)ret;  // 탐색 결과를 반환함
}

int opcodeF(Token* token){  // opcodeF 함수 : 넘겨받은 op instruction의 op code를 출력함
	Hash* tmp;

	tmp=findBucket(token->token[0]);  // 해당 instruction이 속한 bucket을 탐색함

	if(tmp!=NULL)  // 있으면 해당 op code를 출력함
		printf("\nopcode is %s\n\n", tmp->opCode);
	else{  // 없으면 경고문을 출력함
		printf("\nERROR\n\n");
		
		return -1;
	}

	return 0;
}

void opcodelistF(){  // opcodelistF 함수 : 모든 op code가 담긴 hash table을 출력함
	Node* tmp;
	int i=0;

	printf("\n");

	for(; i<20; i++){
		printf("%02d : ", i);

		tmp=opList[i];

		printLL(tmp);
	}

	printf("\n");

	return;
}

void CreateTable(){  // CreateTable 함수 : opcode.txt 파일을 바탕으로 하여 hash table을 생성함
	FILE *fp=fopen("opcode.txt", "r");
	char opCode[4], opInst[12], opMod[4];
	void* now;
	int hashvalue;

	if(!fp){
		printf("\nERROR\n\n");
		
		return;
	}

	while(fscanf(fp, "%s %s %s", opCode, opInst, opMod)==3){  // 해당 txt파일에는 각 instruction별로 3개의 정보가 있기에, 총 3개의 문자열을 입력받음
		now=malloc(sizeof(Hash));

		strcpy(((Hash*)now)->opCode, opCode);
		strcpy(((Hash*)now)->opInst, opInst);
		((Hash*)now)->mod=opMod[0]-49;  // 지금은 안 쓰이는 것 같은데, 일단 저장해두도록 함
		((Hash*)now)->next=NULL;

		hashvalue=hashfunction(((Hash*)now)->opInst);  // 해당 instruction의 해싱 값을 생성함

		ADDLL(&opList[hashvalue], now);  // Hash 구조체 object를 해싱 값에 해당하는 list에 추가함
		ADDHT(now, hashvalue);  // Hash 구조체 object를 해싱 값에 해당하는 hash table에 추가함
	}

	if(fclose(fp))
		printf("\nError\n\n");

	return;
}

int hashfunction(char* str){  // hashfunction 함수 : 주어진 정보를 토대로 해싱 값을 생성함
	return abs((int)(str[0]+2*str[1]+3*str[2]+7*str[3]))%20;  // 임의로 수식을 정해서 총 20개의 list에 나뉘어 담기도록 함
}

void ADDHT(void* hash, int value){  // ADDHT 함수 : 넘겨받은 object와 해싱 값을 이용해 해당 object를 저장함
	Hash* now=(Hash*)(opList[value]->data);

	if(!strcmp(now->opInst, ((Hash*)hash)->opInst))
		return;

	while((now->next)!=NULL)
		now=now->next;

	now->next=(Hash*)hash;

	return;
}
