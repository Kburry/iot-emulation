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
	UPDATE
}Dev_Command;

typedef struct {
	pid_t pid;
	char name[NAMESIZE];
	Dev_Type dev_type;
	Dev_Command command;
	int current_value;
}message_data_st;

typedef struct{
	long int message_type;
	message_data_st data;
}message_package_st;


