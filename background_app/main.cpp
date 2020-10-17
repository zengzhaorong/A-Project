#include <iostream>
#include <unistd.h>
#include "opencv_face_process.h"
#include "socket_server.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "user_mngr.h"
#ifdef __cplusplus
}
#endif

using namespace std;


int main(int argc, char* argv[])
{
	int sec = 0;
	
	(void)argc;
	(void)argv;

	cout << "hello background_app" << endl;

	start_socket_server_task();

	start_face_process_task();

	user_mngr_init();

	while(1)
	{
		printf("sec: %d\n", (sec++)*3);
		sleep(3);
	}

	return 0;
}
