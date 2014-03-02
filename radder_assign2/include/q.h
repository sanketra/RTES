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


TASK list_switch(TASK l1, TASK l2)
{
    l1->next = l2->next;
    l2->next = l1;
    return l2;
}


TASK sort(TASK start, int flag) {
    TASK p, q, top;
    int changed = 1;

    top = malloc(sizeof(struct task));

    top->next = start;
    if(start != NULL && start->next != NULL) {
        while( changed ) {
            changed = 0;
            q = top;
            p = top->next;
            while( p->next != NULL ) {
                /* push bigger items down */
				if(flag == 0) { //RM
                	if( p->period > p->next->period ) {
                    	q->next = list_switch( p, p->next );
                    	changed = 1;
                	}
				} else if(flag == 1) { //  DM
					if( p->deadline > p->next->deadline ) {
                    	q->next = list_switch( p, p->next );
                    	changed = 1;
                	}
				} else {// FP
					if( p->priority > p->next->priority ) {
                    	q->next = list_switch( p, p->next );
                    	changed = 1;
                	}
				}
                q = p;
                if( p->next != NULL )
                    p = p->next;
            }
        }
    }
    p = top->next;
    free( top );
    return p;
}
