#include <iostream>
#include <QApplication>
#include "opencv_test.h"
#include "mainwindow.h"
#include "capture.h"

/* C++ ���� C */
#ifdef __cplusplus
	extern "C" {
#endif

#include "socket_client.h"

#ifdef __cplusplus
	}
#endif


using namespace std;


int main(int argc, char* argv[])
{
    QApplication qtApp(argc, argv);
	
	cout << "hello foreground_app" << endl;
	
	//hello_opencv();

	start_mainwindow_task();

	start_capture_task();

	start_socket_client_task();


	return qtApp.exec();		// ����QTӦ�õ�ִ�У���Ϣѭ�����¼�����
}
