// Possibly the Producer class...

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/sem.h>
#include <sys/shm.h>

#include "functions.h"

// Internal Functions
static int writeItem(char text[], int count);

static FILE *in;

int main()
{
	// Variable initialization
	char str[128];
	int totalBytesRead;

	totalBytesRead = 0;
	nBuffers = 100;
	in_item = 0;
	out_item = 0;

	// Opening input file...
	in = fopen("inputfile.txt","r");

	if (in == NULL) {
		fprintf(stderr, "Failed to open inputfile.txt.\n");
		exit(EXIT_FAILURE);
	}
	
	// Initializing semaphores and shared memory....		
	init_semaphores();
	init_sharedMemory();

	// Reading input file, 128 bytes at a time...
	while (fgets(str, 128, in) != NULL) {

		writeItem(str, strlen(str));
		totalBytesRead = totalBytesRead + strlen(str);
		usleep(100);
	}

	writeItem("EOF\0", totalBytesRead);

	sleep(20);

	// Cleaning up semaphores and shared memory....		
	remove_semaphores();
	del_sharedMemory();
	fclose(in);

    exit(EXIT_SUCCESS);
}

static int writeItem(char text[], int count)
{
	sem_wait(sem_E_id);
	//sem_wait(sem_S_id);
	
	strcpy(item[in_item].text, text);
	item[in_item].byte_count = count;
	
	if (in_item == 99)
		in_item = 0;
	else
		in_item++;

	//sem_signal(sem_S_id);
	sem_signal(sem_N_id);
	
	return 1;
}
