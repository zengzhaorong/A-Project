#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user_db.h"



/* asynchronous read */
int userdb_write(sqlite3 *db, struct userdb_user *user)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* check user if exist or not */
    ret = userdb_check_user_exist(db, user->id);
    if(ret == 0)    // exist
    {
        ret = userdb_update(db, user);
    }
    else
    {
        sprintf(sql_cmd, "INSERT INTO %s(%s, %s, %s, %s, %s, %s, %s) VALUES(%d, '%s', %d, %d, %d, %d, '%s');", \
                        USERDB_TABLE, USERDB_COL_ID, USERDB_COL_NAME, USERDB_COL_TIMEIN, \
                        USERDB_COL_TIMEOUT, USERDB_COL_STAIN, USERDB_COL_STAOUT, USERDB_COL_FACEPATH, \
                        user->id, user->name, user->in_time, user->out_time, user->in_sta, user->out_sta, user->facepath);
        ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
        if(ret != SQLITE_OK)    // may be already exist
        {
            printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
            return -1;
        }
    }
    printf("%s: id=%d, name: %s\n", __FUNCTION__, user->id, user->name);

    return 0;
}

int userdb_update(sqlite3 *db, struct userdb_user *user)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* CMD: UPDATE USER_TBL set a=%d, b=%d where ID=%d */
    sprintf(sql_cmd, "UPDATE %s set %s='%s',%s=%d,%s=%d,%s=%d,%s=%d,%s='%s' where %s=%d;", \
                    USERDB_TABLE, USERDB_COL_NAME, user->name, USERDB_COL_TIMEIN,user->in_time, \
                    USERDB_COL_TIMEOUT,user->out_time, USERDB_COL_STAIN,user->in_sta,\
                    USERDB_COL_STAOUT,user->out_sta, USERDB_COL_FACEPATH,user->facepath, USERDB_COL_ID,user->id);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

/* synchronous read */
int userdb_read_byId(sqlite3 *db, int id, struct userdb_user *user)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

	sprintf(sql_cmd, "SELECT * from %s where %s=%d;", USERDB_TABLE, USERDB_COL_ID, id);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    ret = -1;
    while(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		user->id = sqlite3_column_int(pStmt, 0);
		strncpy(user->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(user->name));
        user->in_time = sqlite3_column_int(pStmt, 2);
        user->out_time = sqlite3_column_int(pStmt, 3);
        user->in_sta = sqlite3_column_int(pStmt, 4);
        user->out_sta = sqlite3_column_int(pStmt, 5);
		strncpy(user->facepath, (const char *)sqlite3_column_text(pStmt, 6), sizeof(user->facepath));
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, user->id, user->name);
        ret = 0;
    }

    sqlite3_finalize(pStmt);

    return ret;
}

/* synchronous read */
int userdb_read_byName(sqlite3 *db, char *name, struct userdb_user *user)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

	sprintf(sql_cmd, "SELECT * from %s where %s='%s';", USERDB_TABLE, USERDB_COL_NAME, name);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    ret = -1;
    while(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		user->id = sqlite3_column_int(pStmt, 0);
		strncpy(user->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(user->name));
        user->in_time = sqlite3_column_int(pStmt, 2);
        user->out_time = sqlite3_column_int(pStmt, 3);
        user->in_sta = sqlite3_column_int(pStmt, 4);
        user->out_sta = sqlite3_column_int(pStmt, 5);
		strncpy(user->facepath, (const char *)sqlite3_column_text(pStmt, 6), sizeof(user->facepath));
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, user->id, user->name);
        ret = 0;
    }

    sqlite3_finalize(pStmt);

    return ret;
}

/* asynchronous delete */
int userdb_delete_byId(sqlite3 *db, int id)
{
    char sql_cmd[128] = {0};
    char *errMsg = NULL;
    int ret;

    sprintf(sql_cmd, "DELETE from %s where %s=%d;", \
                    USERDB_TABLE, USERDB_COL_ID, id);
    ret = sqlite3_exec(db, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
        return -1;
    }

    return 0;
}

/* asynchronous delete */
int userdb_delete_byName(sqlite3 *db, char *name)
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

