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

// Shared Memory Functions
int init_sharedMemory(void);
int del_sharedMemory(void);

// Variables...
int sem_S_id;	// Semaphore S - Buffer
int sem_N_id;	// Semaphore N - Produced Items
int sem_E_id;	// Semaphore E - Empty Item

int shmid;		// Shared Memory Id

void *objShm;

// Semaphores Functions
int init_semaphores(void);
int remove_semaphores(void);

int sem_wait(int sem_id);
int sem_signal(int sem_id);