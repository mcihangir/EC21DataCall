/*
 * main.c
 *
 *  Created on: Dec 21, 2021
 *      Author: quectel
 */

#include <stdio.h>
#include <pthread.h>


#include "ql_oe.h"
#include "ql_gpio.h"
#include "ql_mcm_sim.h"
#include "ql_mcm_nw.h"

#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include "ql_wwan_v2.h"

#include "mc_modem.h"
#include "mc_modem_at.h"
#include "mc_wifi.h"
#include "mc_gps.h"


static void callback_onAlarm(int param);
//
//
//
typedef enum {
	MC_INIT_MODEM = 0,
	MC_CHECK_SIM,
	MC_CHECK_SIGNAL,
	MC_CHECK_NETWORK,
	MC_SET_PDP,
	MC_DATA_CALL_START,
	MC_DATA_CALL_RESTART,
	MC_WIFI_START,
	MC_WAIT
} main_states;

typedef struct {
    int  taskID;
    main_states  mainState;
} main_task_info;

typedef enum {
	MC_SOCKET_CONNECT = 0,
	MC_SOCKET_CONNECTED,
	MC_SOCKET_SEND,
	MC_SOCKET_READ,
	MC_SOCKET_WAIT
} socket_states;

socket_states socketState;
//
//
//
static main_task_info mainTask;
static Enum_PinName m_GpioPin = PINNAME_PMU_GPIO1;	// EVB NET_STATUS
static nw_client_handle_type    handle_nw = 0;
static int dataCallState = 0;
/********************************************************************************************
 *
 *
 *
 *
 *
 */
