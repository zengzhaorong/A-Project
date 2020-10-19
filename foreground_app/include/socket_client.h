#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include <netinet/in.h>
#include "type.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "ringbuffer.h"
#include "protocol.h"
#ifdef __cplusplus
}
#endif


typedef enum
{
	STATE_DISABLE,
	STATE_DISCONNECT,
	STATE_CONNECTED,
}sockState_e;

struct clientInfo
{
	int fd;
	int protoHandle;
	sockState_e state;
	struct sockaddr_in 	srv_addr;		// server 地址
	//struct sockaddr_in 	cli_addr;		// client 地址

	struct ringbuffer recvRingBuf;			// 接收环形缓冲区
	//struct ringbuffer sendRingBuf;			// 发送环形缓冲区
	struct detect_info detectInfo;
	uint8_t packBuf[PROTO_PACK_MAX_LEN];		// 协议包数据缓冲区
	int packLen;
};

int start_socket_client_task(void);


#endif	// _SOCKET_CLIENT_H_
