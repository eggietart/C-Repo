// Possibly the Producer class...

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/sem.h>
#include <sys/shm.h>

#include "functions.h"

void append(void);
/*
#define TEXT_SIZE 128;

// Functions...
static int set_semvalue(int sem_id, int value);
static void del_semvalue(int sem_id);
static int sem_wait(int sem_id);
static int sem_signal(int sem_id);

static int init_semphores(void);
static int init_sharedMemory(void);
static int del_sharedMemory(void);

static int sem_S_id;	// Semaphore S - Buffer
static int sem_N_id;	// Semaphore N - Produced Items
static int sem_E_id;	// Semaphore E - Empty Item

static int shmid;		// Shared Memory Id

static void *shared_memory = (void *)0;

struct buf_element {
	char 	text[128];
	int		byte_count;
};
*/

int main()
{
	// Initializing semaphores and shared memory....		
	init_semaphores();
	init_sharedMemory();

	sem_wait(sem_E_id);
	sem_wait(sem_S_id);

	sem_signal(sem_S_id);
	sem_signal(sem_N_id);

	// Cleaning up semaphores and shared memory....		
	remove_semaphores();
	del_sharedMemory();

    exit(EXIT_SUCCESS);
}
