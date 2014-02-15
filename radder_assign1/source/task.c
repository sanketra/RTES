#include "../include/task.h"

/* Function Prototypes */
void task_wait_for_activation();
void periodic_task();
void aperiodic_task();
static void release_tp(int);

/* Task exit handler. 
   Releases allocated memory in each task */
static void task_exit_handler() {
	// Clean up
	//printf("Clean up of task %d\n", task_idx);
	release_nodes(_tp[task_idx].arg);
	release_tp(task_idx);
}

/* Common task body initially exeuted by all tasks
   before calling respective task function.*/
static void *task_std_body(void *arg) {
	struct task_param *tdes = (struct task_param *)arg;
	task_idx = tdes->index;
	pthread_cleanup_push(task_exit_handler, 0);
	(tdes->body)();
	pthread_cleanup_pop(1);
	return 0;
}

/* Creates a mutex variable */
int pmux_create_pi(pthread_mutex_t *m)
{
	pthread_mutexattr_t mta;
	pthread_mutexattr_init(&mta);
	pthread_mutexattr_setprotocol(&mta, PTHREAD_PRIO_INHERIT);
	int ret = pthread_mutex_init(m, &mta);
	pthread_mutexattr_destroy(&mta);
	return ret;
}

/* Initialises all semaphores and creates mutex used in the program */
void system_init() {
	int i = 0;
	
	for (i = 0; i < MAX_TASKS; i++) {
		sem_init(&_tsem[i], 0, 0);
	}

	for (i = 0; i < MAX_EVENTS; i++) {
		sem_init(&_event_sem[i], 0, 0);
	}

	for (i = 0; i < MAX_MUTEXES; i++) {
		pmux_create_pi(&task_mut[i]);
	}
	
	pmux_create_pi(&_tp_mutex);
}

/* Returns a tspec instance for a period. */
tspec tspec_from(long tu)
{
    tspec t;
    long mm = tu % 1000;

    t.tv_sec = tu / 1000;
    t.tv_nsec = mm * 1000000;

    return t;
}

/* Initialses task params for each task */
void set_taskparam(char *task_spec, int i) {		
	char *pch = strtok(task_spec, " ");
	int val = 0;
	char c;

  	if (strcmp(pch, "P") == 0) {
		_tp[i].body = periodic_task;
		_tp[i].period = tspec_from(atoi(strtok(NULL, " ")));
		
	} else {
		_tp[i].body = aperiodic_task;
		_tp[i].event = atoi(strtok(NULL, " "));
	}
	
	_tp[i].priority = atoi(strtok(NULL, " "));
		
	while ((pch = strtok(NULL, " ")) != NULL) {
		c = pch[0];
		if (c == 'L') {
			//printf("Lock variable: %s\n", pch);								
			sscanf(pch, "L(%d)", &val);
			_tp[i].arg = insert_rear(_tp[i].arg, val, 'L');
		} else if (c == 'U') {
			//printf("Unlock variable: %s\n", pch);								
			sscanf(pch, "U(%d)", &val);
			_tp[i].arg = insert_rear(_tp[i].arg, val, 'U');
		} else {
			//printf("Iteration variable: %s\n", pch);								
			sscanf(pch, "%d", &val);
			_tp[i].arg = insert_rear(_tp[i].arg, val, 'I');
		} 
	} 

	_tp[i].index = i;
	pmux_create_pi(&_tp[i].mux);	
}

/* Releases mutex that exists with each task */
static void release_tp(int i) {
	pthread_mutex_lock(&_tp_mutex);
	pthread_mutex_destroy(&_tp[i].mux);
	pthread_mutex_unlock(&_tp_mutex);
}

/* Parses the input from the file
   Creates periodic and aperiodic task based on the task spec
   Assigns the priority for each task */
int create_task(char *task_spec) {
	pthread_attr_t myatt;
	struct sched_param mypar;
	int i = task_count++;

	set_taskparam(task_spec, i);
	pthread_attr_init(&myatt);
	pthread_attr_setschedpolicy(&myatt, task_policy);
	mypar.sched_priority = _tp[i].priority;
	pthread_attr_setschedparam(&myatt, &mypar);
	
	int ret = pthread_create(&_taskid[i], &myatt, task_std_body, (void *)(&_tp[i]));
	//pthread_attr_destory(&myatt);
	
	
	if (ret == 0) {
		return i;
	} else {
		release_tp(i);
		return -1;
	}
}

/* Adds to tspec instances */
tspec tspec_add(const tspec *a, const tspec *b) {
	tspec s;
	s.tv_nsec = a->tv_nsec + b->tv_nsec;
	s.tv_sec = a->tv_sec + b->tv_sec;
	
	while(s.tv_nsec >= 1000000000) {
		s.tv_nsec = s.tv_nsec - 1000000000;
		s.tv_sec+= 1;
	}
	
	return s;
}

/* Blocks the calling task until its period elapses */
void task_wait_for_period() {
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(_tp[task_idx].at), NULL);
	pthread_mutex_lock(&_tp[task_idx].mux);
	_tp[task_idx].at = tspec_add(&(_tp[task_idx].at), &(_tp[task_idx].period));
	pthread_mutex_unlock(&_tp[task_idx].mux);
}

