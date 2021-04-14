#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include "config.h"
#include "mainwindow.h"
#include "image_convert.h"


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


static MainWindow *mainwindow;
extern struct main_mngr_info main_mngr;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	QImage image;
	QFont font;
	QPalette pa;
	QTextCodec *codec = QTextCodec::codecForName("GBK");

	/* can show Chinese word */
	setWindowTitle(codec->toUnicode(MAINWINDOW_TITLE));
	
	resize(MAIN_WIN_ROW, MAIN_WIN_COL);

	mainWindow = new QWidget;
	setCentralWidget(mainWindow);

	backgroundImg.load(WIN_BACKGRD_IMG);

	/* show video area */
	videoArea = new QLabel(mainWindow);
	videoArea->setPixmap(QPixmap::fromImage(backgroundImg));
	videoArea->setGeometry(0, 0, VIDEO_AREA_ROW, VIDEO_AREA_COL);
	videoArea->show();
	
	font.setPointSize(20);
	pa.setColor(QPalette::WindowText,Qt::yellow);
	textOnVideo = new QLabel(mainWindow);
	textOnVideo->setFont(font);
	textOnVideo->setPalette(pa);

	/* clock */
	clockLabel = new QLabel(mainWindow);
	clockLabel->setWordWrap(true);	// adapt to text, can show multi row
	clockLabel->setGeometry(650, 0, 140, 90);	// height: set more bigger to adapt to arm
	clockLabel->show();

#if defined(USER_CLIENT_ENABLE) && !defined(MANAGER_CLIENT_ENABLE)
	image.load(OTHER_INFO_IMG);
	otherInfo = new QLabel(mainWindow);
	otherInfo->setPixmap(QPixmap::fromImage(image));
	otherInfo->setGeometry(640, 100, 160, 400);
	otherInfo->show();
#endif

