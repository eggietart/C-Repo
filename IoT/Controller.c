// Required C Libraries
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include <sys/msg.h>
#include <sys/types.h>

#include "client.h"

int running_state = 1;

// Stores Pid of all devices risgetered onto this Controller
int numSensor = 0;
int numActuator = 0;
int numDevices = 0;
pid_t sensorDevices[10];
pid_t actuatorDevices[10];

// Catching signal when message is in MQ
void sigint_handler(int sig) {
	printf("Caught SIGINT\n");
}

long int getActuator(pid_t pid) {
	long int aPid;
	int i;

	for (i = 0; i < 5; i++) {
		if (sensorDevices[i] == pid) {
			if (actuatorDevices[i])
				aPid = actuatorDevices[i];
			else
				aPid = 0;
		}
	}
	return aPid;
}

// Sends out msg via message queue
void msg_out(int mq_id, long int my_msg_type, pid_t pid, char name[], int dev, int message_type, int threshold, int current_value) {

	struct msg_st send_msg;
	send_msg.my_msg_type = my_msg_type;
	send_msg.info.sender_pid = pid;
	strcpy(send_msg.info.name, name);
	send_msg.info.device_type = dev;
	send_msg.info.message_type = message_type;
	send_msg.info.threshold = threshold;
	send_msg.info.current_value = current_value;
	
	if (msgsnd(mq_id, (void *)&send_msg, sizeof(send_msg.info), 0) == -1) {
    	fprintf(stderr, "Controller %s could not send message.\n", name);
    	exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[])
{
	int arg;  // Command-line Name Argument
	int stat_val;
	int rand_stop, rand_alarm, stopChild;
	int id_msgQueue; // Message Queue ID
	int i;
	long int msg_to_receive;
	pid_t child_pid;
	pid_t pid;

	struct msg_st msg;

	// Declarations for FIFOs
	int server_fifo_fd, client_fifo_fd;
    struct data_to_pass_st my_data;
    int client_res;
    char client_fifo[256];

	// Handles incoming SIGINT Signals
	struct sigaction act;

	act.sa_handler = sigint_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, 0);

	// Create and access a single message queue to be shared among processes
	id_msgQueue = msgget((key_t) 1234, 0666 | IPC_CREAT);

	system("clear");

	// When the MQ fails to be created...
	if (id_msgQueue == -1) {
		fprintf(stderr, "Creation of message queue failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	// Ensures that the Controller is invoked with a name
	if (argc != 2) {
		printf("Execute with proper parameters. E.g. ./controller __name__\n");
		exit(0);
	}
	else
		printf("=================== Controller Process ===================\nController Name: %s\n", argv[1]);


	printf("Parent Pid: %d\n", getpid());
	// Creating parent and child processs
	pid = fork();

	switch (pid) {
		case -1:
			perror("Fork was unsuccessful.\n");
			exit(EXIT_FAILURE);
		case 0:
			printf("Child Pid: %d\n\n", getpid());

			// Listening for any msgs from devices
			printf("Waiting messages from devices...\n");

			while(running_state) {

				while(1) {
					// When registration msg is received...
					msg_to_receive = 999;
					if (msgrcv(id_msgQueue, (void *)&msg, BUFSIZ, msg_to_receive, 0) == -1) {
						fprintf(stdout, "Message failed to receive with error: %d\n", errno);
						perror("msgrcv");
						if (errno == EINTR) {
							printf("Caught signal while blocked on msgrcv(). Retrying... msgrcv()\n");
							continue;
						}
						else {
							exit(EXIT_FAILURE);
						}
					}

					if (stopChild) {
						exit(EXIT_SUCCESS);
					}
					break;
				}
				if (msg.info.message_type == 1) {
					// Registering the new device...
					printf("Registering device...\n");
					printf("Device Name: %s\nDevice Type: %d\nPid: %d\nThreshold Value: %d\nMessage Type: %d\n\n",
						msg.info.name,
						msg.info.device_type,
						msg.info.sender_pid,
						msg.info.threshold,
						msg.info.message_type);

					// Register device into the system
					if (msg.info.device_type == 1) {
						sensorDevices[numSensor] = msg.info.sender_pid;
						numSensor++;
					}
					else if (msg.info.device_type == 2) {
						actuatorDevices[numActuator] = msg.info.sender_pid;
						numActuator++;
					}
					numDevices++;

			    	msg_out(id_msgQueue, msg.info.sender_pid, getpid(), argv[1], 0, 5, msg.info.threshold, msg.info.current_value);
				}
				else if (msg.info.message_type == 2) {
					printf("Device Name: %s\tPid: %d\tThreshold Value: %d\tCurrent Value: %d\n",
						msg.info.name,
						msg.info.sender_pid,
						msg.info.threshold,
						msg.info.current_value);

					rand_alarm = rand() % 10;

					// If sensing value read is greater than the device threshold...
					if (msg.info.current_value > msg.info.threshold) {
						// Inform actuator to do some actions
						printf("Inform Actuator #%d of alarm #%d! Send action!!!!\n", msg.info.sender_pid, msg.info.current_value);
						msg_out(id_msgQueue, getActuator(msg.info.sender_pid), getpid(), argv[1], 0, 3, msg.info.threshold, rand_alarm);

				    	// Send alarm to parent process
				    	msg_out(id_msgQueue, 777, msg.info.sender_pid, msg.info.name, rand_alarm, 1, msg.info.threshold, msg.info.current_value);
					}

					// Sends a random stop signal to the sensor and its corresponding actuator
					srand(time(NULL));
			    	rand_stop = rand() % 100;
			    	//printf("Random Stop Num: %d\n", rand_stop);
			    	if (rand_stop >= 90) {

			    		// Stop Sensor...
						printf("Stopping Sensor #%d!!\n", msg.info.sender_pid);
						msg_out(id_msgQueue, msg.info.sender_pid, getpid(), argv[1], 0, 4, 0, 0);

						// Stop Actuator...
						printf("Stopping Actuator #%ld!!\n", getActuator(msg.info.sender_pid));
						msg_out(id_msgQueue, getActuator(msg.info.sender_pid), getpid(), argv[1], 0, 4, 0, 0);

				    	numDevices = numDevices - 2;

				    	if (numDevices <= 0) {
							// Sends the message to parent to end the controller
					    	msg_out(id_msgQueue, 777, getpid(), argv[1], 0, 2, 0, 0);
					   		// Sends msg to self...
					    	msg_out(id_msgQueue, 999, getpid(), argv[1], 0, 6, 0, 0);
				    	}
			    	}
				}
				else if (msg.info.message_type == 5) {
					printf("Actuator #%d received the action for alarm %d!\n", msg.info.sender_pid, msg.info.current_value);
				}
				else if (msg.info.message_type == 6) {
					running_state = 0;
				}
			}
    		printf("Exiting child process...\n");
			
			break;
		default:
			// This is the Parent Process

			// Creating FIFO with Cloud Process
		    server_fifo_fd = open(SERVER_FIFO_NAME, O_WRONLY);
		    
		    if (server_fifo_fd == -1) {
		        fprintf(stderr, "Sorry, no Cloud Process Server.\n");
		        exit(EXIT_FAILURE);
		    }

    		// Attempt to create a FIFO to the server, first checks whether FIFO already exists  
		    // Server ----msg----> Client
		    /*
	        my_data.client_pid = getpid();
		    sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
		    if (mkfifo(client_fifo, 0777) == -1) {
		        fprintf(stderr, "Sorry, can't make %s\n", client_fifo);
		        exit(EXIT_FAILURE);
		    }
		    my_data.client_pid = getpid();
		    write(server_fifo_fd, &my_data, sizeof(my_data));
		    printf("Successfully created client FIFO.\n");
			*/
			while(1) {
				// When registration msg is received...
				msg_to_receive = 777;
				if (msgrcv(id_msgQueue, (void *)&msg, BUFSIZ, msg_to_receive, 0) > 0) {
					// Alarm occurred, trigger action
					if (msg.info.message_type == 1) {
						printf("\n---------- ALARM TRIGGERED ----------\n\nAlarm: %d\nDevice Name: %s\nPid: %d\nThreshold Value: %d\nSensing Value: %d\n\n",
							msg.info.device_type,
							msg.info.name,
							msg.info.sender_pid,
							msg.info.threshold,
							msg.info.current_value);

						// Send Alarm Triggering to the Cloud Process via FIFO
						msg.info.message_type = 0;
				        write(server_fifo_fd, &msg, sizeof(msg));
				    }
				    else if (msg.info.message_type == 2) { // No more devices on the Controller, shut down...
				    	sleep(5);

	    				// End the Cloud Process
						msg.info.message_type = 2;
				        write(server_fifo_fd, &msg, sizeof(msg));
				    	printf("Exiting parent process...\n");
				    	break;
				    }
				}
			}

			// Waits for the Child Process to end before Parent ends
			waitpid(pid, &stat_val, 0);

			if (msgctl(id_msgQueue, IPC_RMID, 0) == -1) {
		        fprintf(stderr, "Unable to delete message queue\n");
		        exit(EXIT_FAILURE);
			}

			if(WIFEXITED(stat_val))
				printf("Child proccess exited with code %d\n", WEXITSTATUS(stat_val));
			else
				printf("Child terminated abnormally.\n");

			// Clean up for FIFO
		    close(server_fifo_fd);
		    //unlink(client_fifo);

			break;			
	}
	exit(EXIT_SUCCESS);
}