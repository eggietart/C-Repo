#include "functions.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/shm.h>

int allocate_sharedMemory(struct buf_element* item) {

	// Allocating shared memory for the finite circular buffer
	int shmid;

	shmid = shmget((key_t)1234, sizeof(*item) * nBuffers, 0666 | IPC_CREAT);

	if (shmid == 1) {
		fprintf(stderr, "Shared memory could not be allocated.\n");
		exit(EXIT_FAILURE);
	}
	
	return shmid;
}

struct buf_element* attach_sharedMemory(int shmid) {
	
	// Attach shared memory to this process' address space
	struct buf_element *item;

	item = (struct buf_element*) shmat(shmid, (void *)0, 0);

	if (item == (void *)-1) {
		fprintf(stderr, "Shared memory could not be allocated.\n");
		exit(EXIT_FAILURE);
	}

	printf("Memory attached at %X\n", (int)item);
	
	return item;
}

int del_sharedMemory(struct buf_element* item, int shmid) {

	// Release shared memory and delete it
    if (shmdt(item) == -1) {
        fprintf(stderr, "Shared memory could not be released.\n");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Shared memory could not be deleted.\n");
        exit(EXIT_FAILURE);
    }

    return 1;
}