#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include "mainwindow.h"
#include "capture.h"


#define MAIN_WIN_ROW			900
#define MAIN_WIN_COL			480
#define VIDEO_AREA_ROW			640
#define VIDEO_AREA_COL			480


#define WIN_BACKGRD_IMG				"resource/gdut.jpg"		// 界面背景图

static MainWindow *mainwindow;		// 主界面

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	
	setWindowTitle("mainwindow");
	
	resize(MAIN_WIN_ROW, MAIN_WIN_COL);

	mainWindow = new QWidget;
	setCentralWidget(mainWindow);

	initWinImg.load(WIN_BACKGRD_IMG);

	videoArea = new QLabel(mainWindow);
	videoArea->setPixmap(QPixmap::fromImage(initWinImg));
	videoArea->setGeometry(0, 0, VIDEO_AREA_ROW, VIDEO_AREA_COL);
	videoArea->show();

}

MainWindow::~MainWindow()
{
	
}

int mainwindow_init(void)
{
	mainwindow = new MainWindow;

	mainwindow->show();
	
	return 0;
}

void mainwindow_deinit(void)
{

}

void *mainwindow_thread(void *arg)
{

	mainwindow_init();

	while(1)
	{
		printf("%s %d: running ...\n", __FUNCTION__, __LINE__);
		sleep(3);
	}

	mainwindow_deinit();

}

int start_mainwindow_task(void)
{
	pthread_t tid;
	int ret;
	
	ret = pthread_create(&tid, NULL, mainwindow_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}

	return 0;
}


