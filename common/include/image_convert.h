#ifndef _IMAGE_CONVERT_H_
#define _IMAGE_CONVERT_H_

#include <opencv2/opencv.hpp>  
#include <opencv2/core.hpp>
#include <opencv2/imgproc/types_c.h>  
#include <QImage>


QImage v4l2_to_QImage(unsigned char *data, int len);

cv::Mat QImage_to_cvMat(QImage image);


#endif	// _IMAGE_CONVERT_H_
