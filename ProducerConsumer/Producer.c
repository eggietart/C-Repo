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
static int init_all(void);
static int clean_all(void);

static FILE *in;

static int sem_S_id;	// Semaphore S - Buffer
static int sem_N_id;	// Semaphore N - Produced Items
static int sem_E_id;	// Semaphore E - Empty Item

static int shmid;

static struct buf_element *item;

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
	
	init_all();

	// Reading input file, 128 bytes at a time...
	while (fgets(str, 128, in) != NULL) {

		writeItem(str, strlen(str));
		totalBytesRead = totalBytesRead + strlen(str);
		usleep(100);
	}

	writeItem("EOF\0", totalBytesRead);

	sleep(20);
	clean_all();

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

static int init_all(void)
{
	// Initialize all semaphores...
    sem_S_id = init_sem((key_t)8000, 1);
    sem_N_id = init_sem((key_t)8001, 0);
    sem_E_id = init_sem((key_t)8002, nBuffers);

	// Initialize shared memory...
	shmid = allocate_sharedMemory(item);
	item = attach_sharedMemory(shmid);

	return 1;
}

static int clean_all()
{
	// Delete semaphores...
    del_semvalue(sem_S_id);
    del_semvalue(sem_N_id);
    del_semvalue(sem_E_id);

	// Delete shared memory...
	del_sharedMemory(item, shmid);

	// Close input file...
	fclose(in);

	return 1;
}