#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QTextCodec>
#include <QComboBox>
#include <QMessageBox>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QDateTime>
#include <QDateTimeEdit>


/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#ifdef __cplusplus
}
#endif


#define MAIN_WIN_ROW			800
#define MAIN_WIN_COL			480
#define VIDEO_AREA_ROW			640
#define VIDEO_AREA_COL			480

#define TIMER_INTERV_MS			1
#define TIMER_ADDUSER_OK_MS		(3*1000)

#define WIN_BACKGRD_IMG			"resource/backgrd.jpg"		// 界面背景图

/* tips text, support Chinese */
#define BEGIN_ADD_FACE_TEXT		"录入人脸：请正对摄像头"
#define SUCCESS_ADD_FACE_TEXT	"录入人脸成功"
#define RECOGN_SUCCESS_TEXT		"签到成功"			// "识别成功"、"签到成功"、"欢迎回家"
#define ATTEND_LATE_TEXT		"签到[迟到]"

typedef enum {
	MAINWIN_MODE_NORAML,
	MAINWIN_MODE_ADDUSER,
	MAINWIN_MODE_ADDUSER_OK,
	MAINWIN_MODE_RECOGN,
}mainwin_mode_e;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void showMainwindow(void);
	void setAttendTime(void);
	void addUser(void);
	void deleteUser(void);
	void showTimeSheet(void);
	void textOnVideo_show_over(void);
	
public:
	int switch_mainwin_mode(mainwin_mode_e mode);

private:
	QWidget			*mainWindow;		// main window
	QLabel 			*videoArea;			// video area
	QImage			initWinImg;			// background image
	QTimer 			*timer;				// display timer
	QLabel			*clockLabel;		// display clock
	QDateTimeEdit	*attendTimeEdit;	// attend time base
	QPushButton 	*setAtdtimeBtn;		// set attend time
	QLineEdit		*userNameEdit;		// edit add user name
	QPushButton 	*addUserBtn;		// add user button
	QPushButton 	*delUserBtn;		// delete user button
	QPushButton 	*timeSheetBtn;		// time sheet button
	unsigned char 	*video_buf;
	unsigned int 	buf_size;
	
public:
	QComboBox		*userListBox;		// user list box
	QLabel 			*textOnVideo;		// text show on video
	QTimer 			*tmpShowTimer;		// control temple show, few second
	QTableView		*tableView;
	QStandardItemModel *userModel;
	int 			face_id;
	char 			userRecogn[USER_NAME_LEN];
	uint8_t			confidence;
	int 			face_status;
	workstate_e		*sys_state;			// is system work state (main_mngr.work_state)
	int 			mainwin_mode;
	int 			stateTick;			// 
};

int mainwin_set_userList(int flag, int userCnt, char *usr_name);
int mainwin_set_attendList(int id, char *usr_name, uint32_t time, int status);
void mainwin_reset_attendList(void);

int mainwin_set_recognInfo(int id, uint8_t confid, char *usr_name, int status);

int start_mainwindow_task(void);


#endif	// _MAINWINDOW_H_