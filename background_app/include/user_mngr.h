#ifndef _USER_MNGR_H_
#define _USER_MNGR_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#define FACES_LIB_PATH		"faces"
#define FACES_CSV_FILE		"faces.csv"

#define USER_NAME_LEN		64

using namespace cv;
using namespace std;

struct userInfo_Stru
{
	int 	seq;					// seq number
	char	name[USER_NAME_LEN];	// user name
};

struct userMngr_Stru
{
	int		userCnt;		// user count
	struct userInfo_Stru	*pstUserInfo;		// user name information
	char 	add_userdir[64];	// use when add user
	int 	add_index;		// add user num
};


int user_get_userList(char *faces_lib, struct userInfo_Stru **ppUserList, int *Count);
int user_create_dir(char *base_dir, char *usr_name, char *usr_dir);
int user_mngr_init(void);
int user_create_csv(char *dir_path, char *csv_file);
void user_read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator);


#endif	// _USER_MNGR_H_