#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include "mainwindow.h"
#include "image_convert.h"
#include "opencv_image_process.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "capture.h"
#ifdef __cplusplus
}
#endif

#define MAIN_WIN_ROW			900
#define MAIN_WIN_COL			480
#define VIDEO_AREA_ROW			640
#define VIDEO_AREA_COL			480

#define TIMER_INTERV_MS			1

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

	buf_size = VIDEO_AREA_ROW*VIDEO_AREA_COL*3;
	video_buf = (unsigned char *)malloc(buf_size);
	if(video_buf == NULL)
	{
		buf_size = 0;
		printf("ERROR: malloc for video_buf failed!");
	}

	/* 定时任务，显示图像 */
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(showMainwindow()));
	timer->start(TIMER_INTERV_MS);

}

MainWindow::~MainWindow()
{
	
}

void MainWindow::showMainwindow()
{
	static Rect old_rect;
	static int old_rect_cnt = 0;
	int len;
	int ret;

	timer->stop();

	ret = capture_getframe(video_buf, buf_size, &len);
	if(ret == 0)
	{
		QImage videoQImage;
		videoQImage = v4l2_to_QImage(video_buf, len);

		Rect rects;
		ret = get_rect_param(rects);
		if(ret == 0)
		{
			opencv_image_add_rect(videoQImage, rects);
			old_rect = rects;
		}
		else if(old_rect.width > 0)
		{
			opencv_image_add_rect(videoQImage, old_rect);
			old_rect_cnt ++;
			if(old_rect_cnt *TIMER_INTERV_MS > 100)
			{
				old_rect.width = 0;
				old_rect_cnt = 0;
			}
		}
			
		videoArea->setPixmap(QPixmap::fromImage(videoQImage));
		videoArea->show();
	}
	
	timer->start(TIMER_INTERV_MS);
	
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

/* 注：
 * 用定时器触发刷新显示主界面 
 * 用线程循环有问题！未解 
 */
int start_mainwindow_task(void)
{
	mainwindow_init();

	return 0;
}


