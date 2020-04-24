#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QWidget>
#include <QLabel>


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();


private:
	QWidget		*mainWindow;		// 中心窗口
	QLabel 		*videoArea;			// 图像显示区
	QImage		initWinImg;			// 初始背景图
	
	
};



int start_mainwindow_task(void);


#endif	// _MAINWINDOW_H_