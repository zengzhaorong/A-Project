#ifndef _ATTENDANCE_H_
#define _ATTENDANCE_H_

#include <time.h>

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#ifdef __cplusplus
}
#endif


struct user_attend_info
{
	int 	id;					// user id
	char	name[USER_NAME_LEN];// user name
    time_t  time;               // attend time
    attend_sta_e status;        // status
};

struct attend_mngr_Stru
{
    int userCnt;
    struct user_attend_info *attend_list;
};

attend_sta_e attendance_set_one(int id, time_t time);
void attendance_reset_all(void);
int attendance_sync_userlist(void);

int attendance_init(void);

#endif	// _ATTENDANCE_H_