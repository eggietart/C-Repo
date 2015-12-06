// Common functions, libraries and initializations
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/sem.h>
#include <sys/shm.h>

#define TEXT_SIZE 128;

struct buf_element {
	char 	text[128];
	int		byte_count;
};

// Process Information
struct task_struct {
	int pid;				// Process ID --> manually generated
	int static_prio;		// Static Priority --> 1-130
	int prio;				// Dynamic Priority
	int execution_time;		// Execution Time --> in s
	int time_slice;			// Time Slice --> in ms
	int accu_time_slice;	// Accumulated Time Slice --> in ms
	int last_cpu;			// Last CPU that the process last ran --> thread id
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

// Producer/Consumer Functions
int writeItem(char text[], int count, FILE *in);
int readItem(FILE* out);

// Cleaning...
int init_all(void);
int clean_all(void);