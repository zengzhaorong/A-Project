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

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#ifdef __cplusplus
}
#endif


#define MAIN_WIN_ROW			900
#define MAIN_WIN_COL			480
#define VIDEO_AREA_ROW			640
#define VIDEO_AREA_COL			480

#define TIMER_INTERV_MS			1
#define TIMER_ADDUSER_OK_MS		(3*1000)

#define WIN_BACKGRD_IMG			"resource/backgrd.jpg"		// ���汳��ͼ

/* tips text, support Chinese */
#define BEGIN_ADD_FACE_TEXT		"¼������������������ͷ"
#define SUCCESS_ADD_FACE_TEXT	"¼�������ɹ�"
#define SUCCESS_RECOGN_TEXT		"ʶ��ɹ�"			// ��ǩ���ɹ���������ӭ�ؼҡ�

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
	void showMainwindow();
	void addUser();
	void deleteUser();
	void textOnVideo_show_over();
	
public:
	int switch_mainwin_mode(mainwin_mode_e mode);

private:
	QWidget			*mainWindow;		// ���Ĵ���
	QLabel 			*videoArea;			// ͼ����ʾ��
	QImage			initWinImg;			// ��ʼ����ͼ
	QTimer 			*timer;				// ˢ����ʾ��ʱ��
	QLineEdit		*userNameEdit;		// edit add user name
	QPushButton 	*addUserBtn;		// add user button
	QPushButton 	*delUserBtn;		// delete user button

	unsigned char 	*video_buf;
	unsigned int 	buf_size;
	
public:
	QComboBox		*userListBox;		// user list box
	QLabel 			*textOnVideo;		// text show on video
	QTimer 			*tmpShowTimer;		// control temple show, few second
	int 			face_id;
	char 			userRecogn[USER_NAME_LEN];
	uint8_t			confidence;
	workstate_e		*sys_state;			// is system work state (main_mngr.work_state)
	int 			mainwin_mode;
	int 			stateTick;			// 
};

int mainwin_set_userList(int flag, int userCnt, char *usr_name);

int mainwin_set_recognInfo(int id, uint8_t confid, char *usr_name);

int start_mainwindow_task(void);


#endif	// _MAINWINDOW_H_