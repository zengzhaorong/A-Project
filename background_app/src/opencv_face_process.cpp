#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv_face_process.h"
#include "image_convert.h"
#include "config.h"


using namespace std;
using namespace cv;

static struct face_process_unit face_unit;
static uint8_t detect_buf[ONE_CAP_FRAME_SIZE] = {0};

static sem_t detect_sem;

int opencv_face_init(void)
{
	int ret;

	memset(detect_buf, 0, sizeof(detect_buf));

	face_unit.frame_size = ONE_CAP_FRAME_SIZE;
	face_unit.frame_buf = (uint8_t *)malloc(face_unit.frame_size);
	if(face_unit.frame_buf == NULL)
		return -1;

	ret = sem_init(&detect_sem, 0, 0);
	if(ret != 0)
	{
		printf("sem_init detect_sem failed !\n");
		return -1;
	}


	return 0;
}

void opencv_face_deinit(void)
{
	sem_destroy(&detect_sem);
}

/* put frame to detect buffer */
int opencv_put_frame_detect(uint8_t *buf, uint32_t len)
{
	int tmpLen;

	if(buf == NULL)
		return -1;

	tmpLen = (len>sizeof(detect_buf) ? sizeof(detect_buf):len);
	memcpy(detect_buf, buf, tmpLen);

	sem_post(&detect_sem);

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

	ret = sem_trywait(&detect_sem);
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
	int ret;

	printf("%s enter ++\n", __FUNCTION__);
	
	opencv_face_init();

	QLabel 		videoArea;			// 图像显示区
	while(1)
	{

		/* 获取协议传输的原图像-进行检测 */
		ret = opencv_get_frame_detect(face_unit.frame_buf, face_unit.frame_size);
		if(ret <= 0)
			continue;

#if 1	// ***** test for image transferation
		QImage q_image;
		q_image = v4l2_to_QImage(face_unit.frame_buf, ret);
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

	opencv_face_deinit();

	return NULL;
}

void *opencv_face_recognize_thread(void *arg)
{

	printf("%s enter ++\n", __FUNCTION__);

	while(1)
	{
		/* 获取经人脸检测图像-进行识别 */
	
		sleep(1);
	}

	return NULL;
}

int start_opencv_face_task(void)
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


