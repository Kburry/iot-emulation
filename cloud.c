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


#define FIFO_NAME "/tmp/cloud_fifo"
#define BUFFER_SIZE PIPE_BUF


int main(int argc, char argv[]){
	char buffer[BUFFER_SIZE+1];
	int pipe_fd;
	int res;
	memset(buffer, '\0', sizeof(buffer));
	
	if (access(FIFO_NAME,F_OK) == -1) {
		if (mkfifo(FIFO_NAME,0777) != 0) {
			fprintf(stderr,"Could not create fifo: %s\n", FIFO_NAME);
			exit(EXIT_FAILURE);
		} 
	}
	
	pipe_fd = open(FIFO_NAME,O_RDONLY);
	if (pipe_fd == -1) {
		fprintf(stderr, "Could not open fifo: %s\n", FIFO_NAME);
		exit(EXIT_FAILURE);
	}
	while(1) {
		res = read(pipe_fd, buffer, BUFFER_SIZE);
		if(res > 0){
			printf("%s\n", buffer);
		}
	}
	
	
}
