#include <sys/types.h>

#define MSG_Q_KEY 1234
#define SRV_Q_KEY 1
#define NAMESIZE 25

typedef enum {
	sensor,
	actuator
}Dev_Type;

typedef struct {
	pid_t pid;
	char name[NAMESIZE];
	Dev_Type device_type;
	int threshold;
	int current_value;
}message_data_st;

typedef struct{
	long int message_type;
	message_data_st data;
}message_package_st;


