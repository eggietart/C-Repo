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

static int c_sem_S_id;
static int c_sem_N_id;
static int c_sem_E_id;

static int c_shmid;

static struct buf_element *c_item;

static FILE *out;

int main()
{
	int bytesRead, totalBytesRead;

	bytesRead = 0;
	totalBytesRead = 0;
	out = fopen("outputfile.txt", "w");
	
	init_all();

	while (bytesRead <= 128) {

		bytesRead = readItem();
		
		if (bytesRead <= 128) 
			totalBytesRead = totalBytesRead + bytesRead;
		//sleep(1);
	}

	if (bytesRead == totalBytesRead)
		printf("It matches! Total characters: %d\n", totalBytesRead);
	else
		printf("Total bytes read does not match the total bytes written.\nRead: %d\nWritten: %d\n", totalBytesRead, bytesRead);

	clean_all();

    exit(EXIT_SUCCESS);
}

static int readItem(void)
{
	struct buf_element temp;

	sem_wait(c_sem_N_id);
	//sem_wait(c_sem_S_id);

	temp = c_item[out_item];

	if (strcmp(temp.text, "EOF"))
		fputs(temp.text, out);
	
	if (out_item == 99)
		out_item = 0;
	else
		out_item++;

	//sem_signal(c_sem_S_id);
	sem_signal(c_sem_E_id);

	return temp.byte_count;
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

    c_shmid = allocate_sharedMemory(c_item);
	c_item = attach_sharedMemory(c_shmid);

	return 1;
}

static int clean_all(void)
{
	// Cleaning up shared memory....		
	shmdt(c_item);

	// Close file descriptor...
	fclose(out);

	return 1;
}