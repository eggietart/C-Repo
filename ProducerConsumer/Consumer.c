#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/sem.h>
#include <sys/shm.h>

#include "functions.h"

// Internal Functions
static int readItem(void);
static int init_all(void);
static int clean_all(void);

int c_sem_S_id, c_sem_N_id, c_sem_E_id;
int c_shmid;

struct buf_element *c_item;

int main()
{
	init_all();

	int i;
	for (i = 0; i < 10; i++) {
		sem_wait(c_sem_N_id);
		sem_wait(c_sem_S_id);
		readItem();
		sem_signal(c_sem_S_id);
		sem_signal(c_sem_E_id);
		sleep(1);
	}

	clean_all();

    exit(EXIT_SUCCESS);
}

static int readItem(void)
{
	struct buf_element temp;
	temp = c_item[out_item];

	printf("%s", temp.text);
	printf("Length: %d\n", 	temp.byte_count);

	out_item++;

	return 1;
}

static int init_all(void)
{
	// Initializing semaphores and shared memory....		
    c_sem_S_id = semget((key_t)8000, 1, 0666 | IPC_CREAT);

    if(c_sem_S_id == -1) {
        fprintf(stderr, "Semaphore S could not be initialized.\n");
        exit(EXIT_FAILURE);
    }

    c_sem_N_id = semget((key_t)8001, 1, 0666 | IPC_CREAT);

    if(c_sem_N_id == -1) {
        fprintf(stderr, "Semaphore N could not be initialized.\n");
        exit(EXIT_FAILURE);
    }

    c_sem_E_id = semget((key_t)8002, 1, 0666 | IPC_CREAT);

    if(c_sem_E_id == -1) {
        fprintf(stderr, "Semaphore E could not be initialized.\n");
        exit(EXIT_FAILURE);
    }

    c_shmid = shmget((key_t)1234, sizeof(*item) * nBuffers, 0666 | IPC_CREAT);

	if (c_shmid == 1) {
		fprintf(stderr, "Shared memory could not be allocated.\n");
		exit(EXIT_FAILURE);
	}

	c_item = (struct buf_element*) shmat(c_shmid, (void *)0, 0);

	if (c_item == (void *)-1) {
		fprintf(stderr, "Shared memory could not be allocated.\n");
		exit(EXIT_FAILURE);
	}

	printf("Memory attached at %X\n", (int)c_item);

	return 1;
}

static int clean_all(void)
{
	// Cleaning up shared memory....		
	shmdt(c_item);

	return 1;
}