#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>


#define CTRL_FIFO_NAME "/tmp/ctrl_cloud_fifo"
#define USER_FIFO_NAME "/tmp/user_cloud_fifo"
#define BUFFER_SIZE PIPE_BUF


int main(int argc, char argv[]){
	char receive_buffer[BUFFER_SIZE+1];
	char send_buffer[BUFFER_SIZE+1];
	int receive_pipe_fd;
	int send_pipe_fd;
	int pid;
	int res;
	memset(receive_buffer, '\0', sizeof(receive_buffer));
	memset(send_buffer, '\0', sizeof(send_buffer));
	
	pid = fork();
	
	switch(pid){
	case -1:
		printf("fork failed");
		exit(EXIT_FAILURE);
		break;
	case 0:
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
	
	if (access(USER_FIFO_NAME,F_OK) == -1) {
			if (mkfifo(USER_FIFO_NAME,0777) != 0) {
				fprintf(stderr,"Could not create fifo: %s\n", USER_FIFO_NAME);
				exit(EXIT_FAILURE);
			} 
		}
	
	char usr_str[50];
	while(1) {
		// Format: GET sensor_name			-->	Returns: Sensor X with threshhold Y paired with Actuator Z 
		// Format: PUT actuator_name ON/OFF	--> Returns: User turned ON/OFF Actuator Z
		printf("Enter Command (GET SENSOR_NAME or PUT ACTUATOR_NAME):\n");
		scanf("%s", &usr_str);
	}
	
	
	
	
	
	
}
