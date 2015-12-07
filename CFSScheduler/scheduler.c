#include "functions.h"

const int NUM_THREADS = 6;
const int NUM_PROCESSES = 20;
const int NUM_CPUS = 4;

const int DEFAULT_STATIC_PRIO = 120;
const int SCHED_FIFO_TYPE = 1;
const int SCHED_RR_TYPE = 2;
const int SCHED_NORMAL_TYPE = 3;

const int MAX_SLEEP_AVG = 100;   // 100 ms

int main() {

    init_all();

    while(!thread_finished)
        sleep(1);

    clean_all();

    exit(EXIT_SUCCESS);
}

void *consumer_function(void *arg) {

    int my_id = *(int *)arg;
    int current_qt;
    int highest_priority;
    int time_slept;
    struct node *current_node;
    struct node *previous_node;
    struct node *conductor;
    struct task_struct *task;

    printf("This is a consumer thread. Argument was %d\n", my_id);
    
    // Initializating CPU's run queue
    root[my_id] = (struct node *) malloc(sizeof(struct node));
    root[my_id]->thread_id = pthread_self();
    root[my_id]->next_node = 0;

    printf("Root Node created for Consumer %d with pthread_id = %ld.\n", my_id, (long) pthread_self());

    sleep(10);

    while (root[my_id]->next_node != NULL) {
        
        highest_priority = 140;
        conductor = root[my_id];
        while (conductor->next_node != NULL) {
            if (conductor->next_node->task_info.static_prio < highest_priority) {
                highest_priority = conductor->next_node->task_info.static_prio;
            }
            conductor = conductor->next_node;
        }

        // Finds the noode with the highest priority...
        previous_node = root[my_id];
        conductor = root[my_id]->next_node;
        while (conductor->task_info.static_prio != highest_priority) {
            previous_node = conductor;
            conductor = conductor->next_node;
        }

        // Found it...
        sem_wait(sem_S_id);
        previous_node->next_node = conductor->next_node;
        current_node = conductor;
        sem_signal(sem_S_id);

        printf("C%d: Process Info: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", 
            my_id,
            current_node->task_info.pid,
            current_node->task_info.sched_type,
            current_node->task_info.static_prio,
            current_node->task_info.prio,
            current_node->task_info.execution_time,
            current_node->task_info.time_slice,
            current_node->task_info.accu_time_slice,
            current_node->task_info.accu_sleep_time,
            current_node->task_info.sleep_avg,
            current_node->task_info.last_cpu
        );
        if (current_node->task_info.sched_type != SCHED_FIFO_TYPE) {

            current_qt = calculate_quantum(current_node->task_info.static_prio);

            // Calculate sleep average...
            time_slept  = calculate_sleep_time(current_node->task_info.time_slept);
            current_node->task_info.sleep_avg = calculate_sleep_avg(current_node->task_info.sleep_avg, time_slept, current_qt);
        
            // If Quantom Slot is >> than left over execution time of the ps...
            if (current_qt >= current_node->task_info.execution_time) {
                usleep(current_node->task_info.execution_time * 1000);
                sem_wait(sem_S_id);
                previous_node->next_node = current_node->next_node;
                printf("Process #%d has completed:\tTurnaround Time: %dusec\t\n", current_node->task_info.pid, (current_node->accu_sleep_time + current_node->accu_time_slice));
                free(current_node);
                sem_signal(sem_S_id);
            }
            else {
                usleep(current_qt * 1000);

                // Update process state information...
                current_node->task_info.execution_time = current_node->task_info.execution_time - current_qt;
                current_node->task_info.time_slice = current_qt;
                current_node->task_info.accu_time_slice = current_node->task_info.accu_time_slice + current_qt;
                current_node->task_info.accu_sleep_time = current_node->task_info.accu_sleep_time + calculate_sleep_time(current_node->task_info.time_slept);
                gettimeofday(&current_node->task_info.time_slept, NULL);
                current_node->task_info.last_cpu = my_id;

                // Find the last node...
                conductor = root[my_id];
                while (conductor->next_node != NULL) {
                    conductor = conductor->next_node;
                }
            
                sem_wait(sem_S_id);
                conductor->next_node = current_node;
                current_node->next_node = 0;
                printf("Process #%d has been preempted:\tService Time Left: %dusec\t\n", current_node->task_info.pid, current_node->task_info.execution_time);
                sem_signal(sem_S_id);
            }
        }
        else {
            usleep(current_node->task_info.execution_time * 1000);
            sem_wait(sem_S_id);
            previous_node->next_node = current_node->next_node;
            free(current_node);
            sem_signal(sem_S_id);
        }
    }

    free(root[my_id]);
    pthread_exit(NULL);
}

