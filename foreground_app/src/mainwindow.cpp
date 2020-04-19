#include <iostream>
#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	
	setWindowTitle("mainwindow");
	
	resize(900, 480);

	mainWidget = new QWidget;
	setCentralWidget(mainWidget);

	helloLabel = new QLabel(mainWidget);
	helloLabel->setText("Hello Qt");
	helloLabel->resize(100,50);
	helloLabel->show();
	
}

MainWindow::~MainWindow()
{
	
}


