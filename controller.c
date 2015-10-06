#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/msg.h>
#include "message_struct.h"

int start_controller();
int get_device_index(device_st*, pid_t, int);

typedef struct {
	char sensor_name[NAMESIZE];		// if NULL, unknown sensor
	char actuator_name[NAMESIZE];	// if NULL, unknown actuator
	int sensor_pid;					// Is -1 if no device.
	int actuator_pid;				// Is -1 if no device.
	int threshold;					// Taken from SENSOR
	int is_on;						// Only for ACTUATOR
}device_st;

// Global Actuator Message: toggle ON/OFF
message_package_st toggle_act_message;
	message_data_st toggle_act_data {
		.pid = -1,
		.name = "Actuator-TOGGLE",
		.dev_type = ACTUATOR,
		.command = UPDATE,
		.current_value = -1
	};
	toggle_act_message.data = toggle_act_data;

int main(int argc, char argv[]){
	message_package_st received_message;
	message_data_st received_data;

	device_st devices[50] = {};
	int number_of_devices = 0;
	int msgid = start_controller();

	// Managing devices	
	while(1){
		printf("\nwaiting on message\n");
		if(msgrcv(msgid,(void *)&received_message,sizeof(received_message.data),0,0) == -1){
			fprintf(stderr, "Error: %d\n",errno);		
		}
		printf("message received\n");
		received_data = received_message.data;

		int device_index = get_device_index(*devices, received_data.pid, number_of_devices);

		// Add new device
		if (device_index == -1 && number_of_devices < 50) {
			device_index = number_of_devices - 1;
			number_of_devices++;
			devices[device_index].sensor_name = received_data.name;	

			// New SENSOR		
			if (received_data.dev_type == SENSOR) {
				devices[device_index].sensor_pid = received_data.pid;
				devices[device_index].threshold = received_data.current_value
			}
			// New ACTUATOR
			else if (received_data.dev_type == ACTUATOR) {
				devices[device_index].actuator_pid = received_data.pid;
				devices[device_index].is_on = received_data.current_value;
			}
		}
		// Device already stored --> Update
		else if (device_index >= 0) {
			if (received_data.dev_type == SENSOR) {
				// Turn OFF Actuator
				if (received_data.current_value >= devices[device_index].threshold 
					&& devices[device_index].is_on == ON) {
					devices[device_index].is_on = OFF;
					toggle_act_data.current_value = OFF;
					toggle_act_data.pid = devices[device_index].actuator_pid
					//toggle_act_message.key = toggle_act_data.pid
				}
			}
			else if (received_data.dev_type == ACTUATOR) {
			
			}
		}

		printf("%s:%d",received_data.name,received_data.pid);
	};
}

/**
 * Return message queue ID.
 **/
int start_controller(){
	int msgid = msgget((key_t) MSG_Q_KEY,0666 | IPC_CREAT);
	if (msgid == -1){
		fprintf(stderr,"Message get failed %d\n",errno);
		exit(EXIT_FAILURE);
	}
	//printf("%d\n",msgid);
	return msgid;
}

/**
 * Return index of device (specified by Process ID)
 **/
int get_device_index(device_st* devices, pid_t pid, int number_of_devices) {	
	for (int i = 0; i < number_of_devices; i++) {
		if (devices[i].sensor_pid == pid || devices[i].actuator_pid == pid)
			return i;
	}
	return -1;
}



//stopping()//make sure you stop all devices as well
//register_device()

