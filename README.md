one message queue for devices and controller
-server should read all message coming in with a certain long int
-connection to controller established through messsage queue
-devices should stop when message received is "stop".
-devices first message should be it command-line arguments and its pid.(monitors should receive messages from it pid)
-sensor monitor a task every 2 seconds and sends results to the controller.(sensor probably can receive a stop command and change threshold from controller. it expects a echo from the controller. (controller controls threshold?)
-actuator should receive a on or off command.
-if on is sent prints a statement.
	tempsense templivingroom 22
./sensor sensor_name threshold
./actuator actuator_name sensor_name on/off(optional default=off)
ideas:
struct for sensor actuator controller can send this to parent maybe?
	name		(char[]) keyword to make a sensor and actuator talk to each other
	sensor_pid  	(int) set to -1 if no sensor assigned used for message queue
	actuator_pid 	(int) set to -1 if no actuator assigned used for message queue
	threshold 	(int) first message from sensor
	is_on		(int) set to 1 if on, 0 if off

struct for message queue
	pid
	char name[25]
	char device_type
	int threashold
	int current_value;

controller switch case for parent and child