void *gpsThread(void *param)
{
	int ret;
	static int cnt = 0;
	//
	//
	//read loc
	while(1)
	{
		printf("MC: GPS Thread:%d\n", cnt++);
		ret = MC_GPS_Read();
		sleep(3);
	}
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
void *socketThread(void *param)
{
	int ret;
	static int cnt = 0;
	int socket_desc;
	struct sockaddr_in server;
	char *message;
	int socketError;


	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("MC: Could not create socket\n");
	}
	//
	server.sin_addr.s_addr = inet_addr("188.38.161.39");
	server.sin_family = AF_INET;
	server.sin_port = htons( 40000 );

	socketState = MC_SOCKET_CONNECT;

	//
	//
	//
	while(1)
	{
		printf("MC: Socket Thread:%d\n", cnt++);

		if( dataCallState == 1)
		{
			switch ( socketState )
			{
			case MC_SOCKET_CONNECT:
				//Connect to remote server
				if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
				{
					printf("MC: Client Socket Connect Error...\n");
				}
				else
				{
					printf("MC: Client Socket Connected...\n");
					socketError = 0;
					socketState = MC_SOCKET_SEND;
				}
				break;
			case MC_SOCKET_SEND:
				//Send
				//Send some data
				message = "Socket Test\n";
				if( send(socket_desc , message , strlen(message) , 0) < 0)
				{
					printf("MC: Client Socket Send Failed...\n");
					socketError++;
				}
				else
				{
					printf("MC: Client Socket Sent the message...\n");
				}
				if ( socketError > 3 )
				{
					close (socket_desc);
					socketState = MC_SOCKET_CONNECT;
				}

				break;
			default:
				break;
			}

		}
		sleep(3);
	}
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
static int initPeripheral(void)
{
	int ret;
	//
	//
	//Init GPIOs
	ret = Ql_GPIO_Init(m_GpioPin, PINDIRECTION_OUT, PINLEVEL_HIGH, PINPULLSEL_DISABLE);
	printf("< Init GPIO: pin=%d, dir=%d, level=%d, ret=%d >\n", m_GpioPin, PINDIRECTION_OUT, PINLEVEL_HIGH, ret);
	return ret;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
static int waitStateMachine(void)//main_task_info *stateId)
{
	int cmdIdx = 100;

    printf("0 = Exit\n");
    printf("1 = Reset\n");
    printf("2 = WiFi Stop\n");
    printf("3 = WiFi Start\n");
    printf("4 = CFUN=0\n");



    printf("please input command index:\n");
    //scanf("%d", &cmdIdx);
    //cmdIdx = getch();
    switch (cmdIdx)
    {
    case 0:
    	MC_WiFi_Stop();
    	MC_Data_Call_Stop();
    	QL_MCM_NW_Client_Deinit(handle_nw);
    	MC_CFUN(0);
    	return -1;
    	break;

    case 1:
    	//stateId->mainState = MC_INIT_MODEM;
		mainTask.mainState = MC_INIT_MODEM;
		return 0;
		break;

    case 2:
    	MC_WiFi_Stop();
		return 0;
		break;

    case 3:
    	MC_WiFi_Start();
		return 0;
		break;

    case 4:
    	MC_CFUN(0);
		return 0;
		break;

    default:
		return 0;
    	break;
    }
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */

int main(int argc, char* argv[])
{
	int ret;
	int stateCounter = 0;
    pthread_t gpsth1;
    pthread_t socketHandler;

	printf("MC: QuecOpen v10...\r\n");

	//
	//
	//Set up for Initialization
	mainTask.taskID = 1;
	mainTask.mainState = MC_INIT_MODEM;

	ret = initPeripheral();
	printf("MC: Peripreral init: %d\n, ret");

	ret = MC_GPS_Init();

	signal(SIGALRM, callback_onAlarm);
	pthread_create(&gpsth1, NULL, gpsThread, NULL);
	pthread_create(&socketHandler, NULL, socketThread, NULL);

	//
	//
	//
	while(1)
	{
		switch(mainTask.mainState)
		{
		case MC_INIT_MODEM:
			alarm(0);
			dataCallState = 0;

			ret = MC_Modem_Init(&handle_nw);
			if (ret < 0)
			{
				printf("MC: Modem Init Fail!\n");
				sleep(1);
				break;
			}
			stateCounter = 0;
			mainTask.mainState = MC_CHECK_SIM;
			break;

		case MC_CHECK_SIM:
			ret = MC_Check_SIM();
			if (ret < 0)
			{
				printf("MC: Modem SIM Fail!\n");
				if( stateCounter++ > 10 )
				{
					mainTask.mainState = MC_INIT_MODEM;
					//MC_CFUN_Reset();
				}
				sleep(1);
				break;
			}
			stateCounter = 0;
			mainTask.mainState = MC_CHECK_SIGNAL;
			break;

		case MC_CHECK_SIGNAL:
			ret = MC_Check_Signal(handle_nw);
			if (ret < 0)
			{
				printf("MC: Modem No Signal!\n");
				if( stateCounter++ > 10 )
				{
					mainTask.mainState = MC_CHECK_SIM;
				}
				sleep(1);
				break;
			}
			stateCounter = 0;

			mainTask.mainState = MC_CHECK_NETWORK;
			break;

		case MC_CHECK_NETWORK:
			ret = MC_Check_Netwrok(handle_nw);
			if (ret < 0)
			{
				printf("MC: Modem Registration Failed!\n");
				if( stateCounter++ > 10 )
				{
					mainTask.mainState = MC_INIT_MODEM;
				}
				sleep(1);
				break;
			}
			stateCounter = 0;
			mainTask.mainState = MC_DATA_CALL_START;
			break;

		case MC_DATA_CALL_START:
			ret = MC_Data_Call_Start();//handle_nw);
			if (ret < 0)
			{
				printf("MC: Modem Data Call Fail!\n");
				if( stateCounter++ > 10 )
				{
					mainTask.mainState = MC_CHECK_NETWORK;
				}
				stateCounter = 0;
				sleep(1);
				break;
			}
			printf("MC: Modem Data Call Established!\n");
			stateCounter = 0;
			mainTask.mainState = MC_WIFI_START;
			alarm(1);
			dataCallState = 1;
			break;

		case MC_DATA_CALL_RESTART:
			dataCallState = 0;
			ret = MC_Data_Call_Stop();
			ret = MC_Data_Call_Start();//handle_nw);
			if (ret < 0)
			{
				printf("MC: Modem Data Call Fail!\n");
				if( stateCounter++ > 10 )
				{
					mainTask.mainState = MC_CHECK_NETWORK;
				}
				stateCounter = 0;
				sleep(1);
				break;
			}
			printf("MC: Modem Data Call Established!\n");
			stateCounter = 0;
			mainTask.mainState = MC_WAIT;
			dataCallState = 1;
			break;

		case MC_WIFI_START:
			ret = MC_WiFi_Start();
			if (ret < 0)
			{
				printf("MC: WiFi Fail!\n");
				if( stateCounter++ > 10 )
				{
					mainTask.mainState = MC_INIT_MODEM;
				}
				sleep(1);
				break;
			}
			printf("MC: WiFi Start!\n");
			stateCounter = 0;
			mainTask.mainState = MC_WAIT;
			break;

		default:
			if( waitStateMachine() < 0 )
			{
				return 0;
			}
			sleep(1);
			break;
		}
	}


	printf("End\r\n");
	return 0;
}
/********************************************************************************************
 *
 *
 *
 *
 *
 */
void callback_onAlarm(int param)
{
	int ret;

	ret = MC_Check_Netwrok(handle_nw);
	if (ret < 0)
	{
		printf("MC: Modem Registration Failed!\n");
		mainTask.mainState = MC_CHECK_SIM;
		alarm(0);
		sleep(1);
		return;
	}
	/*
	int lvl = Ql_GPIO_GetLevel(m_GpioPin);
	if (lvl < 0)
	{
		printf("< fail to read pin level >\n");
		return;
	}
	if (1 == lvl)
	{
		Ql_GPIO_SetLevel(m_GpioPin, PINLEVEL_LOW);
		printf("< Pull pin level to low >\n");
	}else{
		Ql_GPIO_SetLevel(m_GpioPin, PINLEVEL_HIGH);
		printf("< Pull pin level to high >\n");
	}
*/
	alarm(1);
}
