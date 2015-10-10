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
#include "message_struct.h"

#define CTRL_FIFO_NAME "/tmp/ctrl_cloud_fifo"
#define USER_FIFO_NAME "/tmp/user_cloud_fifo"
#define BUFFER_SIZE PIPE_BUF


/**
 * Function declaration
 **/
int start_controller();
void stop_system(int msgid, group_st *devices, int index);
void update_actuator(int msgid, group_st *devices, int index, int is_on, int sensor_data);

int get_index_by_pid(group_st *devices, int number_of_groups, pid_t pid);
int get_index_by_name(group_st *devices, int number_of_groups, char *name);

void child();
void parent();

int pipe_fd;

// Actuator Message: used to toggle actuator ON/OFF
message_package_st toggle_act_message;
message_data_st toggle_act_data = {
	.pid = -1,
	.name = "Actuator-TOGGLE",
	.dev_type = ACTUATOR,
	.command = UPDATE,
	.current_value = -1
};

// Stop Message: used to signal that paired devices must stop
message_package_st stop_msg;
message_data_st stop_data = {
	.name = "Stop Command",
	.pid = -1,
	.dev_type = ACTUATOR,
	.command = STOP
};


int main(int argc, char argv[]){
	stop_msg.data = stop_data;

	if (access(CTRL_FIFO_NAME,F_OK) == -1) {
		printf("can't access fifo, making one\n");
		if (mkfifo(CTRL_FIFO_NAME,0777) != 0) {
			fprintf(stderr,"Could not create fifo: %s\n", CTRL_FIFO_NAME);
			exit(EXIT_FAILURE);
		} 
	}

	pipe_fd = open(CTRL_FIFO_NAME, O_WRONLY);
	printf("Process result %d\n", pipe_fd);
	
	
	int pid = fork();
	switch(pid) {
    case -1: // Failure
    	perror("fork failed");
    	exit(1);
		break;
    case 0: // Child
		child();
		exit(1);
		break;
	default:
		break;
	};
	parent();
	exit(1);
}

/**
 * Handle CTRL-C terminal input
 * Signal paired devices (sensors & actuators) to shut down
 **/
void sigint_parent_handler(int sig) {
	printf(" Caught SIGINT\n");
	stop_msg.message_type = 1;
	int msgid = msgget((key_t) MSG_Q_KEY, 0666);
	
	if ( msgsnd(msgid, (void *) &stop_msg, sizeof(stop_msg.data), 0) == -1 ) {
		fprintf(stderr, "Child Process not stopped\n");
	}
	(void)close(pipe_fd);
	exit(EXIT_SUCCESS);
}

void sigusr1_parent_handler(int sig){
	int msgid = msgget((key_t) MSG_Q_KEY, 0666);
	char buffer[BUFFER_SIZE+1];
	update_parent_msg_st message;
	
	if ( msgrcv(msgid, (void *) &message, sizeof(message.data), getpid(), 0) == -1 ) {
		fprintf(stderr, "Parent could not receive message from message queue\n");
	}
	update_parent_st message_data = message.data;
	
	if (message_data.is_on) {
		sprintf(buffer, "Sensor \"%s\" with value %d, turned ON %s", message_data.sensor_name, 
				message_data.sensor_value, message_data.actuator_name);
	} else {
		sprintf(buffer, "Sensor \"%s\" with value %d, turned OFF %s", message_data.sensor_name, 
				message_data.sensor_value, message_data.actuator_name);
	}
	//printf("%s\n",buffer);
	if ( write(pipe_fd,buffer,BUFFER_SIZE) == -1 ){
		fprintf(stderr,"write error on pipe\n");
		exit(EXIT_FAILURE);
	}
}

// Function required so child doesn't close from external interrupt
void sigint_child_handler(int sig) {}


/**
 *  Parent process
 **/
