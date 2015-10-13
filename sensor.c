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

	printf("Sensor %s with PID: %d has Started\n\n", sensor_data.name, sensor_data.pid);
	running = 1;

	// Waiting for controller to start
	while ( (msgid = msgget((key_t) MSG_Q_KEY, 0666)) == -1) {}
	printf("Acknowledging that controller has started\n");

	// Send first message
	sensor_package.data = sensor_data;
	sensor_package.message_type = 1;
	if(msgsnd(msgid,(void *)&sensor_package,sizeof(sensor_package.data),0) == -1){
			fprintf(stderr, "Message Sent failed. Error: %d\n",errno);
			exit(EXIT_FAILURE);
	}
	sensor_data.command = UPDATE;

	// Send subsequent message(s)
	while(running) {
		// Randomly generated sensor range [-50, 50]
		sensor_data.current_value = (rand() % 100) - 50;
		sensor_package.data = sensor_data;
		printf("Sensor \"%s\" Value: %d\n",sensor_data.name, sensor_data.current_value);

		msgrcv(msgid, (void *) &sensor_package, sizeof(sensor_package.data), sensor_data.pid, IPC_NOWAIT);
		//Stop Everything
		if (sensor_data.command == STOP) {
			printf("Shut-down command received: Shutting down\n");
			exit(EXIT_SUCCESS);
		}

		if(msgsnd(msgid,(void *)&sensor_package,sizeof(sensor_package.data),0) == -1){
			printf("Controller not running: Shutting down\n");
			exit(EXIT_FAILURE);
		}

		// Check every 2 seconds
		sleep(2);
	}
}

void e_print(char * err_ptr){
	fprintf(stderr, "%s Error: %d\n",err_ptr,errno);
	exit(EXIT_FAILURE);
}

