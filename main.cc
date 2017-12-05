#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <fstream>
#include <SPICommInterface.h>

#include "epuck.hh"
#include "args.hh"

#define ON 1
#define OFF 0


char *g_name("robot A");
char *g_host("localhost");

int g_sensor_triggered_threshold(30);
int g_sensor_triggered_min_interval(50);
int g_sensor_triggered_counts(4);

uint32_t currentTime = 0;
uint32_t lastupdateTime = 0;

int userQuit = 0;
void signalHandler(int dummy);
void timerHandler(int dummy);
int timecount = 0;
int sensorcount[8] = {0};
int filecount = 0;

int main(int argc, char*argv[])
{
	parse_args(argc, argv);
	//set signal handler to capture "ctrl+c" event
	if (signal(SIGINT, signalHandler) == SIG_ERR)
	{
		printf("signal(2) failed while setting up for SIGINT");
		return -1;
	}

	if (signal(SIGTERM, signalHandler) == SIG_ERR)
	{
		printf("signal(2) failed while setting up for SIGTERM");
		return -1;
	}

	if (signal(SIGALRM, timerHandler)==SIG_ERR)
	{
		printf("signal(2) failed while setting up for SIGALRM");
		return -1;
	}

	Robot *robot=new Robot(g_name);

	if(!robot->Init())
	{
		printf("ERROR!!! Fail to initialise robot\n");
		return -1;
	}

	//set timer to be every 100 ms
	struct itimerval tick;
	memset(&tick, 0, sizeof(tick));
	tick.it_value.tv_sec = 0;
	tick.it_value.tv_usec = 10000;
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 10000;

	//set timer
	if (setitimer(ITIMER_REAL, &tick, NULL)){
		printf("Set timer failed!!\n");
	}


	SPICommInterface *SPI;  
	SPI = new SPICommInterface();
	//main loop

	while (userQuit != 1)
	{
		//block until timer event occurs
		while(currentTime==lastupdateTime)
		  usleep(5000);

		lastupdateTime = currentTime;

		//robot update
		//printf("timecount %d \n",timecount);
		robot->Update(currentTime);
		/*SPI->Run();
		SPI->SetSpeeds(800,800);*/


		//robot->Turnright();
		timecount ++;
		filecount++;

	}

	printf("clean up\n");
	robot->Stop();

	usleep(1000000);
		
	// robot->Log();

	delete robot;
	return 0;
}

void signalHandler(int dummy){
	printf("Ctrl+C captured, exit program!\n");
	// set_board_led(1, OFF);
	userQuit = 1;
}
void timerHandler(int dummy){
	currentTime++;
}
