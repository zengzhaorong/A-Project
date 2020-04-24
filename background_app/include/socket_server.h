#ifndef _SOCKET_SERVER_H_
#define _SOCKET_SERVER_H_

#include <netinet/in.h>


#define MAX_CLIENT_NUM 		3

struct clientInfo
{
	int fd;
	struct sockaddr_in addr;
};

struct serverInfo
{
	int fd;
	int client_cnt;
	struct sockaddr_in 	srv_addr;		// server µÿ÷∑
	struct clientInfo	client[MAX_CLIENT_NUM];
};


int start_socket_server_task(void);


#endif	// _SOCKET_SERVER_H_
