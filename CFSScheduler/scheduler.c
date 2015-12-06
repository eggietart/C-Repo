#include "functions.h"

const int NUM_THREADS = 6;
const int NUM_PROCESSES = 20;
const int NUM_CPUS = 4;

const int DEFAULT_STATIC_PRIO = 120;

int main() {

    init_all();

    while(!thread_finished)
        sleep(1);

    clean_all();

    exit(EXIT_SUCCESS);
}

void *consumer_function(void *arg) {

    int my_id = *(int *)arg;
    struct node *conductor;
    struct task_struct *task;

    printf("This is a consumer thread. Argument was %d\n", my_id);
    
    // Initializating CPU's run queue
    root[my_id] = (struct node *) malloc(sizeof(struct node));
    root[my_id]->thread_id = pthread_self();
    root[my_id]->next_node = 0;

    printf("Root Node created for Consumer %d with pthread_id = %u.\n", my_id, pthread_self());

    sleep(10);

    while (root[my_id]->next_node != 0) {
        
        conductor = root[my_id]->next_node;
        printf("C%d: Process Info: %d, %d, %d, %d, %d, %d, %d\n", 
            my_id,
            conductor->task_info.pid,
            conductor->task_info.static_prio,
            conductor->task_info.prio,
            conductor->task_info.execution_time,
            conductor->task_info.time_slice,
            conductor->task_info.accu_time_slice,
            conductor->task_info.last_cpu
        );

        root[my_id]->next_node = conductor->next_node;
        sleep(conductor->task_info.execution_time);
        free(conductor);
    }

    free(root[my_id]);
    //thread_finished = 1;
    pthread_exit(NULL);
}

void *producer_function(void *arg) {
    
    // Variable initialization
    int itr, ps_itr;
    struct node *conductor;
    struct task_struct *task;

    ps_itr = 0;
    srand(time(NULL));

    printf("This is a producer thread. Argument was %u\n", pthread_self());
    sleep(5);

    for (itr = 0; itr < NUM_PROCESSES; itr++) {

        // Creating process information struct...
        task = (struct task_struct *) malloc(sizeof(struct task_struct));
        task->pid = itr;
        task->static_prio = DEFAULT_STATIC_PRIO;
        task->prio = DEFAULT_STATIC_PRIO;
        task->execution_time = 5 + rand() / (RAND_MAX / (50 - 5 + 1) + 1);
        task->time_slice = 20;
        task->accu_time_slice = 0;
        task->last_cpu = ps_itr;

        conductor = root[ps_itr];

        // Find the last node...
        while (conductor->next_node != 0) {
            conductor = conductor->next_node;
        }

        // Add process information into the run queue...
        conductor->next_node = (struct node *) malloc(sizeof(struct node));
        conductor = conductor->next_node;

        if (conductor == 0) {
            printf("Out of memory.\n");
            pthread_exit(NULL);
        }
        else {
            conductor->thread_id = root[ps_itr]->thread_id;
            conductor->task_info = *task;
            conductor->next_node = 0;
        }

        // Priting out created process information...
        printf("P: Process Info: %d, %d, %d, %d, %d, %d, %d\n", 
            conductor->task_info.pid,
            conductor->task_info.static_prio,
            conductor->task_info.prio,
            conductor->task_info.execution_time,
            conductor->task_info.time_slice,
            conductor->task_info.accu_time_slice,
            conductor->task_info.last_cpu
        );

        ps_itr++;
        ps_itr = ps_itr % 4;
    }

    sleep(180);
    thread_finished = 1;
    pthread_exit(NULL);
}

void *process_balancer_function(void *arg) {

    printf("This is a process balancer thread. Argument was %u\n", pthread_self());
    sleep(4);
    
    while (!thread_finished) {
        printf("Balancing processes...\n");
        sleep(3);
    }
}

int writeItem(char text[], int count, FILE *in)
{
    sem_wait(sem_E_id);
    sem_wait(sem_S_id);

    sem_signal(sem_S_id);
    sem_signal(sem_N_id);

    return 1;
}

int readItem(FILE* out)
{
    sem_wait(sem_N_id);
    sem_wait(sem_S_id);
        
    sem_signal(sem_S_id);
    sem_signal(sem_E_id);

    return 1;
}

int init_all(void)
{
    int res, j;
    pthread_t a_thread[NUM_THREADS];
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
    sem_E_id = init_sem((key_t)8002, nBuffers);

    res = pthread_attr_init(&thread_attr);
    if (res != 0) {
        perror("Attribute Creation failed.\n");
        exit(EXIT_FAILURE);
    }

    res = pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    if (res != 0) {
        perror("Setting schedpolicy failed.\n");
        exit(EXIT_FAILURE);
    }

    res = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (res != 0) {
        perror("Setting detached attribute failed.\n");
        exit(EXIT_FAILURE);
    }

    // Creating Consumer threads...
    for (j = 0; j < 4; j++) {
        
        res = pthread_create(&(a_thread[j]), &thread_attr, consumer_function, (void *)&j);
        if (res != 0) {
            perror("Consumer thread creation failed.\n");
            exit(EXIT_FAILURE);
        }
        sleep(1);
    }

    // Creating Producer thread...
    res = pthread_create(&(a_thread[4]), &thread_attr, producer_function, (void *)"4");
    if (res != 0) {
        perror("Producer thread creation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Creating Process Balancer thread...
    res = pthread_create(&(a_thread[5]), &thread_attr, process_balancer_function, (void *)"5");
    if (res != 0) {
        perror("Process Balancer thread creation failed.\n");
        exit(EXIT_FAILURE);
    }

    max_priority = sched_get_priority_max(SCHED_RR);
    min_priority = sched_get_priority_min(SCHED_RR);
    scheduling_value.sched_priority = min_priority;
    
    res = pthread_attr_setschedparam(&thread_attr, &scheduling_value);
    if (res != 0) {
        perror("Setting schedpolicy failed.\n");
        exit(EXIT_FAILURE);
    }

    (void)pthread_attr_destroy(&thread_attr);

    return 1;
}

int clean_all(void)
{
    // Delete semaphores...
    del_semvalue(sem_S_id);
    del_semvalue(sem_N_id);
    del_semvalue(sem_E_id);

    return 1;
}