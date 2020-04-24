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
	QWidget		*mainWindow;		// ���Ĵ���
	QLabel 		*videoArea;			// ͼ����ʾ��
	QImage		initWinImg;			// ��ʼ����ͼ
	
	
};



int start_mainwindow_task(void);


#endif	// _MAINWINDOW_H_