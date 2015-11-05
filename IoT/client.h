#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>

#define SERVER_FIFO_NAME "/tmp/serv_fifo"
#define CLIENT_FIFO_NAME "/tmp/cli_%d_fifo"

#define BUFFER_SIZE 20

struct data_to_pass_st {
	pid_t client_pid;
	char some_data[BUFFER_SIZE - 1];
};

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