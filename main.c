#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * This program is responsible for demoing IOT emulation
 * Note: please read the README file to run without this program.
 **/
 
int main(int argc, char *argv[])
{
	system("gnome-terminal -e ./controller");
	
	system("gnome-terminal -e ./cloud");
	system("sleep 2");
	system("gnome-terminal -e \"./sensor thermometer 25\"");
	system("gnome-terminal -e \"./actuator Heater thermometer 1\"");
	
	system("gnome-terminal -e \"./actuator Tardis DalekDetector 0\"");
	system("gnome-terminal -e \"./sensor DalekDetector  1\"");
	
	return 0;
}
