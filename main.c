#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<time.h>

#define MAX_TASKS 20

typedef struct timespec tspec;

struct task_param {
	void *arg;
	int index;
	tspec period;
	int priority;
	tspec at;
	void (*body)(void *);
	pthread_mutex_t mux;
};

/* Prototypes */
void task_wait_for_activation();

/* Globals */
static int task_count = 0;
pthread_t _taskid[MAX_TASKS];
struct task_param _tp[MAX_TASKS];
static pthread_mutex_t _tp_mutex;
sem_t _tsem[MAX_TASKS];
tspec sys_start_time;
int task_policy = 0;   // FIFO has to set.
int task_protocol = 0;
static __thread int task_idx;


static void *task_std_body(void *arg) {
	struct task_param *tdes = (struct task_param *)arg;
	task_idx = tdes->index;
	(tdes->body)(tdes->arg);
	return 0;
}

int pmux_create_pi(pthread_mutex_t *m)
{
  pthread_mutexattr_t mta;
  pthread_mutexattr_init(&mta);
  pthread_mutexattr_setprotocol(&mta, PTHREAD_PRIO_INHERIT);

  int ret = pthread_mutex_init(m, &mta);

  pthread_mutexattr_destroy(&mta);
  return ret;
}

void system_init() {
	int i = 0;
	
	for (i = 0; i < MAX_TASKS; i++) {
		sem_init(&_tsem[i], 0, 0);
	}
	
	pmux_create_pi(&_tp_mutex);
}

tspec tspec_from(long tu)
{
    tspec t;
    long mm = tu % 1000;

    t.tv_sec = tu / 1000;
    t.tv_nsec = mm * 1000000;

    return t;
}

void set_taskparam(void (*task)(void *), char *task_spec, int i) {
	// TODO
	_tp[i].arg = task_spec;
	_tp[i].index = i;
	_tp[i].period = tspec_from(500);
	_tp[i].priority = 10;
	_tp[i].body = task;
	pmux_create_pi(&_tp[i].mux);	
}

static void release_tp(int i) {
	pthread_mutex_lock(&_tp_mutex);
	pthread_mutex_destroy(&_tp[i].mux);
	pthread_mutex_unlock(&_tp_mutex);
}

int create_task(void (*task)(void *), char *task_spec) {
	pthread_attr_t myatt;
	struct sched_param mypar;
	int i = task_count++;

	set_taskparam(task, task_spec, i);
	pthread_attr_init(&myatt);
	pthread_attr_setschedpolicy(&myatt, task_policy);
	mypar.sched_priority = _tp[i].priority;
	pthread_attr_setschedparam(&myatt, &mypar);
	
	int ret = pthread_create(&_taskid[i], &myatt, task_std_body, (void *)(&_tp[i]));
//	pthread_attr_destory(&myatt);
	
	
	if (ret == 0) {
		return i;
	} else {
		release_tp(i);
		return -1;
	}
}

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

void task_wait_for_period() {
	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(_tp[task_idx].at), NULL);
	pthread_mutex_lock(&_tp[task_idx].mux);
	_tp[task_idx].at = tspec_add(&(_tp[task_idx].at), &(_tp[task_idx].period));
	pthread_mutex_unlock(&_tp[task_idx].mux);
}

void task_wait_for_activation() {
	sem_wait(&_tsem[task_idx]);
}

void task_activation(int i) {
	pthread_mutex_lock(&_tp[i].mux);
	clock_gettime(CLOCK_MONOTONIC, &t);
	_tp[i].at = tspec_add(&t, &_tp[i].period);
	sem_post(&_tsem[i]);
	pthread_mutex_unlock(&_tp[i].mux);
}

void task(void *arg) {
	int i = 0;
	printf("created\n\n");
	task_wait_for_activation();
	while(1) {
		for (i = 0; i < 1000; i++) {
			printf("%d\t", i);
		}
		task_wait_for_period();
	}
}

int main(int argc, char **argv) {
	FILE *fp;
	char *line = NULL;
	size_t n = 0;
	size_t len;

	fp = fopen(argv[1], "r");

	if (fp == NULL)
    		return 1;

	while ((len = getline(&line, &n, fp)) != -1) {
    	printf("Retrieved line of length %zd:\n", len);
    	printf("%s\n", line);
			
		/*char *pch = strtok(line, " ");
  		
		while (pch != NULL) {
    			printf ("%s\n", pch);
    			pch = strtok(NULL, " ");
 		}*/
		printf("\n%d\n", create_task(task, line));
	}

	//if (line)
    	//	free(line);
	int i = 0;
	for (i = 0; i < task_count; i++) task_activation(i);
	while(1);

	return 0;
}
