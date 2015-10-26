#include "functions.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/shm.h>

int init_sharedMemory(void) {

	// Allocating shared memory for the finite circular buffer
	shmid = shmget((key_t)1234, sizeof(struct buf_element) * 100, 0666 | IPC_CREAT);

	if (shmid == 1) {
		fprintf(stderr, "Shared memory could not be allocated.\n");
		exit(EXIT_FAILURE);
	}
	
	// Attach shared memory to this process' address space
	objShm = (char*) shmat(shmid, (void *)0, 0);

	if (objShm == (void *)-1) {
		fprintf(stderr, "Shared memory could not be allocated.\n");
		exit(EXIT_FAILURE);
	}

	printf("Memory attached at %X\n", (int)objShm);
	
	return 1;
}

int del_sharedMemory(void) {

	// Release shared memory and delete it
    if (shmdt(objShm) == -1) {
        fprintf(stderr, "Shared memory could not be released.\n");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Shared memory could not be deleted.\n");
        exit(EXIT_FAILURE);
    }

    return 1;
}