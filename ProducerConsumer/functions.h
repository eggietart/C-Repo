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

// Variables...
int nBuffers;
int in_item;
int out_item;

// Shared Memory Functions
int allocate_sharedMemory(struct buf_element* item);
struct buf_element* attach_sharedMemory(int shmid);
int del_sharedMemory(struct buf_element* item, int shmid);

// Semaphores Functions
int init_sem(key_t key, int value);
void del_semvalue(int sem_id);

int sem_wait(int sem_id);
int sem_signal(int sem_id);