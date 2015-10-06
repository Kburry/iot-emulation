#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


struct device_st {
	char sensor_name[NAMESIZE];
	char actuator_name[NAMESIZE];
	int sensor_pid; // Is -1 if no device.
	int actuator_pid; // Is -1 if no device.
	int threashold;
	int is_on; 
}

int main(int argc, char argv[]){

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
	return msgid;
}
stopping()//make sure you stop all devices as well
register_device()

