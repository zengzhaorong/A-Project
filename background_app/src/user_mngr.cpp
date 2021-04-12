#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "user_mngr.h"
#include "user_db.h"


struct userMngr_Stru		user_mngr_unit;

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

int user_delete(int userCnt, char *username)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;
	struct userdb_user userInfo;
	char dir_name[64];
	int i, ret;

	for(i=0; i<userCnt; i++)
	{
		ret = userdb_read_byName(user_mngr->userdb, username+i*USER_NAME_LEN, &userInfo);
		if(ret != 0)
		{
			continue;
		}
		ret = userdb_delete_byName(user_mngr->userdb, username+i*USER_NAME_LEN);

		memset(dir_name, 0, sizeof(dir_name));
		sprintf(dir_name, "%s/%d_%s", FACES_DATABASE_PATH, userInfo.id, userInfo.name);
		remove_dir(dir_name);
	}

	return 0;
}

// create/update .csv file: dir_path-face lib path, csv_file: target .csv file
int user_create_csv(char *dir_path, char *csv_file)
{
	struct stat statbuf;
	DIR *dir = NULL;
	DIR *dirFile = NULL;
	struct dirent *dirp;
	struct dirent *direntFile;
	char 	dir_path2[64] = {0};
	char 	fileName[128] = {0};
	char 	witeBuf[128] = {0};
	char 	label[10] = {0};
	int 	fd;
	int 	i;

	if(dir_path==NULL || csv_file==NULL)
		return -1;

	if(dir_path[strlen(dir_path)-1] == '/')		// eg: "/mnt/"-> "/mnt"
		dir_path[strlen(dir_path)-1] = 0;

	if(lstat(dir_path, &statbuf) < 0)
	{
		printf("lstat(%s) failed !\n", dir_path);
		return -1;
	}

	if(S_ISDIR(statbuf.st_mode) != 1)
	{
		printf("%s is not dir !\n", dir_path); 
		return -1;
	}

	dir = opendir(dir_path);
	if( dir ==NULL)
	{
		printf("opendir failed.\n");
		return -1;
	}

	fd = open(csv_file, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(fd < 0)
	{
		printf("open file failed.\n");
		return -1;
	}

	lseek(fd, 0, SEEK_SET);

	while((dirp = readdir(dir)) != NULL)
	{
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;

		memset(dir_path2, 0, sizeof(dir_path2));
		strcat(dir_path2, dir_path);
		strcat(dir_path2, "/");
		strcat(dir_path2, dirp->d_name);

		if(lstat(dir_path2, &statbuf) < 0)
		{
			continue;
		}

		if(S_ISDIR(statbuf.st_mode) != 1)
		{
			continue;
		}

		dirFile = opendir(dir_path2);
		if( dirFile ==NULL)
		{
			printf("opendir failed.\n");
			return -1;
		}

		while((direntFile = readdir(dirFile)) != NULL)
		{
			if( strncmp(direntFile->d_name, ".", strlen(direntFile->d_name))==0 || 
				 strncmp(direntFile->d_name, "..", strlen(direntFile->d_name))==0 )
				continue;

			// whole dir path
			memset(fileName, 0, sizeof(fileName));
			strcat(fileName, dir_path2);
			strcat(fileName, "/");
			strcat(fileName, direntFile->d_name);

			if(lstat(fileName, &statbuf) < 0)
			{
				printf("lstat(%s) failed !\n", fileName);
				continue;
			}
			if(S_ISREG(statbuf.st_mode) != 1)
			{
				printf("%s is not reg file !\n", fileName); 
				continue;
			}

			memset(label, 0, sizeof(label));
			memset(witeBuf, 0, sizeof(witeBuf));
			memcpy(witeBuf, fileName, strlen(fileName));
			strcat(witeBuf, ";");

			for(i=0; i<10; i++)		// get seq
			{
				if(dirp->d_name[i] != '_')
					label[i] = dirp->d_name[i];
				else
					break;
			}
			if(i == 10)
				continue; 
			
			strcat(witeBuf, label);
			strcat(witeBuf, "\n");

			// write
			if(write(fd, witeBuf, strlen(witeBuf)) != (ssize_t)strlen(witeBuf))
			{
				printf("%s: write failed!\n", __FUNCTION__);
				closedir(dirFile);
				closedir(dir);
				close(fd);
				return -1;
			}
		}
	}

	closedir(dirFile);
	closedir(dir);
	close(fd);

	return 0;
}


int user_read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') 
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) 
	{
        printf("No csv file!");
		return -1;
    }
	
    string line, path, classlabel;
    while (getline(file, line)) {
        stringstream liness(line);
        getline(liness, path, separator);
        getline(liness, classlabel);
        if(!path.empty() && !classlabel.empty()) {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }

	return 0;
}

/* check dir begin with "%d_" exist or not */
/* 0 -exist, -1 -not exist */
int user_checkdir(char *base_dir, char *dir_head)
{
	struct stat statbuf;
	DIR *dir;
	struct dirent *dirent;
	int ret;

	ret = lstat(base_dir, &statbuf);
	if(ret < 0)
		return -1;

	if(S_ISDIR(statbuf.st_mode) != 1)
		return -1;

	dir = opendir(base_dir);
	if(dir == NULL)
		return -1;

	/* traversal dir */
	while((dirent=readdir(dir)) != NULL)
	{
		/* skip "." and ".." */
		if(!strncmp(dirent->d_name, ".", strlen(dirent->d_name)) ||
			!strncmp(dirent->d_name, "..", strlen(dirent->d_name)))
		{
			continue;
		}

		if(strncmp(dirent->d_name, dir_head, strlen(dir_head)) == 0)
		{
			return 0;
		}
	}

	return -1;
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
	ret = user_checkdir(base_dir, dir_head);
	if(ret == 0)
	{
		printf("ERROR: %s: user id [%d] is exist!\n", __FUNCTION__, id);
		return -1;
	}

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
		return -1;
	}

	memcpy(usr_dir, dir_path, strlen(dir_path));
	printf("mkdir: %s\n", dir_path);

	return 0;
}

int user_mngr_init(void)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;

	userdb_init(&user_mngr->userdb);

	return 0;
}


