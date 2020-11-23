#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QTextCodec>


#define MAIN_WIN_ROW			900
#define MAIN_WIN_COL			480
#define VIDEO_AREA_ROW			640
#define VIDEO_AREA_COL			480

#define TIMER_INTERV_MS			1
#define TIMER_ADDUSER_OK_MS		(3*1000)

#define WIN_BACKGRD_IMG			"resource/gdut.jpg"		// ���汳��ͼ

#define BEGIN_ADD_FACE_TEXT		"¼������������������ͷ"				// support Chinese
#define SUCCESS_ADD_FACE_TEXT	"¼�������ɹ�"				// support Chinese

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

	unsigned char 	*video_buf;
	unsigned int 	buf_size;
	
public:
	QLabel 			*textOnVideo;		// text show on video
	QTimer 			*tmpShowTimer;		// control temple show, few second
	char 			userName[64];
	int 			mainwin_state;
	int 			stateTick;			// 
};


int mainwin_change_worksta(int state);

int mainwin_set_userInfo(int id, char *usr_name);

int start_mainwindow_task(void);


#endif	// _MAINWINDOW_H_