######################################################################
# Automatically generated by qmake (3.1) Fri Apr 17 22:23:30 2020
######################################################################

TEMPLATE = app
TARGET = foreground_app
INCLUDEPATH += .
INCLUDEPATH +=  ./include \
				/usr/local/opencv4/include/opencv4 \
				/usr/local/opencv4/include/opencv4/opencv2
				
LIBS += /usr/local/opencv4/lib/libopencv_*

# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
HEADERS += include/mainwindow.h include/open_pic.h
SOURCES += main.cpp \
           src/mainwindow.cpp \
           src/open_pic.cpp 
		   
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets