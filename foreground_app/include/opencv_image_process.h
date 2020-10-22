#ifndef _OPENCV_IMAGE_PROCESS_H_
#define _OPENCV_IMAGE_PROCESS_H_

#include <iostream>
#include <QImage>
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

int set_rect_param(Rect &rects);
int get_rect_param(Rect &rects);
int opencv_image_add_rect(QImage &qimage, Rect &rects);


#endif
