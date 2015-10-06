#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <limits.h>
//#include "device.h"
#include "message_struct.h"

void e_print(char * err_ptr);


int main(int argc, char * argv[]){

	message_data_st sensor_data;
	message_package_st sensor_package;
	int running;
	int msgid;

	sensor_data.pid = getpid();
	sensor_data.device_type = sensor;
	sprintf(sensor_data.name,"%s",argv[1]);
	sensor_data.threshold = atoi(argv[2]);
	sensor_data.current_value = -1;

	running = 1;
	msgid = msgget((key_t) MSG_Q_KEY, 0666);
	if(msgid == -1){ //e_print("Server message queue not started.");
		fprintf(stderr,"Server message queue not started. Error: %d\n",errno);
		exit(EXIT_FAILURE);
	}
	sensor_package.data = sensor_data;
	sensor_package.message_type = SRV_Q_KEY;
	while(running){
		if(msgsnd(msgid,(void *)&sensor_package,sizeof(sensor_package.data),0) == -1){
			fprintf(stderr, "Message sent failed. Error: %d\n",errno);
			exit(EXIT_FAILURE);
		}
		if(msgrcv(msgid,(void *)&sensor_package,sizeof(sensor_package.data),sensor_data.pid,0) == -1){
			;
		}
		sleep(2);
	}
}

void e_print(char * err_ptr){
	fprintf(stderr, "%s Error: %d\n",err_ptr,errno);
	exit(EXIT_FAILURE);
}

