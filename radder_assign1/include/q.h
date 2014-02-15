/* FILE : q.h
   Contains the queue routines */

struct node {
	int val;
	char type;
	struct node *next;
};
typedef struct node* NODE;

NODE insert_rear(NODE first, int val, char type) {
	NODE temp, cur;
	temp = (NODE) malloc(sizeof(struct node));
	temp->val = val;
	temp->type = type;
	temp->next = NULL;

	if (first == NULL) {
    	first = temp;
	} else {
		cur = first;
		while (cur->next != NULL) cur = cur->next;
		cur->next = temp;
	}
	return first;
}

void release_nodes(NODE first) {
	if(first == NULL) return;

	NODE temp;
	
	while(first != NULL) {
		temp = first;		
		first = first->next;
		free(temp);
	}
}





	
