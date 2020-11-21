#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include "config.h"
#include "mainwindow.h"
#include "image_convert.h"
#include "opencv_image_process.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "protocol.h"
#include "capture.h"
#include "public.h"
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
extern struct main_mngr_info main_mngr;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	
	setWindowTitle(tr(MAINWIN_TITLE));
	
	resize(MAIN_WIN_ROW, MAIN_WIN_COL);

	mainWindow = new QWidget;
	setCentralWidget(mainWindow);

	initWinImg.load(WIN_BACKGRD_IMG);

	videoArea = new QLabel(mainWindow);
	videoArea->setPixmap(QPixmap::fromImage(initWinImg));
	videoArea->setGeometry(0, 0, VIDEO_AREA_ROW, VIDEO_AREA_COL);
	videoArea->show();

	/* user name QString */
	userNameStr = QString(tr("welcome"));
	showUserTick = 0;
	
	/* label to show user name */
	userInfoLab = new QLabel(mainWindow);
	userInfoLab->setGeometry(650,140,250,30);
	userInfoLab->setText(userNameStr);
	userInfoLab->hide();
	/* set font size */
	QFont font;
	font.setPointSize(16);
	userInfoLab->setFont(font);

	userNameEdit = new QLineEdit(mainWindow);
	userNameEdit->setPlaceholderText(tr("Add User Name"));
	userNameEdit->setGeometry(650, 230, 150, 40);

	addUserBtn = new QPushButton(mainWindow);
	addUserBtn->setText(tr("Add user"));
    connect(addUserBtn, SIGNAL(clicked()), this, SLOT(addUser()));
	addUserBtn->setGeometry(810, 230, 80, 40);

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

	/* show capture image */
	ret = capture_getframe(video_buf, buf_size, &len);
	if(ret == 0)
	{
		QImage videoQImage;
#if defined(CAP_V4L2_FMT_JPEG)
		videoQImage = jpeg_to_QImage(video_buf, len);
#elif defined(CAP_V4L2_FMT_YUV)
		videoQImage = yuv_to_QImage(0, video_buf, CAPTURE_PIX_WIDTH, CAPTURE_PIX_HEIGH);
#elif defined(CAP_V4L2_FMT_MJPEG)
		videoQImage = jpeg_to_QImage(video_buf, len);
#endif

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
			if(old_rect_cnt *TIMER_INTERV_MS > 10)
			{
				old_rect.width = 0;
				old_rect_cnt = 0;
			}
		}

		if(showUserTick > 30)
		{
			mainwin_set_userInfo(-1, (char *)"welcome");
		}
		else
		{
			showUserTick ++;
		}

		videoArea->setPixmap(QPixmap::fromImage(videoQImage));
		videoArea->show();
	}

	/* show other label */
	userInfoLab->setText(userNameStr);
	userInfoLab->show();
	
	timer->start(TIMER_INTERV_MS);
	
}

void MainWindow::addUser()
{
	QString editStr;
	QByteArray ba;
	uint8_t username[32];

	/* get QLineEdit input text */
	editStr = userNameEdit->text();
	ba = editStr.toLatin1();
	memset(username, 0, sizeof(username));
	strncpy((char *)username, ba.data(), strlen(ba.data()));

	main_mngr.work_state = WORK_STA_ADDUSER;

	proto_0x04_switchWorkSta(main_mngr.socket_handle, main_mngr.work_state, username);
}


/* id: -1, only show usr_name */
int mainwin_set_userInfo(int id, char *usr_name)
{

	if(usr_name == NULL)
		return -1;

	if(id == -1)
	{
		mainwindow->userNameStr = QString("%1").arg(usr_name);
	}
	else
	{
		mainwindow->userNameStr = QString("[id:%1] name:%2").arg(id).arg(usr_name);
		mainwindow->showUserTick = 0;
	}

	return 0;
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


