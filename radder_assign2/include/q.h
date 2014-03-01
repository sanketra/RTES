/* FILE : q.h
   Contains the queue routines */
struct task {
	double wcet;
	double deadline;
	double period;
	int priority;
	struct task *next;
};
typedef struct task* TASK;

TASK insert_rear(TASK first, double wcet, double deadline, double period, int priority) {
	TASK temp, cur;
	temp = (TASK) malloc(sizeof(struct task));
	temp->wcet = wcet;
	temp->deadline = deadline;
	temp->period = period;
	temp->priority = priority;
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

TASK release_nodes(TASK first) {
	if(first == NULL) return NULL;

	TASK temp;
	
	while(first != NULL) {
		temp = first;		
		first = first->next;
		free(temp);
	}
	return NULL;
}
