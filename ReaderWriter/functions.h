#include <stdio.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>

int allocateSharedMemory(int n);
void* mapSharedMemory(int id);
int createSemaphores(int n, short* vals);
void deleteSemaphores(int id);
void lockSemaphore(int id, int i);
void unlockSemaphore(int id, int i);
