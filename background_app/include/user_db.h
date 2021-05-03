#ifndef _USER_DB_H_
#define _USER_DB_H_

#include <sqlite3.h>
#include "public.h"

#define USERDB_FILE_NAME       "userInfo.db"
#define USERDB_TABLE		    "USER_TBL"
#define USERDB_COL_ID		    "ID"
#define USERDB_COL_NAME		    "NAME"
#define USERDB_COL_TIMEIN		"TIMEIN"
#define USERDB_COL_TIMEOUT		"TIMEOUT"
#define USERDB_COL_STAIN		"STAIN"
#define USERDB_COL_STAOUT		"STAOUT"
#define USERDB_COL_FACEPATH		"FACEPATH"

#define TABLE_NAME_PRE      "TBL_"
#define TABLE_NAME_LEN      64

struct userdb_user
{
    int id;
    char name[USER_NAME_LEN];
    int in_time;       // attend in time
    int out_time;      // attend out time
    int in_sta;     // attend in status
    int out_sta;    // attend in status
    char facepath[DIR_PATH_LEN];    // face path
};

int userdb_write(sqlite3 *db, char *tbl_name, struct userdb_user *user);
int userdb_update(sqlite3 *db, char *tbl_name, struct userdb_user *user);
int userdb_read_byId(sqlite3 *db, char *tbl_name, int id, struct userdb_user *user);
int userdb_read_byName(sqlite3 *db, char *tbl_name, char *name, struct userdb_user *userInfo);
int userdb_delete_byId(sqlite3 *db, char *tbl_name, int id);
int userdb_delete_byName(sqlite3 *db, char *tbl_name, char *name);
int userdb_get_total(sqlite3 *db);
int userdb_traverse_user(sqlite3 *db, char *tbl_name, int *cursor, struct userdb_user *userInfo);
int userdb_check_user_exist(sqlite3 *db, char *tbl_name, int id);
int userdb_traverse_tbl(sqlite3 *db, int *cursor, char *tbl_name);
int userdb_creat_tbl(sqlite3 *db, char *tbl_name);

int userdb_init(sqlite3 **ppdb);

#endif	// _USER_DB_H_