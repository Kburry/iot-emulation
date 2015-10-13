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

#define CTRL_FIFO_NAME "/tmp/ctrl_cloud_fifo"
#define USER_FIFO_NAME "/tmp/user_cloud_fifo"
#define BUFFER_SIZE PIPE_BUF

#define MSG_Q_KEY 1111
#define SRV_Q_KEY 1
#define NAMESIZE 25

#define ON 1
#define OFF 0

typedef enum {
	SENSOR,
	ACTUATOR
}Dev_Type;

typedef enum {
	START,
	STOP,
	UPDATE,
	GET,
	PUT
}Dev_Command;


/**
 * Structure for data to be sent between devices and controller
 **/
typedef struct {
	pid_t pid;
	char name[NAMESIZE];
	char actuator_name[NAMESIZE];
	Dev_Type dev_type;
	Dev_Command command;
	int current_value;
}message_data_st;

/**
 * Structure for message to be sent between devices and controller.
 * Contains message_data_st.
 **/
typedef struct{
	long int message_type;
	message_data_st data;
}message_package_st;


/**
 * Structure responsible for grouping actuators and sensors.
 **/
typedef struct {
	char sensor_name[NAMESIZE];		// if NULL, unknown sensor
	char actuator_name[NAMESIZE];	// if NULL, unknown actuator
	int sensor_pid;					// Is 0 if no device
	int actuator_pid;				// Is 0 if no device
	int threshold;					// Taken from SENSOR
	int sensor_value;				// Taken from SENSOR
	int is_on;						// Only for ACTUATOR
}group_st;


/**
 * Structure for the message that will update the parent process.

 **/
typedef struct {
	long int message_type;
	char parent_msg[BUFFER_SIZE];
}update_parent_msg_st;




