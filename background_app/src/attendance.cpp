#include <stdio.h>
#include <string.h>
#include "attendance.h"
#include "user_mngr.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#include "user_db.h"
#ifdef __cplusplus
}
#endif


extern struct main_mngr_info main_mngr;
extern struct userMngr_Stru	user_mngr_unit;

/* return: attend status */
attend_sta_e attendance_set_one(int id, uint32_t time)
{
    struct userMngr_Stru *user_mngr = &user_mngr_unit;
    struct db_attend attend;
    uint32_t adt_sec_day;
    uint32_t mid_time;
    attend_sta_e status = ATTEND_STA_IN_OK;
    int ret;

    adt_sec_day = time_t_to_sec_day(time);
    mid_time = (main_mngr.atdin_secday +main_mngr.atdout_secday)/2;

    ret = db_attend_read_byId(user_mngr->userdb, user_mngr->curr_tbl, id, &attend);
    if(ret != 0)
    {
        printf("ERROR: %s: not find user!\n", __FUNCTION__);
        return status;
    }

    /* attend in: use the first time */
    if(adt_sec_day <= mid_time)
    {
        if(adt_sec_day <= main_mngr.atdin_secday)
        {
            status = ATTEND_STA_IN_OK;
            printf("user [id=%d]: attend in ok.\n", id);
        }
        else
        {
            status = ATTEND_STA_IN_LATE;
            printf("user [id=%d]: attend in late.\n", id);
        }
        if(attend.in_sta == ATTEND_STA_NONE)
        {
            attend.in_time = time;
            attend.in_sta = status;
            db_attend_write(user_mngr->userdb, user_mngr->curr_tbl, &attend);
        }
    }
    /* attend out: use the last time */
    else
    {
        if(adt_sec_day < main_mngr.atdout_secday)
        {
            status = ATTEND_STA_OUT_EARLY;
            printf("user [id=%d]: attend out early.\n", id);
        }
        else
        {
            status = ATTEND_STA_OUT_OK;
            printf("user [id=%d]: attend out ok.\n", id);
        }

        /* add attend times */
        if(attend.in_sta!=ATTEND_STA_NONE && attend.out_sta==ATTEND_STA_NONE)
        {
            attend.times ++;
        }

        attend.out_time = time;
        attend.out_sta = status;
        db_attend_write(user_mngr->userdb, user_mngr->curr_tbl, &attend);
    }

    return status;
}

/* reset all data */
void attendance_reset_tbl(char *tbl_name)
{
    struct userMngr_Stru *user_mngr = &user_mngr_unit;
    struct db_attend attend;
    int total, cursor = 0;
    int i, ret;
    
    total = db_user_get_total(user_mngr->userdb);

    // reset all user attend info
    for(i=0; i<total +1; i++)
    {
        ret = db_attend_traverse_user(user_mngr->userdb, tbl_name, &cursor, &attend);
        if(ret != 0)
            break;

        attend.in_time = 0;
        attend.out_time = 0;
        attend.in_sta = ATTEND_STA_NONE;
        attend.out_sta = ATTEND_STA_NONE;
        attend.times = 0;
        db_attend_write(user_mngr->userdb, user_mngr->curr_tbl, &attend);
    }
    
}

/* not clear times */
void attendance_clear_tbl(char *tbl_name)
{
    struct userMngr_Stru *user_mngr = &user_mngr_unit;
    struct db_attend attend;
    int total, cursor = 0;
    int i, ret;
    
    total = db_user_get_total(user_mngr->userdb);

    // reset all user attend info
    for(i=0; i<total +1; i++)
    {
        ret = db_attend_traverse_user(user_mngr->userdb, tbl_name, &cursor, &attend);
        if(ret != 0)
            break;

        attend.in_time = 0;
        attend.out_time = 0;
        attend.in_sta = ATTEND_STA_NONE;
        attend.out_sta = ATTEND_STA_NONE;
        db_attend_write(user_mngr->userdb, user_mngr->curr_tbl, &attend);
    }
    
}

/* save data as .csv file */
int attendance_save_data_csv(char *filename)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;
    struct db_attend attend;
	struct tm *ptm;
    string atdin_str, atdout_str;
    int cursor = 0;
	ofstream csvFile;
	char intime_str[16] = {0};
	char outtime_str[16] = {0};
	char insta_str[16] = {0};
	char outsta_str[16] = {0};
    time_t timein;
    time_t timeout;
    int ret;

	csvFile.open(filename);
    if(csvFile.fail())
    {
        printf("ERROR: %s: open file failed!\n", __FUNCTION__);
        return -1;
    }

	csvFile << TEXT_USER_ID << ',' << TEXT_USER_NAME << ',' << TEXT_ATTEND_IN << ',' << TEXT_ATTEND_OUT << ',' << TEXT_STATUS << endl;  
	
	/* list all attend table */
    ret = 0;
    for(; ret == 0; )
    {
        ret = db_attend_traverse_user(user_mngr->userdb, user_mngr->curr_tbl, &cursor, &attend);
        if(ret != 0)
            break;

        timein = attend.in_time;
        timeout = attend.out_time;
		memset(intime_str, 0, sizeof(intime_str));
		memset(outtime_str, 0, sizeof(outtime_str));
		ptm = localtime((time_t *)&timein);
		if(ptm != NULL)
		{
			sprintf(intime_str, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		}
		ptm = localtime((time_t *)&timeout);
		if(ptm != NULL)
		{
			sprintf(outtime_str, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
		}

        memset(insta_str, 0, sizeof(insta_str));
        if(attend.in_sta == ATTEND_STA_IN_OK)
        {
            strcpy(insta_str, TEXT_ATTEND_OK);
        }
        else if(attend.in_sta == ATTEND_STA_IN_LATE)
        {
            strcpy(insta_str, TEXT_ATTEND_IN_LATE);
        }
        else
        {
            strcpy(insta_str, TEXT_ATTEND_NULL);
        }

        memset(outsta_str, 0, sizeof(outsta_str));
        if(attend.out_sta == ATTEND_STA_OUT_OK)
        {
            strcpy(outsta_str, TEXT_ATTEND_OK);
        }
        else if(attend.out_sta == ATTEND_STA_OUT_EARLY)
        {
            strcpy(outsta_str, TEXT_ATTEND_OUT_EARLY);
        }
        else
        {
            strcpy(outsta_str, TEXT_ATTEND_NULL);
        }

		csvFile << attend.id << ',' << attend.name << ',' << intime_str << ',' << outtime_str \
                << ',' << insta_str << ':' << outsta_str << endl;  
    }
	csvFile.close();

    return 0;
}

int attendance_set_tbl(char *tbl_name)
{

    memcpy(user_mngr_unit.curr_tbl, tbl_name, TABLE_NAME_LEN);

    db_attend_creat_tbl(user_mngr_unit.userdb, tbl_name);

    return 0;
}
