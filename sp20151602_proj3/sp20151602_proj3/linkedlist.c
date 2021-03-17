#include "20151602.h"
#include "linkedlist.h"

void ADDLL(LL* list, void* data){  // ADDLL 함수 : 넘겨받은 list에 data를 넣는 함수
	Node* now=(Node*)(*list);
	Node* putNode=(Node*)malloc(sizeof(Node));

	putNode->data=(void*)data;
//	putNode->prev=;                보류
	putNode->next=NULL;

	if(now==NULL){
		*list=putNode;

		return;
	}

	while(now->next!=NULL)
		now=(Node*)(now->next);

	(now->next)=putNode;

	return;
}

void FREELL(LL* list){  // FREELL 함수 : 넘겨받은 list의 메모리 해제를 담당하는 함수
	Node* now=*list;
	Node* next;

	while(now){
		next=(Node*)(now->next);
		free(now->data);
		free(now);

		now=next;
	}

	*list=NULL;

	return;
}

void printLL(void* ll){  // printLL 함수 : 넘겨받은 linked list의 내용물을 처음부터 끝까지 출력함
						 // 본래 그럴 목적으로 만들었는데, 일단 지금은 Hash 구조체 전용이 되어버림
	Node* tmp=(Node*)ll;
	
	while(tmp!=NULL){
		printf("[%s, %s]", ((Hash*)(tmp->data))->opInst, ((Hash*)(tmp->data))->opCode);

		if(((Hash*)(tmp->data))->next!=NULL)
			printf(" -> ");
		
		tmp=(Node*)(tmp->next);
	}

	printf("\n");

	return;
}
