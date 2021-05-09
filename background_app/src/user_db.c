#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user_db.h"



/* asynchronous read */
int db_user_write(sqlite3 *db, struct db_userinfo *user)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* check user if exist or not */
    ret = db_user_check_exist(db, user->id);
    if(ret == 0)    // exist
    {
        ret = db_user_update(db, user);
    }
    else
    {
        sprintf(sql_cmd, "INSERT INTO %s(%s, %s, %s) VALUES(%d, '%s', '%s');", \
                        USERDB_TABLE, USERDB_COL_ID, USERDB_COL_NAME, USERDB_COL_FACEPATH, \
                        user->id, user->name, user->facepath);
        //printf("%s: sql_cmd: %s\n", __FUNCTION__, sql_cmd);
        ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
        if(ret != SQLITE_OK)    // may be already exist
        {
            printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
            return -1;
        }
    }

    return 0;
}

int db_user_update(sqlite3 *db, struct db_userinfo *user)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* CMD: UPDATE USER_TBL set a=%d, b=%d where ID=%d */
    sprintf(sql_cmd, "UPDATE %s set %s='%s', %s='%s' where %s=%d;", \
                    USERDB_TABLE, USERDB_COL_NAME, user->name, \
                    USERDB_COL_FACEPATH,user->facepath, USERDB_COL_ID,user->id);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

/* synchronous read */
int db_user_read_byId(sqlite3 *db, int id, struct db_userinfo *user)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

	sprintf(sql_cmd, "SELECT * from %s where %s=%d;", USERDB_TABLE, USERDB_COL_ID, id);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    ret = -1;
    while(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		user->id = sqlite3_column_int(pStmt, 0);
		strncpy(user->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(user->name));
		strncpy(user->facepath, (const char *)sqlite3_column_text(pStmt, 2), sizeof(user->facepath));
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, user->id, user->name);
        ret = 0;
    }

    sqlite3_finalize(pStmt);

    return ret;
}

/* synchronous read */
int db_user_read_byName(sqlite3 *db, char *name, struct db_userinfo *user)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

	sprintf(sql_cmd, "SELECT * from %s where %s='%s';", USERDB_TABLE, USERDB_COL_NAME, name);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    ret = -1;
    while(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		user->id = sqlite3_column_int(pStmt, 0);
		strncpy(user->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(user->name));
		strncpy(user->facepath, (const char *)sqlite3_column_text(pStmt, 2), sizeof(user->facepath));
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, user->id, user->name);
        ret = 0;
    }

    sqlite3_finalize(pStmt);

    return ret;
}

/* asynchronous delete */
int db_user_delete_byId(sqlite3 *db, int id)
{
    char sql_cmd[128] = {0};
    char *errMsg = NULL;
    int ret;

    sprintf(sql_cmd, "DELETE from %s where %s=%d;", \
                    USERDB_TABLE, USERDB_COL_ID, id);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

/* asynchronous delete */
int db_user_delete_byName(sqlite3 *db, char *name)
{
    char sql_cmd[128] = {0};
    char *errMsg = NULL;
    int ret;

    sprintf(sql_cmd, "DELETE from %s where %s='%s';", \
                    USERDB_TABLE, USERDB_COL_NAME, name);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

int db_user_get_total(sqlite3 *db)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int total;
    int ret;

	sprintf(sql_cmd, "SELECT COUNT(%s) from %s;", USERDB_COL_ID, USERDB_TABLE);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    if(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		total = sqlite3_column_int(pStmt, 0);
    }

    sqlite3_finalize(pStmt);

    return total;
}

int db_user_check_exist(sqlite3 *db, int id)
{
    struct db_userinfo userInfo;
    int ret;

    ret = db_user_read_byId(db, id, &userInfo);

    return ret;
}

int db_user_traverse_user(sqlite3 *db, int *cursor, struct db_userinfo *user)
{
	static sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

    if(*cursor == 0)
    {
        sprintf(sql_cmd, "SELECT * from %s;", USERDB_TABLE);
        ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
        if(ret != SQLITE_OK)
        {
            printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
            return -1;
        }
    }

    if(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		user->id = sqlite3_column_int(pStmt, 0);
		strncpy(user->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(user->name));
		strncpy(user->facepath, (const char *)sqlite3_column_text(pStmt, 2), sizeof(user->facepath));
        //printf("%s: id=%d, name=%s, face: %s\n", __FUNCTION__, user->id, user->name, user->facepath);
        (*cursor) ++;
        ret = 0;
    }
    else
    {
        sqlite3_finalize(pStmt);
        //printf("%s: over, no more user.\n", __FUNCTION__);
        ret = -1;
    }

    return ret;
}

int db_user_creat_tbl(sqlite3 *db, char *tbl_name)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* create db table if not exist */
	sprintf(sql_cmd, "CREATE TABLE IF NOT EXISTS %s(%s INT PRIMARY KEY NOT NULL, %s CHAR(%d) NOT NULL, %s TEXT);", \
					tbl_name, USERDB_COL_ID, USERDB_COL_NAME, USER_NAME_LEN, USERDB_COL_FACEPATH);
    //printf("%s: cmd: %s\n", __FUNCTION__, sql_cmd);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: %s\n", __FUNCTION__, sqlite3_errmsg(db));
    }

    return 0;
}


