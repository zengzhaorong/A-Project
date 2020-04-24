#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "socket_client.h"


#define DEFAULT_SERVER_IP			"127.0.0.1"
#define DEFAULT_SERVER_PORT			9100

struct clientInfo client_info;


int client_init(struct clientInfo *client, char *srv_ip, int srv_port)
{
	int ret;

	memset(client, 0, sizeof(struct clientInfo));

	client->state = STATE_DISCONNECT;

	client->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(client->fd < 0)
	{
		ret = -1;
		goto ERR_0;
	}

	client->srv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, srv_ip, &client->srv_addr.sin_addr);
	client->srv_addr.sin_port = htons(srv_port);

	return 0;

ERR_0:

	return ret;
}

void client_deinit(void)
{
	
}

void *socket_client_thread(void *arg)
{
	struct clientInfo *client = &client_info;
	char buf[64] = "hello server";
	int ret;

	ret = client_init(client, DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT);
	if(ret != 0)
	{
		return NULL;
	}

	while(1)
	{
		switch (client->state)
		{
			case STATE_DISABLE:
				printf("%s %d: state STATE_DISABLE ...\n", __FUNCTION__, __LINE__);
				break;

			case STATE_DISCONNECT:
				printf("%s %d: state STATE_DISCONNECT ...\n", __FUNCTION__, __LINE__);
				ret = connect(client->fd, (struct sockaddr *)&client->srv_addr, sizeof(client->srv_addr));
				if(ret == 0)
				{
					client->state = STATE_CONNECTED;
					printf("********** socket connect successfully.\n");
				}
				
				break;

			case STATE_CONNECTED:
				ret = send(client->fd, buf, strlen(buf), 0);
				printf("send[%d]: %s\n", ret, buf);
				sleep(3);
				break;

			default:
				printf("%s %d: state ERROR !!!\n", __FUNCTION__, __LINE__);
				break;
		}
		
		sleep(1);
	}

	client_deinit();

}


int start_socket_client_task(void)
{

	pthread_t tid;
	int ret;

	ret = pthread_create(&tid, NULL, socket_client_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}


	return 0;
}





