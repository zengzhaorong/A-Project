#ifndef _PUBLIC_H_
#define _PUBLIC_H_

#include <time.h>
#include "type.h"


#define USER_NAME_LEN		32
#define DIR_PATH_LEN		64

/* user client login name */
#define USER_CLIENT_NAME	"user_name"
/* manager client login name */
#define MNGR_CLIENT_NAME	"manager_name"
/* root client login name */
#define ROOT_CLIENT_NAME	"root_name"

#define TEXT_USER_ID			"±àºÅ"
#define TEXT_USER_NAME			"ÐÕÃû"
#define TEXT_STATUS				"×´Ì¬"
#define TEXT_ATTEND_IN			"Ç©µ½"
#define TEXT_ATTEND_OUT			"Ç©ÍË"
#define TEXT_ATTEND_OK			"Õý³£"
#define TEXT_ATTEND_IN_LATE		"³Ùµ½"
#define TEXT_ATTEND_OUT_EARLY	"ÔçÍË"
#define TEXT_ATTEND_NULL		"È±ÇÚ"


typedef enum {
	IDENTITY_USER,		// user client
	IDENTITY_MANAGER,	// manager client
	IDENTITY_ROOT,		// root client, equal to user and manager
}client_identity_e;


typedef enum {
	WORK_STA_DISCONNECT,	// not connect server
	WORK_STA_NORMAL,		// normal work
	WORK_STA_ADDUSER,		// add user
	WORK_STA_RECOGN,		// has recognize success
}workstate_e;

typedef enum {
	ATTEND_STA_NONE,		// not attend
	ATTEND_STA_IN_OK,		// attend in ok
	ATTEND_STA_OUT_OK,		// attend out ok
	ATTEND_STA_IN_LATE,		// attend in late
	ATTEND_STA_OUT_EARLY,	// attend out early
}attend_sta_e;

struct daytm
{
	int hour;
	int min;
	int sec;
};

struct main_mngr_info
{
	workstate_e work_state;
	int user_handle;	// user client socket handle index
	int mngr_handle;	// manager client socket handle index
	uint32_t atdin_time;	// attend in time
	uint32_t atdout_time;	// attend out time
};


void print_hex(char *text, uint8_t *buf, int len);

uint32_t time_t_to_sec_day(time_t time);
int sec_day_to_daytm(int sec_day, struct daytm *tm_day);

#endif	// _PUBLIC_H_
