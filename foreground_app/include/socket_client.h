#ifndef _SOCKET_CLIENT_H_
#define _SOCKET_CLIENT_H_

#include <netinet/in.h>


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
	struct sockaddr_in 	srv_addr;		// server µÿ÷∑
	//struct sockaddr_in 	cli_addr;		// client µÿ÷∑
};

int start_socket_client_task(void);


#endif	// _SOCKET_CLIENT_H_
