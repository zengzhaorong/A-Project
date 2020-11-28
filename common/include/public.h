#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "type.h"


#define USER_NAME_LEN		32


typedef enum {
	WORK_STA_NORMAL,		// normal work
	WORK_STA_ADDUSER,		// add user
	WORK_STA_RECOGN,		// has recognize success
}workstate_e;

struct main_mngr_info
{
	workstate_e work_state;
	int socket_handle;
};


void print_hex(char *text, uint8_t *buf, int len);


#endif	// _PUBLIC_H_
