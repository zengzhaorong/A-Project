#include <iostream>
#include <QApplication>
#include "opencv_test.h"
#include "mainwindow.h"

using namespace std;


int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
	
	cout << "hello foreground_app" << endl;
	
	MainWindow mainWin;
	mainWin.show();
	
	hello_opencv();

	return app.exec();		// 启动QT应用的执行，消息循环和事件处理
}
