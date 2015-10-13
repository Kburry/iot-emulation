NOTE: Please set Tab Width to 2 for best readability. Thank you :)

The following program was created for Carleton University's SYSC 4001 Operating Systems course (Fall 2015). It was written by Reid Cain-Mondoux and Noah Segal via peer programming. This program emulates a network of devices connected via the Internet of Things (IOT). It consists of a Controller, Sensor(s), Actuator(s), and a Cloud.

Reid Cain-Mondoux: 100945700
Noah Segal: 100911661

Supported Platform: Fedora (tested on CB-5109 Lab Computers)


This program must be run via a terminal. There are two options:

1.0 Quick & Easy -- Demo
	1.1 Perform a "make clean" followed by a "make". This creates all required files.

	1.2 Execute main.c by typing ./main into the terminal

	1.3 Watch! :)

	1.4 Ok, step 1.3 should say that the Controller & Cloud will Run.
			After two seconds, 2 Actuators and 2 sensors will be created.
			(Each device will appear in its own terminal window)

	1.5 The appropriate Sensor & Actuator will be paired.
			Sensor Readings / Actuator State changes echo in their respective terminals.

	1.6 User can GET or PUT device information via the Cloud terminal.
			Commands are as follows:
				GET sensor_name
				PUT actuator_name
			NOTE: GET / PUT must be in uppercase
						sensor/actuator_name must be the same case as when created

	1.7 To exit, press CTRL-C in the Controller terminal.
			This shuts down Controller & all paired devices.
			Please shut down Controller BEFORE Cloud. Thank you :)
			Must independantly shut down Cloud (CTRL-C in its terminal)

	1.8 A-bde, a-bde, a-bde th...that's all folks!


2.0 Long & Arduous -- Testing
	2.1 Perform a "make clean" followed by a "make". This creates all required files.

	2.2 Create the Controller by typing ./controller into a terminal window

	2.3 Create the Cloud by typing ./cloud into a NEW terminal window

	2.4 NOTE: Steps 2.5 & 2.6 can be swapped.
			NOTE: Ok, if you really want you can swap steps 2.2 - 2.6 (But whatever you do, do NOT swap step 2.4)

	2.5 Create an Actuator in a NEW terminal window. Actuator takes parameters:
			./actuator actuator_name sensor_name actuator_state
			example: ./actuator AC thermometer 1

			Where:
				actuator_name = whatever you want
				sensor_name = name of the sensor it will pair with
				actuator_state = 1 or 0 (1 = ON or 0 = OFF)

	2.6 Create a Sensor in a NEW terminal window. Sensor takes parameters:
			./sensor sensor_name threshold
			example: ./sensor thermometer 20

		Where:
			sensor_name = name of the sensor (Actuator will pair with this)
			threshold: Crossing this threshold notifies the actuator

	2.7 User can GET or PUT device information via the Cloud terminal
				Commands are as follows:
					GET sensor_name
					PUT actuator_name
					
					example:
					GET thermometer
					PUT AC
				
				Output (in Cloud terminal):
					GET --> Sensor thermometer with Current Value 5, paired with Actuator AC
					PUT --> User turned ON Actuator AC

				NOTE1:	GET / PUT must be in uppercase
								sensor_name / actuator_name must be the same case as when created
							
				NOTE2:	Cloud terminal may update while user is inputting command.
								Don't worry! The Commands will still execute (assuming they are valid)

	2.8 To exit, press CTRL-C in the Controller terminal.
			This shuts down Controller & all devices.
			Please shut down Controller BEFORE Cloud. Thank you :)
			Must independantly shut down Cloud (CTRL-C in its terminal)

	2.9 Stand up, take a deep breath in, stretch, exhale, then retake your seat.


Included Files:
	controller.c
	sensor.c
	actuator.c
	cloud.c
	main.c
	message_struct.h
	Makefile
	README.md
	
	
	
Additional Information:

Art by Shanaka Dias
                    .==.
                   ()''()-.
        .---.       ;--; /
      .'_:___". _..'.  __'.
      |__ --==|'-''' \'...;
      [  ]  :[|       |---\
      |__| I=[|     .'    '.
      / / ____|     :       '._
     |-/.____.'      | :       :
snd /___\ /___\      '-'._----'

http://www.ascii-code.com/ascii-art/movies/star-wars.php
