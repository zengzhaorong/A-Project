#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include "socket_server.h"

#define DEFAULT_SERVER_PORT 		9100
#define MAX_LISTEN_NUM 				5

static struct serverInfo server_info;

int server_init(struct serverInfo *server, int port)
{
	int ret;

	memset(server, 0, sizeof(struct serverInfo));

	server->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(server->fd < 0)
	{
		ret = -1;
		goto ERR_0;
	}

	server->srv_addr.sin_family = AF_INET;
	server->srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server->srv_addr.sin_port = htons(port);

	ret = bind(server->fd, (struct sockaddr *)&server->srv_addr, sizeof(server->srv_addr));
	if(ret != 0)
	{
		ret = -2;
		goto ERR_1;
	}

	ret = listen(server->fd, MAX_LISTEN_NUM);
	if(ret != 0)
	{
		ret = -3;
		goto ERR_2;
	}

	return 0;

ERR_2:
	
ERR_1:
	close(server->fd);

ERR_0:
	
	return ret;
}

void server_deinit(void)
{
	
}

void *socket_cli_handle_thread(void *arg)
{
	struct clientInfo *client = (struct clientInfo *)arg;
	char buf[64] = {0};
	int ret;

	printf("%s %d: enter ++\n", __FUNCTION__, __LINE__);

	while(1)
	{
		memset(buf, 0, sizeof(buf));
		ret = recv(client->fd, buf, sizeof(buf), 0);
		if(ret > 0)
			printf("recv[%d]: %s\n", ret, buf);
	}

}

void *socket_srv_listen_thread(void *arg)
{
	struct serverInfo *server = &server_info;
	struct sockaddr_in cli_addr;
	pthread_t tid;
	int tmpSock;
	int tmpLen;
	int ret;

	ret = server_init(server, DEFAULT_SERVER_PORT);
	if(ret != 0)
	{
		return NULL;
	}

	while(1)
	{
	
		// 暂只测试连接1个client
		if(server->client_cnt >= MAX_CLIENT_NUM)
		{
			sleep(3);
			continue;
		}
		
		memset(&cli_addr, 0, sizeof(struct sockaddr_in));
		tmpSock = accept(server->fd, (struct sockaddr *)&cli_addr, (socklen_t *)&tmpLen);
		if(tmpSock < 0)
			continue;
		printf("%s %d: *************** accept socket success, sock fd: %d ...\n", __FUNCTION__, __LINE__, tmpSock);

		server->client_cnt ++;
		server->client[server->client_cnt-1].fd = tmpSock;
		memcpy(&server->client[server->client_cnt-1].addr, &cli_addr, sizeof(struct sockaddr_in));
		
		ret = pthread_create(&tid, NULL, socket_cli_handle_thread, &server->client[server->client_cnt-1]);
		if(ret != 0)
		{
			printf("ERROR: %s %d: pthread_create failed !!!\n", __FUNCTION__, __LINE__);
		}

	}

	server_deinit();

}


int start_socket_server_task(void)
{
	pthread_t tid;
	int ret;

	ret = pthread_create(&tid, NULL, socket_srv_listen_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}

	return 0;
}





