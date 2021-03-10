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
#include "attendance.h"

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

extern struct main_mngr_info main_mngr;
extern struct userMngr_Stru	user_mngr_unit;

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
	string fdb_csv;
    vector<Mat> images;
    vector<int> labels;
	int ret;

	ret = sem_init(&this->recogn_sem, 0, 0);
	if(ret != 0)
	{
		return -1;
	}

#if defined(OPENCV_VER_3_X_X)
	this->mod_LBPH = createLBPHFaceRecognizer();
#else
	this->mod_LBPH = LBPHFaceRecognizer::create();
#endif
	recogn_state = 0;

	ret = face_database_train();
	if(ret < 0)
	{
		return -1;
	}

	/* wether save model file */
    //this->mod_LBPH->write("MyFaceLBPHModel.xml");  
	//this->mod_LBPH->read("MyFaceLBPHModel.xml");

	this->mod_LBPH->setThreshold(FACE_RECOGN_THRES);

	recogn_state = 1;
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
	int value = 0;
	int tmpLen;

	if(buf == NULL)
		return -1;

	tmpLen = (len>sizeof(detect_buf) ? sizeof(detect_buf):len);
	memcpy(detect_buf, buf, tmpLen);

	sem_getvalue(&face_detect_unit.detect_sem, &value);

	if(value <= 0)
		sem_post(&face_detect_unit.detect_sem);

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

	return tmpLen;
}

/* put frame to recognize buffer */
int opencv_put_frame_recogn(Mat& face_mat)
{
	int value = 0;

	if(face_mat.empty())
		return -1;

	face_recogn_unit.face_mat = face_mat;

	sem_getvalue(&face_recogn_unit.recogn_sem, &value);

	if(value <= 0)
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

int face_resize_save(Mat& faceImg, char *path, int index)
{
	string file_path;
	Mat faceSave;
	int ret;

	if(faceImg.empty())
		return -1;

	if(faceImg.cols < 100)
	{
		printf("face image is too small, skip!\n");
		return -1;
	}

	resize(faceImg, faceSave, Size(92, 112));
	file_path = format("%s/%d.jpg", path, index);
	ret = imwrite(file_path, faceSave);
	if(ret == false)
		return -1;

	printf("[Add user]***save face: %s\n", file_path.c_str());
	
	return 0;
}

int face_database_train(void)
{
	struct userMngr_Stru *user_mngr = &user_mngr_unit;
	string fdb_csv;
    vector<Mat> images;
    vector<int> labels;
	int ret;

	ret = user_get_userList((char *)FACES_DATABASE_PATH, &user_mngr->userInfo, &user_mngr->userCnt);
	if(ret < 0)
		goto ERR_TRAIN;

	for(int i=0; i<user_mngr->userCnt; i++)
	{
		printf("[%d].id=%d, name: %s\n", i, user_mngr->userInfo[i].id, user_mngr->userInfo[i].name);
	}

	ret = user_create_csv((char *)FACES_DATABASE_PATH, (char *)FACES_DB_CSV_FILE);
	if(ret != 0)
		goto ERR_TRAIN;

	fdb_csv = string(FACES_DB_CSV_FILE);

	ret = user_read_csv(fdb_csv, images, labels, ';');
	if(ret<0 || images.size() <= 0)
	{
		printf("read csv: No images !\n");
		goto ERR_TRAIN;
	}

	face_recogn_unit.mod_LBPH->train(images, labels);

	face_recogn_unit.mod_LBPH->setThreshold(FACE_RECOGN_THRES);
	
	printf("face_database_retrain successfully.\n");

	return 0;

ERR_TRAIN	:
	face_recogn_unit.recogn_state = 0;

	return -1;
}

int opencv_face_detect( Mat& img, CascadeClassifier& cascade,
                    double scale, bool tryflip, Mat& gray_face, vector<Rect> &faces)
{
    double t = 0;
    vector<Rect> faces2;
    Mat gray, smallImg;

    cvtColor( img, gray, COLOR_BGR2GRAY );
    double fx = 1 / scale;
    resize( gray, smallImg, Size(), fx, fx, CV_INTER_LINEAR );
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
    //printf( "detection time = %g ms\n", t*1000/getTickFrequency());

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
	int socket_handle = -1;
	vector<Rect> faces;
	QImage qImage;
	Mat detectMat, face_mat;
	int i, ret;

	printf("%s enter ++\n", __FUNCTION__);
	
	detect_unit->face_detect_init();

	/* only support one client at one time */
	/* if manager is adding user, user client can not work in face detect nad recognize */
	while(1)
	{
		/* get manager client */
		if(main_mngr.work_state == WORK_STA_ADDUSER)
		{
			socket_handle = main_mngr.mngr_handle;
		}
		/* get user client */
		else
		{
			socket_handle = main_mngr.user_handle;
		}

		if(socket_handle < 0)
		{
			usleep(300*1000);
			continue;
		}
		
		proto_0x10_getOneFrame(socket_handle);

		/* get one frame which from client */
		ret = opencv_get_frame_detect(detect_unit->frame_buf, detect_unit->frame_size);
		if(ret <= 0)
		{
			continue;
		}

		/* convert v4l2 data to qimage */
#if defined(CAP_V4L2_FMT_JPEG)
		qImage = jpeg_to_QImage(detect_unit->frame_buf, ret);
#elif defined(CAP_V4L2_FMT_YUV)
		qImage = yuv_to_QImage(0, detect_unit->frame_buf, CAPTURE_PIX_WIDTH, CAPTURE_PIX_HEIGH);
#elif defined(CAP_V4L2_FMT_MJPEG)
		qImage = jpeg_to_QImage(detect_unit->frame_buf, ret);
#endif
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
			struct Rect_params rect;
			for(uint32_t i=0; i<faces.size(); i++)
			{
				rect.x = faces[i].x;
				rect.y = faces[i].y;
				rect.w = faces[i].width;
				rect.h = faces[i].height;
				//printf("face: x=%d, y=%d, w=%d, h=%d\n", faces[i].x, faces[i].y, faces[i].width, faces[i].height);
			}
			proto_0x11_sendFaceDetect(socket_handle, 1, &rect);
			
			if(main_mngr.work_state == WORK_STA_NORMAL)
			{
				opencv_put_frame_recogn(face_mat);
			}
			else if(main_mngr.work_state == WORK_STA_ADDUSER)
			{
				ret = face_resize_save(face_mat, user_mngr_unit.add_userdir, user_mngr_unit.add_index +1);
				if(ret != 0)
					continue;
				
				user_mngr_unit.add_index ++;

				/* finish add user, change work state */
				if(user_mngr_unit.add_index >= FACE_CNT_PER_USER)
				{
					proto_0x04_switchWorkSta(socket_handle, WORK_STA_NORMAL, NULL);
					proto_0x05_addUser(socket_handle, 1, user_mngr_unit.newuser);
					main_mngr.work_state = WORK_STA_NORMAL;
					face_recogn_unit.recogn_state = 0;
					ret = face_database_train();
					if(ret == 0)
					{
						face_recogn_unit.recogn_state = 1;
					}
					attendance_sync_userlist();
				}
			}
			else
			{
			}
		}
	}

	detect_unit->face_detect_deinit();

	return NULL;
}

