#include "message_struct.h"




/**
 * Function declaration
 **/
int start_controller();
void stop_system(int msgid, group_st *devices, int index);

void update_actuator(int msgid, group_st *devices, int index, int is_on, int sensor_data);
void toggle_actuator(int msgid, group_st *devices, int index, int is_on);
void notify_parent(int msgid, char *message_to_send);

int get_index_by_pid(group_st *devices, int number_of_groups, pid_t pid);
int get_index_by_name(group_st *devices, int number_of_groups, char *name, Dev_Type device_type);

void send_cloud_message(char *command, char *device);

void child();
void parent();

int send_pipe_fd;

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

	printf("Controller has Started\n");

	if (access(CTRL_FIFO_NAME,F_OK) == -1) {
		printf("can't access fifo, making one\n");
		if (mkfifo(CTRL_FIFO_NAME,0777) != 0) {
			fprintf(stderr,"Could not create fifo: %s\n", CTRL_FIFO_NAME);
			exit(EXIT_FAILURE);
		} 
	}

	send_pipe_fd = open(CTRL_FIFO_NAME, O_WRONLY);	
	
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
	(void)close(send_pipe_fd);
	exit(EXIT_SUCCESS);
}

void sigusr1_parent_handler(int sig){
	int msgid = msgget((key_t) MSG_Q_KEY, 0666);
	char buffer[BUFFER_SIZE+1];
	update_parent_msg_st message;
	
	if ( msgrcv(msgid, (void *) &message, sizeof(message.parent_msg), getpid(), 0) == -1 ) {
		fprintf(stderr, "Parent could not receive message from message queue\n");
	}
	
	strcpy(buffer, message.parent_msg);
	
	if ( write(send_pipe_fd, buffer, BUFFER_SIZE) == -1 ){
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
	
	int user_cmnd;
	int receive_pipe_fd;
	char receive_buffer[BUFFER_SIZE];
	char *test;
	char *word;
	char cloud_request[2][50];
	int number_of_words = 0;

	struct sigaction parent_action;
	struct sigaction notification_action;

	if (access(USER_FIFO_NAME,F_OK) == -1) {
		if (mkfifo(USER_FIFO_NAME,0777) != 0) {
			fprintf(stderr,"Could not create fifo: %s\n", USER_FIFO_NAME);
			exit(EXIT_FAILURE);
		} 
	}
	
	receive_pipe_fd = open(USER_FIFO_NAME, O_RDONLY);
	
	parent_action.sa_handler = sigint_parent_handler;
	notification_action.sa_handler = sigusr1_parent_handler;
	
	sigemptyset(&parent_action.sa_mask);
	sigemptyset(&notification_action.sa_mask);

	parent_action.sa_flags = 0;
	notification_action.sa_flags = 0;
	
	sigaction(SIGINT, &parent_action, 0);
	sigaction(SIGUSR1, &notification_action, 0);
	
	while(1) {
		user_cmnd = read(receive_pipe_fd, receive_buffer, BUFFER_SIZE);
		if(user_cmnd > 0){
			test = strdup(receive_buffer);
			word = test;
			number_of_words = 0;
			while( (word = strsep(&test, " ")) != NULL && number_of_words < 2 ) {
				strcpy(cloud_request[number_of_words], word); 
				number_of_words++;
			}
			if ( (number_of_words == 2) && (word == NULL) )  {
				send_cloud_message(cloud_request[0], cloud_request[1]);
			}
		}
	}
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
				printf("\nCaught external signal interrupt.\n");
			if (errno == EINTR) {
				continue;
			} else {
				fprintf(stderr, "Message Received failed with Error: %d\n", errno);
				exit(EXIT_FAILURE);
			}
		}
		received_data = received_message.data;
		int device_index = get_index_by_pid(devices, number_of_groups, received_data.pid);
		int name_index = get_index_by_name(devices, number_of_groups, received_data.name, SENSOR);
		
		// Stop everything --> Shut down Sensors, Actuators, and Controller
		if (received_data.command == STOP) {
			stop_system(msgid, devices, number_of_groups);
		}
		
		// Add new device --> No group exists
		else if (received_data.command == START && name_index == -1 && number_of_groups < 50){
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
			// New device is a SENSOR	
			if (received_data.dev_type == SENSOR) {
				devices[name_index].sensor_pid = received_data.pid;
				devices[name_index].threshold = received_data.current_value;
				printf("Sensor added to \"%s\"\n\n", devices[name_index].sensor_name);
			}
			// New device is an ACTUATOR
			else if (received_data.dev_type == ACTUATOR) {
				devices[name_index].actuator_pid = received_data.pid;
				devices[name_index].is_on = received_data.current_value;
				strcpy(devices[name_index].actuator_name,received_data.actuator_name);
				printf("Actuator added to \"%s\"\n\n", devices[name_index].sensor_name);
			}
		}

		// Update Device --> Device has already been added
		else if (received_data.command == UPDATE && device_index >= 0) {
			if (received_data.dev_type == SENSOR) {
				devices[device_index].sensor_value = received_data.current_value;
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

		else if (received_data.command == GET && name_index != -1) {
			char get_cmnd_msg[BUFFER_SIZE];
			
			if (strcmp(devices[name_index].actuator_name, "\0")) {
				sprintf(get_cmnd_msg, "\nSensor %s with Current Value %d, paired with Actuator %s\n",
						devices[name_index].sensor_name, 
						devices[name_index].sensor_value, 
						devices[name_index].actuator_name
					);
				notify_parent(msgid, get_cmnd_msg);
			}
		}
		
		else if (received_data.command == PUT) {
			char put_cmnd_msg[BUFFER_SIZE];
			int actuator_index = get_index_by_name(devices, number_of_groups, 
													received_data.actuator_name, ACTUATOR);
			if(actuator_index != -1){
				if (devices[actuator_index].is_on) {
					sprintf(put_cmnd_msg, "\nUser turned OFF Actuator %s\n", 
							devices[actuator_index].actuator_name);
					printf("\nUser turned OFF Actuator %s\n", devices[actuator_index].actuator_name);
					devices[actuator_index].is_on = OFF;	
					toggle_actuator(msgid, devices, actuator_index, OFF);
				} else  {
					sprintf(put_cmnd_msg, "\nUser turned ON Actuator %s\n", 
							devices[actuator_index].actuator_name);
					printf("\nUser turned ON Actuator %s\n", devices[actuator_index].actuator_name);
					devices[actuator_index].is_on = ON;
					
					toggle_actuator(msgid, devices, actuator_index, ON);
				}
				notify_parent(msgid, put_cmnd_msg);
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
int get_index_by_name(group_st *devices, int number_of_groups, char *name, Dev_Type device_type) {
	if (device_type == SENSOR) {
		for (int i = 0; i < number_of_groups; i++) {
			if (strcmp(name, devices[i].sensor_name) == 0) {
				return i;
			}
		}
	}
	else if (device_type == ACTUATOR) {

		for (int i = 0; i < number_of_groups; i++) {
			if (strcmp(name, devices[i].actuator_name) == 0) {
				return i;
			}
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
 * Notify the parent
 **/
void notify_parent(int msgid, char *message_to_send) {
	update_parent_msg_st message;
	message.message_type = getppid();
	
	strcpy(message.parent_msg, message_to_send);
	
	if ( msgsnd(msgid, (void *) &message, sizeof(message.parent_msg), 0) == -1 ) {
		fprintf(stderr, "Messaging parent failed %d\n", errno);
		exit(EXIT_FAILURE);
	}
	else {
		kill(getppid(), SIGUSR1);
	}

}

/**
 *  Update Actuator --> Send a message to the actuator turning it ON/OFF
 **/
void update_actuator(int msgid, group_st *devices, int index, int is_on, int sensor_data) {
	char message_to_parent[BUFFER_SIZE];
	devices[index].is_on = is_on;
	
	if (is_on) {
		printf("%s threshold crossed. Turning ON\n", devices[index].actuator_name);
		sprintf(message_to_parent, "Sensor \"%s\" with value %d, turned ON %s", 
				devices[index].sensor_name, sensor_data, devices[index].actuator_name);

	} else {
		printf("%s threshold crossed. Turning OFF\n", devices[index].actuator_name);
		sprintf(message_to_parent, "Sensor \"%s\" with value %d, turned OFF %s", 
				devices[index].sensor_name, sensor_data, devices[index].actuator_name);

	}
	
	toggle_actuator(msgid, devices, index, is_on);
	notify_parent(msgid, message_to_parent);
	
}

/**
 * Toggle the Actuator ON/OFF
 **/
void toggle_actuator(int msgid, group_st *devices, int index, int is_on) {

	toggle_act_data.current_value = is_on;
	toggle_act_data.pid = devices[index].actuator_pid;
	toggle_act_message.message_type = devices[index].actuator_pid;

	strcpy(toggle_act_data.actuator_name, devices[index].actuator_name);
	toggle_act_message.data = toggle_act_data;
	
	if ( msgsnd(msgid, (void *) &toggle_act_message, sizeof(toggle_act_message.data), 0) == -1 ) {
		fprintf(stderr, "Message sent failed. Error: %d\n", errno);
		exit(EXIT_FAILURE);
	}
}

/**
 *
 */
void send_cloud_message(char *command, char *device) {
	message_data_st cloud_data;
	message_package_st cloud_message;
	
	// GET
	if (strcmp(command, "GET") == 0) {
		cloud_data.command = GET;
		strcpy(cloud_data.name, device);
		
		cloud_message.message_type = 1;
		cloud_message.data = cloud_data;
		
		int msgid = msgget((key_t) MSG_Q_KEY, 0666);
		if (msgsnd(msgid, (void *) &cloud_message, sizeof(cloud_message.data), 0) == -1) {
			fprintf(stderr, "Message Send failed\n", errno);
		}
		
	}
	// PUT
	else if (strcmp(command, "PUT") == 0) {
		cloud_data.command = PUT;
		strcpy(cloud_data.actuator_name, device);
		
		cloud_message.message_type = 1;
		cloud_message.data = cloud_data;
		
		int msgid = msgget((key_t) MSG_Q_KEY, 0666);
		if (msgsnd(msgid, (void *) &cloud_message, sizeof(cloud_message.data), 0) == -1) {
			fprintf(stderr, "Message Send failed\n", errno);
		}
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



