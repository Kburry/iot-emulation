The following program was created for Carleton University's SYSC 4001 Operating Systems course (Fall 2015). It was written by Reid Cain-Mondoux and Noah Segal via peer programming.

Reid Cain-Mondoux: XXXYYYZZZ
Noah Segal: 100911661

This program emulates a network of devices connected via the Internet of Things (IOT). It consists of a Controller, Sensor(s), Actuator(s), and a Cloud.

Platform: Fedora (tested)

This program must be run via a terminal. There are two options:

1.0 Quick & Easy -- Demo
	1.1 Perform a "make clean" followed by a "make". This creates all required files.

	1.2 Execute main.c by typing ./main into the terminal

	1.3 Watch! :)

	1.4 Ok, step 1.3 should say that the Controller & Cloud will Run.
			Then, after two seconds, 2 Actuators and 2 sensors will be created.
			(Each device will appear in its own terminal window)

	1.5 The appropriate Sensor & Actuator will be paired.
			Sensor readings / actuator states will echo in their respective terminals.

	1.6 User can GET or PUT device information via the Cloud terminal
				Commands are as follows:
					GET sensor_name
					PUT actuator_name
				NOTE: GET / PUT must be in uppercase
							sensor/actuator_name must be the same case as when created

	1.7 To exit, press CTRL-C in the Controller terminal.

	1.8 A-bde, a-bde, a-bde that's all folks!

2. Long & Arduous -- Testing
	2.1 Perform a "make clean" followed by a "make". This creates all required files.

	2.2 Create the Controller by typing ./controller into a terminal window

	2.3 Create the Cloud by typing ./cloud into a NEW terminal window

	2.4 NOTE: Steps 2.5 & 2.6 can be swapped.
			NOTE: Ok, if you really want you can swap steps 2.2 - 2.6 (But whatever you do, do NOT swap step 2.4)

	2.5 Create an Actuator in a NEW terminal window. Actuator takes parameters:
			./actuator actuator_name sensor_name actuator_state

		Where:
			actuator_name = whatever you want
			sensor_name = name of the sensor it will pair with
			actuator_state = 1 or 0 (to start ON or OFF)

	2.6 Create a Sensor in a NEW terminal window. Sensor takes parameters:
			./sensor sensor_name actuator_threshold

		Where:
			sensor_name = name of the sensor (Actuator will pair with this)
			actuator_threshold: Crossing this threshold notifies the actuator

	2.7 User can GET or PUT device information via the Cloud terminal
				Commands are as follows:
					GET sensor_name
					PUT actuator_name

				NOTE: GET / PUT must be in uppercase
							sensor/actuator_name must be the same case as when created

	2.8 To exit, press CTRL-C in the Controller terminal.

	2.9 Stand up, take a deep breath in, stretch, exhale, then retake your seat.



ORIGINAL DOCUMENTATION:

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
