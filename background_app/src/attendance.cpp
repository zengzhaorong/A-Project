#include <stdio.h>
#include <string.h>
#include "attendance.h"
#include "user_mngr.h"


struct attend_mngr_Stru     attend_mngr_unit;

extern struct main_mngr_info main_mngr;
extern struct userMngr_Stru	user_mngr_unit;

/* return: attend status */
attend_sta_e attendance_set_one(int id, uint32_t time)
{
    struct attend_mngr_Stru *atd_mngr = &attend_mngr_unit;
    uint32_t adt_sec_day;
    uint32_t mid_time;
    attend_sta_e status = ATTEND_STA_IN_OK;
    int i;

    adt_sec_day = time_t_to_sec_day(time);
    mid_time = (main_mngr.atdin_time +main_mngr.atdout_time)/2;

    for(i=0; i<atd_mngr->userCnt; i++)
    {
        if(atd_mngr->attend_list[i].id == id)
        {
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
                if(atd_mngr->attend_list[i].atdin_sta == ATTEND_STA_NONE)
                {
                    atd_mngr->attend_list[i].atdin_time = time;
                    atd_mngr->attend_list[i].atdin_sta = status;
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
                atd_mngr->attend_list[i].atdout_time = time;
                atd_mngr->attend_list[i].atdout_sta = status;
            }
        }
    }

    return status;
}

void attendance_reset_all(void)
{
    struct attend_mngr_Stru *atd_mngr = &attend_mngr_unit;
    int i;
    
    // reset all user attend info
    for(i=0; i<atd_mngr->userCnt; i++)
    {
        atd_mngr->attend_list[i].atdin_time = 0;
        atd_mngr->attend_list[i].atdout_time = 0;
        atd_mngr->attend_list[i].atdin_sta = ATTEND_STA_NONE;
        atd_mngr->attend_list[i].atdout_sta = ATTEND_STA_NONE;
    }
    
}

int attendance_sync_userlist(void)
{
    struct attend_mngr_Stru *atd_mngr = &attend_mngr_unit;
    struct userMngr_Stru *usr_mngr = &user_mngr_unit;
    void *new_buf = NULL;
    int find_flag;
    int i, j;

    /* if add user: malloc more buffer for attend_list, first copy old info */
    if(usr_mngr->userCnt > atd_mngr->userCnt)
    {
        new_buf = malloc(usr_mngr->userCnt * sizeof(struct user_attend_info));
        if(new_buf == NULL)
        {
            printf("%s %d: malloc for new_buf failed!!!\n", __FUNCTION__, __LINE__);
            return -1;
        }
        memcpy(new_buf, atd_mngr->attend_list, atd_mngr->userCnt * sizeof(struct user_attend_info));

        free(atd_mngr->attend_list);
        atd_mngr->attend_list = (user_attend_info *)new_buf;

        for(i=0; i<usr_mngr->userCnt; i++)
        {
            find_flag = 0;
            for(j=0; j<atd_mngr->userCnt; j++)
            {
                if(usr_mngr->userInfo[i].id == atd_mngr->attend_list[j].id)
                {
                    find_flag = 1;
                    break;
                }
            }
            /* add not exist user */
            if(find_flag == 0)
            {
                atd_mngr->attend_list[atd_mngr->userCnt].id = usr_mngr->userInfo[i].id;
                memcpy(atd_mngr->attend_list[atd_mngr->userCnt].name, usr_mngr->userInfo[i].name, USER_NAME_LEN);
                atd_mngr->attend_list[atd_mngr->userCnt].atdin_time = 0;
                atd_mngr->attend_list[atd_mngr->userCnt].atdout_time = 0;
                atd_mngr->attend_list[atd_mngr->userCnt].atdin_sta = ATTEND_STA_NONE;
                atd_mngr->attend_list[atd_mngr->userCnt].atdout_sta = ATTEND_STA_NONE;
                atd_mngr->userCnt ++;
            }
        }
    }
    /* if delete user: only support delete one */
    else if(usr_mngr->userCnt < atd_mngr->userCnt)
    {
    DELETE_ONE_USER:
        for(j=0; j<atd_mngr->userCnt; j++)
        {
            find_flag = 0;
            for(i=0; i<usr_mngr->userCnt; i++)
            {
                if(usr_mngr->userInfo[i].id == atd_mngr->attend_list[j].id)
                {
                    find_flag = 1;
                    break;
                }
            }
            /* find need delete user */
            if(find_flag == 0)
                break;
        }
            /* delete user */
        for(; j<atd_mngr->userCnt -1; j++)
        {
            memcpy(&atd_mngr->attend_list[j], &atd_mngr->attend_list[j+1], sizeof(struct user_attend_info));
        }
        atd_mngr->userCnt --;

        if(usr_mngr->userCnt < atd_mngr->userCnt)
            goto DELETE_ONE_USER;
    }

    return 0;
}

int attendance_init(void)
{
    struct attend_mngr_Stru *atd_mngr = &attend_mngr_unit;
    struct userMngr_Stru *usr_mngr = &user_mngr_unit;
    int i;

    memset(atd_mngr, 0, sizeof(struct attend_mngr_Stru));

    atd_mngr->userCnt = usr_mngr->userCnt;
    atd_mngr->attend_list = (struct user_attend_info *)malloc(atd_mngr->userCnt * sizeof(struct user_attend_info));
    if(atd_mngr->attend_list == NULL)
    {
        printf("%s %d: malloc for attend_list failed!!!\n", __FUNCTION__, __LINE__);
        return -1;
    }

    for(i=0; i<atd_mngr->userCnt; i++)
    {
        atd_mngr->attend_list[i].id = usr_mngr->userInfo[i].id;
        memcpy(atd_mngr->attend_list[i].name, usr_mngr->userInfo[i].name, USER_NAME_LEN);
        atd_mngr->attend_list[i].atdin_time = 0;
        atd_mngr->attend_list[i].atdout_time = 0;
        atd_mngr->attend_list[i].atdin_sta = ATTEND_STA_NONE;
        atd_mngr->attend_list[i].atdout_sta = ATTEND_STA_NONE;
    }

    return 0;
}
