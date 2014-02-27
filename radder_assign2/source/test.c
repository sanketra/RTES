#include<stdio.h>
#include<stdlib.h>
#include "../include/q.h"
#include<string.h>
#include<math.h>

#define MAX_TASKSET 10000

TASK task_set[MAX_TASKSET];
int task_count[MAX_TASKSET];
int taskset_count;

void display() {
	int i = 0;
	printf("Displaying task details\n");
	for(i = 0; i < taskset_count; i++) {
		TASK first = task_set[i];
		while(first != NULL) {
			printf("C:%lf \t D:%lf \t T:%lf \t P:%d\n", first->wcet, first->deadline, first->period, first->priority);
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
	printf("Utilisation: %lf\n", sum);
	return sum;	
}

int test_preutilisation(TASK first) {
	TASK temp = first;
	while(temp != NULL) {
		if(temp->deadline < temp->period)
			return -1;
		temp = temp->next;
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

double h_cal(TASK first, int t) {
	int i = 0, l;
	TASK temp = first;
	double h = 0.0;
	while(temp != NULL) {
		while(1) {
			l = (temp->period * i) + temp->deadline;
			if(l > t) break;
			h += temp->wcet;
			i++;	
		}
		i = 0;
		temp = temp->next;
	}
	
	return h;
}

int test_load_factor(TASK first, int l_factor) {
	int i = 0, t;
	double h, u;
	TASK temp = first;

	while(1) {
		temp = first;
		while(temp != NULL) {
			t = (temp->period * i) + temp->deadline;
			h = h_cal(first, t);
			u = h/t;
			printf("t: %d  h:%lf  u: %lf\n", t, h, u);
			if(t == l_factor) return 1;
			if(u > 1.0) return -1;
			temp = temp->next;
		}
		i++;
	}
}

void test_EDF() {
	int i = 0, l_factor;
	printf("EDF started\n");
	for(i = 0; i < taskset_count; i++) {
		if(test_preutilisation(task_set[i]) == 1) {
			// Utilisation based test.
			printf("EDF Utilisation Ana\n");		
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
				printf("Proceeding to loading factor test\n");
				l_factor = calculate_busyperiod(task_set[i]);
				printf("Calulated load factor: %d\n", l_factor);
				if(test_load_factor(task_set[i], l_factor) == 1) 
					printf("Task set %d EDF Scheduable\n", i);
				else
					printf("Task set %d EDF not Scheduable\n", i);
			}
		}
	}		
}

double cal_utilisation_val(int n) {
	return (n * (pow(2.0, 1.0/n) - 1));
}

double test_rt_utilisation(TASK first, int j, int check) {
	double sum = 0.0;
	TASK temp = first;
	int count = 0;

	while(temp != NULL) {
		if(check == 0)
			sum += temp->wcet/temp->period;
		else
			sum += temp->wcet/(temp->period > temp->deadline ? temp->deadline:temp->deadline);
		temp = temp->next;
		count++;
		if(count == j) break;	
	}

	return sum;	
}

void test_rt(TASK first, int i) {
	int prevsum = 0, cursum = 0, count = 0;
	TASK temp = first;
	printf("RT test for job %d\n", i);	
	while(temp != NULL) {
		prevsum += temp->wcet;
		temp = temp->next;
		count++;
		if(count == i) break;
	}
	count = 0;

	while(1) {
	 	temp = first;
		cursum = 0;
		count = 0;
		while(temp != NULL) {
			cursum += ceil(prevsum/temp->period) * temp->wcet;
			temp = temp->next;
			count++;
			if(count == (i - 1)) {
				cursum += temp->wcet;
				break;
			}	
		}
		//printf("\nP: %d  C: %d\n", prevsum, cursum);
		if(prevsum == cursum) { 
			if(cursum < temp->period) {
				printf("Schedulable...response time for job %d is: %d\n", i, cursum);
			} else {
				printf("Not Schedulable...response time for jobs %d is: %d\n", i, cursum);
			}
			break;
		} else {
			prevsum = cursum;
		}
	}

}

void rt_analysis(TASK first, int n, int check) {
	int i = 0;
	double res = 0.0;
	printf("RT ana for taskset %d\n", n);	
	for(i = 1; i <= task_count[n]; i++) {
		res = test_rt_utilisation(first, i, check);

		if(res <= cal_utilisation_val(i))
			printf("Schedulable...utilisation for RT with %d jobs: %lf\n", i, res);
		else
			test_rt(first, i);
	}
}

void test_RM_DM() {
	int i = 0, response_t;
	double U = 0.0, N = 0.0;
	
	for(i = 0; i < taskset_count; i++) {
		if(test_preutilisation(task_set[i]) == 1) {
			// Utilisation based test for D = P.
			U = test_utilisation(task_set[i], 0);
			N =	cal_utilisation_val(task_count[i]);
			printf("U: %lf  N: %lf\n", U, N);
			if(U <= N)
				printf("Task set %d RM Scheduable\n", i);
			else if(U > 1.0) 
				printf("Task set %d RM not Scheduable\n", i);
			else {
				// Response time analysis for RM.
				rt_analysis(task_set[i], i, 0);			
			}
		} else {
			// Density check for D < P.
			U = test_utilisation(task_set[i], 1);
			N =	cal_utilisation_val(task_count[i]);
			if(U <= N) {
				printf("Task set %d DM Scheduable\n", i);
			} else {
				// Response time analysis for DM.
				rt_analysis(task_set[i], i, 1);
			}
		}
	}		
}

void cal_effective_utilisation(TASK first) {
	TASK cur = first, temp;
	double f = 0.0;
	int h_count = 0, i = 0;

	while(cur != NULL) {
		temp = first;
		while(temp != NULL) {
			if(temp == cur) {
				f += temp->wcet/temp->period;
			} else {
				if(cur->priority < temp->priority) {
					if(temp->period <= cur->deadline) {
						f += temp->wcet/temp->period;
						h_count++;
					} else {
						f += temp->wcet/cur->period;
					}
				}
			}
			printf("f: %lf\n", f);
			temp = temp->next;
		}
		i++;
		if(f <= cal_utilisation_val(h_count + 1)) {
			printf("FP schedulable..f: %lf\n", f);
		} else {
			printf("FP not schedulable..f: %lf\n", f);
			test_rt(first, i);
		}

		cur = cur->next;
		f = 0.0;
		h_count = 0;
	}
}

void test_FP() {
	int i = 0;
	for(i = 0; i < taskset_count; i++) {
		cal_effective_utilisation(task_set[i]);
	}
}

void UUnifast(int n, double U, double *a) {
	double sumU = U, nextsumU;
	int i = 0;

	for(i = 0; i < n-1; i++) {
		nextsumU = sumU * 0.9;
		a[i] = sumU - nextsumU;
		//printf("a[%d] = %lf\n", i, a[i]);
		sumU = nextsumU;
	}
	a[i] = sumU;
	//printf("a[%d] = %lf\n", i, a[i]);
}

void generate_taskset() {
	double u = 0.05, t, c, a[10];
	int i = 0, j = 0;
	//for(u = 0.05; u < 0.95; u += 0.1) {
		for(j = 0; j < 10000; j++) {
			UUnifast(10, u, a);
			task_count[taskset_count] = 10;
			for(i = 0; i < task_count[taskset_count]; i++) {
				t = 1000 + rand() / (RAND_MAX/(10000 - 1000 + 1) + 1);
				c = a[i] * t;
				task_set[taskset_count] = insert_rear(task_set[taskset_count], c, t+(rand()%10), t, i);
			}
			taskset_count++;
		}
	//}
}

int main() {
	int i, pri;
	char *line = NULL, c, *pch;
	size_t n = 0;
	double wcet = 0.0, deadline = 0.0, period = 0.0;
	//system("clear");
	/*while(1) {
		printf("Enter the number of tasks\n");
		scanf("%d", &task_count[taskset_count]);
		
		if(task_count[taskset_count] == 0) break;
		
		printf("Enter task priorities\n");
		while(c = getchar() != '\n');
		getline(&line, &n, stdin);		
		pri = atoi(strtok(line, " "));

		printf("Enter tasks\n");
		for(i = 0; i < task_count[taskset_count]; i++) {
			scanf("%lf %lf %lf", &wcet, &deadline, &period);
			task_set[taskset_count] = insert_rear(task_set[taskset_count], wcet, deadline, period, pri);
			if ((pch = strtok(NULL, " ")) != NULL)
				pri = atoi(pch);
			while(c = getchar() != '\n');
		}
		taskset_count++;	
	}*/
	//printf("started ana\n");
	generate_taskset();
	//display();
	test_EDF();
	//test_RM_DM();	
	//test_FP();	
	
	return 0;
}
