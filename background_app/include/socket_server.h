#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#include <netinet/in.h>

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "ringbuffer.h"
#include "protocol.h"
#ifdef __cplusplus
}
#endif

#define MAX_CLIENT_NUM 		1

struct clientInfo
{
	int fd;
	int protoHandle;
	struct sockaddr_in addr;
	struct ringbuffer recvRingBuf;			// ���ջ��λ�����
	struct detect_info detectInfo;
	uint8_t packBuf[PROTO_PACK_MAX_LEN];		// Э������ݻ�����
	int packLen;
};

struct serverInfo
{
	int fd;
	int client_cnt;
	struct sockaddr_in 	srv_addr;		// server ��ַ
	struct clientInfo	client[MAX_CLIENT_NUM];
};


int start_socket_server_task(void);


#endif	// _SOCKET_SERVER_H_
