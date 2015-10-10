all:	sensor actuator controller cloud

sensor.c: message_struct.h
actuator.c: message_struct.h
controller.c: message_struct.h

clean:
	rm -f sensor actuator controller cloud
	
