#include <stdio.h>
#include <string.h>
#include "attendance.h"
#include "user_mngr.h"


extern struct main_mngr_info main_mngr;
extern struct userMngr_Stru	user_mngr_unit;

/* return: attend status */
attend_sta_e attendance_set_one(int id, uint32_t time)
{
    struct userMngr_Stru *user_mngr = &user_mngr_unit;
    struct userdb_user user;
    uint32_t adt_sec_day;
    uint32_t mid_time;
    attend_sta_e status = ATTEND_STA_IN_OK;
    int ret;

    adt_sec_day = time_t_to_sec_day(time);
    mid_time = (main_mngr.atdin_time +main_mngr.atdout_time)/2;

    ret = userdb_read_byId(user_mngr->userdb, id, &user);
    if(ret != 0)
    {
        printf("ERROR: %s: not find user!\n", __FUNCTION__);
        return status;
    }

    /* attend in: use the first time */
    if(adt_sec_day <= mid_time)
    {
        if(adt_sec_day <= main_mngr.atdin_time)
        {
            status = ATTEND_STA_IN_OK;
            printf("user id [%d]: attend in ok.\n", id);
        }
        else
        {
            status = ATTEND_STA_IN_LATE;
            printf("user id [%d]: attend in late.\n", id);
        }
        if(user.in_sta==ATTEND_STA_NONE || adt_sec_day<(uint32_t)user.in_time)
        {
            user.in_time = time;
            user.in_sta = status;
        }
    }
    /* attend out: use the last time */
    else
    {
        if(adt_sec_day < main_mngr.atdout_time)
        {
            status = ATTEND_STA_OUT_EARLY;
            printf("user id [%d]: attend out early.\n", id);
        }
        else
        {
            status = ATTEND_STA_OUT_OK;
            printf("user id [%d]: attend out ok.\n", id);
        }
        user.out_time = time;
        user.out_sta = status;
    }
    userdb_write(user_mngr->userdb, &user);

    return status;
}

void attendance_reset_all(void)
{
    struct userMngr_Stru *usr_mngr = &user_mngr_unit;
    struct userdb_user user;
    int total, cursor = 0;
    int i, ret;
    
    total = userdb_get_total(usr_mngr->userdb);

    // reset all user attend info
    for(i=0; i<total +1; i++)
    {
        ret = userdb_traverse_user(usr_mngr->userdb, &cursor, &user);
        if(ret != 0)
            break;

        user.in_time = 0;
        user.out_time = 0;
        user.in_sta = ATTEND_STA_NONE;
        user.out_sta = ATTEND_STA_NONE;
        userdb_write(usr_mngr->userdb, &user);
    }
    
}

