#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include "type.h"


typedef enum {
	WORK_STA_NORMAL,
	WORK_STA_ADDUSER,
}workstate_e;

struct main_mngr_info
{
	workstate_e work_state;
	int socket_handle;
};


void print_hex(char *text, uint8_t *buf, int len);


#endif	// _PUBLIC_H_
