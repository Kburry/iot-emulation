#include <sys/types.h>

#define MSG_Q_KEY 1234
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
	int sensor_pid;					// Is 0 if no device.
	int actuator_pid;				// Is 0 if no device.
	int threshold;					// Taken from SENSOR
	int is_on;						// Only for ACTUATOR
}group_st;


/**
 * Structure for the data that will update the parent process.
 **/
typedef struct {
	char sensor_name[NAMESIZE];
	char actuator_name[NAMESIZE];
	int sensor_value;
	int is_on;
}update_parent_st;

/**
 * Structure for the message that will update the parent process.
 * Contains update_parent_st.
 **/
typedef struct {
	long int message_type;
	update_parent_st data;
}update_parent_msg_st;




