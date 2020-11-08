#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "opencv_face_process.h"
#include "image_convert.h"
#include "config.h"
#include "socket_server.h"
#include "user_mngr.h"

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "protocol.h"
#ifdef __cplusplus
}
#endif

using namespace std;
using namespace cv;

/* 临时测试 */
struct clientInfo *face_client = NULL;

extern struct userMngr_Stru		user_mngr_unit;

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
    vector<Mat> images;
    vector<int> labels;
	int ret;

	ret = sem_init(&this->recogn_sem, 0, 0);
	if(ret != 0)
	{
		printf("sem_init recogn_sem failed !\n");
		return -1;
	}


	fdb_csv = string(FACES_CSV_FILE);

	user_read_csv(fdb_csv, images, labels, ';');

	if(images.size() <= 1)
	{
		printf("images size: %d, not support !\n", (int)images.size());
		return -1;
	}

	this->mod_LBPH = LBPHFaceRecognizer::create();
	this->mod_LBPH->train(images, labels);

	/* wether save model file */
    //this->mod_LBPH->write("MyFaceLBPHModel.xml");  
	//this->mod_LBPH->read("MyFaceLBPHModel.xml");

	this->mod_LBPH->setThreshold(FACE_RECO_THRES);

	printf("%s: --------- Face model train succeed ---------\n", __FUNCTION__);

	return 0;
}

void face_recogn::face_recogn_deinit(void)
{
	sem_destroy(&this->recogn_sem);
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
	struct timespec ts;
	int tmpLen;
	int ret;

	if(buf == NULL)
		return -1;

	if(clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		return -1;
	}

	ts.tv_sec += 3;
	ret = sem_timedwait(&face_detect_unit.detect_sem, &ts);
	//ret = sem_trywait(&face_detect_unit.detect_sem);
	if(ret != 0)
		return -1;

	tmpLen = (size>sizeof(detect_buf) ? sizeof(detect_buf):size);

	memcpy(buf, detect_buf, tmpLen);

	printf("opencv_get_frame_detect success.\n");

	return tmpLen;
}

/* put frame to recognize buffer */
int opencv_put_frame_recogn(Mat& face_mat)
{
	if(face_mat.empty())
		return -1;

	face_recogn_unit.face_mat = face_mat;
	
	sem_post(&face_recogn_unit.recogn_sem);

	return 0;
}

/* get frame from recognize buffer */
int opencv_get_frame_recogn(Mat& face_mat)
{
	struct timespec ts;
	int ret;
	
	if(clock_gettime(CLOCK_REALTIME, &ts) == -1)
	{
		return -1;
	}

	ts.tv_sec += 3;
	ret = sem_timedwait(&face_recogn_unit.recogn_sem, &ts);
	//ret = sem_trywait(&face_detect_unit.detect_sem);
	if(ret != 0)
		return -1;

	face_mat = face_recogn_unit.face_mat;
	
	return 0;
}

int opencv_face_detect( Mat& img, CascadeClassifier& cascade,
                    double scale, bool tryflip, Mat& gray_face, vector<Rect> &faces)
{
    double t = 0;
    vector<Rect> faces2;
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

	if(faces.size() <= 0)
		return -1;
		
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

	/* restore face size */
	for(uint32_t i=0; i<faces.size(); i++)
	{
		faces[i].x *= scale;
		faces[i].y *= scale;
		faces[i].width *= scale;
		faces[i].height *= scale;
	}

	gray_face = gray(faces[0]);

	return faces.size();
}

void *opencv_face_detect_thread(void *arg)
{
	class face_detect *detect_unit = &face_detect_unit;
	vector<Rect> faces;
	QImage qImage;
	Mat detectMat, face_mat;
	int ret;

	printf("%s enter ++\n", __FUNCTION__);
	
	detect_unit->face_detect_init();

	while(face_client == NULL)
	{
		usleep(300*1000);
	}

	while(1)
	{
		proto_0x10_getOneFrame(face_client->protoHandle);

		/* 获取协议传输的原图像-进行检测 */
		ret = opencv_get_frame_detect(detect_unit->frame_buf, detect_unit->frame_size);
		if(ret <= 0)
		{
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
		
		ret = opencv_face_detect(detectMat, face_detect_unit.face_cascade, 3, 0, face_mat, faces);
		if(ret > 0)
		{
			opencv_put_frame_recogn(face_mat);
			printf("put frame recogn successfully.\n");
		
			struct Rect_params rect;
			for(uint32_t i=0; i<faces.size(); i++)
			{
				rect.x = faces[i].x;
				rect.y = faces[i].y;
				rect.w = faces[i].width;
				rect.h = faces[i].height;
				//printf("face: x=%d, y=%d, w=%d, h=%d\n", faces[i].x, faces[i].y, faces[i].width, faces[i].height);
			}
			proto_0x11_sendFaceDetect(face_client->protoHandle, 1, &rect);
		}
	}

	detect_unit->face_detect_deinit();

	return NULL;
}

int opencv_face_recogn(Mat &face_mat, int *face_id)
{
	Mat recogn_mat;
	int predict;
	double confidence = 0.0;

	if(face_mat.empty())
	{
		printf("ERROR: face mat is empty!\n");
		return -1;
	}

	if(face_mat.rows < FACE_ROW_MIN)
	{
		printf("ERROR: face mat row [%d] is too short!\n", face_mat.rows);
		return -1;
	}

	resize(face_mat, recogn_mat, Size(92, 112));
	if(recogn_mat.empty())
		printf("recogn_mat is empty!!!\n");

	face_recogn_unit.mod_LBPH->predict(recogn_mat , predict, confidence);

	if(predict < 0)
		return -1;
	
	*face_id = predict;
	printf("[recogn]*** predict: %d, confidence: %f\n", predict, confidence);
	
	return 0;
}

void *opencv_face_recogn_thread(void *arg)
{
	class face_recogn *recogn_unit = &face_recogn_unit;
	Mat face_mat;
	int face_id;
	int ret;
	int i;

	printf("%s enter ++\n", __FUNCTION__);

	recogn_unit->face_recogn_init();

	while(1)
	{
		/* 获取经人脸检测图像-进行识别 */
		ret = opencv_get_frame_recogn(face_mat);
		if(ret != 0)
		{
			continue;
		}

		printf("get frame recogn successfully.\n");

		ret = opencv_face_recogn(face_mat, &face_id);
		if(ret == 0)
		{
			for(i=0; i<user_mngr_unit.userCnt; i++)
			{
				if(user_mngr_unit.pstUserInfo[i].seq == face_id)
					break;
			}
			proto_0x12_sendFaceRecogn(face_client->protoHandle, face_id, user_mngr_unit.pstUserInfo[i].name);
		}
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

	ret = pthread_create(&tid, NULL, opencv_face_recogn_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}

	return 0;
}


