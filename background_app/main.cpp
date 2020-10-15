#include <iostream>
#include <unistd.h>
#include "opencv_face_process.h"
#include <QApplication>

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "socket_server.h"
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

	start_opencv_face_task();

	user_mngr_init();
	return qtApp.exec();		// ����QTӦ�õ�ִ�У���Ϣѭ�����¼�����

	while(1)
	{
		printf("sec: %d\n", (sec++)*3);
		sleep(3);
	}

	return 0;
}
