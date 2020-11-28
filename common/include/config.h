#ifndef _CONFIG_H_
#define _CONFIG_H_


/********** MAIN WINDOW **********/
#define MAINWIN_TITLE		"人脸识别系统"			// support Chinese


/********** LCD **********/

#define LCD_PIX_WIDTH		640
#define LCD_PIX_HEIGH		480


/********** CAPTURE **********/

/* select video device, from /dev/video* */
#define VIDEO_DEV_NAME 		"/dev/video0"

/* choose one format from below 3 macro */
// CAP_V4L2_FMT_JPEG , CAP_V4L2_FMT_YUV , CAP_V4L2_FMT_MJPEG
#define CAP_V4L2_FMT_MJPEG

#define CAPTURE_PIX_WIDTH		LCD_PIX_WIDTH
#define CAPTURE_PIX_HEIGH		LCD_PIX_HEIGH
#define ONE_CAP_FRAME_SIZE		(CAPTURE_PIX_WIDTH*CAPTURE_PIX_HEIGH)


/********** FACE RECOGNIZE **********/

#define FACE_RECOGN_THRES 		FACE_RECOGN_THRES_80	// face recognize threshold
#define FACE_ROW_MIN			60

#define FACE_CNT_PER_USER		10		// face image count per user

#define RECOGN_OK_DELAY_MS		(2*1000)	// delay time after rocognize success

#endif	// _CONFIG_H_
