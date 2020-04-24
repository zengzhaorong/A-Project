# project private setting
set(PROJECT_NAME background_app)

# cmake needs this line
cmake_minimum_required(VERSION 3.1)

# Enable C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


# 指定编译参数	-fPIC: Qt request
#set(CMAKE_CXX_FLAGS -Wall)
add_compile_options(-Wall)
# Define project name
project(${PROJECT_NAME})

# 工程H头文件路径
set(HEAD_FILE_PATH ${HEAD_FILE_PATH} ${PROJECT_SOURCE_DIR}/include)
file(GLOB HEAD_FILE_LIST ${HEAD_FILE_LIST} ${HEAD_FILE_PATH}/*.h)

# 工程C/C++源文件 --- globbing-通配
file(GLOB SRC_LIST_C ${SRC_LIST_C} ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB SRC_LIST_CPP ${SRC_LIST_CPP} ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB SRC_LIST_CPP ${SRC_LIST_CPP} ${PROJECT_SOURCE_DIR}/src/*.cpp)

#指定头文件目录
include_directories(${HEAD_FILE_PATH})

#指定链接库文件目录
#link_libraries(${OPENCV_LIB_FILE} ${QT5_LIB_FILE})

# Declare the executable target built from your sources
add_executable(${PROJECT_NAME} ${SRC_LIST_CPP} ${SRC_LIST_C} ${HEAD_FILE_LIST})

# Link your application with 3rd libraries
#link_directories()
target_link_libraries(${PROJECT_NAME} -lpthread)


