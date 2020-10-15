#ifndef _IMAGE_CONVERT_H_
#define _IMAGE_CONVERT_H_

#include <QImage>


QImage v4l2_to_QImage(unsigned char *data, int len);

#endif	// _IMAGE_CONVERT_H_
