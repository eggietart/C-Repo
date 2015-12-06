#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "functions.h"

int main() {
    int res, j;
    pthread_t a_thread;
    void *thread_result;
    pthread_attr_t thread_attr;

    int max_priority;
    int min_priority;
    struct sched_param scheduling_value;

    in_item = 0;
    out_item = 0;
    thread_finished = 0;
    c_totalBytesRead = 0;
    nBuffers = 100;

    // Initialize all semaphores...
    sem_S_id = init_sem((key_t)8000, 1);
    sem_N_id = init_sem((key_t)8001, 0);
    sem_E_id = init_sem((key_t)8002, 100);

    res = pthread_attr_init(&thread_attr);
    if (res != 0) {
        perror("Attribute creation failed");
        exit(EXIT_FAILURE);
    }

    res = pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    if (res != 0) {
        perror("Setting schedpolicy failed");
        exit(EXIT_FAILURE);
    }

    res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (res != 0) {
        perror("Setting detached attribute failed");
        exit(EXIT_FAILURE);
    }

    // Creating Consumer threads...
    for (j = 1; j <= 4; j++) {
        
        res = pthread_create(&a_thread, &thread_attr, consumer_function, (void *)&j);
        if (res != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    // Creating Producer thread...
    res = pthread_create(&a_thread, &thread_attr, producer_function, (void *)"1");
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    max_priority = sched_get_priority_max(SCHED_RR);
    min_priority = sched_get_priority_min(SCHED_RR);
    scheduling_value.sched_priority = min_priority;
    
    res = pthread_attr_setschedparam(&thread_attr, &scheduling_value);
    if (res != 0) {
        perror("Setting schedpolicy failed");
        exit(EXIT_FAILURE);
    }
    
    (void)pthread_attr_destroy(&thread_attr);
    while(!thread_finished) {
        sleep(1);
    }

    // Delete semaphores...
    del_semvalue(sem_S_id);
    del_semvalue(sem_N_id);
    del_semvalue(sem_E_id);

    exit(EXIT_SUCCESS);
}

void *consumer_function(void *arg) {
    printf("This is a consumer thread. Argument was %u\n", pthread_self());
    sleep(4);

    int bytesRead;
    
    char str_pid[16];
    char out_name[256];

    FILE *out;
    
    c_totalBytesRead = 0;
    bytesRead = 0;

    // Individual output files are created for each consumer alive
    sprintf(str_pid, "%u", pthread_self());

    strcpy(out_name, "outputfile");
    strcat(out_name, "-");
    strcat(out_name, str_pid);
    strcat(out_name, ".txt");

    out = fopen(out_name, "w");

    while (bytesRead <= 128) {

        bytesRead = readItem(out);
        if (bytesRead <= 128) 
            c_totalBytesRead = c_totalBytesRead + bytesRead;
        //usleep(500000);
    }

    if (bytesRead == c_totalBytesRead)
        printf("It matches! Total characters: %d\n", c_totalBytesRead);
    else
        printf("Total bytes read does not match the total bytes written.\nRead: %d\nWritten: %d\n", c_totalBytesRead, bytesRead);

    fclose(out);
    //thread_finished = 1;
    pthread_exit(NULL);
}

void *producer_function(void *arg) {
    printf("This is a producer thread. Argument was %u\n", pthread_self());
    sleep(4);
    // Variable initialization
    int p_totalBytesRead;
    char str[128];
    p_totalBytesRead = 0;

    // Opening input file...
    FILE *in;
    in = fopen("inputfile.txt","r");

    if (in == NULL) {
        fprintf(stderr, "Failed to open inputfile.txt.\n");
        exit(EXIT_FAILURE);
    }

    // Reading input file, 128 bytes at a time...
    while (fgets(str, 128, in) != NULL) {

        writeItem(str, strlen(str), in);
        p_totalBytesRead = p_totalBytesRead + strlen(str);
        //sleep(1);
    }

    writeItem("EOF\0", p_totalBytesRead, in);

    fclose(in);
    sleep(20);
    thread_finished = 1;
    pthread_exit(NULL);
}

int writeItem(char text[], int count, FILE *in)
{
    sem_wait(sem_E_id);
    sem_wait(sem_S_id);
    printf("%d\n", in_item);
    strcpy(item[in_item].text, text);
    item[in_item].byte_count = count;

    if (in_item == 99)
        in_item = 0;
    else
        in_item++;

    sem_signal(sem_S_id);
    sem_signal(sem_N_id);
    return 1;
}

int readItem(FILE* out)
{
    struct buf_element temp;

    sem_wait(sem_N_id);
    sem_wait(sem_S_id);
    
    temp = item[out_item];
    
    if (strcmp(temp.text, "EOF")) {
        fputs(temp.text, out);
    
        if (out_item == 99)
            out_item = 0;
        else
            out_item++;
        
        sem_signal(sem_S_id);
        sem_signal(sem_E_id);
    }
    else {
        sem_signal(sem_S_id);
        sem_signal(sem_N_id);
    }

    return temp.byte_count;
}