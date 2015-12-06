// Common functions, libraries and initializations
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/sem.h>
#include <sys/shm.h>

#define TEXT_SIZE 128;

struct buf_element {
	char 	text[128];
	int		byte_count;
};

// Process Information
struct task_struct {
	int pid;				// Process ID
	int static_prio;		// Static Priority
	int prio;				// Dynamic Priority
	int execution_time;		// Execution Time
	int time_slice;			// Time Slice
	int accu_time_slice;	// Accumulated Time Slice
	int last_cpu;			// Last CPU that the process last ran
};

// Variables...
int nBuffers;
int in_item;
int out_item;
int c_totalBytesRead;
int thread_finished;
int shmid;

struct buf_element item[100];

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