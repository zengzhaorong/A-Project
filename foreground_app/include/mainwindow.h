#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QTimer>


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void showMainwindow();
	
public:
	void showVideo(QImage vdoImg);

private:
	QWidget		*mainWindow;		// ���Ĵ���
	QLabel 		*videoArea;			// ͼ����ʾ��
	QImage		initWinImg;			// ��ʼ����ͼ
	QTimer 		*timer;				// ˢ����ʾ��ʱ��
	QLabel		*userInfoLab;		// label to show user info

	unsigned char *video_buf;
	unsigned int buf_size;
	
public:
	QString		userNameStr;		// recognize user face name
};


int mainwin_set_userInfo(int id, char *usr_name);

int start_mainwindow_task(void);


#endif	// _MAINWINDOW_H_