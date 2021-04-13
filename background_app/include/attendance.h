#ifndef _ATTENDANCE_H_
#define _ATTENDANCE_H_

#include <time.h>

/* C++ include C */
#ifdef __cplusplus
extern "C" {
#endif
#include "public.h"
#ifdef __cplusplus
}
#endif


attend_sta_e attendance_set_one(int id, uint32_t time);
void attendance_reset_all(void);
int attendance_save_data_csv(char *filename);

#endif	// _ATTENDANCE_H_