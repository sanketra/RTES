#include "../include/test.h"

void display() {
	int i = 0;
	freopen("Tasks_log.txt", "w", stdout);
	printf("Displaying task details\n");
	for(i = 0; i < taskset_count; i++) {
		TASK first = task_set[i];
		while(first != NULL) {
			printf("C:%lf \t D:%lf \t T:%lf \t P:%d\n", first->wcet, first->deadline, first->period, first->priority);
			first = first->next;
		}
	}
	freopen("/dev/tty", "w", stdout);
}

double test_utilisation(TASK first, int density_check) {
	double sum = 0.0;
	TASK temp = first;

	while(temp != NULL) {
		if(density_check == 0)
			sum += temp->wcet/temp->period;
		else
			sum += temp->wcet/(temp->period > temp->deadline ? temp->deadline:temp->period);
		printf("%lf\t %lf\t %lf\n", temp->period, temp->deadline, sum);
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

long calculate_busyperiod(TASK first) {
	long prevsum = 0, cursum = 0;
	TASK temp = first;
		
	while(temp != NULL) {
		prevsum += temp->wcet;
		//printf("E: %lf D: %lf P: %lf\n", temp->wcet, temp->deadline, temp->period);
		temp = temp->next;
	}

	while(1) {
		temp = first;
		cursum = 0;		
		while(temp != NULL) {
			cursum += ceil(prevsum/temp->period) * temp->wcet;
			temp = temp->next;	
			//printf("Cur: %ld \t", cursum);
		}
		//printf("\nP: %ld  C: %ld\n", prevsum, cursum);
		if(prevsum == cursum || cursum < 0) 
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

int test_load_factor(TASK first, long l_factor) {
	long i = 0, t;
	double h, u;
	TASK temp = first;

	while(temp != NULL) {
		while(1) {
			t = (temp->period * i) + temp->deadline;
			h = h_cal(first, t);
			u = h/t;
			//printf("t: %ld  h:%lf  u: %lf\n", t, h, u);
			if(t > l_factor) break;
			if(u > 1.0) return -1;
			i++;
		}
		temp = temp->next;
		i = 0;
	}
	return 1;
}

void test_EDF() {
	int i = 0;
	long l_factor;
	//printf("EDF started\n");
	for(i = 0; i < taskset_count; i++) {
		if(test_preutilisation(task_set[i]) == 1) {
			// Utilisation based test.
			printf("Utilisation based test\n");		
			if(test_utilisation(task_set[i], 0) <= 1.0) {
				s_count_edf[u_count_edf]++;
				printf("Task set %d EDF Schedulable\n", i);
			} else {
				printf("Task set %d EDF not Schedulable\n", i);
			}
		} else {
			// Density check.
			if(test_utilisation(task_set[i], 1) <= 1.0) {
				s_count_edf[u_count_edf]++;
				printf("Task set %d EDF Schedulable\n", i);
			} else {
				// Loading factor.
				printf("Loading factor test\n");
				l_factor = calculate_busyperiod(task_set[i]);
				if((test_load_factor(task_set[i], l_factor) == 1) && (l_factor > 0)) {
					s_count_edf[u_count_edf]++;
					printf("Task set %d EDF Schedulable with load factor: %ld\n", i, l_factor);
				} else {
					if(l_factor > 0)
						printf("Task set %d EDF not Schedulable with load factor: %ld\n", i, l_factor);
					else
						printf("Task set %d EDF not Schedulable load factor doesn't exists\n", i);	
				}
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
			sum += temp->wcet/(temp->period > temp->deadline ? temp->deadline:temp->period);
		temp = temp->next;
		count++;
		if(count == j) break;	
	}

	return sum;	
}

int test_rt(TASK first, int i) {
	int prevsum = 0, cursum = 0, count = 0, flag = 1;
	TASK temp = first;
	printf("RT analysis for job %d\n", i);	
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
			if(count >= (i - 1)) {
				cursum += temp->wcet;
				break;
			}	
		}
		//printf("\nP: %d  C: %d\n", prevsum, cursum);
		if(prevsum == cursum) { 
			if(cursum < (temp->period>temp->deadline?temp->deadline:temp->period) && (cursum > 0)) {
				printf("RT analysis: Schedulable RT for job %d is: %d\n", i, cursum);
			} else {
				flag = 0;
				if (cursum > 0)
					printf("RT analysis: Not Schedulable RT for job %d is: %d\n", i, cursum);
				else 
					printf("RT analysis: Not Schedulable RT for job doesn't exists\n");
			}
			break;
		} else {
			prevsum = cursum;
		}
	}
	return flag;
}

int rt_analysis(TASK first, int n, int check) {
	int i = 0, flag = 1;
	double res = 0.0;
	//printf("RT ana for taskset %d\n", n);	
	for(i = 1; i <= task_count[n]; i++) {
		res = test_rt_utilisation(first, i, check);

		if(res <= cal_utilisation_val(i)) {
			//s_count_rm[u_count_rm]++;
			printf("RT analysis: Schedulable U for RT with %d jobs: %lf\n", i, res);
		} else {
			flag = test_rt(first, i);
		}
	}
	
	return flag;
}

void test_RM() {
	int i = 0, response_t, flag = 0;
	double U = 0.0, N = 0.0;
	
	for(i = 0; i < taskset_count; i++) {
		if(test_preutilisation(task_set[i]) == 1) {
			// Utilisation based test for D = P.
			U = test_utilisation(task_set[i], 0);
			N =	cal_utilisation_val(task_count[i]);
			//printf("U: %lf  N: %lf\n", U, N);
			if(U <= N) {
				s_count_rm[u_count_rm]++;
				printf("RM Utilisation test: Task set %d is RM Scheduable\n", i);
			} else if(U > 1.0) 
				printf("RM Utilisation test: Task set %d is RM not Scheduable\n", i);
			else {
				// Response time analysis.
				task_set[i] = sort(task_set[i], 0);
				flag = rt_analysis(task_set[i], i, 0);
				if(flag == 1)s_count_rm[u_count_rm]++;			
			}
		} else {
			// Density check for D < P.
			U = test_utilisation(task_set[i], 1);
			N =	cal_utilisation_val(task_count[i]);
			if(U <= N) {
				s_count_rm[u_count_rm]++;
				printf("RM Utilisation test: Task set %d RM Scheduable\n", i);
			} else {
				// Response time analysis.
				task_set[i] = sort(task_set[i], 0);
				flag = rt_analysis(task_set[i], i, 1);
				if(flag == 1)s_count_rm[u_count_rm]++;
			}
		}
	}		
}

void test_DM() {
	int i = 0, response_t, flag = 0;
	double U = 0.0, N = 0.0;
	
	for(i = 0; i < taskset_count; i++) {
		if(test_preutilisation(task_set[i]) == 1) {
			// Utilisation based test for D = P.
			U = test_utilisation(task_set[i], 0);
			N =	cal_utilisation_val(task_count[i]);
			//printf("U: %lf  N: %lf\n", U, N);
			if(U <= N) {
				s_count_dm[u_count_dm]++;
				printf("DM Utilisation test: Task set %d is DM Scheduable\n", i);
			} else if(U > 1.0) 
				printf("DM Utilisation test: Task set %d is DM not Scheduable\n", i);
			else {
				// Response time analysis.
				task_set[i] = sort(task_set[i], 1);
				flag = rt_analysis(task_set[i], i, 0);
				if(flag == 1)s_count_dm[u_count_dm]++;			
			}
		} else {
			// Density check for D < P.
			U = test_utilisation(task_set[i], 1);
			N =	cal_utilisation_val(task_count[i]);
			if(U <= N) {
				s_count_dm[u_count_dm]++;
				printf("DM Utilisation test: Task set %d DM Scheduable\n", i);
			} else {
				// Response time analysis.
				task_set[i] = sort(task_set[i], 1);
				flag = rt_analysis(task_set[i], i, 1);
				if(flag == 1)s_count_dm[u_count_dm]++;
			}
		}
	}		
}

void cal_effective_utilisation(TASK first) {
	TASK cur = first, temp;
	double f = 0.0;
	int h_count = 0, i = 0, flag = 1;

	while(cur != NULL) {
		temp = first;
		while(temp != NULL) {
			if(temp == cur) {
				f += temp->wcet/(temp->period > temp->deadline ? temp->deadline:temp->period);
			} else {
				if(cur->priority > temp->priority) {
					if(temp->period <= cur->deadline) {
						f += temp->wcet/temp->period;
						h_count++;
					} else {
						f += temp->wcet/cur->period;
					}
				}
			}
			//printf("f: %lf\n", f);
			temp = temp->next;
		}
		i++;
		if(f <= cal_utilisation_val(h_count + 1)) {
			printf("FP effective utilisation f: %lf for job %d schedulable\n", f, i);
		} else {
			printf("FP effective utilisation f: %lf for job %d not schedulable\n", f, i);
			printf("Performing RT analysis\n");
			flag = test_rt(first, i);
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
	double sumU = U, nextsumU = 0.0;
	double b;
	int i = 0;
	
	for(i = 1; i < n; i++) {
		nextsumU = sumU * pow(drand48(), (double)1.0/((double)n - (double)i));
		a[i-1] = sumU - nextsumU;
		sumU = nextsumU;
		//printf("%lf\t", a[i-1]);
	}
	a[i-1] = sumU;
	//printf("%lf\n", a[i-1]);
}

double cal_period(int i, int t_count) {
	
	if(i < t_count/2) {
		return (1000 + rand() / (RAND_MAX/(10000 - 1000 + 1) + 1));
	} else {
		return (10000 + rand() / (RAND_MAX/(100000 - 10000 + 1) + 1));
	}
}

void taskset_with_utilisation(double u, int t_count, int d_range) {
	double t, c, d, c1, a[t_count];
	int i = 0, j = 0;
	
	printf("Tasks generation started...\n");
	for(j = 0; j < 10000; j++) {
		UUnifast(t_count, u, a);
		task_count[taskset_count] = t_count;
		for(i = 0; i < t_count; i++) {
			t = cal_period(i, t_count);
			c = a[i] * t;
			if(d_range == 1) {
				d = (rand() % (int)(t + 1 - c)) + c;
			} else {
				c1 = c + (t-c)/2.0;
				d = (rand() % (int)(t + 1 - c)) + c;
			}
			task_set[taskset_count] = insert_rear(task_set[taskset_count], c, d, t, i);
			//printf("U :%lf C :%lf \t D :%lf \t T :%lf\n", a[i], c, d, t);
		}
		taskset_count++;
		//printf("Gen TS: %d\n", taskset_count);
	}
	//printf("Tasks generation succeeded...\n");
}

void write_utilisation(int count[], char *algo, int c, int d) {
	int i = 0;
	float u = 0.05;
	freopen(OUTPUT_FILE, "a", stdout);
	if(d == 1)
		printf("\n\n*************** ALGORITHM %s D[C, T] ***************\n", algo);
	else
		printf("\n\n*************** ALGORITHM %s D[C+(T-C)/2, T] ***************\n", algo);

	for(i = 0; i < 10; i++) {
		printf("U = %f \t S = %d \t Percentage = %f\n", u, count[i], (count[i]/(c * 10000.0))*100.0);
		u += 0.1;
	}
	freopen("/dev/tty", "w", stdout);
}

void perform_test() {
	printf("Schedulability tests started...\n");
	freopen(EDF_FILE, "a", stdout);
	test_EDF();
	freopen(RM_FILE, "a", stdout);
	test_RM();
	freopen(DM_FILE, "a", stdout);
	test_DM();
	freopen("/dev/tty", "w", stdout);
	//printf("Schedulability tests concluded...\n");
}

void release_tasks() {
	int i = 0;
	//printf("clean up started..\n");
	memset(task_count, 0, MAX_TASKSET);
	taskset_count = 0;

	for(i = 0; i < MAX_TASKSET; i++) {
		task_set[i] = release_nodes(task_set[i]);
	}
	//printf("clean up completed..\n");
}

void reset_counters() {
	memset(s_count_edf, 0, sizeof(int) * 10);
	memset(s_count_rm, 0, sizeof(int) * 10);
	memset(s_count_dm, 0, sizeof(int) * 10);
}

void generate_taskset() {
	double u;
	int t, d, count = 0;
	for(d = 1; d < 3; d++) {
		for(u = 0.05; u <= 0.95; u += 0.1) {
			taskset_with_utilisation(u, TASKS_PER_SET, d);
			perform_test();
			u_count_edf++;
			u_count_rm++;
			u_count_dm++;
			release_tasks();
			//printf("U: %f\t C: %d\n", u, u_count_edf-1);
		}

		u_count_edf = 0;
		u_count_rm = 0;
		u_count_dm = 0;
		count ++;

		write_utilisation(s_count_edf, "EDF", count, d);
		write_utilisation(s_count_rm, "RM", count, d);
		write_utilisation(s_count_dm,"DM", count, d);
		reset_counters();
		count = 0;
	}
}

void remove_outputfiles() {
	remove(OUTPUT_FILE);
	remove(EDF_FILE);
	remove(RM_FILE);
	remove(DM_FILE);
}

int main() {
	int i, pri;
	char *line = NULL, c, *pch, s;
	size_t n = 0;
	double wcet = 0.0, deadline = 0.0, period = 0.0;
	system("clear");
	while(1) {
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
	}
	
	srand(time(NULL));
	srand48(time(NULL));

	printf("\n*************** ALGORITHM EDF ***************\n");
	test_EDF();
	printf("\n*************** ALGORITHM RM ***************\n");
	test_RM();
	printf("\n*************** ALGORITHM DM ***************\n");
	test_DM();
	printf("\n*************** ALGORITHM FP ***************\n");	
	test_FP();
	release_tasks();
	reset_counters();

	printf("\nDo you wish generate synthetic tasks?(y or n)\n");
	scanf(" %c", &c);
	if(c == 'y' || c == 'Y') {
		remove_outputfiles();
		generate_taskset();
		printf("\n Analysis completed\n");
		printf("Please check the file named output.txt in ../test directory\n");
	} else {
		printf("Terminating...\n");
	}
	return 0;
}
