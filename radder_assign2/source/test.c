#include<stdio.h>
#include<stdlib.h>
#include "../include/q.h"
#include<string.h>
#include<math.h>

#define MAX_TASKSET 10

TASK task_set[MAX_TASKSET];
int taskset_count;

void display() {
	int i = 0;
	printf("Displaying task details\n");
	for(i = 0; i < taskset_count; i++) {
		TASK first = task_set[i];
		while(first != NULL) {
			printf("%lf %lf %lf %d\n", first->wcet, first->deadline, first->period, first->priority);
			first = first->next;
		}
	}
}

double test_utilisation(TASK first, int density_check) {
	double sum = 0.0;
	TASK temp = first;

	while(temp != NULL) {
		if(density_check == 0)
			sum += temp->wcet/temp->period;
		else
			sum += temp->wcet/(temp->period > temp->deadline ? temp->deadline:temp->deadline);
		temp = temp->next;	
	}
	printf("SUM: %lf\n", sum);
	return sum;	
}

int test_preutilisation(TASK first) {
	TASK temp = first;
	while(temp != NULL) {
		if(temp->deadline < temp->period)
			return -1;
	}
	return 1;
}

int calculate_busyperiod(TASK first) {
	int prevsum = 0, cursum = 0;
	TASK temp = first;
		
	while(temp != NULL) {
		prevsum += temp->wcet;
		temp = temp->next;
	}

	while(1) {
		temp = first;
		cursum = 0;		
		while(temp != NULL) {
			cursum += ceil(prevsum/temp->period) * temp->wcet;
			temp = temp->next;	
			//printf("Cur: %d \t", cursum);
		}
		//printf("\nP: %d  C: %d\n", prevsum, cursum);
		if(prevsum == cursum) 
			return cursum;
		else
			prevsum = cursum;	
	}
}

int test_load_factor(TASK first, int l_factor) {
	int i = 0, t;
	double h, u;
	TASK temp = first;

	while(1) {
		temp = first;
		while(temp != NULL) {
			t = (temp->period * i) + temp->deadline;
			if(t > l_factor) return 1;
			h += temp->wcet;
			u = h/t;
			printf("t: %d  h:%lf  u: %lf\n", t, h, u);
			if(u > 1.0) return -1;
			temp = temp->next;
		}
		i++;
	}
}

void test_EDF() {
	int i = 0, l_factor;
	
	for(i = 0; i < taskset_count; i++) {
		if(test_preutilisation(task_set[i]) == 1) {
			// Utilisation based test.		
			if(test_utilisation(task_set[i], 0) <= 1.0)
				printf("Task set %d EDF Scheduable\n", i);
			else
				printf("Task set %d EDF not Scheduable\n", i);
		} else {
			// Density check.
			if(test_utilisation(task_set[i], 1) <= 1.0) {
				printf("Task set %d EDF Scheduable\n", i);
			} else {
				// Loading factor.
				printf("Loading factor test\n");
				l_factor = calculate_busyperiod(task_set[i]);
				printf("l_factor: %d\n", l_factor);
				if(test_load_factor(task_set[i], l_factor) == 1) 
					printf("Task set %d EDF Scheduable\n", i);
				else
					printf("Task set %d EDF not Scheduable\n", i);
			}
		}
	}		
}

int main() {
	int i, task_count, pri;
	char *line = NULL, c, *pch;
	size_t n = 0;
	double wcet = 0.0, deadline = 0.0, period = 0.0;
	system("clear");
	while(1) {
		printf("Enter the number of tasks\n");
		scanf("%d", &task_count);

		if(task_count == 0) break;
		

		printf("Enter task priorities\n");
		while(c = getchar() != '\n');
		getline(&line, &n, stdin);		
		pri = atoi(strtok(line, " "));

		printf("Enter tasks\n");
		for(i = 0; i < task_count; i++) {
			scanf("%lf %lf %lf", &wcet, &deadline, &period);
			task_set[taskset_count] = insert_rear(task_set[taskset_count], wcet, deadline, period, pri);
			if ((pch = strtok(NULL, " ")) != NULL)
				pri = atoi(pch);
			while(c = getchar() != '\n');
		}
		taskset_count++;	
	}
	test_EDF();
	//display();

return 0;
}
