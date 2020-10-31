#ifndef _USER_MNGR_H_
#define _USER_MNGR_H_


#define FACES_LIB_PATH		"faces"
#define FACES_CSV_FILE		"faces.csv"

#define USER_NAME_LEN		64

struct userInfo_Stru
{
	int 	seq;					// seq number
	char	name[USER_NAME_LEN];	// user name
};

int user_mngr_init(void);


#endif	// _USER_MNGR_H_