void *producer_function(void *arg) {
    
    // Variable initialization
    int itr, ps_itr;
    struct node *conductor;
    struct task_struct *task;

    ps_itr = 0;
    srand(time(NULL));

    printf("This is a producer thread. Argument was %ld\n", (long) pthread_self());
    sleep(5);

    for (itr = 0; itr < NUM_PROCESSES; itr++) {

        // Creating process information struct...
        task = (struct task_struct *) malloc(sizeof(struct task_struct));
        task->pid = itr;
        task->sched_type = (rand() % 3) + 1;        // Between 1-3

        // Calculate static priority based on scheduling type...
        if ((task->sched_type == SCHED_FIFO_TYPE) || (task->sched_type == SCHED_RR_TYPE))
            task->static_prio = (rand() % 100) + 1;
        else if (task->sched_type == SCHED_NORMAL_TYPE)
            task->static_prio = (rand() % 40) + 100;
        else
            task->static_prio = DEFAULT_STATIC_PRIO;

        task->prio = task->static_prio;
        task->execution_time = (5 + rand() / (RAND_MAX / (50 - 5 + 1) + 1)) * 1000; // Between 5 secs to 50 secs
        task->time_slice = 20;
        task->accu_time_slice = 0;
        task->accu_sleep_time = 0;
        task->sleep_avg = 0;
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

            // Priting out created process information...
            printf("P: Process Info: %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n", 
                conductor->task_info.pid,
                conductor->task_info.sched_type,
                conductor->task_info.static_prio,
                conductor->task_info.prio,
                conductor->task_info.execution_time,
                conductor->task_info.time_slice,
                conductor->task_info.accu_time_slice,
                conductor->task_info.accu_sleep_time,
                conductor->task_info.sleep_avg,
                conductor->task_info.last_cpu
            );
        }

        ps_itr++;
        ps_itr = ps_itr % 4;
    }

    sleep(180);
    thread_finished = 1;
    pthread_exit(NULL);
}

void *process_balancer_function(void *arg) {

    int itr;
    int queue_length[4];
    struct node *conductor;
    double queue_length_avg;

    printf("This is a process balancer thread. Argument was %ld\n", (long) pthread_self());
    sleep(4);
    
    while (!thread_finished) {

        queue_length_avg = 0;
        printf("Balancing process...\n");
        
        for (itr = 0; itr < 4; itr++) {
            conductor = root[itr];
            queue_length[itr] = 0;
            
            sem_wait(sem_S_id);
            while (conductor->next_node != NULL) {
                conductor = conductor->next_node;
                queue_length[itr]++;
            }
            sem_signal(sem_S_id);

            queue_length_avg = queue_length_avg + queue_length[itr];
            printf("Queue Length #%d: %d\n", itr, queue_length[itr]);
        }

        queue_length_avg = queue_length_avg / 4;
        printf("Queue Length Average %0.2lf\n", queue_length_avg);

        for (itr = 0; itr < 4; itr++) {
            if (!(queue_length[itr] == (int) queue_length_avg) && !(queue_length[itr] == (int) (queue_length_avg + 1))) {
                printf("Rebalancing required...\n");
            }
        }

        sleep(3);
    }
    pthread_exit(NULL);
}

int calculate_quantum(int static_p) {
    
    int qt = 0;

    if (static_p < 120) 
        qt = (140 - static_p) * 20;
    else if (static_p >= 120)
        qt = (140 - static_p) * 5;
    return qt;
}

int calculate_priority(int static_p, int sleep_average) {
    return 1;
}

int calculate_sleep_time(struct timeval sleep_t) {
    
    struct timeval current_time;
    int time_delta;

    gettimeofday(&current_time, NULL);
    time_delta = (current_time.tv_usec - sleep_t.tv_usec) / 1000;

    return abs(time_delta);
}

int calculate_sleep_avg(int sleep_average, int time_slept, int qt) {

    int ret = (sleep_average + time_slept) > MAX_SLEEP_AVG ? MAX_SLEEP_AVG : (sleep_average + time_slept);
    ret = ret - qt;

    return (ret < 0 ? 0 : ret);
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