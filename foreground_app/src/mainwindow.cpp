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


static MainWindow *mainwindow;		// 主界面
extern struct main_mngr_info main_mngr;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	QFont font;
	QPalette pa;

	/* can show Chinese word */
	QTextCodec *codec = QTextCodec::codecForName("GBK");
	setWindowTitle(codec->toUnicode(MAINWIN_TITLE));
	
	resize(MAIN_WIN_ROW, MAIN_WIN_COL);

	mainWindow = new QWidget;
	setCentralWidget(mainWindow);

	initWinImg.load(WIN_BACKGRD_IMG);

	/* show video area */
	videoArea = new QLabel(mainWindow);
	videoArea->setPixmap(QPixmap::fromImage(initWinImg));
	videoArea->setGeometry(0, 0, VIDEO_AREA_ROW, VIDEO_AREA_COL);
	videoArea->show();
	
	font.setPointSize(26);
	pa.setColor(QPalette::WindowText,Qt::yellow);
	textOnVideo = new QLabel(mainWindow);
	textOnVideo->setFont(font);
	textOnVideo->setPalette(pa);

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

	tmpShowTimer = new QTimer(this);

	mainwin_state = WORK_STA_NORMAL;

}

MainWindow::~MainWindow()
{
	
}

void MainWindow::showMainwindow()
{
	static Rect old_rect;
	static int old_rect_cnt = 0;
	static int old_state = WORK_STA_NORMAL;
	mainwin_mode_e mode;
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

		videoArea->setPixmap(QPixmap::fromImage(videoQImage));
		videoArea->show();
	}

	if(mainwin_state != old_state)
	{
		if(mainwin_state == WORK_STA_ADDUSER)
		{
			mode = MAINWIN_MODE_ADDUSER;
		}
		else if(old_state==WORK_STA_ADDUSER && mainwin_state==WORK_STA_NORMAL)
		{
			mode = MAINWIN_MODE_ADDUSER_OK;
		}
		else if(mainwin_state == WORK_STA_RECOGN)
		{
			mode = MAINWIN_MODE_RECOGN;
		}
		else
		{
			mode = MAINWIN_MODE_NORAML;
		}
		
		switch_mainwin_mode(mode);
		old_state = mainwin_state;
	}

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
	mainwin_change_worksta(WORK_STA_ADDUSER);

	proto_0x04_switchWorkSta(main_mngr.socket_handle, main_mngr.work_state, username);
}

void MainWindow::textOnVideo_show_over()
{
	tmpShowTimer->stop();
	textOnVideo->hide();
	mainwin_state = WORK_STA_NORMAL;
}

int MainWindow::switch_mainwin_mode(mainwin_mode_e mode)
{
	int addface_x;

	if(mode == MAINWIN_MODE_ADDUSER)
	{
		tmpShowTimer->stop();
		addface_x = 200;
		textOnVideo->setGeometry(addface_x, 0, VIDEO_AREA_ROW -addface_x, 50);
		QTextCodec *codec = QTextCodec::codecForName("GBK");
		textOnVideo->setText(codec->toUnicode(BEGIN_ADD_FACE_TEXT));
		textOnVideo->show();
	}
	else if(mode == MAINWIN_MODE_ADDUSER_OK)
	{
		addface_x = 230;
		textOnVideo->setGeometry(addface_x, 0, VIDEO_AREA_ROW -addface_x, 50);
		QTextCodec *codec = QTextCodec::codecForName("GBK");
		textOnVideo->setText(codec->toUnicode(SUCCESS_ADD_FACE_TEXT));
		textOnVideo->show();
		QObject::connect(tmpShowTimer, SIGNAL(timeout()), this, SLOT(textOnVideo_show_over()));
		tmpShowTimer->start(TIMER_ADDUSER_OK_MS);
	}
	else if(mode == MAINWIN_MODE_RECOGN)
	{
		addface_x = 200;
		textOnVideo->setGeometry(addface_x, 0, VIDEO_AREA_ROW -addface_x, 50);
		QTextCodec *codec = QTextCodec::codecForName("GBK");
		textOnVideo->setText(codec->toUnicode(userName));
		textOnVideo->show();
		QObject::connect(tmpShowTimer, SIGNAL(timeout()), this, SLOT(textOnVideo_show_over()));
		tmpShowTimer->start(RECOGN_OK_DELAY_MS);
	}
	else
	{
	}

	return 0;
}

int mainwin_change_worksta(int state)
{
	mainwindow->mainwin_state = state;

	return 0;
}

/* id: -1, only show usr_name */
int mainwin_set_userInfo(int id, char *usr_name)
{

	if(usr_name == NULL)
		return -1;

	memset(mainwindow->userName, 0, sizeof(mainwindow->userName));
	
	if(id == -1)
	{
		sprintf(mainwindow->userName, "%s", usr_name);
	}
	else
	{
		sprintf(mainwindow->userName, "%d: %s", id, usr_name);
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


