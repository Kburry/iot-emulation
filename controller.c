#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/msg.h>
#include "message_struct.h"



typedef struct {
	char sensor_name[NAMESIZE];		// if NULL, unknown sensor
	char actuator_name[NAMESIZE];	// if NULL, unknown actuator
	int sensor_pid;					// Is -1 if no device.
	int actuator_pid;				// Is -1 if no device.
	int threshold;					// Taken from SENSOR
	int is_on;						// Only for ACTUATOR
}group_st;

int start_controller();
int get_index_by_pid(group_st *devices, int number_of_groups, pid_t pid);
int get_index_by_name(group_st *devices, int number_of_groups, char *name);
void update_actuator(int msgid, group_st *devices, int index, int is_on);


// Global Actuator Message: toggle ON/OFF
message_package_st toggle_act_message;
message_data_st toggle_act_data = {
		.pid = -1,
		.name = "Actuator-TOGGLE",
		.dev_type = ACTUATOR,
		.command = UPDATE,
		.current_value = -1
};


int main(int argc, char argv[]){
	message_package_st received_message;
	message_data_st received_data;
	toggle_act_message.data = toggle_act_data;

	group_st devices[50] = {};
	int number_of_groups = 0;
	int msgid = start_controller();

	// Managing devices	
	while(1){
		if(msgrcv(msgid,(void *)&received_message,sizeof(received_message.data),0,0) == -1){
			fprintf(stderr, "Error: %d\n",errno);		
		}
		received_data = received_message.data;
		int device_index = get_index_by_pid(devices, number_of_groups,received_data.pid);
		int name_index = get_index_by_name(devices, number_of_groups, received_data.name);
		
		// Add new device --> No group exists
		if (device_index == -1 && name_index == -1 && number_of_groups < 50) {
			int new_index = number_of_groups;
			printf("\nNew Device\nGroup: %s  Threshold: %d\n\n",received_data.name, received_data.current_value);
			number_of_groups++;
			strcpy(devices[new_index].sensor_name, received_data.name);	

			// New SENSOR		
			if (received_data.dev_type == SENSOR) {
				devices[new_index].sensor_pid = received_data.pid;
				devices[new_index].threshold = received_data.current_value;
			}
			// New ACTUATOR
			else if (received_data.dev_type == ACTUATOR) {
				devices[new_index].actuator_pid = received_data.pid;
				devices[new_index].is_on = received_data.current_value;
			}
		}

		// Add new device --> Group already exists
		else if (device_index == -1 && name_index >= 0) {
			// New SENSOR		
			if (received_data.dev_type == SENSOR) {
				devices[name_index].sensor_pid = received_data.pid;
				devices[name_index].threshold = received_data.current_value;
				printf("Sensor added to \"%s\"\n", devices[name_index].sensor_name);
			}
			// New ACTUATOR
			else if (received_data.dev_type == ACTUATOR) {
				devices[name_index].actuator_pid = received_data.pid;
				devices[name_index].is_on = received_data.current_value;
				printf("Actuator added to \"%s\"\n", devices[name_index].sensor_name);
			}
		}

		// Device already stored --> Update
		else if (device_index >= 0) {
			if (received_data.dev_type == SENSOR) {
				
				// Check Threshold && Actuator status (Actuator must exist)
				if (devices[device_index].actuator_pid != 0) {


					if (received_data.current_value >= devices[device_index].threshold && devices[device_index].is_on == ON) {
						update_actuator(msgid, devices, device_index, OFF);
					}
					else if (received_data.current_value < devices[device_index].threshold && devices[device_index].is_on == OFF) {
						update_actuator(msgid, devices, device_index, ON);
					}
				}
			}
			else if (received_data.dev_type == ACTUATOR) {
			
			}
		}

		//printf("%s: %d\n",received_data.name,received_data.pid);
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
 * Return index of device (specified by Name)
 **/
int get_index_by_name(group_st *devices, int number_of_groups, char *name){
	for (int i = 0; i < number_of_groups; i++) {
		//printf("devices[i].pid: %d, pid: %d\n",devices[i].sensor_pid,pid);
		if (strcmp(name,devices[i].sensor_name)==0){
			//printf("pid is:%d\n",pid);
			return i;
		}
	}
	return -1;
}

/**
 *  Update Actuator --> Message the actuator, turning it ON/OFF
 **/
void update_actuator(int msgid, group_st *devices, int index, int is_on) {
	devices[index].is_on = is_on;
	toggle_act_data.current_value = is_on;
	toggle_act_data.pid = devices[index].actuator_pid;

	toggle_act_message.message_type = toggle_act_data.pid;
	toggle_act_message.data = toggle_act_data;
	if ( msgsnd(msgid, (void *) &toggle_act_message, sizeof(toggle_act_message.data), 0) == -1) {
		fprintf(stderr, "Message sent failed. Error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

/**
 * Return index of device (specified by Process ID)
 **/
int get_index_by_pid(group_st *devices, int number_of_groups, pid_t pid) {	
	for (int i = 0; i < number_of_groups; i++) {
		//printf("devices[i].pid: %d, pid: %d\n",devices[i].sensor_pid,pid);
		if (devices[i].sensor_pid == pid || devices[i].actuator_pid == pid){
			//printf("pid is:%d\n",pid);
			return i;
		}
	}
	return -1;
}


//stopping()//make sure you stop all devices as well
//register_device()

