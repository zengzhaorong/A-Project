#include <stdio.h>
#include <string.h>
#include "public.h"



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