#ifdef MANAGER_CLIENT_ENABLE
	/* attend in time edit */
	LabelAtdIn = new QLabel(mainWindow);
	LabelAtdIn->setGeometry(660, 158, 50, 30);
	LabelAtdIn->setText(codec->toUnicode(TEXT_ATTEND_IN":"));
	LabelAtdIn->show();
	TimeEditAtdIn = new QDateTimeEdit(/*QDateTime::currentDateTime(),*/ this);
	TimeEditAtdIn->setDisplayFormat("HH:mm:ss");
	TimeEditAtdIn->setGeometry(700, 158, 90, 30);
	TimeEditAtdIn->show();

	/* set datetime */
	//QDateTime datetime = QDateTime::fromString(QString("01:02:03"),"HH:mm:ss");
	//TimeEditAtdIn->setDateTime(datetime);

	/* attend out time edit */
	LabelAtdOut = new QLabel(mainWindow);
	LabelAtdOut->setGeometry(660, 190, 50, 30);
	LabelAtdOut->setText(codec->toUnicode(TEXT_ATTEND_OUT":"));
	LabelAtdOut->show();
	TimeEditAtdOut = new QDateTimeEdit(/*QDateTime::currentDateTime(),*/ this);
	TimeEditAtdOut->setDisplayFormat("HH:mm:ss");
	TimeEditAtdOut->setGeometry(700, 190, 90, 30);
	TimeEditAtdOut->show();

	/* attend time set button */
	setAtdtimeBtn = new QPushButton(mainWindow);
	setAtdtimeBtn->setText(codec->toUnicode(TEXT_SET_ATD_TIME));
    connect(setAtdtimeBtn, SIGNAL(clicked()), this, SLOT(setAttendTime()));
	setAtdtimeBtn->setGeometry(660, 222, 120, 30);

	/* user id - name edit */
	userIdEdit = new QLineEdit(mainWindow);
	userIdEdit->setPlaceholderText(codec->toUnicode(TEXT_USER_ID));
	userIdEdit->setGeometry(645, 265, 50, 30);
	userNameEdit = new QLineEdit(mainWindow);
	userNameEdit->setPlaceholderText(codec->toUnicode(TEXT_USER_NAME));
	userNameEdit->setGeometry(695, 265, 100, 30);
	/* add user button */
	addUserBtn = new QPushButton(mainWindow);
	addUserBtn->setText(codec->toUnicode(TEXT_ADD_USER));
    connect(addUserBtn, SIGNAL(clicked()), this, SLOT(addUser()));
	addUserBtn->setGeometry(670, 297, 100, 30);

	/* user list box */
	userListBox = new QComboBox(mainWindow);
	userListBox->setGeometry(645, 340, 150, 30);
	userListBox->setEditable(true);
	/* delete user button */
	delUserBtn = new QPushButton(mainWindow);
	delUserBtn->setText(codec->toUnicode(TEXT_DEL_USER));
    connect(delUserBtn, SIGNAL(clicked()), this, SLOT(deleteUser()));
	delUserBtn->setGeometry(670, 372, 100, 30);

	/* display time sheet button */
	timeSheetBtn = new QPushButton(mainWindow);
	timeSheetBtn->setText(codec->toUnicode(TEXT_TIMESHEET));
    connect(timeSheetBtn, SIGNAL(clicked()), this, SLOT(showTimeSheet()));
	timeSheetBtn->setGeometry(670, 415, 100, 30);
	/* save time sheet button */
	saveTimeSheetBtn = new QPushButton(mainWindow);
	saveTimeSheetBtn->setText(codec->toUnicode(TEXT_SAVE));
    connect(saveTimeSheetBtn, SIGNAL(clicked()), this, SLOT(saveTimeSheet()));
	saveTimeSheetBtn->setGeometry(660, 447, 60, 30);
	saveTimeSheetBtn->hide();
	/* reset time sheet button */
	resetTimeSheetBtn = new QPushButton(mainWindow);
	resetTimeSheetBtn->setText(codec->toUnicode(TEXT_RESET));
    connect(resetTimeSheetBtn, SIGNAL(clicked()), this, SLOT(resetTimeSheet()));
	resetTimeSheetBtn->setGeometry(723, 447, 60, 30);
	resetTimeSheetBtn->hide();

	tableView = new QTableView(mainWindow);
	userModel = new QStandardItemModel();
	userModel->setHorizontalHeaderLabels({"ID", "Name", "Time", "Status"});
	tableView->setSelectionBehavior(QAbstractItemView::SelectRows);		// set select the whole row 
	//tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);	// adapt to table veiw
	tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents );	// adapt to text
	tableView->setGeometry(0, 0, VIDEO_AREA_ROW, VIDEO_AREA_COL);
	tableView->hide();
#endif

	buf_size = VIDEO_AREA_ROW*VIDEO_AREA_COL*3;
	video_buf = (unsigned char *)malloc(buf_size);
	if(video_buf == NULL)
	{
		buf_size = 0;
		printf("ERROR: malloc for video_buf failed!");
	}

	/* set timer to show image */
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(showMainwindow()));
	timer->start(TIMER_INTERV_MS);

	tmpShowTimer = new QTimer(this);

	sys_state = &main_mngr.work_state;

}

MainWindow::~MainWindow(void)
{
	
}

void MainWindow::showMainwindow(void)
{
	static int old_state = WORK_STA_NORMAL;
	mainwin_mode_e mode;
	int len;
	int ret;

	timer->stop();

	QDateTime time = QDateTime::currentDateTime();
	QString str = time.toString("yyyy-MM-dd hh:mm:ss dddd");
	clockLabel->setText(str);

	/* show capture image */
	ret = capture_getframe(video_buf, buf_size, &len);
	if(ret == 0)
	{
		QImage videoQImage;

		videoQImage = jpeg_to_QImage(video_buf, len);

		/* draw face rectangles */
		drawFaceRectangle(videoQImage);

		videoArea->setPixmap(QPixmap::fromImage(videoQImage));
		videoArea->show();
	}

	if(*sys_state != old_state)
	{
		if(*sys_state == WORK_STA_DISCONNECT)
		{
			mode = MAINWIN_MODE_DISCONECT;
		}
		else if(*sys_state == WORK_STA_ADDUSER)
		{
			mode = MAINWIN_MODE_ADDUSER;
		}
		else if(old_state==WORK_STA_ADDUSER && *sys_state==WORK_STA_NORMAL)
		{
			mode = MAINWIN_MODE_ADDUSER_OK;
		}
		else if(*sys_state == WORK_STA_RECOGN)
		{
			mode = MAINWIN_MODE_RECOGN;
		}
		else
		{
			mode = MAINWIN_MODE_NORAML;
		}
		
		switch_mainwin_mode(mode);
		old_state = *sys_state;
	}

	timer->start(TIMER_INTERV_MS);
	
}

