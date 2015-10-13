/**
 * cloud.c made by:
 *   Reid Cain-Mondoux: 100945700
 *   Noah Segal: 100911661
 **/

#include "message_struct.h"

/**
 * Function Declaration(s)
 **/
void clear_whitespace(char *string);

/**
 * Code responsible for initializing the Cloud
 *
 * Cloud can receive input (case sensitive):
 *	GET sensor_name
 *	PUT actuator_name
 **/
int main(int argc, char argv[]){
	char receive_buffer[BUFFER_SIZE+1];
	char send_buffer[BUFFER_SIZE+1];
	int receive_pipe_fd;
	int send_pipe_fd;
	int pid;
	int res;
	memset(receive_buffer, '\0', sizeof(receive_buffer));
	memset(send_buffer, '\0', sizeof(send_buffer));

	printf("Cloud has Started\n\n");
	pid = fork();
	
	switch(pid){
	case -1: // Failed
		printf("fork failed");
		exit(EXIT_FAILURE);
		break;
	case 0: // Child
		if (access(CTRL_FIFO_NAME,F_OK) == -1) {
			if (mkfifo(CTRL_FIFO_NAME,0777) != 0) {
				fprintf(stderr,"Could not create fifo: %s\n", CTRL_FIFO_NAME);
				exit(EXIT_FAILURE);
			} 
		}
		receive_pipe_fd = open(CTRL_FIFO_NAME, O_RDONLY);
	
		if (receive_pipe_fd == -1) {
			fprintf(stderr, "Could not open fifo: %s\n", CTRL_FIFO_NAME);
			exit(EXIT_FAILURE);
		}
		// Read input from the Parent (in controller.c)
		while(1) {
			res = read(receive_pipe_fd, receive_buffer, BUFFER_SIZE);
			if(res > 0){
				printf("%s\n", receive_buffer);
			}
		}
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	};	
	
	// Create a new fifo (responsible for user input)
	if (access(USER_FIFO_NAME, F_OK) == -1) {
		if (mkfifo(USER_FIFO_NAME, 0777) != 0) {
			fprintf(stderr, "Could not create fifo: %s\n", USER_FIFO_NAME);
			exit(EXIT_FAILURE);
		} 
	}
	send_pipe_fd = open(USER_FIFO_NAME, O_WRONLY);
	
	// Wait on user input
	while(1) {
		printf("Enter Command with format \"GET SENSOR_NAME\" or \"PUT ACTUATOR_NAME\":\n");
		fgets(send_buffer, BUFFER_SIZE, stdin);
		clear_whitespace(send_buffer);
		
		if ( write(send_pipe_fd, send_buffer, BUFFER_SIZE) == -1 ){
			fprintf(stderr,"write error on pipe\n");
			exit(EXIT_FAILURE);
		}
		

	}
}

/**
 * Responsible for clearing whitespace generated from fgets()
 * (This bug was pain in the @** to catch)
 **/
void clear_whitespace(char *string){
	char *string_copy = strdup(string);
	strcpy(string, strsep(&string_copy, "\n"));
}	
