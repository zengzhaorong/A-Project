#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include "capture.h"
#include "config.h"


#define VIDEO_DEV_NAME 		"/dev/video0"

#define FRAME_BUF_SIZE		(ONE_CAP_FRAME_SIZE*3)

struct v4l2cap_info capture_info;


int capture_init(struct v4l2cap_info *capture)
{
	struct v4l2_fmtdesc fmtdesc;
	struct v4l2_format format;
	struct v4l2_requestbuffers reqbuf_param;
	struct v4l2_buffer buffer[QUE_BUF_MAX_NUM];
	int i, ret;

	memset(capture, 0, sizeof(struct v4l2cap_info));

	pthread_mutex_init(&capture->frameMut, NULL);

	capture->fd = open(VIDEO_DEV_NAME, O_RDWR);
	if(capture->fd < 0)
	{
		printf("ERROR: open video dev [%s] failed !\n", VIDEO_DEV_NAME);
		ret = -1;
		goto ERR_1;
	}
	printf("open video dev [%s] successfully .\n", VIDEO_DEV_NAME);

	/* get supported format */
	memset(&fmtdesc, 0, sizeof(struct v4l2_fmtdesc));
	fmtdesc.index = 0;
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	do{
		ret = ioctl(capture->fd, VIDIOC_ENUM_FMT, &fmtdesc);
		printf("[ret:%d]video description: %s\n", ret, fmtdesc.description);
		fmtdesc.index ++;
	}while(ret == 0);

	/* configure video format */
	memset(&format, 0, sizeof(struct v4l2_format));
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	format.fmt.pix.width = CAPTURE_PIX_WIDTH;
	format.fmt.pix.height = CAPTURE_PIX_HEIGH;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	format.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ret = ioctl(capture->fd, VIDIOC_S_FMT, &format);
	if(ret < 0)
	{
		ret = -3;
		goto ERR_3;
	}
	printf("set video width * height = %d * %d\n", format.fmt.pix.width, format.fmt.pix.height);

	/* get video format */
	capture->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(capture->fd, VIDIOC_G_FMT, &capture->format);
	if(ret < 0)
	{
		printf("ERROR: get video format failed[ret:%d] !\n", ret);
		ret = -2;
		goto ERR_2;
	}
	
	printf("get video width * height = %d * %d\n", capture->format.fmt.pix.width, capture->format.fmt.pix.height);
	printf("video pixelformat: ");
	switch(capture->format.fmt.pix.pixelformat)
	{
 		case V4L2_PIX_FMT_JPEG: printf("V4L2_PIX_FMT_JPEG \n");
			break;
		case V4L2_PIX_FMT_YUYV: printf("V4L2_PIX_FMT_YUYV \n");
			break;
		case V4L2_PIX_FMT_MJPEG: printf("V4L2_PIX_FMT_MJPEG \n");
			break;
		
		default:
			printf("ERROR: value is illegal !\n");
	}

	capture->frameBuf = (unsigned char *)calloc(1, FRAME_BUF_SIZE);
	if(capture->frameBuf == NULL)
	{
		ret = -4;
		goto ERR_4;
	}

	memset(&reqbuf_param, 0, sizeof(struct v4l2_requestbuffers));
	reqbuf_param.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf_param.memory = V4L2_MEMORY_MMAP;
	reqbuf_param.count = QUE_BUF_MAX_NUM;
	ret = ioctl(capture->fd, VIDIOC_REQBUFS, &reqbuf_param);
	if(ret < 0)
	{
		ret = -5;
		goto ERR_5;
	}

	/* set video queue buffer */
	for(i=0; i<QUE_BUF_MAX_NUM; i++)
	{
		memset(&buffer[i], 0, sizeof(struct v4l2_buffer));
		buffer[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer[i].memory = V4L2_MEMORY_MMAP;
		buffer[i].index = i;
		ret = ioctl(capture->fd, VIDIOC_QUERYBUF, &buffer[i]);
		if(ret < 0)
		{
			ret = -6;
			goto ERR_6;
		}

		capture->buffer[i].len = buffer[i].length;
		capture->buffer[i].addr = (unsigned char *)mmap(NULL, buffer[i].length, PROT_READ | PROT_WRITE, \
									MAP_SHARED, capture->fd, buffer[i].m.offset);
		printf("buffer[%d]: addr = %p, len = %d\n", i, capture->buffer[i].addr, capture->buffer[i].len);

		ret = ioctl(capture->fd, VIDIOC_QBUF, &buffer[i]);
		if(ret < 0)
		{
			ret = -7;
			goto ERR_7;
		}
	}

	return 0;
	
	ERR_7:
	ERR_6:
		for(; i>=0; i--)
		{
			if(capture->buffer[i].addr != NULL)
				munmap(capture->buffer[i].addr, capture->buffer[i].len);
		}
	ERR_5:
		free(capture->frameBuf);
	ERR_4:
	ERR_3:
	ERR_2:
		close(capture->fd);

	ERR_1:

	return ret;
}

void capture_deinit(struct v4l2cap_info *capture)
{
	int i;

	for(i=0; i<QUE_BUF_MAX_NUM; i++)
	{
		munmap(capture->buffer[i].addr, capture->buffer[i].len);
	}
	
	free(capture->frameBuf);

	close(capture->fd);
}

int v4l2cap_start(struct v4l2cap_info *capture)
{
	enum v4l2_buf_type type;
	int ret;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(capture->fd, VIDIOC_STREAMON, &type);
	if(ret < 0)
		return -1;

	return 0;
}

void v4l2cap_stop(struct v4l2cap_info *capture)
{
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(capture->fd, VIDIOC_STREAMOFF, &type);
}

int v4l2cap_flushframe(struct v4l2cap_info *capture)
{
	struct v4l2_buffer v4l2buf;
	static unsigned int index = 0;
	int ret;

	memset(&v4l2buf, 0, sizeof(struct v4l2_buffer));
	v4l2buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4l2buf.memory = V4L2_MEMORY_MMAP;
	v4l2buf.index = index % QUE_BUF_MAX_NUM;
	
	ret = ioctl(capture->fd, VIDIOC_DQBUF, &v4l2buf);
	if(ret < 0)
	{
		printf("ERROR: get VIDIOC_DQBUF failed, ret: %d\n", ret);
		return -1;
	}

	pthread_mutex_lock(&capture->frameMut);
	memset(capture->frameBuf, 0, FRAME_BUF_SIZE);
	memcpy(capture->frameBuf, capture->buffer[v4l2buf.index].addr, capture->buffer[v4l2buf.index].len);
	pthread_mutex_unlock(&capture->frameMut);
	capture->frameLen = capture->buffer[v4l2buf.index].len;

	ret = ioctl(capture->fd, VIDIOC_QBUF, &v4l2buf);
	if(ret < 0)
	{
		printf("ERROR: get VIDIOC_QBUF failed, ret: %d\n", ret);
		return -1;
	}

	index ++;

	return 0;
}

int capture_getframe(unsigned char *data, int size, int *len)
{
	struct v4l2cap_info *capture = &capture_info;
	int tmpLen;

	if(data==NULL || size<=0)
		return -1;

	tmpLen = (capture->frameLen <size ? capture->frameLen:size);
	if(tmpLen < capture->frameLen)
	{
		printf("Warning: %s: bufout size[%d] < frame size[%d] !!!\n", __FUNCTION__, size, capture->frameLen);
	}

	pthread_mutex_lock(&capture->frameMut);
	memcpy(data, capture->frameBuf, tmpLen);
	pthread_mutex_unlock(&capture->frameMut);
	*len = tmpLen;

	return 0;
}


void *capture_thread(void *arg)
{
	struct v4l2cap_info *capture = &capture_info;
	int ret;

	ret = capture_init(capture);
	if(ret != 0)
	{
		printf("ERROR: capture init failed, ret: %d\n", ret);
		return NULL;
	}
	printf("capture init successfully .\n");

	v4l2cap_start(capture);

	while(1)
	{
	
		ret = v4l2cap_flushframe(capture);
		if(ret == 0)
		{
		}
		else
		{
			printf("ERROR: get capture frame failed, ret: %d\n", ret);
		}
	}

	v4l2cap_stop(capture);

	capture_deinit(capture);

}

int start_capture_task(void)
{
	pthread_t tid;
	int ret;

	ret = pthread_create(&tid, NULL, capture_thread, NULL);
	if(ret != 0)
	{
		return -1;
	}

	return 0;
}