int userdb_get_total(sqlite3 *db)
{
	sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int total;
    int ret;

	sprintf(sql_cmd, "SELECT COUNT(%s) from %s;", USERDB_COL_ID, USERDB_TABLE);
    ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
    if(ret != SQLITE_OK)    // may be already exist
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

int userdb_traverse_user(sqlite3 *db, int *cursor, struct userdb_user *user)
{
	static sqlite3_stmt *pStmt;
    char sql_cmd[128] = {0};
    const char *pzTail;
    int ret;

    if(*cursor == 0)
    {
        sprintf(sql_cmd, "SELECT * from %s;", USERDB_TABLE);
        ret = sqlite3_prepare_v2(db, sql_cmd, strlen(sql_cmd), &pStmt, &pzTail);
        if(ret != SQLITE_OK)    // may be already exist
        {
            printf("%s: failed: %s\n", __FUNCTION__, sqlite3_errmsg(db));
            return -1;
        }
    }

    if(sqlite3_step(pStmt) == SQLITE_ROW)
    {
		user->id = sqlite3_column_int(pStmt, 0);
		strncpy(user->name, (const char *)sqlite3_column_text(pStmt, 1), sizeof(user->name));
        user->in_time = sqlite3_column_int(pStmt, 2);
        user->out_time = sqlite3_column_int(pStmt, 3);
        user->in_sta = sqlite3_column_int(pStmt, 4);
        user->out_sta = sqlite3_column_int(pStmt, 5);
		strncpy(user->facepath, (const char *)sqlite3_column_text(pStmt, 6), sizeof(user->facepath));
        //printf("%s: id=%d, name=%s\n", __FUNCTION__, user->id, user->name);
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

int userdb_check_user_exist(sqlite3 *db, int id)
{
    struct userdb_user userInfo;
    int ret;

    ret = userdb_read_byId(db, id, &userInfo);

    return ret;
}

int userdb_init(sqlite3 **ppdb)
{
    char sql_cmd[256] = {0};
    char *errMsg = NULL;
    int ret;

    /* open or create database */
    ret = sqlite3_open(USERDB_FILE_NAME, ppdb);
    if(ret != 0)
    {
        printf("%s: Can't open database: %s\n", __FUNCTION__, sqlite3_errmsg(*ppdb));
        return -1;
    }

    /* create db table */
	sprintf(sql_cmd, "CREATE TABLE %s(%s INT PRIMARY KEY NOT NULL, %s CHAR(%d) NOT NULL, %s INT, %s INT, %s INT, %s INT, %s TEXT);", \
					USERDB_TABLE, USERDB_COL_ID, USERDB_COL_NAME, USER_NAME_LEN, USERDB_COL_TIMEIN, \
                    USERDB_COL_TIMEOUT, USERDB_COL_STAIN, USERDB_COL_STAOUT, USERDB_COL_FACEPATH);
    //printf("%s: cmd: %s\n", __FUNCTION__, sql_cmd);
    ret = sqlite3_exec(*ppdb, sql_cmd, NULL, NULL, &errMsg);
    if(ret != SQLITE_OK)    // may be already exist
    {
        printf("%s: %s\n", __FUNCTION__, sqlite3_errmsg(*ppdb));
    }
    
    printf("%s: successfully.\n", __FUNCTION__);
#if 0
    struct userdb_user user = {0};
    int i, total, cursor;
    /* test */
    user.id = 1;
    strcpy(user.name, "Jenny");
    userdb_write(*ppdb, &user);
    user.id = 2;
    strcpy(user.name, "Tony");
    userdb_write(*ppdb, &user);
    user.id = 3;
    strcpy(user.name, "Gogo");
    userdb_write(*ppdb, &user);
    user.id = 4;
    strcpy(user.name, "Jimmy");
    userdb_write(*ppdb, &user);
    user.id = 5;
    strcpy(user.name, "paul");
    userdb_write(*ppdb, &user);
    user.id = 2;
    strcpy(user.name, "Tony_bak");
    userdb_write(*ppdb, &user);
    printf("---------- write -----------\n");
    total = userdb_get_total(*ppdb);
    cursor = 0;
    for(i=0; i<total+1; i++)
    {
        ret = userdb_traverse_user(*ppdb, &cursor, &user);
        if(ret != 0)
        {
            printf("---------traverse over.\n");
            break;
        }
        printf("[%d] user id=%d name: %s\n", cursor, user.id, user.name);
    }
    
    userdb_read_byId(*ppdb, 1, &user);
    userdb_read_byName(*ppdb, "Tony", &user);
    userdb_read_byName(*ppdb, "Tony_bak", &user);
    userdb_read_byId(*ppdb, 3, &user);
    printf("---------- read -----------\n");
    userdb_get_total(*ppdb);

    userdb_delete_byId(*ppdb, 1);
    userdb_delete_byName(*ppdb, "Tony_bak");
    printf("---------- delete -----------\n");
    userdb_get_total(*ppdb);

    userdb_read_byId(*ppdb, 1, &user);
    userdb_read_byName(*ppdb, "Tony_bak", &user);
    userdb_read_byId(*ppdb, 3, &user);
    printf("---------- read -----------\n");

    exit(0);
#endif
    return 0;
}
