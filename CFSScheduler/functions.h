// Common functions, libraries and initializations
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include <sys/sem.h>
#include <sys/shm.h>

#define TEXT_SIZE 128;

struct buf_element {
	char 	text[128];
	int		byte_count;
};

// Process Information
struct task_struct {
	int pid;					// Process ID --> manually generated
	int sched_type;				// Scheduling Type --> 1: FIFO, 2: RR, 3: NORMAL
	int static_prio;			// Static Priority --> 1-130
	int prio;					// Dynamic Priority
	int execution_time;			// Execution Time --> in ms
	int time_slice;				// Time Slice --> in ms
	int accu_time_slice;		// Accumulated Time Slice --> in ms
	int accu_sleep_time;		// Accumulated Sleep Time -> in ms
	int sleep_avg;				// Sleep average -> in ms
	int last_cpu;				// Last CPU that the process last ran --> thread id
	struct timeval time_slept;	// The time when the process went to sleep
};

struct node {
	pthread_t thread_id;
	struct task_struct task_info;
	struct node *next_node;
};

// Variables...
int nBuffers;
int in_item;
int out_item;
int c_totalBytesRead;
int thread_finished;

struct buf_element item[100];
struct node *root[4];			// 4 Pointers to Run Queues --> 1/CPU

// Semaphores...
int sem_S_id;	// Semaphore S - Buffer
int sem_N_id;	// Semaphore N - Produced Items
int sem_E_id;	// Semaphore E - Empty Item

// Semaphores Functions
int init_sem(key_t key, int value);
void del_semvalue(int sem_id);

int sem_wait(int sem_id);
int sem_signal(int sem_id);

// Threaded Functions
void *consumer_function(void *arg);
void *producer_function(void *arg);
void *process_balancer_function(void *arg);

// Scheduling Functions
int calculate_quantum(int static_p);
int calculate_priority(int static_p, int sleep_average);
int calculate_sleep_time(struct timeval sleep_t);
int calculate_sleep_avg(int sleep_average, int time_slept, int qt);

// Cleaning...
int init_all(void);
int clean_all(void);