#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>


struct node {
	int data;
	struct node *next;
};

struct node * list_init(int array[],int length)
{
	struct node *head;
	struct node *p;
	int i;
	
	head = (struct node *)malloc(sizeof(struct node));
	head->next = NULL;

	for(i = 0; i < length; i++){
		p = (struct node *)malloc(sizeof(struct node));
		p->data = array[i];
		p->next = head->next;
		head->next = p;			
	}

	return head;
}

void list_revert(struct node *head)
{
	struct node *p1;
	struct node *p2;
	struct node *temp;

	p1 = NULL;
	p2 = head->next;
	while(p2 != NULL){
		temp = p2->next;
		p2->next = p1;
		p1 = p2;
		p2 = temp;
	}
	head->next = p1;
}


void list_print(struct node *head)
{
	struct node *p;

	p = head->next;
	while(p != NULL){
		printf("%d ",p->data);
		p = p->next;
	}
	printf("\n");
}

void list_free(struct node *head)
{
	struct node *p;
	struct node *temp;

	p = head;
	while(p != NULL){
		temp = p->next;
		free(p);
		p = temp;
	}
}

int main()
{
	int array[] = {1,2,3,4,5,6,7,8,9};

	struct node *head = list_init(array,sizeof(array)/sizeof(array[0]));

	list_print(head);
	list_revert(head);
	list_print(head);
	list_free(head);
}