/* Blocks the calling task until a activation signal is received */
void task_wait_for_activation() {
	sem_wait(&_tsem[task_idx]);
}

/* Activates the task i */
void task_activation(int i) {
	tspec t;
	pthread_mutex_lock(&_tp[i].mux);
	clock_gettime(CLOCK_MONOTONIC, &t);
	_tp[i].at = tspec_add(&t, &_tp[i].period);
	sem_post(&_tsem[i]);
	pthread_mutex_unlock(&_tp[i].mux);
}

/* Task work body which iterates for x times */
void task_iterate(int x) {
	int i = 0, j = 0;	
	for (i = 0; i < x; i++) {
		j += i;
	}
}

/* Periodic task function */
void periodic_task() {
	//printf("P task created\n\n");
	NODE first;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	task_wait_for_activation();
	pthread_barrier_wait(&barrier);

	while(1) {
		//printf("P task exe...\n");
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		first = _tp[task_idx].arg;
		while (first != NULL) {
			if (first->type == 'L') {
				//printf("Lock detected\n");
				pthread_mutex_lock(&task_mut[first->val]);
			} else if(first->type == 'U') {
				//printf("Unlock detected\n");
				pthread_mutex_unlock(&task_mut[first->val]);
			} else {
				//printf("Periodic Iteration detected\n");
				task_iterate(first->val);
			}
			first = first->next;			
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		task_wait_for_period();
	}
}

/* Aperiodic task function */
void aperiodic_task() {		
	//printf("A P task created count: %d\n", ++_twait_count[_tp[task_idx].event]);
	NODE first;	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	task_wait_for_activation();
	pthread_barrier_wait(&barrier);

	while(1) {
		sem_wait(&_event_sem[_tp[task_idx].event]);	
		//printf("A task exe...\n");
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		first = _tp[task_idx].arg;

		while (first != NULL) {
			if (first->type == 'L') {
				//printf("Lock detected\n");
				pthread_mutex_lock(&task_mut[first->val]);
			} else if(first->type == 'U') {
				//printf("Unlock detected\n");
				pthread_mutex_unlock(&task_mut[first->val]);
			} else {
				//printf("Aperiodic Iteration detected\n");
				task_iterate(first->val);
			}
			first = first->next;			
		}
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
}

/*Key press handler thread*/
static void* wait_for_keypress() {
	int fd = open(KBD_PATH, O_RDONLY);
    struct input_event ev;
	int i = 0;
	//printf("New thread listening to key-press event...\n");
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while(1)
    {     
        read(fd, &ev, sizeof(struct input_event));
        if(ev.type == 1) {
            if(ev.value == 0 && (ev.code > 0 && ev.code <= 11)) {
                //printf("Key-Pressed : [key %i]\n ", (ev.code - 1) % 10);
				for(i = 0; i < _twait_count[(ev.code - 1) % 10]; i++) {
					//printf("Waking...%d \n", i);
					sem_post(&_event_sem[(ev.code - 1) % 10]);
				}
			}
		}
    }
}

/* Main reads the input file
   Activates the tasks
   Sleeps till the total execution time elapses
   Signals other threads to terminate */
int main(int argc, char **argv) {
	FILE *fp;
	char *line = NULL;
	size_t n = 0;
	size_t len;
	system_init();
	fp = fopen(argv[1], "r");

	if (fp == NULL)
    		return 1;
	
	getline(&line, &n, fp);
	char *pch = strtok(line, " ");
	int time = atoi(strtok(NULL, " "));

	while ((len = getline(&line, &n, fp)) != -1) {
    	//printf("Retrieved line of length %zd:\n", len);
    	//printf("%s\n", line);
		create_task(line);
	}

	//printf("*************task count for barrier %d **************\n", task_count+1);	
	pthread_barrier_init(&barrier, NULL, (task_count + 1));

	pthread_attr_t myatt;
	struct sched_param mypar;
	pthread_attr_init(&myatt);
	pthread_attr_setschedpolicy(&myatt, task_policy);
	mypar.sched_priority = EVENT_THREAD_PRIORITY;
	pthread_attr_setschedparam(&myatt, &mypar);
	pthread_create(&event_thread, &myatt, wait_for_keypress, NULL);
	
	int i = 0;
	for (i = 0; i < task_count; i++) task_activation(i);
	pthread_barrier_wait(&barrier);

	tspec sys_time;
	clock_gettime(CLOCK_MONOTONIC, &sys_time);
	tspec total = tspec_from(time);
	tspec end_time = tspec_add(&sys_time, &total);
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &end_time, NULL);
	
	//printf("\n\n\n******Main thread woke up*****\n\n\n");
	for(i = 0; i < task_count; i++) {
    	pthread_cancel(_taskid[i]);
	}
    for(i = 0; i < task_count; i++) {
    	pthread_join(_taskid[i], NULL);
	}
	pthread_cancel(event_thread);
	pthread_join(event_thread, NULL);
 
	//printf("\n\n\n*****Main thread exiting*****\n\n\n");
	return 0;
}
