#include <iostream>
#include <unistd.h>
#include "opencv_face_process.h"
#include "socket_server.h"
#include "user_mngr.h"
#include <QApplication>


/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
/* C head file */
#ifdef __cplusplus
}
#endif

using namespace std;

struct main_mngr_info main_mngr;

int main(int argc, char* argv[])
{
    QApplication qtApp(argc, argv);
	(void)argc;
	(void)argv;

	cout << "hello background_app" << endl;

	memset(&main_mngr, 0, sizeof(struct main_mngr_info));
	/* load config file */
	main_mngr.config_ini = iniparser_load(PATH_CONFIG_INI);
	if(main_mngr.config_ini == NULL)
	{
		printf("WARNING: %s: load [%s] failed, will use default value.\n", __FUNCTION__, PATH_CONFIG_INI);
		//return -1;	// will use default value
	}
	main_mngr.work_state = WORK_STA_NORMAL;
	main_mngr.user_handle = -1;
	main_mngr.mngr_handle = -1;
	main_mngr.capture_flag = 0;
	
	user_mngr_init();

	start_face_process_task();

	start_socket_server_task();

	return qtApp.exec();		// start qt application, message loop ...
}