/*----------------------------- user information table --------------------------------*/
/*-------------------------------------------------------------------------------------*/
/*----------------------------- user attendance table ---------------------------------*/

/* asynchronous read */
int db_attend_write(sqlite3 *db, char *tbl_name, struct db_attend *attend)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* check user if exist or not */
    ret = db_attend_check_user_exist(db, tbl_name, attend->id);
    if(ret == 0)    // exist
    {
        ret = db_attend_update(db, tbl_name, attend);
    }
    else
    {
        sprintf(sql_cmd, "INSERT INTO %s(%s, %s, %s, %s, %s, %s) VALUES(%d, '%s', %d, %d, %d, %d);", \
                        tbl_name, USERDB_COL_ID, USERDB_COL_NAME, USERDB_COL_TIMEIN, \
                        USERDB_COL_TIMEOUT, USERDB_COL_STAIN, USERDB_COL_STAOUT, \
                        attend->id, attend->name, attend->in_time, attend->out_time, attend->in_sta, attend->out_sta);
        ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
        if(ret != SQLITE_OK)    // may be already exist
        {
            printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
            return -1;
        }
    }
    //printf("%s: table: %s, id=%d, name: %s\n", __FUNCTION__, tbl_name, attend->id, attend->name);

    return 0;
}

int db_attend_update(sqlite3 *db, char *tbl_name, struct db_attend *attend)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* CMD: UPDATE USER_TBL set a=%d, b=%d where ID=%d */
    sprintf(sql_cmd, "UPDATE %s set %s='%s',%s=%d,%s=%d,%s=%d,%s=%d where %s=%d;", \
                    tbl_name, USERDB_COL_NAME, attend->name, USERDB_COL_TIMEIN,attend->in_time, \
                    USERDB_COL_TIMEOUT,attend->out_time, USERDB_COL_STAIN,attend->in_sta,\
                    USERDB_COL_STAOUT,attend->out_sta, USERDB_COL_ID,attend->id);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

/* synchronous read */
int db_attend_read_byId(sqlite3 *db, char *tbl_name, int id, struct db_attend *attend)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

	sprintf(sql_cmd, "SELECT * from %s where %s=%d;", tbl_name, USERDB_COL_ID, id);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    ret = -1;
    while(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		attend->id = sqlite3_column_int(pStmt, 0);
		strncpy(attend->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(attend->name));
        attend->in_time = sqlite3_column_int(pStmt, 2);
        attend->out_time = sqlite3_column_int(pStmt, 3);
        attend->in_sta = sqlite3_column_int(pStmt, 4);
        attend->out_sta = sqlite3_column_int(pStmt, 5);
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, attend->id, attend->name);
        ret = 0;
    }

    sqlite3_finalize(pStmt);

    return ret;
}

/* synchronous read */
int db_attend_read_byName(sqlite3 *db, char *tbl_name, char *name, struct db_attend *attend)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

	sprintf(sql_cmd, "SELECT * from %s where %s='%s';", tbl_name, USERDB_COL_NAME, name);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    ret = -1;
    while(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		attend->id = sqlite3_column_int(pStmt, 0);
		strncpy(attend->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(attend->name));
        attend->in_time = sqlite3_column_int(pStmt, 2);
        attend->out_time = sqlite3_column_int(pStmt, 3);
        attend->in_sta = sqlite3_column_int(pStmt, 4);
        attend->out_sta = sqlite3_column_int(pStmt, 5);
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, attend->id, attend->name);
        ret = 0;
    }

    sqlite3_finalize(pStmt);

    return ret;
}

