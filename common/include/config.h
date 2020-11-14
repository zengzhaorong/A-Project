#ifndef _CONFIG_H_
#define _CONFIG_H_

/* LCD */
#define LCD_PIX_WIDTH		640
#define LCD_PIX_HEIGH		480

/* CAPTURE */
#define VIDEO_DEV_NAME 		"/dev/video0"

/* choose one format from below 3 type */
// CAP_V4L2_FMT_JPEG CAP_V4L2_FMT_YUV CAP_V4L2_FMT_MJPEG
#define CAP_V4L2_FMT_YUV

#define CAPTURE_PIX_WIDTH		LCD_PIX_WIDTH
#define CAPTURE_PIX_HEIGH		LCD_PIX_HEIGH
#define ONE_CAP_FRAME_SIZE		(CAPTURE_PIX_WIDTH*CAPTURE_PIX_HEIGH)

#define LBPH_REC_THRES_0		125.0	// LBPH设定阈值/相似度0
#define LBPH_REC_THRES_80		85.0	// LBPH识别过滤阈值/相似度80
#define LBPH_REC_THRES_100		65.0	// LBPH相似度100阈值

#define FACE_RECO_THRES 		LBPH_REC_THRES_0
#define FACE_ROW_MIN			45

#endif	// _CONFIG_H_
