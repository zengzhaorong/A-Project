# project private setting
set(PROJECT_NAME foreground_app)

# cmake needs this line
cmake_minimum_required(VERSION 3.1)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# 设置自动生成moc文件，一定要设置
# QObject派生的C++类需要通过moc进行处理(Q_OBJECT)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)

# 指定编译参数	-fPIC: Qt request
#set(CMAKE_CXX_FLAGS -fPIC)
add_compile_options(-Wall -fPIC)

# Define project name
project(${PROJECT_NAME})

# opencv4库路径及文件
#find_package(OpenCV REQUIRED)
set(LIB_OPENCV_PATH /usr/local/opencv4)
set(LIB_OPENCV_INC ${LIB_OPENCV_PATH}/include/opencv4)
set(LIB_OPENCV_LIB ${LIB_OPENCV_PATH}/lib)
file(GLOB OPENCV_LIB_FILE ${LIB_OPENCV_LIB}/libopencv_*.so*)

# QT5库路径及文件
find_package(Qt5Widgets)
find_package(Qt5Gui)
find_package(Qt5Core)
set(LIB_QT5_PATH /usr/local/Qt-5.12.8)
set(LIB_QT5_INC ${LIB_QT5_PATH}/include ${LIB_QT5_PATH}/include/QtWidgets ${LIB_QT5_PATH}/include/QtGui ${LIB_QT5_PATH}/include/QtCore)
set(LIB_QT5_LIB ${LIB_QT5_PATH}/lib)
file(GLOB QT5_LIB_FILE ${LIB_QT5_LIB}/libQt5*.so*)

# 工程H头文件路径
set(HEAD_FILE_PATH ${HEAD_FILE_PATH} ${PROJECT_SOURCE_DIR}/include)
set(HEAD_FILE_PATH ${HEAD_FILE_PATH} ${PROJECT_SOURCE_DIR}/../common/include)
set(QT_HEAD_FILE_LIST ${QT_HEAD_FILE_LIST} ${PROJECT_SOURCE_DIR}/include/mainwindow.h)

# 工程C/C++源文件 --- globbing-通配
file(GLOB SRC_LIST_C ${SRC_LIST_C} ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB SRC_LIST_C ${SRC_LIST_C} ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB SRC_LIST_C ${SRC_LIST_C} ${PROJECT_SOURCE_DIR}/../common/src/*.c)
file(GLOB SRC_LIST_CPP ${SRC_LIST_CPP} ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB SRC_LIST_CPP ${SRC_LIST_CPP} ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB SRC_LIST_CPP ${SRC_LIST_CPP} ${PROJECT_SOURCE_DIR}/../common/src/*.cpp)

#指定头文件目录
include_directories(${LIB_OPENCV_INC} ${LIB_QT5_INC} ${HEAD_FILE_PATH})

#指定链接库文件目录
link_libraries(${OPENCV_LIB_FILE} ${QT5_LIB_FILE})

# Declare the executable target built from your sources
add_executable(${PROJECT_NAME} ${SRC_LIST_CPP} ${SRC_LIST_C} ${QT_HEAD_FILE_LIST})

# Link your application with 3rd libraries
#link_directories()
target_link_libraries(${PROJECT_NAME} -lpthread)


