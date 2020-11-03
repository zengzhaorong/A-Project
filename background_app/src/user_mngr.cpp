#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include "user_mngr.h"


struct userMngr_Stru		user_mngr_unit;


// ��ȡ�û������б�, ppUserList���ָ��, Count����
int user_get_userList(char *faces_lib, struct userInfo_Stru **ppUserList, int *Count)
{
	struct stat statbuf;
	DIR *dir;
	struct dirent *dirp;
	char 	tmpDirPath[128] = {0};
	char	label[10] = {0};
	struct userInfo_Stru 	tmpUserInfo;
	struct userInfo_Stru 	*pUserMem;
	int 	dirCnt = 0;
	int		dirNum = 0;
	int 	i;

	// ��ȡ�ļ�����
	if(lstat(faces_lib, &statbuf) < 0)
	{
		printf("%s: lstat(%s) failed !\n", __FUNCTION__, faces_lib);
		return -1;
	}

	// �ж��Ƿ�ΪĿ¼
	if(S_ISDIR(statbuf.st_mode) != 1)
	{
		printf("%s: %s is not dir !\n", __FUNCTION__, faces_lib); 
		return -1;
	}

	// ��Ŀ¼
	dir = opendir(faces_lib);
	if( dir ==NULL)
	{
		printf("opendir failed.\n");
		return -1;
	}
	// ����һ��Ŀ¼��ͳ������
	while((dirp = readdir(dir)) != NULL)
	{
		// ���� '.' '..'�ļ���linux��
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;
		
		dirCnt ++;
	}
	closedir(dir);

	*Count = dirCnt;	// total dir count
	
	pUserMem = (struct userInfo_Stru *)malloc((*Count) *sizeof(struct userInfo_Stru));
	if(pUserMem == NULL)
	{
		printf("%s: malloc for pUserMem failed.\n", __FUNCTION__);
		return -1;
	}

	// ��Ŀ¼
	dir = opendir(faces_lib);
	if( dir ==NULL)
	{
		free(pUserMem);
		printf("opendir failed.\n");
		return -1;
	}
	// ����һ��Ŀ¼����ȡ��Ϣ
	while((dirp = readdir(dir)) != NULL)
	{
		// ���� '.' '..'�ļ���linux��
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;

		// ��1��Ŀ¼��2��Ŀ¼���
		memset(tmpDirPath, 0, sizeof(tmpDirPath));
		strcat(tmpDirPath, faces_lib);
		strcat(tmpDirPath, "/");
		strcat(tmpDirPath, dirp->d_name);

		if(lstat(tmpDirPath, &statbuf) < 0)
		{
			printf("%s: lstat(%s) failed !\n", __FUNCTION__, tmpDirPath);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode) != 1)
		{
			printf("%s: %s is not dir !\n", __FUNCTION__, tmpDirPath); 
			continue;
		}

		memset(label, 0, sizeof(label));
		for(i=0; i<10; i++) 	// ȡ���
		{
			if(dirp->d_name[i] != '_')
				label[i] = dirp->d_name[i];
			else
				break;
		}
		if(i == 10)
			continue; 

		// get seq
		memset(&tmpUserInfo, 0, sizeof(tmpUserInfo));
		tmpUserInfo.seq = atoi(label);
		strncpy(tmpUserInfo.name, dirp->d_name + i+1, strlen(dirp->d_name)-(i+1));

		memcpy(pUserMem+dirNum, &tmpUserInfo, sizeof(struct userInfo_Stru));
		dirNum ++;
//		printf("seq: %d\t name: %s\n", (pUserMem+dirNum)->seq , (pUserMem+dirNum)->name);
	}

	closedir(dir);

	if(*ppUserList != NULL)		// free last time source
	{
		free(*ppUserList);
		*ppUserList = NULL;
	}
	*ppUserList = pUserMem;

	return 0;
}

