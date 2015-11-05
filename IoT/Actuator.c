// Required C Libraries
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#include <sys/msg.h>
#include <sys/types.h>

#define MAX_TEXT 512

int running_state = 1;

struct msg_st {
	long int my_msg_type;
	struct private_info {
		pid_t sender_pid;
		char name[25];
		int device_type;
		int message_type;
		int threshold;
		int current_value;
	} info;
};

// Catching signal when message is in MQ
void sigint_handler(int sig) {
	printf("Caught SIGINT\n");
}

int main(int argc, char *argv[])
{
	int arg;  // Command-line Name Argument
	int id_msgQueue; // Message Queue ID

	struct msg_st msg;

	// Handles incoming SIGINT Signals
	struct sigaction act;

	act.sa_handler = sigint_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, 0);

	system("clear");

	// Ensures that the Controller is invoked with a name
	if (argc != 2) {
		printf("Execute with proper parameters. E.g. ./actuator __name__\n");
		exit(0);
	}
	else {
		printf("=================== Actuator Process (Pid #%d) ===================\n", getpid());
		printf("Actuator Name: %s\t\tPid:%d\n", argv[1], getpid());
	}

	/* Create and access a single message queue to be shared among processes
		key: id of particular message queue
		msgflg: permission flags
		returns: positive number on success, -1 on failure
	*/
	id_msgQueue = msgget((key_t) 1234, 0666 | IPC_CREAT);

	// When the MQ fails to be created...
	if (id_msgQueue == -1) {
		fprintf(stderr, "Creation of message queue failed with error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
	//printf("Returned %d\n", id_msgQueue);

	// Registering the sensor to the Controller
	msg.my_msg_type = 999; // Maps to Message for Controller
	msg.info.device_type = 2; // Maps to Actuator Device
	msg.info.sender_pid = getpid(); // Current proccess id
	msg.info.message_type = 1;
	strcpy(msg.info.name, argv[1]);

	// Sending registration to the Controller
	if (msgsnd(id_msgQueue, (void *)&msg, sizeof(msg.info), 0) == -1) {
        fprintf(stderr, "Actuator %s could not send message.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    while (running_state) {
		while(1) {
			// When registration msg is received...
			if (msgrcv(id_msgQueue, (void *)&msg, BUFSIZ, getpid(), 0) == -1) {
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
			break;
		}
		if (msg.info.message_type == 3) {
			printf("%s says there is an alarm #%d!!! Do this action!\n", msg.info.name, msg.info.current_value);

			msg.my_msg_type = 999; // Maps to Message for Controller
			msg.info.device_type = 2; // Maps to Actuator Device
			msg.info.sender_pid = getpid(); // Current proccess id
			msg.info.message_type = 5;
			strcpy(msg.info.name, argv[1]);

			if (msgsnd(id_msgQueue, (void *)&msg, sizeof(msg.info), 0) == -1) {
		        fprintf(stderr, "Sensor %s could not send message.\n", argv[1]);
		        exit(EXIT_FAILURE);
		    }
		}
		else if (msg.info.message_type == 4) {
			printf("Action from Controller to stop actuator!\n");
    		exit(EXIT_SUCCESS);
		}
		else if (msg.info.message_type == 5) {
			printf("%s received registration info!\n", msg.info.name);
		}
	}

	if (msgctl(id_msgQueue, IPC_RMID, 0) == -1) {
        fprintf(stderr, "Unable to delete message queue\n");
        exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}