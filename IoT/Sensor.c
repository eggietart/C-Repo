// Required C Libraries
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include <sys/msg.h>
#include <sys/types.h>

#define MAX_TEXT 512

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

int isNumber(const char* s)
{
	if (s == NULL || *s == '\0' || isspace(*s))
		return 0;
	char* p;
	strtod(s, &p);
	return *p == '\0';
}

int main(int argc, char *argv[])
{
	int arg;  // Command-line Name Argument
	int id_msgQueue; // Message Queue ID
	int num_readings; // Number of Sensing Values
	int random_read;

	struct msg_st msg;

	system("clear");

	// Ensures that the Controller is invoked with a name
	if (argc != 3) {
		printf("Execute with proper parameters. E.g. ./sensor __name__ __threshold_value__\n");
		exit(0);
	}
	else {
		if (!isNumber(argv[2])) {
			printf("Invalid Threshold Value: %s.\n", argv[2]);
			exit(0);
		}
		printf("=================== Sensor Process (Pid #%d) ===================\n", getpid());
		printf("Sensor Name: %s\t\tThreshold Value:%s\t\tPid:%d\n", argv[1], argv[2], getpid());
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
	msg.info.device_type = 1; // Maps to Sensor Device
	msg.info.sender_pid = getpid(); // Current proccess id
	msg.info.threshold = atoi(argv[2]);
	msg.info.message_type = 1;
	strcpy(msg.info.name, argv[1]);

	// Sending registration to the Controller
	if (msgsnd(id_msgQueue, (void *)&msg, sizeof(msg.info), 0) == -1) {
        fprintf(stderr, "Sensor %s could not send message.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

	// Receive acknowledgement from Controller  
    if (msgrcv(id_msgQueue, (void *)&msg, BUFSIZ, getpid(), 0) == -1) {
		fprintf(stdout, "Message failed to receive with error: %d\n", errno);
		perror("msgrcv");
		if (errno == EINTR) {
			printf("Caught signal while blocked on msgrcv(). Retrying... msgrcv()\n");
			//continue;
		}
		else {
			exit(EXIT_FAILURE);
		}
	}
	if (msg.info.message_type == 5) {
		printf("%s received registration info!\n", msg.info.name);
	}
	/*else if (msg.info.message_type == 4) {
		printf("Action from Controller to stop sensor!\n");
		//sleep(5);
		exit(EXIT_SUCCESS);
	}*/

	srand(time(NULL));
    // Make periodic readings of values and sending it to the Controller
    for (num_readings = 0; num_readings < 50; num_readings++) {
    
    	sleep(5);
    	random_read = rand() % 100;

		msg.my_msg_type = 999; // Maps to Message for Controller
		msg.info.device_type = 1; // Maps to Sensor Device
		msg.info.sender_pid = getpid(); // Current proccess id
		msg.info.message_type = 2; // Maps to Current Value Messsage
		msg.info.current_value = random_read;
		strcpy(msg.info.name, argv[1]);

		printf("Pid: %d Current Value: %d\n", getpid(), msg.info.current_value);

		if (msgsnd(id_msgQueue, (void *)&msg, sizeof(msg.info), 0) == -1) {
        	fprintf(stderr, "Sensor %s could not send message.\n", argv[1]);
        	exit(EXIT_FAILURE);
    	}

		// Receive "stop" cmd from Controller  
	    if (msgrcv(id_msgQueue, (void *)&msg, BUFSIZ, getpid(), IPC_NOWAIT) > 0) {
	    	if (msg.info.message_type == 4) {
	    		printf("Action from Controller to stop sensor!\n");
	    		break;
	    	}
		}
    }

	exit(EXIT_SUCCESS);
}