/*
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
#include <limits.h>
//#include "device.h"
*/
#include "message_struct.h"

void e_print(char * err_ptr);
void print_state(char *name, int is_on);

/**
 *  Code responisble for initializing an Actuator
 *  
 *  From command line: ./actuator actuator_name paired_sensor_name ON/OFF
 *  ON  = 1
 *  OFF = 0
 **/
int main(int argc, char * argv[]){
	
	message_data_st actuator_data;
	message_package_st actuator_package;
	int running;
	int msgid;

	// Populate initial message
	actuator_data.pid = getpid();
	actuator_data.dev_type = ACTUATOR;
	actuator_data.command = START;

	strcpy(actuator_data.actuator_name, argv[1]);
	strcpy(actuator_data.name, argv[2]);
	actuator_data.current_value = atoi(argv[3]);

	running = 1;
	
	// Waiting for controller to start
	while ( (msgid = msgget((key_t) MSG_Q_KEY, 0666)) == -1) {}
	printf("Acknowledging that controller has started\n");

	// Send first message --> "It's alive!!!!"
	actuator_package.data = actuator_data;
	actuator_package.message_type = 1;
	
	if(msgsnd(msgid, (void *) &actuator_package, sizeof(actuator_package.data), 0) == -1){
			fprintf(stderr, "Message sent failed. Error: %d\n", errno);
			exit(EXIT_FAILURE);

	}
	printf("%s Actuator PID: %d\n", actuator_data.actuator_name, actuator_data.pid);

	// Receive subsequent message(s)
	while(running) {

		if(msgrcv(msgid, (void *) &actuator_package, sizeof(actuator_package.data), actuator_data.pid, 0) == -1){
			fprintf(stderr, "Failed receiving message. Error: %d\n", errno);
			exit(EXIT_FAILURE);
		}
		actuator_data = actuator_package.data;

		// Shut Down
		if (actuator_data.command == STOP) {
			printf("Shut-down command received: Shutting down\n");
			exit(EXIT_SUCCESS);
		}
		print_state(actuator_data.actuator_name, actuator_data.current_value);
	}
}

/**
 *  Print Actuator's state
 **/
void print_state(char *name, int is_on) {
	if (is_on) {
		printf("%s is ON\n", name);
	}
	else {
		printf("%s is OFF\n", name);
	}
}

void e_print(char * err_ptr){
	fprintf(stderr, "%s Error: %d\n", err_ptr,errno);
	exit(EXIT_FAILURE);
}

