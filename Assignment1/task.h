#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<time.h>
#include<linux/input.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include "q.h"


#define MAX_TASKS 20
#define MAX_EVENTS 10
#define EVENT_THREAD_PRIORITY 99
#define MAX_MUTEXES 10
#define KBD_PATH "/dev/input/by-path/platform-i8042-serio-0-event-kbd"

typedef struct timespec tspec;

struct task_param {
	NODE arg;
	int index;
	tspec period;
	int priority;
	tspec at;
	void (*body)();
	int event;
	pthread_mutex_t mux;
};

/* Global Declarations */
static int task_count = 0;
pthread_t _taskid[MAX_TASKS];
struct task_param _tp[MAX_TASKS];
static pthread_mutex_t _tp_mutex;
sem_t _tsem[MAX_TASKS];
tspec sys_start_time;
int task_policy = 0;   // FIFO has to set.
int task_protocol = 0;
static __thread int task_idx;
sem_t _event_sem[MAX_EVENTS];
int _twait_count[MAX_EVENTS];
pthread_t event_thread;
pthread_mutex_t task_mut[MAX_MUTEXES];
pthread_barrier_t barrier;

