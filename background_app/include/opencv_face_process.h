#ifndef _OPENCV_FACE_PROCESS_H_
#define _OPENCV_FACE_PROCESS_H_

#include <semaphore.h>
#include "opencv2/objdetect.hpp"
#include "type.h"

using namespace cv;

class face_detect
{
public:
	face_detect(void);
	int face_detect_init(void);
	void face_detect_deinit(void);

public:
	uint8_t *frame_buf;
	uint32_t frame_size;
	sem_t detect_sem;
	CascadeClassifier face_cascade;
};

class face_recogn
{
public:
	face_recogn(void);
	int face_recogn_init(void);
	void face_recogn_deinit(void);
	
	public:
};

int opencv_put_frame_detect(uint8_t *buf, uint32_t len);
int opencv_get_frame_detect(uint8_t *buf, uint32_t size);
int start_face_process_task(void);


#endif	// _OPENCV_FACE_PROCESS_H_
