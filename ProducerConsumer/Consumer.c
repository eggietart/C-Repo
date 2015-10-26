#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/sem.h>
//#include "semaphores.h"

#include <sys/shm.h>

#define TEXT_SIZE 128;


int main()
{
	// Initializing semaphores and shared memory....		
	init_semaphores();
	init_sharedMemory();

	sem_wait(sem_N_id);
	sem_wait(sem_S_id);

	sem_signal(sem_S_id);
	sem_signal(sem_E_id);

	// Cleaning up semaphores and shared memory....		
	remove_semaphores();
	del_sharedMemory();

    exit(EXIT_SUCCESS);
}