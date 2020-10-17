#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv_face_process.h"
#include "image_convert.h"
#include "config.h"


using namespace std;
using namespace cv;

class face_detect face_detect_unit;
class face_recogn face_recogn_unit;

static uint8_t detect_buf[ONE_CAP_FRAME_SIZE] = {0};


face_detect::face_detect(void)
{
	printf("%s: enter ++\n", __FUNCTION__);
}

int face_detect::face_detect_init(void)
{
	int ret;

	this->frame_size = ONE_CAP_FRAME_SIZE;
	this->frame_buf = (uint8_t *)malloc(this->frame_size);
	if(this->frame_buf == NULL)
		return -1;

	ret = sem_init(&this->detect_sem, 0, 0);
	if(ret != 0)
	{
		printf("sem_init detect_sem failed !\n");
		return -1;
	}

	this->face_cascade.load("resource/haarcascade_frontalface_alt.xml");

	return 0;
}

void face_detect::face_detect_deinit(void)
{
	sem_destroy(&this->detect_sem);
}


face_recogn::face_recogn(void)
{
	printf("%s: enter ++\n", __FUNCTION__);
}


int face_recogn::face_recogn_init(void)
{
	return 0;
}

void face_recogn::face_recogn_deinit(void)
{
}

/* put frame to detect buffer */
int opencv_put_frame_detect(uint8_t *buf, uint32_t len)
{
	int tmpLen;

	if(buf == NULL)
		return -1;

	tmpLen = (len>sizeof(detect_buf) ? sizeof(detect_buf):len);
	memcpy(detect_buf, buf, tmpLen);

	sem_post(&face_detect_unit.detect_sem);

	printf("opencv_put_frame_detect.\n");
	
	return 0;
}

/* get frame from detect buffer */
int opencv_get_frame_detect(uint8_t *buf, uint32_t size)
{
	int tmpLen;
	int ret;

	if(buf == NULL)
		return -1;

	ret = sem_trywait(&face_detect_unit.detect_sem);
	if(ret != 0)
		return -1;

	tmpLen = (size>sizeof(detect_buf) ? sizeof(detect_buf):size);

	memcpy(buf, detect_buf, tmpLen);

	printf("opencv_get_frame_detect success.\n");

	return tmpLen;
}

void opencv_face_detect( Mat& img, CascadeClassifier& cascade,
                    double scale, bool tryflip )
{
    double t = 0;
    vector<Rect> faces, faces2;
    const static Scalar colors[] =
    {
        Scalar(255,0,0),
        Scalar(255,128,0),
        Scalar(255,255,0),
        Scalar(0,255,0),
        Scalar(0,128,255),
        Scalar(0,255,255),
        Scalar(0,0,255),
        Scalar(255,0,255)
    };
    Mat gray, smallImg;

    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );
    equalizeHist( smallImg, smallImg );

    t = (double)getTickCount();
    cascade.detectMultiScale( smallImg, faces,
        1.1, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(30, 30) );
    if( tryflip )
    {
        flip(smallImg, smallImg, 1);
        cascade.detectMultiScale( smallImg, faces2,
                                 1.1, 2, 0
                                 //|CASCADE_FIND_BIGGEST_OBJECT
                                 //|CASCADE_DO_ROUGH_SEARCH
                                 |CASCADE_SCALE_IMAGE,
                                 Size(30, 30) );
        for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); ++r )
        {
            faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
        }
    }
    t = (double)getTickCount() - t;
    printf( "detection time = %g ms\n", t*1000/getTickFrequency());
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        Mat smallImgROI;
        Point center;
        Scalar color = colors[i%8];
        int radius;

        double aspect_ratio = (double)r.width/r.height;
        if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
        {
            center.x = cvRound((r.x + r.width*0.5)*scale);
            center.y = cvRound((r.y + r.height*0.5)*scale);
            radius = cvRound((r.width + r.height)*0.25*scale);
            circle( img, center, radius, color, 3, 8, 0 );
        }
        else
            rectangle( img, Point(cvRound(r.x*scale), cvRound(r.y*scale)),
                       Point(cvRound((r.x + r.width-1)*scale), cvRound((r.y + r.height-1)*scale)),
                       color, 3, 8, 0);
    }
    imshow( "result", img );
}

void *opencv_face_detect_thread(void *arg)
{
	class face_detect *detect_unit = &face_detect_unit;
	QImage qImage;
	Mat detectMat;
	int ret;

	printf("%s enter ++\n", __FUNCTION__);
	
	detect_unit->face_detect_init();

	while(1)
	{

		/* 获取协议传输的原图像-进行检测 */
		ret = opencv_get_frame_detect(detect_unit->frame_buf, detect_unit->frame_size);
		if(ret <= 0)
		{
			usleep(30*1000);
			continue;
		}

		/* convert v4l2 data to qimage */
		qImage = v4l2_to_QImage(detect_unit->frame_buf, ret);
		if(qImage.isNull())
		{
			printf("ERROR: qImage is null !\n");
			continue;
		}

		/* convert qimage to cvMat */
		detectMat = QImage_to_cvMat(qImage).clone();
		if(detectMat.empty())
		{
			printf("ERROR: detectMat is empty\n");
			continue;
		}
		
		opencv_face_detect(detectMat, face_detect_unit.face_cascade, 3, 0);
	}

	detect_unit->face_detect_deinit();

	return NULL;
}

void *opencv_face_recognize_thread(void *arg)
{
	class face_recogn *recogn_unit = &face_recogn_unit;

	printf("%s enter ++\n", __FUNCTION__);

	recogn_unit->face_recogn_init();

	while(1)
	{
		/* 获取经人脸检测图像-进行识别 */
	
		sleep(1);
	}

	return NULL;
}

int face_process_init()
{
	memset(detect_buf, 0, sizeof(detect_buf));

	return 0;
}

int start_face_process_task(void)
{
	pthread_t tid;
	int ret;

	ret = pthread_create(&tid, NULL, opencv_face_detect_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}

	ret = pthread_create(&tid, NULL, opencv_face_recognize_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}

	return 0;
}


