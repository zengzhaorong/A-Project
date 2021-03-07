#ifndef _USER_MNGR_H_
#define _USER_MNGR_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#ifdef __cplusplus
}
#endif

#define FACES_DATABASE_PATH		"faces"
#define FACES_DB_CSV_FILE			(FACES_DATABASE_PATH"/""faces.csv")


using namespace cv;
using namespace std;

struct userInfo_Stru
{
	int 	id;					// user id
	char	name[USER_NAME_LEN];	// user name
};

struct userMngr_Stru
{
	int		userCnt;		// user count
	struct userInfo_Stru	*userInfo;		// user name information
	char 	add_userdir[64];	// use when add user
	char 	newuser[USER_NAME_LEN];	// the newest user
	int 	add_index;		// add user num
};


int user_delete(int userCnt, char *username);
int user_get_userList(char *faces_lib, struct userInfo_Stru **ppUserList, int *Count);
int user_create_dir(char *base_dir, char *usr_name, char *usr_dir);
int user_create_csv(char *dir_path, char *csv_file);
int user_read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator);
int user_mngr_init(void);


#endif	// _USER_MNGR_H_