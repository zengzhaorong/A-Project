#include <QImage>
#include "image_convert.h"


QImage v4l2_to_QImage(unsigned char *data, int len)
{
	QImage qtImage;

	if(data==NULL || len<=0)
		return qtImage;
	
	qtImage.loadFromData(data, len);
	if(qtImage.isNull())
	{
		printf("ERROR: QImage is null !\n");
	}

	return qtImage;
}



