#include<stdio.h>
#include<stdlib.h>
#include "../include/q.h"
#include<string.h>
#include<math.h>
#include<time.h>

#define MAX_TASKSET 10000
#define OUTPUT_FILE "../test/output.txt"
#define EDF_FILE "../test/EDF_log.txt"
#define RM_FILE "../test/RM_log.txt"
#define DM_FILE "../test/DM_log.txt"
#define TASKS_PER_SET 10

TASK task_set[MAX_TASKSET];
int task_count[MAX_TASKSET];
int taskset_count;
int s_count_edf[10];
int u_count_edf;

int s_count_rm[10];
int u_count_rm;
int s_count_dm[10];
int u_count_dm;
