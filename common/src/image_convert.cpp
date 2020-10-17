#include "image_convert.h"

//using namespace cv;

QImage v4l2_to_QImage(unsigned char *data, int len)
{
	QImage qtImage;

	if(data==NULL || len<=0)
		return qtImage;
	
	qtImage.loadFromData(data, len);
	if(qtImage.isNull())
	{
		printf("ERROR: QImage is null !\n");
	}

	return qtImage;
}

/* 调用时要加.clone(), testMat = QImage_to_cvMat(qiamge).clone() */
cv::Mat QImage_to_cvMat(QImage qimage)
{
    cv::Mat mat;

	if(qimage.isNull())
	{
		printf("ERROR: QImage is null !\n");
		return mat;
	}
	
    switch(qimage.format())
    {
	    case QImage::Format_ARGB32:
	    case QImage::Format_RGB32:
	    case QImage::Format_ARGB32_Premultiplied:
	        mat = cv::Mat(qimage.height(), qimage.width(), CV_8UC4, (void*)qimage.bits(), qimage.bytesPerLine());
	        break;
	    case QImage::Format_RGB888:
	        mat = cv::Mat(qimage.height(), qimage.width(), CV_8UC3, (void*)qimage.bits(), qimage.bytesPerLine());
	        cv::cvtColor(mat, mat, CV_BGR2RGB);
	        break;
	    case QImage::Format_Indexed8:
	        mat = cv::Mat(qimage.height(), qimage.width(), CV_8UC1, (void*)qimage.bits(), qimage.bytesPerLine());
	        break;

		default:
	        printf("ERROR: %s case default !\n", __FUNCTION__);
    }

	if(mat.empty())
	{
        printf("ERROR: %s: Mat is empty !\n", __FUNCTION__);
	}

    return mat;
}



