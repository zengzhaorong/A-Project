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

#include <QLabel>

void *opencv_face_detect_thread(void *arg)
{
	class face_detect *detect_unit = &face_detect_unit;
	int ret;

	printf("%s enter ++\n", __FUNCTION__);
	
	detect_unit->face_detect_init();

	QLabel 		videoArea;			// 图像显示区
	while(1)
	{

		/* 获取协议传输的原图像-进行检测 */
		ret = opencv_get_frame_detect(detect_unit->frame_buf, detect_unit->frame_size);
		if(ret <= 0)
			continue;

#if 1	// ***** test for image transferation
		QImage q_image;
		q_image = v4l2_to_QImage(detect_unit->frame_buf, ret);
		if(q_image.isNull())
		{
			printf("ERROR: q_image is null !\n");
			continue;
		}
		
		videoArea.setPixmap(QPixmap::fromImage(q_image));
		videoArea.show();
#endif
		sleep(1);
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


