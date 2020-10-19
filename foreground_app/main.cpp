#include <iostream>
#include <QApplication>
#include "mainwindow.h"
#include "socket_client.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "capture.h"
#ifdef __cplusplus
}
#endif


using namespace std;


int main(int argc, char* argv[])
{
    QApplication qtApp(argc, argv);
	
	cout << "hello foreground_app" << endl;
	
	start_mainwindow_task();

	start_capture_task();

	start_socket_client_task();


	return qtApp.exec();		// 启动QT应用的执行，消息循环和事件处理
}
