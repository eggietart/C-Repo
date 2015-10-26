#include "functions.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/sem.h>

static int set_semvalue(int sem_id, int value);
static void del_semvalue(int sem_id);

// #if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
//     /* union semun is defined by including <sys/sem.h> */	
// #else
//     /* according to X/OPEN we have to define it ourselves */
//     union semun {
//         int val;                    /* value for SETVAL */
//         struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
//         unsigned short int *array;  /* array for GETALL, SETALL */
//         struct seminfo *__buf;      /* buffer for IPC_INFO */
//     };
// #endif

int init_sem(key_t key, int value)
{
    // Creating Semaphore S...
    int sem_id;
    sem_id = semget((key_t)key, 1, 0666| IPC_CREAT);

    if(!set_semvalue(sem_id, value)) {
        fprintf(stderr, "Semaphore S could not be initialized.\n");
        exit(EXIT_FAILURE);
    }
    printf("Semaphore created -> %d.\n", sem_id);

    return sem_id;
}

// Function to decrement the inputted semaphore...
int sem_wait(int sem_id)
{    
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "Semaphore wait call failed.\n");
        return(0);
    }
    return(1);
}

// Function to increment the inputted semaphore...
int sem_signal(int sem_id)
{
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = 0;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "Semaphore signal call failed.\n");
        return(0);
    }
    return(1);
}

int init_semaphores(void)
{
    sem_S_id = init_sem((key_t)8000, 1);
    sem_N_id = init_sem((key_t)8001, 0);
    sem_E_id = init_sem((key_t)8002, nBuffers);

    return 1;
}

int remove_semaphores(void)
{
    del_semvalue(sem_S_id);
    del_semvalue(sem_N_id);
    del_semvalue(sem_E_id);

    return 1;
}

// Function for initializing the semaphores...
static int set_semvalue(int sem_id, int value)
{
    union semun sem_union;

    sem_union.val = value;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) return(0);
    return(1);
}

// Function for releasing the semaphores...
static void del_semvalue(int sem_id)
{
    union semun sem_union;
    
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore.\n");
}