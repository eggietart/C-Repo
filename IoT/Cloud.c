// Cloud Process - Server Side of FIFO
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include "client.h"

#define MAX_TEXT 512

int main()
{
    int server_fifo_fd, client_fifo_fd;
    int i;
    pid_t pid, child_pid, client_pid;
    struct data_to_pass_st my_data;
    struct msg_st msg, msg_to_controller;
    int read_res, serv_res, stat_val;
    char client_fifo[256];
    char buffer[MAX_TEXT];
    char *tmp_char_ptr;

    system("clear");
/*
    // Gets the Pid of the Client (Controller)
	read_res = read(server_fifo_fd, &my_data, sizeof(my_data));
	if (read_res > 0) {
    	sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
    }
*/
    /*
    pid = fork();

    switch (pid) {
    	case -1:
   			perror("Fork was unsuccessful.\n");
			exit(EXIT_FAILURE);
		case 0:
		*/
		    // Attempt to create a FIFO to the client, first checks whether FIFO already exists  
		    // Server <----msg---- Client
		    if (access(SERVER_FIFO_NAME, F_OK) == -1) {
		        serv_res = mkfifo(SERVER_FIFO_NAME, 0777);
		        if (serv_res != 0) {
		            fprintf(stderr, "Cloud (Server) could not create FIFO %s\n", SERVER_FIFO_NAME);
		            exit(EXIT_FAILURE);
		        }
		    }

		    printf("=================== Cloud Process (Pid #%d) ===================\n", getpid());
			server_fifo_fd = open(SERVER_FIFO_NAME, O_RDONLY);

		    if (server_fifo_fd == -1) {
		        fprintf(stderr, "Cloud (Server) could not create FIFO.\n");	
		        exit(EXIT_FAILURE);
		    }

		    sleep(2);
		    
		    // Get pid of client
		    read_res = read(server_fifo_fd, &my_data, sizeof(my_data));
	        if (read_res > 0) {
	        	//client_pid = my_data.client_pid;
	        	printf("Client Pid: %d\n", my_data.client_pid);
	        }

			do {
		        read_res = read(server_fifo_fd, &msg, sizeof(msg));	
		        if (read_res > 0) {
		        	if (msg.info.message_type == 0) {
						printf("---------- ALARM TRIGGERED ----------\n\nAlarm: %d\nDevice Name: %s\nPid: %d\nThreshold Value: %d\nSensing Value: %d\n\n",
							msg.info.device_type,
							msg.info.name,
							msg.info.sender_pid,
							msg.info.threshold,
							msg.info.current_value);
					}
					else if (msg.info.message_type == 2) {
						break;
					}
		        }
		    } while (read_res >= 0);
/*
		    break;
		 default:
		 	printf("Welcome to the Cloud!  The following are two options you can choose: \"get\" or \"put\" or \"exit\"\n");

		 	while (1) {     
		 		printf("Enter Option: ");
		        if (fgets(buffer, BUFSIZ, stdin) != NULL) {

		        	if (strcmp(buffer, "get\n") == 0) {

		        	}
		        	else if (strcmp(buffer, "put\n") == 0) {
		        		printf("For which actuator would you want to trigger an action for?\n");

		        		msg_to_controller.info.message_type = 2;

		        		sprintf(client_fifo, CLIENT_FIFO_NAME, my_data.client_pid);
				    	client_fifo_fd = open(client_fifo, O_WRONLY);
				        
				        if (client_fifo_fd != -1) {
				            write(client_fifo_fd, &msg_to_controller, sizeof(msg_to_controller));
				            close(client_fifo_fd);
				        }	
		        	}
		        	else if (strcmp(buffer, "exit\n") == 0) {
		        		printf("Exiting Cloud...\n");
		        		break;
		        	}
		        	else {
		        		printf("Invalid command. Please try again.\n");
		        	}
		        }
		    }

			// Waits for the Child Process to end before Parent ends
			child_pid = wait(&stat_val);

			printf("Child has finished.\n");
			if(WIFEXITED(stat_val))
				printf("Child exited with code %d\n", WEXITSTATUS(stat_val));
			else
				printf("Child terminated abnormally.\n");
			break;
    }
*/
    printf("Ending cloud process...\n\nBye!\n");
    close(server_fifo_fd);
    unlink(SERVER_FIFO_NAME);
    exit(EXIT_SUCCESS);
}

