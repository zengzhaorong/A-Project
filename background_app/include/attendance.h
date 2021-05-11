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
void attendance_reset_tbl(char *tbl_name);
void attendance_clear_tbl(char *tbl_name);
int attendance_save_data_csv(char *filename);
int attendance_set_tbl(char *tbl_name);

#endif	// _ATTENDANCE_H_