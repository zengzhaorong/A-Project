#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include "socket_server.h"

#define DEFAULT_SERVER_PORT 		9100
#define MAX_LISTEN_NUM 				5
#define RECV_BUFFER_SIZE			(10*1024)

static struct serverInfo server_info;

static char tmpBuf[1024];


int server_0x03_heartbeat(uint8_t *data, int len)
{
	time_t time;

	proto_0x03_dataAnaly(data, len, &time);
	printf("%s: time: %ld\n", __FUNCTION__, time);

	return 0;
}

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

int server_sendData(int sodkfd, uint8_t *data, int len)
{
	int ret;

	if(data == NULL)
		return -1;
	
	ret = send(sodkfd, data, len, 0);

	return ret;
}

int server_recvData(struct clientInfo *client)
{
	int len;

	if(client == NULL)
		return -1;

	memset(tmpBuf, 0, sizeof(tmpBuf));
	len = recv(client->fd, tmpBuf, sizeof(tmpBuf), 0);
	//printf("server recv %d bytes.\n", len);

	len = ringbuf_write(&client->recvRingBuf, tmpBuf, len);

	return len;
}

int server_protoAnaly(uint8_t *pack, char len)
{
	uint8_t seq = 0, cmd = 0;
	int data_len = 0;
	uint8_t *data = 0;

	if(pack==NULL || len<=0)
		return -1;

	proto_analyPacket(pack, len, &seq, &cmd, &data_len, &data);
	printf("%s: cmd: 0x%02x, seq: %d, len: %d\n", __FUNCTION__, cmd, seq, len);

	switch(cmd)
	{
		case 0x01:
			break;

		case 0x02:
			break;

		case 0x03:
			server_0x03_heartbeat(data, len);
			break;

		default:
			printf("ERROR: protocol cmd[0x%02x] not exist!\n", cmd);
			break;
	}

	return 0;
}

int server_protoHandle(struct clientInfo *client)
{
	int ret;

	if(client == NULL)
		return -1;

	ret = server_recvData(client);

	ret = proto_detectPack(&client->recvRingBuf, &client->detectInfo, client->packBuf, \
							sizeof(client->packBuf), &client->packLen);
	if(ret == 0)
	{
		server_protoAnaly(client->packBuf, client->packLen);
	}

	return 0;
}

void *socket_handle_thread(void *arg)
{
	struct clientInfo *client = (struct clientInfo *)arg;
	int ret;

	printf("%s %d: enter ++\n", __FUNCTION__, __LINE__);

	ret = ringbuf_init(&client->recvRingBuf, RECV_BUFFER_SIZE);
	if(ret < 0)
		return NULL;

	while(1)
	{
		server_protoHandle(client);
	}

	ringbuf_deinit(&client->recvRingBuf);
}

void *socket_listen_thread(void *arg)
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
		
		ret = pthread_create(&tid, NULL, socket_handle_thread, &server->client[server->client_cnt-1]);
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

	ret = pthread_create(&tid, NULL, socket_listen_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}

	return 0;
}





