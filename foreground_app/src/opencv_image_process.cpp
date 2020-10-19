#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv_image_process.h"
#include "image_convert.h"

using namespace std;
using namespace cv;

Rect face_rect;

int set_rect_param(Rect &rects)
{
	//for(uint32_t i=0; i<rects.size(); i++)
	{
		face_rect.x = rects.x;
		face_rect.y = rects.y;
		face_rect.width = rects.width;
		face_rect.height = rects.height;
	}

	return 0;
}

int opencv_image_add_rect(QImage &qimage, Rect &rects)
{
	Mat matImage;

	rects = face_rect;
	
	matImage = QImage_to_cvMat(qimage).clone();

	//for(uint32_t i=0; i<rects.size(); i++)
	{
		rectangle(matImage, Point(rects.x, rects.y), 
			Point(rects.x + rects.width, 
			rects.y + rects.height), 
			Scalar(0, 0, 255), 3, 8);	 // �������
		printf("******* Rect face: x=%d, y=%d, w=%d, h=%d\n", rects.x, rects.y, rects.width, rects.height);
	}

	qimage = cvMat_to_QImage(matImage);

	return 0;
}


