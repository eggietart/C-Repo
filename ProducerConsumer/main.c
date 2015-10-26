// Possibly the Producer class...

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/sem.h>
#include <sys/shm.h>

#include "functions.h"

// Internal Functions
int writeItem(void);

int main()
{
	// Variable initialization
	nBuffers = 100;
	in_item = 0;
	out_item = 0;

	
	// Initializing semaphores and shared memory....		
	init_semaphores();
	init_sharedMemory();

	int i;
	for (i = 0; i < 10; i++) {
		sem_wait(sem_E_id);
		sem_wait(sem_S_id);
		writeItem();
		sem_signal(sem_S_id);
		sem_signal(sem_N_id);
		//sleep(10);
	}
	sleep(20);
	// Cleaning up semaphores and shared memory....		
	remove_semaphores();
	del_sharedMemory();

    exit(EXIT_SUCCESS);
}

int writeItem(void)
{
	strcpy(item[in_item].text, "some text\n");
	item[in_item].byte_count = 10;
	in_item++;
	
	return 1;
}