void MainWindow::drawFaceRectangle(QImage &img)
{
	static QRect old_rect;
	static int old_rect_cnt = 0;
	QRect rects;
	QPainter painter(&img);

	if(face_rects.width() > 0)
	{
		old_rect = face_rects;
		face_rects.setWidth(0);
	}
	if(old_rect.width() > 0)
	{
		painter.setPen(QPen(Qt::green, 3, Qt::SolidLine, Qt::RoundCap));
		painter.drawRect(old_rect.x(), old_rect.y(), old_rect.width(), old_rect.height());
		old_rect_cnt ++;
		if(old_rect_cnt *TIMER_INTERV_MS > 10)
		{
			old_rect.setWidth(0);
			old_rect_cnt = 0;
		}
	}

}

void MainWindow::setAttendTime(void)
{

	QDateTime timeAtdIn = TimeEditAtdIn->dateTime();
	QDateTime timeAtdOut = TimeEditAtdOut->dateTime();

	main_mngr.atdin_secday = time_t_to_sec_day(timeAtdIn.toTime_t());
	main_mngr.atdout_secday = time_t_to_sec_day(timeAtdOut.toTime_t());
	
	proto_0x13_setAttendTime(main_mngr.mngr_handle, main_mngr.atdin_secday, main_mngr.atdout_secday);
}

void MainWindow::addUser(void)
{
	char id_name[64] = {0};
	QString idStr;
	QString NameStr;
	QByteArray ba;
	char name[USER_NAME_LEN] = {0};
	int id;

	/* get input text */
	idStr = userIdEdit->text();
	NameStr = userNameEdit->text();
	if(idStr.length() <= 0 || NameStr.length() <= 0)
	{
		printf("%s: QLineEdit is empty !\n", __FUNCTION__);
		return ;
	}

	ba = idStr.toLatin1();
	id = atoi(ba.data());
	if(id <= 0)
	{
		printf("%s: ID is illegal !\n", __FUNCTION__);
		return ;
	}

	ba = NameStr.toLatin1();
	memset(name, 0, sizeof(name));
	strncpy((char *)name, ba.data(), strlen(ba.data()));

	printf("user Id: %d, name: %s\n", id, name);
	memcpy(id_name, &id, 4);
	memcpy(id_name +4, name, USER_NAME_LEN);

	/* opencv camera */
	#if defined(MANAGER_CLIENT_ENABLE) && !defined(USER_CLIENT_ENABLE)
	start_capture_task();
	#endif

	main_mngr.work_state = WORK_STA_ADDUSER;

	proto_0x04_switchWorkSta(main_mngr.mngr_handle, main_mngr.work_state, (uint8_t *)id_name);
}

