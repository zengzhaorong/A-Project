#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include "config.h"
#include <linux/videodev2.h>


/* capture format */
#if defined(CAP_V4L2_FMT_JPEG)
	#define VIDEO_V4L2_PIX_FMT		V4L2_PIX_FMT_JPEG
#elif defined(CAP_V4L2_FMT_YUV)
	#define VIDEO_V4L2_PIX_FMT		V4L2_PIX_FMT_YUYV
#elif defined(CAP_V4L2_FMT_MJPEG)
	#define VIDEO_V4L2_PIX_FMT		V4L2_PIX_FMT_MJPEG
#endif

#define FRAME_BUF_SIZE		(ONE_CAP_FRAME_SIZE*3)

#define QUE_BUF_MAX_NUM		5

struct buffer_info
{
	unsigned char *addr;
	int len;
};

struct v4l2cap_info
{
	int run;
	int fd;
	unsigned char *frameBuf;
	unsigned int frameLen;
	pthread_mutex_t	frameMut;
	
	struct v4l2_format format;
	struct buffer_info buffer[QUE_BUF_MAX_NUM];
};

int capture_getframe(unsigned char *data, int size, int *len);
int start_capture_task(void);
int capture_task_stop(void);

#endif	// _CAPTURE_H_
