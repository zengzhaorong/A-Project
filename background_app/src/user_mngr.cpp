#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "user_mngr.h"
#include "user_db.h"
#include "config.h"


struct userMngr_Stru		user_mngr_unit;
extern struct main_mngr_info main_mngr;

// remove dir not empty
int remove_dir(const char *dir)
{
    char cur_dir[] = ".";
    char up_dir[] = "..";
    char dir_name[128];
    DIR *dirp;
    struct dirent *dp;
    struct stat dir_stat;

    // dir not exist
    if ( 0 != access(dir, F_OK) ) {
        return 0;
    }

    if ( 0 > stat(dir, &dir_stat) ) {
        perror("get directory stat error");
        return -1;
    }

    if ( S_ISREG(dir_stat.st_mode) ) {  // file
        remove(dir);
    } else if ( S_ISDIR(dir_stat.st_mode) ) {   // dir
        dirp = opendir(dir);
        while ( (dp=readdir(dirp)) != NULL ) {
            //  . & ..
            if ( (0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name)) ) {
                continue;
            }

			memset(dir_name, 0, sizeof(dir_name));
			strcat(dir_name, dir);
			strcat(dir_name, "/");
			strcat(dir_name, dp->d_name);
            //sprintf(dir_name, "%s/%s", dir, dp->d_name);
            remove_dir(dir_name);   // recursive call
        }
        closedir(dirp);

        rmdir(dir);     // delete empty dir
    } else {
        perror("unknow file type!");    
    }

    return 0;
}

int user_add(struct db_userinfo *user)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;
	struct db_attend attend;
	char tbl_name[TABLE_NAME_LEN] = {0};
	int cursor = 0;
	int ret = 0;

	/* write user info to user table */
	db_user_write(user_mngr->userdb, user);
	
	/* create and set user attend info null */
	memset(&attend, 0, sizeof(struct db_attend));
	attend.id = user->id;
	memcpy(attend.name, user->name, USER_NAME_LEN);

	for(; ret==0; )
	{
		ret = db_attend_traverse_tbl(user_mngr->userdb, &cursor, tbl_name);
		if(ret==0 && strncmp(tbl_name, USERDB_TABLE, strlen(USERDB_TABLE)))
		{
			db_attend_write(user_mngr->userdb, tbl_name, &attend);
		}
	}
    printf("%s: id=%d, name: %s, face: %s\n", __FUNCTION__, user->id, user->name, user->facepath);

	return 0;
}

int user_delete(char *username)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;
	struct db_attend attend;
	char tbl_name[TABLE_NAME_LEN] = {0};
	char dir_name[64];
	int cursor = 0;
	int ret;

	ret = 0;
	for(; ret==0; )
	{
		ret = db_attend_traverse_tbl(user_mngr->userdb, &cursor, tbl_name);
		if(ret==0 && strncmp(tbl_name, USERDB_TABLE, strlen(USERDB_TABLE)))
		{
			ret = db_attend_read_byName(user_mngr->userdb, tbl_name, username, &attend);
			if(ret != 0)
			{
				continue;
			}
			ret = db_attend_delete_byName(user_mngr->userdb, tbl_name, username);

			/* remove face images */
			memset(dir_name, 0, sizeof(dir_name));
			sprintf(dir_name, "%s/%d_%s", FACES_IMAGES_PATH, attend.id, attend.name);
			remove_dir(dir_name);
		}
	}

	db_user_delete_byName(user_mngr->userdb, username);
    printf("%s: name: %s\n", __FUNCTION__, username);

	return 0;
}

/* get user face images and label(face id)*/
int user_get_faceimg_label(vector<Mat>& images, vector<int>& labels) 
{
    struct userMngr_Stru *usr_mngr = &user_mngr_unit;
    struct db_userinfo user;
	string img_path;
    int cursor = 0;
    int j, ret;

	ret = 0;
    for(; ret == 0; )
	{
        ret = db_user_traverse_user(usr_mngr->userdb, &cursor, &user);
        if(ret != 0)
            break;
		
		for(j=1; j<=CONFIG_FACE_IMGCNTUSER(main_mngr.config_ini); j++)
		{
			stringstream sstream;
			sstream << user.facepath << "/" << j << ".png";
			sstream >> img_path;
			/* check file if exist */
			if(access(img_path.c_str(), R_OK) == -1)
			{
				printf("%s: Warning: file[%s] not exist.\n", __FUNCTION__, img_path.c_str());
				continue;
			}

			images.push_back(imread(img_path, 0));
			labels.push_back(user.id);
		}
	}

	return 0;
}

/* create user dir by user name, format: i_username, like: 1_Tony, 2_Jenny ... */
int user_create_dir(char *base_dir, int id, char *usr_name, char *usr_dir)
{
	struct stat statbuf;
	char dir_path[64] = {0};
	char dir_head[8] = {0};
	int ret;

	if(base_dir==NULL || usr_name==NULL || usr_dir==NULL)
		return -1;

	/* check if dir or not */
	ret = lstat(base_dir, &statbuf);
	if(ret < 0)
	{
		ret = mkdir(base_dir, 0777);
		if(ret != 0)
			return -1;
	}
	else if(S_ISDIR(statbuf.st_mode) != 1)
	{
		ret = mkdir(base_dir, 0777);
		if(ret != 0)
			return -1;
	}
	
	sprintf(dir_head, "%d_", id);

	/* current dir is valid */
	memset(dir_path, 0, sizeof(dir_path));
	strcat(dir_path, base_dir);
	strcat(dir_path, "/");
	strcat(dir_path, dir_head);
	strcat(dir_path, usr_name);
	
	// create directory
	ret = mkdir((char *)dir_path, 0777);
	if(ret == -1)
	{
		printf("%s: mkdir %s not succeess, maybe exist.\n", __FUNCTION__, dir_path);
		//return -1;
	}

	memcpy(usr_dir, dir_path, strlen(dir_path));
	printf("mkdir: %s\n", dir_path);

	return 0;
}

int user_mngr_init(void)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;
    struct db_userinfo user;
    int cursor = 0;
    int i, ret;

	memset(user_mngr, 0, sizeof(struct userMngr_Stru));

	userdb_init(&user_mngr->userdb);

	/* list all user */
	ret = 0;
	i = 0;
    for(; ret == 0; )
    {
        ret = db_user_traverse_user(user_mngr->userdb, &cursor, &user);
        if(ret != 0)
            break;

		printf("[%d] user[id: %d], name: %s\n", i++, user.id, user.name);
    }

	return 0;
}

