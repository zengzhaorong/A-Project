#ifndef _IMAGE_CONVERT_H_
#define _IMAGE_CONVERT_H_

#include "config.h"
#include <opencv2/opencv.hpp>  
#include <opencv2/core.hpp>
#include <opencv2/imgproc/types_c.h>  
#include <QImage>

#if defined(CAP_V4L2_FMT_YUV)
QImage yuv_to_QImage(int type, unsigned char *data, int width, int height);
#endif

QImage jpeg_to_QImage(uint8_t *data, int len);

cv::Mat QImage_to_cvMat(QImage image);

QImage cvMat_to_QImage(const cv::Mat& mat);

#endif	// _IMAGE_CONVERT_H_
