#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "capture.h"




int capture_init(void)
{

	return 0;
}

void capture_deinit(void)
{
	
}

void *capture_thread(void *arg)
{

	capture_init();


	while(1)
	{
		printf("%s %d: running ...\n", __FUNCTION__, __LINE__);
		sleep(3);
	}

	capture_deinit();

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