// ����/����CSV�ļ� ����: dir_path-������·��, csv_file: target csv file
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

	if(dir_path[strlen(dir_path)-1] == '/')		// ͳһ���벻��'/'�������� "/mnt/" ��Ϊ "/mnt"
		dir_path[strlen(dir_path)-1] = 0;

	// ��ȡ�ļ�����
	if(lstat(dir_path, &statbuf) < 0)
	{
		printf("lstat(%s) failed !\n", dir_path);
		return -1;
	}

	// �ж��Ƿ�ΪĿ¼
	if(S_ISDIR(statbuf.st_mode) != 1)
	{
		printf("%s is not dir !\n", dir_path); 
		return -1;
	}

	// ��Ŀ¼
	dir = opendir(dir_path);
	if( dir ==NULL)
	{
		printf("opendir failed.\n");
		return -1;
	}

	// �������csv�ļ�
	fd = open(csv_file, O_RDWR | O_CREAT | O_TRUNC, 0777);
	if(fd < 0)
	{
		printf("open file faile.\n");
		return -1;
	}

	// ��λ��дλ��
	lseek(fd, 0, SEEK_SET);

	// ����һ��Ŀ¼
	while((dirp = readdir(dir)) != NULL)
	{
		// ���� '.' '..'�ļ���linux��
		if( strncmp(dirp->d_name, ".", strlen(dirp->d_name))==0 || 
			 strncmp(dirp->d_name, "..", strlen(dirp->d_name))==0 )
			continue;

		// ��1��Ŀ¼��2��Ŀ¼���
		memset(dir_path2, 0, sizeof(dir_path2));
		strcat(dir_path2, dir_path);
		strcat(dir_path2, "/");
		strcat(dir_path2, dirp->d_name);

		if(lstat(dir_path2, &statbuf) < 0)
		{
			printf("lstat(%s) failed !\n", dir_path2);
			continue;
		}

		if(S_ISDIR(statbuf.st_mode) != 1)
		{
			printf("%s is not dir !\n", dir_path2); 
			continue;
		}

		dirFile = opendir(dir_path2);
		if( dirFile ==NULL)
		{
			printf("opendir failed.\n");
			return -1;
		}

		// ��������Ŀ¼
		while((direntFile = readdir(dirFile)) != NULL)
		{
			if( strncmp(direntFile->d_name, ".", strlen(direntFile->d_name))==0 || 
				 strncmp(direntFile->d_name, "..", strlen(direntFile->d_name))==0 )
				continue;

			// ��ȡ�����ļ�·����
			memset(fileName, 0, sizeof(fileName));
			strcat(fileName, dir_path2);
			strcat(fileName, "/");
			strcat(fileName, direntFile->d_name);

			if(lstat(fileName, &statbuf) < 0)
			{
				printf("lstat(%s) failed !\n", fileName);
				continue;
			}
			if(S_ISREG(statbuf.st_mode) != 1)	// ������ͨ�ļ�
			{
				printf("%s is not reg file !\n", fileName); 
				continue;
			}

			memset(label, 0, sizeof(label));
			memset(witeBuf, 0, sizeof(witeBuf));
			memcpy(witeBuf, fileName, strlen(fileName));
			strcat(witeBuf, ";");

			for(i=0; i<10; i++)		// ȡ���
			{
				if(dirp->d_name[i] != '_')
					label[i] = dirp->d_name[i];
				else
					break;
			}
			if(i == 10)
				continue; 
			
			strcat(witeBuf, label);	// ��ǩ: '_'ǰ���ַ�
			strcat(witeBuf, "\n");

			// д����Ϣ
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


void user_read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') 
{
    std::ifstream file(filename.c_str(), ifstream::in);
    if (!file) {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(Error::StsBadArg, error_message);
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
}



int user_mngr_init(void)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;

	int ret;

	user_mngr->pstUserInfo = NULL;
	user_mngr->userCnt = 0;

	user_get_userList((char *)FACES_LIB_PATH, &user_mngr->pstUserInfo, &user_mngr->userCnt);
	for(int i=0; i<user_mngr->userCnt; i++)
	{
		printf("[%d].seq=%d, name: %s\n", i, user_mngr->pstUserInfo[i].seq, user_mngr->pstUserInfo[i].name);
	}

	ret = user_create_csv((char *)FACES_LIB_PATH, (char *)FACES_CSV_FILE);
	if(ret != 0)
		return -1;
	
	printf("create faces csv file successfully.\n");


	return 0;
}