void parent(){
	
	//if (access(CTRL_FIFO_NAME,F_OK) == -1) {
	//	printf("can't access fifo, making one\n");
	//	if (mkfifo(CTRL_FIFO_NAME,0777) != 0) {
	//		fprintf(stderr,"Could not create fifo: %s\n", CTRL_FIFO_NAME);
	//		exit(EXIT_FAILURE);
	//	} 
	//}

	//printf("in parent\n");
	//pipe_fd = open(CTRL_FIFO_NAME, O_WRONLY);
	//printf("Process result %d\n", pipe_fd);
	
	struct sigaction parent_action;
	struct sigaction notification_action;
	
	parent_action.sa_handler = sigint_parent_handler;
	notification_action.sa_handler = sigusr1_parent_handler;
	
	sigemptyset(&parent_action.sa_mask);
	sigemptyset(&notification_action.sa_mask);

	parent_action.sa_flags = 0;
	notification_action.sa_flags = 0;
	
	sigaction(SIGINT, &parent_action, 0);
	sigaction(SIGUSR1, &notification_action, 0);
	
	while(1){};
}


/**
 *  Child process
 **/
void child(){
	int number_of_groups = 0;
	int msgid = start_controller();
	group_st devices[50] = {};
	
	message_package_st received_message;
	message_data_st received_data;
	
	toggle_act_message.data = toggle_act_data;
	
	// Signal Handler
	struct sigaction child_action;
	child_action.sa_handler = sigint_child_handler;
	sigemptyset(&child_action.sa_mask);
	child_action.sa_flags = 0;
	sigaction(SIGINT, &child_action, 0);

	// Device Management -- Sensors & Actuators	
	while(1){
		if ( msgrcv(msgid, (void *) &received_message, sizeof(received_message.data), 1, 0) == -1 ) {
			if (errno == EINTR) {
				printf("Caught external signal interrupt.\n");
				continue;
			} else {
				fprintf(stderr, "Message Received failed with Error: %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}
		received_data = received_message.data;
		int device_index = get_index_by_pid(devices, number_of_groups, received_data.pid);
		int name_index = get_index_by_name(devices, number_of_groups, received_data.name);
		
		// Stop everything --> Shut down Sensors, Actuators, and Controller
		if (received_data.command == STOP) {
			stop_system(msgid, devices, number_of_groups);
		}
		
		// Add new device --> No group exists
		else if (received_data.command == START && name_index == -1 && number_of_groups < 50){
		//else if (device_index == -1 && name_index == -1 && number_of_groups < 50) {
			int new_index = number_of_groups;
			number_of_groups++;
			
			printf("\nNew Device\nGroup: %s  Threshold: %d\n\n", received_data.name, received_data.current_value);
			strcpy(devices[new_index].sensor_name, received_data.name);	

			// New device is a SENSOR		
			if (received_data.dev_type == SENSOR) {
				devices[new_index].sensor_pid = received_data.pid;
				devices[new_index].threshold = received_data.current_value;
			}
			// New device is an ACTUATOR
			else if (received_data.dev_type == ACTUATOR) {
				devices[new_index].actuator_pid = received_data.pid;
				devices[new_index].is_on = received_data.current_value;
				strcpy(devices[new_index].actuator_name, received_data.actuator_name);
			}
		}

		// Add new device --> Pair device with previously added actuator/sensor
		else if(received_data.command == START && name_index >= 0){		
		//else if (device_index == -1 && name_index >= 0) {
			// New device is a SENSOR	
			if (received_data.dev_type == SENSOR) {
				devices[name_index].sensor_pid = received_data.pid;
				devices[name_index].threshold = received_data.current_value;
				printf("Sensor added to \"%s\"\n", devices[name_index].sensor_name);
			}
			// New device is an ACTUATOR
			else if (received_data.dev_type == ACTUATOR) {
				devices[name_index].actuator_pid = received_data.pid;
				devices[name_index].is_on = received_data.current_value;
				strcpy(devices[name_index].actuator_name,received_data.actuator_name);
				printf("Actuator added to \"%s\"\n", devices[name_index].sensor_name);
			}
		}

		// Update Device --> Device has already been added
		else if (device_index >= 0) {
			if (received_data.dev_type == SENSOR) {
			
				// Check Threshold and Actuator status (Actuator must exist)
				if (devices[device_index].actuator_pid != 0) {
					if (received_data.current_value >= devices[device_index].threshold 
						 && devices[device_index].is_on == ON) {
						update_actuator(msgid, devices, device_index, OFF, received_data.current_value);
					}
					else if (received_data.current_value < devices[device_index].threshold 
							  && devices[device_index].is_on == OFF) {
						update_actuator(msgid, devices, device_index, ON, received_data.current_value);
					}
				}
			}
		}
	}
}


/**
 * Return message queue ID.
 **/
int start_controller() {
	int msgid = msgget((key_t) MSG_Q_KEY,0666 | IPC_CREAT);
	if (msgid == -1){
		fprintf(stderr, "Message get failed %d\n", errno);
		exit(EXIT_FAILURE);
	}
	return msgid;
}

/**
 * Return index of device (specified by Name)
 **/
int get_index_by_name(group_st *devices, int number_of_groups, char *name) {
	for (int i = 0; i < number_of_groups; i++) {
		if (strcmp(name, devices[i].sensor_name) == 0) {
			return i;
		}
	}
	return -1;
}

/**
 * Return index of device (specified by Process ID)
 **/
int get_index_by_pid(group_st *devices, int number_of_groups, pid_t pid) {	
	for (int i = 0; i < number_of_groups; i++) {

		if (devices[i].sensor_pid == pid || devices[i].actuator_pid == pid){
			return i;
		}
	}
	return -1;
}

/**
 *  Update Actuator --> Send a message to the actuator turning it ON/OFF
 **/
void update_actuator(int msgid, group_st *devices, int index, int is_on, int sensor_data) {
	devices[index].is_on = is_on;
	
	toggle_act_data.current_value = is_on;
	toggle_act_data.pid = devices[index].actuator_pid;
	toggle_act_message.message_type = toggle_act_data.pid;

	strcpy(toggle_act_data.actuator_name, devices[index].actuator_name);
	toggle_act_message.data = toggle_act_data;

	if (is_on) {
		printf("%s threshold crossed. Turning ON\n", toggle_act_data.actuator_name);
	} else {
		printf("%s threshold crossed. Turning OFF\n", toggle_act_data.actuator_name);
	}

	if ( msgsnd(msgid, (void *) &toggle_act_message, sizeof(toggle_act_message.data), 0) == -1 ) {
		fprintf(stderr, "Message sent failed. Error: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	update_parent_st parent_data = {
		.sensor_name = "",
		.actuator_name = "",
		.sensor_value = sensor_data,
		.is_on = is_on
	};
	
	strcpy(parent_data.sensor_name, devices[index].sensor_name);
	strcpy(parent_data.actuator_name, devices[index].actuator_name);
	update_parent_msg_st message;
	message.message_type = getppid();
	message.data = parent_data;

	if ( msgsnd(msgid, (void *) &message, sizeof(message.data), 0) == -1 ) {
		fprintf(stderr, "Messaging parent failed %d\n", errno);
		exit(EXIT_FAILURE);
	}
	else {
		kill(getppid(), SIGUSR1);
	}
	
}


/**
 * Shut down all sensors and actuators
 */
void stop_system(int msgid, group_st *devices, int index) {
	for (int i = 0; i < index; i++) {
		//Sensor
		if (devices[i].sensor_pid != 0) {
			stop_msg.message_type = devices[i].sensor_pid;
			if ( msgsnd(msgid, (void *) &stop_msg, sizeof(stop_msg.data), 0) == -1 ) {
				fprintf(stderr, "Failed to send STOP message to Sensor: %s\n", devices[i].sensor_name);
			}
		}
		//Actuator
		if (devices[i].actuator_pid != 0) {
			stop_msg.message_type = devices[i].actuator_pid;
			if ( msgsnd(msgid, (void *) &stop_msg, sizeof(stop_msg.data), 0) == -1 ) {
				fprintf(stderr, "Failed to send STOP message to Actuator: %s\n", devices[i].actuator_name);
			}
		}
	}
	//Stop Everything
	if (msgctl(msgid, IPC_RMID, 0) == -1) {
        fprintf(stderr, "msgctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}