int opencv_face_recogn(Mat &face_mat, int *face_id, uint8_t *confid, int *status)
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

	/* calculate confidence */
	if(confidence < FACE_RECOGN_THRES_100)
	{
		*confid = 100;
	}
	else if(confidence < FACE_RECOGN_THRES_80)
	{
		*confid = 80 +(FACE_RECOGN_THRES_80 -confidence)*20/(FACE_RECOGN_THRES_80 -FACE_RECOGN_THRES_100);
	}
	else if(confidence < FACE_RECOGN_THRES_00)
	{
		*confid = (FACE_RECOGN_THRES_00 -confidence)*80 /(FACE_RECOGN_THRES_00 -FACE_RECOGN_THRES_80);
	}
	else
	{
		*confid = 0;
	}
	
	/* set user attend info */
	*status = (int)attendance_set_one(predict, time(NULL));
	
	printf("[recogn]*** predict: %d, confidence: %f = %d%%, status: %d\n", predict, confidence, *confid, *status);

	return 0;
}

void *opencv_face_recogn_thread(void *arg)
{
	class face_recogn *recogn_unit = &face_recogn_unit;
	Mat face_mat;
	int face_id;
	uint8_t confidence;
	int status;
	int ret;
	int i;

	printf("%s enter ++\n", __FUNCTION__);

	recogn_unit->face_recogn_init();

	while(1)
	{
		if(main_mngr.work_state!=WORK_STA_NORMAL || face_recogn_unit.recogn_state!=1 || main_mngr.user_handle<0)
		{
			usleep(300 *1000);
			continue;
		}

	
		/* get one face frame to recognize */
		ret = opencv_get_frame_recogn(face_mat);
		if(ret != 0)
		{
			continue;
		}

		ret = opencv_face_recogn(face_mat, &face_id, &confidence, &status);
		if(ret == 0)
		{
			for(i=0; i<user_mngr_unit.userCnt; i++)
			{
				if(user_mngr_unit.userInfo[i].id == face_id)
					break;
			}
			proto_0x12_sendFaceRecogn(main_mngr.user_handle, face_id, confidence, user_mngr_unit.userInfo[i].name, status);
			sleep(RECOGN_OK_DELAY_MS/1000 +1);	// more 1 second
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