void MainWindow::deleteUser(void)
{
	QString qstrUsrName;
	char user_name[USER_NAME_LEN] = {0};
	QByteArray ba;

	qstrUsrName = userListBox->currentText();
	ba = qstrUsrName.toLatin1();
	strncpy(user_name, ba.data(), strlen(ba.data()));
	if(strlen(user_name) <= 0)
		return ;

	if(QMessageBox::warning(this,"Warning", "Delete "+userListBox->currentText()+" ?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)
	{
		return ;
	}

	proto_0x06_deleteUser(main_mngr.mngr_handle, 1, user_name);
	
	mainwin_set_userList(0, 1, user_name);
		
}

void MainWindow::showTimeSheet(void)
{
	static bool showflag = 0;

	/* get attend list */
	proto_0x14_getAttendList(main_mngr.mngr_handle);
	mainwin_clear_attendList();

	showflag = !showflag;

	if(showflag == 1)
	{
		tableView->setModel(userModel);
		saveTimeSheetBtn->show();
		resetTimeSheetBtn->show();
		tableView->show();
	}
	else
	{
		saveTimeSheetBtn->hide();
		resetTimeSheetBtn->hide();
		tableView->hide();
	}

}

void MainWindow::saveTimeSheet(void)
{
	QString filestr;
    struct daytm tm_day;
    char filename[64] = {0};
	char intime_str[32] = {0};
	char outtime_str[32] = {0};

    memset(intime_str, 0, sizeof(intime_str));
    memset(outtime_str, 0, sizeof(outtime_str));
    sec_day_to_daytm(main_mngr.atdin_secday, &tm_day);
    printf("%s: in time: %02d:%02d:%02d\n", __FUNCTION__,tm_day.hour, tm_day.min, tm_day.sec);
    sprintf(intime_str, "%02d%02d%02d", tm_day.hour, tm_day.min, tm_day.sec);
    
    sec_day_to_daytm(main_mngr.atdout_secday, &tm_day);
    printf("%s:  out time: %02d:%02d:%02d\n", __FUNCTION__,tm_day.hour, tm_day.min, tm_day.sec);
    sprintf(outtime_str, "%02d%02d%02d", tm_day.hour, tm_day.min, tm_day.sec);

    sprintf(filename, "%s_%s_timesheet.csv", intime_str, outtime_str);
	
	proto_0x15_AttendSheetCtrl(main_mngr.mngr_handle, 1, filename);	// save

	filestr = QString(filename);
	QMessageBox::information(NULL, "Save", "timesheet has saved as "+filestr, QMessageBox::Yes, QMessageBox::Yes);
}

void MainWindow::resetTimeSheet(void)
{
	QColor color = QColor(192,80,77);;	
	QTextCodec *codec = QTextCodec::codecForName("GBK");
	int i;

	if(QMessageBox::warning(this,"Warning", "Reset timesheet ?",QMessageBox::Yes,QMessageBox::No)==QMessageBox::No)
	{
		return ;
	}

	proto_0x15_AttendSheetCtrl(main_mngr.mngr_handle, 0, NULL);	// reset
	//mainwin_clear_attendList();
	for(i=0; i<userModel->rowCount(); i++)
	{
		userModel->setItem(i, TIME_TABLE_STATUS_POS, new QStandardItem(QString("%1").arg(codec->toUnicode(TEXT_ATTEND_NULL ":" TEXT_ATTEND_NULL))));
		userModel->item(i, TIME_TABLE_STATUS_POS)->setBackground(QBrush(color));
	}
	

}

void MainWindow::textOnVideo_show_over(void)
{
	tmpShowTimer->stop();
	textOnVideo->hide();
	*sys_state = WORK_STA_NORMAL;
	mainwin_mode = MAINWIN_MODE_NORAML;
}

/* switch mainwindow display mode: normal, add user, add user ok, recognzie ... */
int MainWindow::switch_mainwin_mode(mainwin_mode_e mode)
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");
	char showText[128] = {0};
	int addface_x;

	mainwin_mode = mode;

	if(mode == MAINWIN_MODE_DISCONECT)
	{
		tmpShowTimer->stop();
		addface_x = 240;
		textOnVideo->setGeometry(addface_x, 0, VIDEO_AREA_ROW -addface_x, 50);
		textOnVideo->setText(codec->toUnicode(NOT_CONNECT_SERVER));
		textOnVideo->show();
	}
	else if(mode == MAINWIN_MODE_NORAML)
	{
		textOnVideo->hide();
	}
	else if(mode == MAINWIN_MODE_ADDUSER)
	{
		tmpShowTimer->stop();
		addface_x = 200;
		textOnVideo->setGeometry(addface_x, 0, VIDEO_AREA_ROW -addface_x, 50);
		textOnVideo->setText(codec->toUnicode(BEGIN_ADD_FACE_TEXT));
		textOnVideo->show();
	}
	else if(mode == MAINWIN_MODE_ADDUSER_OK)
	{
		addface_x = 230;
		textOnVideo->setGeometry(addface_x, 0, VIDEO_AREA_ROW -addface_x, 50);
		textOnVideo->setText(codec->toUnicode(SUCCESS_ADD_FACE_TEXT));
		textOnVideo->show();
		QObject::connect(tmpShowTimer, SIGNAL(timeout()), this, SLOT(textOnVideo_show_over()));
		tmpShowTimer->start(TIMER_ADDUSER_OK_MS);
		/* close capture, show background image */
		#if defined(MANAGER_CLIENT_ENABLE) && !defined(USER_CLIENT_ENABLE)
		capture_task_stop();
		videoArea->setPixmap(QPixmap::fromImage(backgroundImg));
		#endif
	}
	else if(mode == MAINWIN_MODE_RECOGN)
	{
		addface_x = 200;
		textOnVideo->setGeometry(addface_x, 0, VIDEO_AREA_ROW -addface_x, 50);
		if(face_status == ATTEND_STA_IN_OK)
		{
			sprintf(showText, "%s: %s - %d%c", TEXT_ATTEND_IN, userRecogn, confidence, '%');
		}
		else if(face_status == ATTEND_STA_OUT_OK)
		{
			sprintf(showText, "%s: %s - %d%c", TEXT_ATTEND_OUT, userRecogn, confidence, '%');
		}
		else if(face_status == ATTEND_STA_IN_LATE)
		{
			sprintf(showText, "%s: %s - %d%c", TEXT_ATTEND_IN_LATE, userRecogn, confidence, '%');
		}
		else if(face_status == ATTEND_STA_OUT_EARLY)
		{
			sprintf(showText, "%s: %s - %d%c", TEXT_ATTEND_OUT_EARLY, userRecogn, confidence, '%');
		}
		textOnVideo->setText(codec->toUnicode(showText));
		textOnVideo->show();
		QObject::connect(tmpShowTimer, SIGNAL(timeout()), this, SLOT(textOnVideo_show_over()));
		tmpShowTimer->start(RECOGN_OK_DELAY_MS);
	}
	else
	{
	}

	return 0;
}

int mainwin_set_rects(int x, int y, int w, int h)
{

	mainwindow->face_rects.setX(x);
	mainwindow->face_rects.setY(y);
	mainwindow->face_rects.setWidth(w);
	mainwindow->face_rects.setHeight(h);

	return 0;
}

/* flag: 0-delete, 1-add */
int mainwin_set_userList(int flag, int userCnt, char *usr_name)
{
	int index;
	int i;

	if(usr_name == NULL)
		return -1;

	for(i=0; i<userCnt; i++)
	{
		if(flag == 0)
		{
			/* delete user */
			index = mainwindow->userListBox->currentIndex();
			mainwindow->userListBox->removeItem(index);
		}
		else
		{
			/* add user */
			mainwindow->userListBox->addItem(usr_name +i*USER_NAME_LEN);
		}
	}

	return 0;
}

/* id: -1, only show usr_name */
int mainwin_set_recognInfo(int id, uint8_t confid, char *usr_name, int status)
{

	if(usr_name == NULL)
		return -1;

	memset(mainwindow->userRecogn, 0, sizeof(mainwindow->userRecogn));
	
	if(id == -1)
	{
		sprintf(mainwindow->userRecogn, "%s", usr_name);
	}
	else
	{
		mainwindow->face_id = id;
		strncpy(mainwindow->userRecogn, usr_name, strlen(usr_name));
	}

	mainwindow->confidence = confid;
	mainwindow->face_status = status;

	return 0;
}

/* set attend info */
int mainwin_set_attendList(int id, char *usr_name, uint32_t time_atdin, int sta_atdin, uint32_t time_atdout, int sta_atdout)
{
	QColor color;
	char usr_id[16] = {0};
	char intime_str[32] = {0};
	char outtime_str[32] = {0};
	char sta_str[32] = {0};
	char insta_str[8] = {0};
	char outsta_str[8] = {0};
	int modelRowCnt = 0;
	struct tm *ptm;
	time_t tmpTime;
	QTextCodec *codec = QTextCodec::codecForName("GBK");

	if(usr_name == NULL)
		return -1;

	sprintf(usr_id, "%d", id);
	tmpTime = time_atdin;
	if(sta_atdin != ATTEND_STA_NONE)
	{
		ptm = localtime(&tmpTime);
		sprintf(intime_str, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	}
	tmpTime = time_atdout;
	if(sta_atdout != ATTEND_STA_NONE)
	{
		ptm = localtime(&tmpTime);
		sprintf(outtime_str, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	}

	if(sta_atdin == ATTEND_STA_IN_OK)
	{
		strcpy(insta_str, TEXT_ATTEND_OK);
	}
	else if(sta_atdin == ATTEND_STA_IN_LATE)
	{
		strcpy(insta_str, TEXT_ATTEND_IN_LATE);
	}
	else
	{
		strcpy(insta_str, TEXT_ATTEND_NULL);
	}
	
	if(sta_atdout == ATTEND_STA_OUT_OK)
	{
		strcpy(outsta_str, TEXT_ATTEND_OK);
	}
	else if(sta_atdout == ATTEND_STA_OUT_EARLY)
	{
		strcpy(outsta_str, TEXT_ATTEND_OUT_EARLY);
	}
	else
	{
		strcpy(outsta_str, TEXT_ATTEND_NULL);
	}
	sprintf(sta_str, "%s : %s", insta_str, outsta_str);
	//printf("usr_id: %s, time: %s, status: %s\n", usr_id, attend_time, attend_sta);

	/* display color */
	if(sta_atdin==ATTEND_STA_IN_OK && sta_atdout==ATTEND_STA_OUT_OK)
	{
		color = QColor(155,187,89);	// light green
	}
	else if(sta_atdin==ATTEND_STA_NONE && sta_atdout==ATTEND_STA_NONE)
	{
		color = QColor(192,80,77);	// light red
	}
	else
	{
		color = QColor(222, 221, 140);	// light yellow
	}

	modelRowCnt = mainwindow->userModel->rowCount();
	mainwindow->userModel->setItem(modelRowCnt, 0, new QStandardItem(QString("%1").arg(usr_id)));
	mainwindow->userModel->setItem(modelRowCnt, 1, new QStandardItem(QString("%1").arg(usr_name)));
	mainwindow->userModel->setItem(modelRowCnt, 2, new QStandardItem(QString("%1").arg(intime_str)));
	mainwindow->userModel->setItem(modelRowCnt, 3, new QStandardItem(QString("%1").arg(outtime_str)));
	mainwindow->userModel->setItem(modelRowCnt, TIME_TABLE_STATUS_POS, new QStandardItem(QString("%1").arg(codec->toUnicode(sta_str))));

	/* set item align center */
	mainwindow->userModel->item(modelRowCnt, 0)->setTextAlignment(Qt::AlignCenter);
	mainwindow->userModel->item(modelRowCnt, 1)->setTextAlignment(Qt::AlignCenter);
	mainwindow->userModel->item(modelRowCnt, 2)->setTextAlignment(Qt::AlignCenter);
	mainwindow->userModel->item(modelRowCnt, 3)->setTextAlignment(Qt::AlignCenter);
	mainwindow->userModel->item(modelRowCnt, TIME_TABLE_STATUS_POS)->setTextAlignment(Qt::AlignCenter);

	/* set item background color */
	mainwindow->userModel->item(modelRowCnt, TIME_TABLE_STATUS_POS)->setBackground(QBrush(color));
	
	/* set item fonts(forground) color */
	//item(x, y)->setForeground(QBrush(QColor(255, 0, 0)));

	return 0;
}

void mainwin_clear_attendList(void)
{
	QTextCodec *codec = QTextCodec::codecForName("GBK");

	mainwindow->userModel->clear();
	mainwindow->userModel->setHorizontalHeaderLabels({codec->toUnicode(TEXT_USER_ID), codec->toUnicode(TEXT_USER_NAME), codec->toUnicode(TEXT_ATTEND_IN), \
														codec->toUnicode(TEXT_ATTEND_OUT), codec->toUnicode(TEXT_STATUS)});
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

/* notice:
 * use timer to display,
 * if use thread, it will occur some error.
 */
int start_mainwindow_task(void)
{
	mainwindow_init();

	return 0;
}


