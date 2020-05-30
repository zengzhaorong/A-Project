#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include <netinet/in.h>
#include "ringbuffer.h"
#include "protocol.h"
#include "type.h"


typedef enum
{
	STATE_DISABLE,
	STATE_DISCONNECT,
	STATE_CONNECTED,
}e_sock_state;

struct clientInfo
{
	int fd;
	e_sock_state state;
	struct sockaddr_in 	srv_addr;		// server ��ַ
	//struct sockaddr_in 	cli_addr;		// client ��ַ

	struct ringbuffer recvRingBuf;			// ���ջ��λ�����
	//struct ringbuffer sendRingBuf;			// ���ͻ��λ�����
	struct detect_info detectInfo;
	uint8_t packBuf[PROTO_PACK_MAX_LEN];		// Э������ݻ�����
	int packLen;
};

int start_socket_client_task(void);


#endif	// _SOCKET_CLIENT_H_
