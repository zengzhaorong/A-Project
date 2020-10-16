#include <iostream>
#include <unistd.h>
#include "opencv_face_process.h"
#include "socket_server.h"
#include <QApplication>

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
    QApplication qtApp(argc, argv);

	cout << "hello background_app" << endl;

	start_socket_server_task();

	start_face_process_task();

	user_mngr_init();
	return qtApp.exec();		// 启动QT应用的执行，消息循环和事件处理

	while(1)
	{
		printf("sec: %d\n", (sec++)*3);
		sleep(3);
	}

	return 0;
}
