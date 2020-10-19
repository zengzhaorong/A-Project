#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "socket_client.h"
#include "opencv_image_process.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "ringbuffer.h"
#include "capture.h"
#ifdef __cplusplus
}
#endif


#define DEFAULT_SERVER_IP			"127.0.0.1"
#define DEFAULT_SERVER_PORT			9100

#define RECV_BUFFER_SIZE			(1 *1024 *1024)
#define SEND_BUFFER_SIZE			(1 *1024 *1024)
#define HEARTBEAT_INTERVAL_S		10

struct clientInfo client_info;
int global_seq;

static uint8_t tmpBuf[1 *1024 *1024];
static int tmpLen = 0;


int client_0x03_heartbeat(uint8_t *data, int len, uint8_t *ack_data, int size, int *ack_len)
{
	time_t tmpTime;
	int ret;

	if(data==NULL || len<=0)
		return -1;

	/* request part */
	proto_0x03_dataAnaly(data, len, PROTO_ACK, &ret, &tmpTime);
	printf("%s: [ret: %d], time: %ld\n", __FUNCTION__, ret, tmpTime);

	if(ack_len != 0)
		*ack_len = 0;

	return 0;
}

int client_0x11_faceDetect(uint8_t *data, int len, uint8_t *ack_data, int size, int *ack_len)
{
	Rect rects;
	uint8_t count;
	int offset = 0;

	printf("%s: enter ++\n", __FUNCTION__);

	count = data[0];
	offset += 1;

	for(int i=0; i<count; i++)
	{
		rects.x = *((int*)(data+offset));
		printf("rect[%d]->x: %d\n", i, rects.x);
		offset += 4;
		rects.y = *((int*)(data+offset));
		printf("rect[%d]->y: %d\n", i, rects.y);
		offset += 4;
		rects.width = *((int*)(data+offset));
		printf("rect[%d]->w: %d\n", i, rects.width);
		offset += 4;
		rects.height = *((int*)(data+offset));
		printf("rect[%d]->h: %d\n", i, rects.height);
		offset += 4;
	}
	set_rect_param(rects);

	return 0;
}

int client_init(struct clientInfo *client, char *srv_ip, int srv_port)
{
	int flags = 0;
	int ret;

	memset(client, 0, sizeof(struct clientInfo));

	client->state = STATE_DISCONNECT;

	client->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(client->fd < 0)
	{
		ret = -1;
		goto ERR_1;
	}

	flags = fcntl(client->fd, F_GETFL, 0);
	fcntl(client->fd, F_SETFL, flags | O_NONBLOCK);

	client->srv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, srv_ip, &client->srv_addr.sin_addr);
	client->srv_addr.sin_port = htons(srv_port);

	ret = ringbuf_init(&client->recvRingBuf, RECV_BUFFER_SIZE);
	if(ret != 0)
	{
		ret = -2;
		goto ERR_2;
	}

	proto_init();

	return 0;

ERR_2:
	close(client->fd);

ERR_1:

	return ret;
}

void client_deinit(struct clientInfo *client)
{
	ringbuf_deinit(&client->recvRingBuf);
	close(client->fd);
}

int client_sendData(int sodkfd, uint8_t *data, int len)
{
	int ret;

	if(data == NULL)
		return -1;

	/* add sequence */
	data[PROTO_SEQ_OFFSET] = global_seq++;

	// lock
	
	ret = send(sodkfd, data, len, 0);

	// unlock

	return ret;
}

int client_recvData(struct clientInfo *client)
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

int client_protoAnaly(struct clientInfo *client, uint8_t *pack, char len)
{
	uint8_t seq = 0, cmd = 0;
	int data_len = 0;
	uint8_t *data = 0;
	uint8_t ack_buf[512] = {0};
	int ack_len = 0;
	int ret;

	if(pack==NULL || len<=0)
		return -1;

	proto_analyPacket(pack, len, &seq, &cmd, &data_len, &data);
	printf("get proto cmd: 0x%02x\n", cmd);

	switch(cmd)
	{
		case 0x01:
			break;

		case 0x02:
			break;

		case 0x03:
			ret = client_0x03_heartbeat(data, data_len, ack_buf, sizeof(ack_buf), &ack_len);
			break;

		case 0x11:
			ret = client_0x11_faceDetect(data, data_len, ack_buf, sizeof(ack_buf), &ack_len);
			break;

		default:
			printf("ERROR: protocol cmd[0x%02x] not exist!\n", cmd);
			break;
	}

	/* send ack data */
	if(ret==0 && ack_len>0)
	{
		proto_makeupPacket(seq, cmd, ack_len, ack_buf, tmpBuf, sizeof(tmpBuf), &tmpLen);
		client_sendData(client->fd, tmpBuf, tmpLen);
	}

	return 0;
}

int client_protoHandle(struct clientInfo *client)
{
	int ret;

	if(client == NULL)
		return -1;

	ret = client_recvData(client);
	
	ret = proto_detectPack(&client->recvRingBuf, &client->detectInfo, client->packBuf, \
							sizeof(client->packBuf), &client->packLen);
	if(ret == 0)
	{
		printf("detect protocol pack len: %d\n", client->packLen);
		client_protoAnaly(client, client->packBuf, client->packLen);
	}


	return 0;
}


void *socket_client_thread(void *arg)
{
	struct clientInfo *client = &client_info;
	time_t heartbeat_time = 0;
	time_t tmpTime;
	int len;
	int ret;

	ret = client_init(client, (char *)DEFAULT_SERVER_IP, DEFAULT_SERVER_PORT);
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
					client->protoHandle = proto_register(client->fd, client_sendData, SEND_BUFFER_SIZE);
					client->state = STATE_CONNECTED;
					printf("********** socket connect successfully, handle: %d.\n", client->protoHandle);
				}
				
				break;

			case STATE_CONNECTED:
				tmpTime = time(NULL);
				if(abs(tmpTime - heartbeat_time) >= HEARTBEAT_INTERVAL_S)
				{
					proto_0x03_sendHeartBeat(client->protoHandle);
					heartbeat_time = tmpTime;
				}
				
				ret = capture_getframe(tmpBuf, sizeof(tmpBuf), &len);
				if(ret == 0)
				{
					proto_0x10_sendOneFrame(client->protoHandle, 0, tmpBuf, len);
				}
				break;

			default:
				printf("%s %d: state ERROR !!!\n", __FUNCTION__, __LINE__);
				break;
		}

		if(client->state == STATE_CONNECTED)
		{
			client_protoHandle(client);
		}
		
		sleep(1);
	}

	client_deinit(client);

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