/* asynchronous delete */
int db_attend_delete_byId(sqlite3 *db, char *tbl_name, int id)
{
    char sql_cmd[128] = {0};
    char *errMsg = NULL;
    int ret;

    sprintf(sql_cmd, "DELETE from %s where %s=%d;", \
                    tbl_name, USERDB_COL_ID, id);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

/* asynchronous delete */
int db_attend_delete_byName(sqlite3 *db, char *tbl_name, char *name)
{
    char sql_cmd[128] = {0};
    char *errMsg = NULL;
    int ret;

    sprintf(sql_cmd, "DELETE from %s where %s='%s';", \
                    tbl_name, USERDB_COL_NAME, name);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

/* tbl_name: NULL-user table, other-attend table */
int db_attend_traverse_user(sqlite3 *db, char *tbl_name, int *cursor, struct db_attend *attend)
{
	static sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

    if(*cursor == 0)
    {
        sprintf(sql_cmd, "SELECT * from %s;", tbl_name);
        ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
        if(ret != SQLITE_OK)    // may be already exist
        {
            printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
            return -1;
        }
    }

    if(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		attend->id = sqlite3_column_int(pStmt, 0);
		strncpy(attend->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(attend->name));
        attend->in_time = sqlite3_column_int(pStmt, 2);
        attend->out_time = sqlite3_column_int(pStmt, 3);
        attend->in_sta = sqlite3_column_int(pStmt, 4);
        attend->out_sta = sqlite3_column_int(pStmt, 5);
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, attend->id, attend->name);
        (*cursor) ++;
        ret = 0;
    }
    else
    {
        sqlite3_finalize(pStmt);
        //printf("%s: over, no more user.\n", __FUNCTION__);
        ret = -1;
    }

    return ret;
}

int db_attend_check_user_exist(sqlite3 *db, char *tbl_name, int id)
{
    struct db_attend attend;
    int ret;

    ret = db_attend_read_byId(db, tbl_name, id, &attend);

    return ret;
}

int db_attend_traverse_tbl(sqlite3 *db, int *cursor, char *tbl_name)
{
	static sqlite3_stmt *pStmt;
    char sql_cmd[128] = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name";
    const char *pzTail;
    int ret;

    if(*cursor == 0)
    {
        ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
        if(ret != SQLITE_OK)    // may be already exist
        {
            printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
            return -1;
        }
    }

    if(sqlite3_step(pStmt) == SQLITE_ROW)
    {
        strncpy(tbl_name, (const char *)sqlite3_column_text(pStmt, 0), TABLE_NAME_LEN);
        (*cursor) ++;
        ret = 0;
    }
    else
    {
        sqlite3_finalize(pStmt);
        ret = -1;
    }

    return ret;
}

int db_attend_creat_tbl(sqlite3 *db, char *tbl_name)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* create db table if not exist */
	sprintf(sql_cmd, "CREATE TABLE IF NOT EXISTS %s(%s INT PRIMARY KEY NOT NULL, %s CHAR(%d) NOT NULL, %s INT, %s INT, %s INT, %s INT);", \
					tbl_name, USERDB_COL_ID, USERDB_COL_NAME, USER_NAME_LEN, USERDB_COL_TIMEIN, \
                    USERDB_COL_TIMEOUT, USERDB_COL_STAIN, USERDB_COL_STAOUT);
    //printf("%s: cmd: %s\n", __FUNCTION__, sql_cmd);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: %s\n", __FUNCTION__, sqlite3_errmsg(db));
    }

    return 0;
}

int userdb_init(sqlite3 **ppdb)
{
    int ret;

    /* open or create database */
    ret = sqlite3_open(USERDB_FILE_NAME, ppdb);
    if(ret != 0)
    {
        printf("%s: Can't open database: %s\n", __FUNCTION__, sqlite3_errmsg(*ppdb));
        return -1;
    }
    
    /* create user information table */
    db_user_creat_tbl(*ppdb, USERDB_TABLE);

    printf("%s: successfully.\n", __FUNCTION__);

    return 0;
}
