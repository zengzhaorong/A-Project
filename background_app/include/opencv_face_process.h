#ifndef _OPENCV_FACE_PROCESS_H_
#define _OPENCV_FACE_PROCESS_H_


struct face_process_unit
{
	uint8_t *frame_buf;
	uint32_t frame_size;
};

/* C Call C++ */
#ifdef __cplusplus
extern "C" {
#endif

int opencv_put_frame_detect(uint8_t *buf, uint32_t len);
int opencv_get_frame_detect(uint8_t *buf, uint32_t size);
int start_opencv_face_task(void);

#ifdef __cplusplus
}
#endif



#endif	// _OPENCV_FACE_PROCESS_H_
