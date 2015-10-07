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

	// Populate initial message
	sensor_data.pid = getpid();
	sensor_data.dev_type = SENSOR;
	sensor_data.command = START;
	sensor_data.current_value = atoi(argv[2]);
	
	strcpy(sensor_data.actuator_name ,"");
	strcpy(sensor_data.name, argv[1]);

	printf("here\n");

	running = 1;
	msgid = msgget((key_t) MSG_Q_KEY, 0666);

	// Ensure Message Queue is running
	if(msgid == -1) {
		fprintf(stderr,"Server message queue not started. Error: %d\n",errno);
		exit(EXIT_FAILURE);
	}

	// Send first message
	sensor_package.data = sensor_data;
	sensor_package.message_type = 1;
	if(msgsnd(msgid,(void *)&sensor_package,sizeof(sensor_package.data),0) == -1){
			fprintf(stderr, "Message sent failed. Error: %d\n",errno);
			exit(EXIT_FAILURE);
	}
	sensor_data.command = UPDATE;
	printf("Sensor PID: %d\n", sensor_data.pid);

	// Send subsequent message(s)
	while(running) {
		if (sensor_data.command == STOP) {
			printf("Shut-down command received: Shutting down\n");
			exit(EXIT_SUCCESS);
		}
		// Randomly generated sensor range [-50, 50]
		sensor_data.current_value = (rand() % 100) - 50;
		sensor_package.data = sensor_data;
		printf("Sensor \"%s\" Value: %d\n",sensor_data.name, sensor_data.current_value);

		if(msgsnd(msgid,(void *)&sensor_package,sizeof(sensor_package.data),0) == -1){
			fprintf(stderr, "Message sent failed. Error: %d\n",errno);
			exit(EXIT_FAILURE);
		}
		if(msgrcv(msgid, (void *) &sensor_package, sizeof(sensor_package.data), sensor_data.pid, IPC_NOWAIT) == -1){
			;
		}

		// Check every 2 seconds
		sleep(2);
	}
}

void e_print(char * err_ptr){
	fprintf(stderr, "%s Error: %d\n",err_ptr,errno);
	exit(EXIT_FAILURE);
}

