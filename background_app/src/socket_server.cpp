#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include "opencv_face_process.h"
#include "socket_server.h"


/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


#define DEFAULT_SERVER_PORT 		9100
#define MAX_LISTEN_NUM 				5
#define RECV_BUFFER_SIZE			(1 *1024 *1024)
#define SEND_BUFFER_SIZE			(1 *1024 *1024)

static struct serverInfo server_info;

static uint8_t tmpBuf[1 *1024 *1024] = {0};
static uint8_t ack_buf[1 *1024 *1024] = {0};
static int tmpLen = 0;

extern struct clientInfo *face_client;

int server_0x03_heartbeat(uint8_t *data, int len, uint8_t *ack_data, int size, int *ack_len)
{
	time_t tmpTime;
	int tmplen = 0;
	int ret;

	if(data==NULL || len<=0)
		return -1;

	/* request part */
	proto_0x03_dataAnaly(data, len, PROTO_REQ, &tmpTime, NULL);
	printf("%s: time: %ld\n", __FUNCTION__, tmpTime);

	/* ack part */
	if(ack_data==NULL || size<4+4 || ack_len==NULL)
		return -2;

	ret = 0;
	memcpy(ack_data +tmplen, &ret, 4);
	tmplen += 4;
	
	tmpTime = time(NULL);
	memcpy(ack_data +tmplen, &tmpTime, 4);
	tmplen += 4;

	*ack_len = tmplen;

	return 0;
}

int server_0x10_getOneFrame(uint8_t *data, int len, uint8_t *ack_data, int size, int *ack_len)
{
	uint8_t type = 0;
	uint32_t frame_len = 0;
	uint8_t *frame = NULL;
	int32_t tmpLen = 0;
	int ret;

	memcpy(&ret, data, 4);
	tmpLen += 4;

	type = data[tmpLen];
	tmpLen += 1;

	memcpy(&frame_len, data+tmpLen, 4);
	tmpLen += 4;
	
	frame = data + tmpLen;
	tmpLen += frame_len;

	//printf("*** recv one frame data: type: %d, data_len: %d\n", type, frame_len);

	/* put frame to detect */
	opencv_put_frame_detect(frame, frame_len);

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

	proto_init();

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

	if(len > 0)
	{
		len = ringbuf_write(&client->recvRingBuf, tmpBuf, len);
	}

	return len;
}

int server_protoAnaly(struct clientInfo *client, uint8_t *pack, uint32_t pack_len)
{
	uint8_t seq = 0, cmd = 0;
	int data_len = 0;
	uint8_t *data = NULL;
	int ack_len = 0;
	int ret;

	if(pack==NULL || pack_len<=0)
		return -1;

	proto_analyPacket(pack, pack_len, &seq, &cmd, &data_len, &data);
	printf("%s: cmd: 0x%02x, seq: %d, pack_len: %d, data_len: %d\n", __FUNCTION__, \
				cmd, seq, pack_len, data_len);

	switch(cmd)
	{
		case 0x01:
			break;

		case 0x02:
			break;

		case 0x03:
			ret = server_0x03_heartbeat(data, data_len, ack_buf, sizeof(ack_buf), &ack_len);
			break;

		case 0x10:
			ret = server_0x10_getOneFrame(data, data_len, ack_buf, sizeof(ack_buf), &ack_len);
			break;

		default:
			printf("ERROR: protocol cmd[0x%02x] not exist!\n", cmd);
			break;
	}

	/* send ack data */
	if(ret==0 && ack_len>0)
	{
		proto_makeupPacket(seq, cmd, ack_len, ack_buf, tmpBuf, sizeof(tmpBuf), &tmpLen);
		server_sendData(client->fd, tmpBuf, tmpLen);
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
		server_protoAnaly(client, client->packBuf, client->packLen);
	}

	return 0;
}

void *socket_handle_thread(void *arg)
{
	struct clientInfo *client = (struct clientInfo *)arg;
	int flags = 0;
	int ret;

	printf("%s %d: enter ++\n", __FUNCTION__, __LINE__);

	// for test
	face_client = client;


	flags = fcntl(client->fd, F_GETFL, 0);
	fcntl(client->fd, F_SETFL, flags | O_NONBLOCK);

	ret = ringbuf_init(&client->recvRingBuf, RECV_BUFFER_SIZE);
	if(ret < 0)
		return NULL;

	client->protoHandle = proto_register(client->fd, server_sendData, SEND_BUFFER_SIZE);

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





