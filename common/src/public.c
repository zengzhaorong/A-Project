#include <stdio.h>
#include <string.h>
#include "public.h"
#include "type.h"


void print_hex(char *text, uint8_t *buf, int len)
{
	int i;

	if(text != NULL)
		printf("%s ", text);

	if(buf!=NULL && len>0)
	{
		for(i=0; i<len; i++)
			printf("0x%02x ", buf[i]);
	}
	printf("\n");
}

/* convert time_t to sec in day */
uint32_t time_t_to_sec_day(time_t time)
{
	struct tm *ptm;
	struct tm local_tm;
	uint32_t sec_day;

	ptm = localtime(&time);
	memcpy(&local_tm, ptm, sizeof(struct tm));

	sec_day = local_tm.tm_hour *3600 + local_tm.tm_min *60 + local_tm.tm_sec;

	return sec_day;
}
