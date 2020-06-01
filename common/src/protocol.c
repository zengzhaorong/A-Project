#include <stdio.h>
#include <string.h>
#include <time.h>
#include "protocol.h"
#include "ringbuffer.h"
#include "public.h"

/* protocol format:
 * | 0xFF | "ABCD" | .... | .... | .... | .... | 0xFE |
 * | HEAD | VERIFY | SEQ  | CMD  | LEN  | DATA | TAIL |
 * |1 byte| 4 byte |1 byte|1 byte|4 byte|N byte|1 byte|
 *
 */

 /* CMD :
  * 0x01: login (reserve)
  *
  * 0x02: logout (reserve)
  *
  * 0x03: heart beat
  *
  */


/* flag: 0-request, 1-ack */
int proto_0x03_dataAnaly(uint8_t *data, int len, ePacket_t type, void *a, void *b)
{
	int tmplen;

	if(data==NULL || len<=0)
		return -1;

	tmplen = 0;

	if(type == PROTO_REQ)	// request
	{
		if(a == NULL)
			return -1;
			
		memcpy(a, data +tmplen, 4);
	}
	else	// ack
	{
		if(a==NULL || b==NULL)
			return -1;

		memcpy(a, data +tmplen, 4);
		tmplen += 4;
		memcpy(b, data +tmplen, 4);
	}

	return 0;
}

int proto_0x03_sendHeartBeat(send_func_t send_func, int fd)
{
	uint8_t protoBuf[128] = {0};
	uint8_t cmd;
	int packLen = 0;
	time_t time_now;

	cmd = 0x03;

	printf("%s: enter ++\n", __FUNCTION__);

	time_now = time(NULL);

	proto_makeupPacket(0, cmd, sizeof(time_now), (uint8_t *)&time_now, protoBuf, sizeof(protoBuf), &packLen);

	send_func(fd, protoBuf, packLen);
	
	return 0;
}


int proto_makeupPacket(uint8_t seq, uint8_t cmd, int len, uint8_t *data, \
								uint8_t *outbuf, int size, int *outlen)
{
	uint8_t *packBuf = outbuf;
	int packLen = 0;

	if((len!=0 && data==NULL) || outbuf==NULL || outlen==NULL)
		return -1;

	/* if outbuf is not enough */
	if(PROTO_DATA_OFFSET +len +1 > size)
		return -2;

	packBuf[PROTO_HEAD_OFFSET] = PROTO_HEAD;
	packLen += 1;
	
	memcpy(packBuf +PROTO_VERIFY_OFFSET, PROTO_VERIFY, 4);
	packLen += 4;

	packBuf[PROTO_SEQ_OFFSET] = seq;
	packLen += 1;
	
	packBuf[PROTO_CMD_OFFSET] = cmd;
	packLen += 1;

	memcpy(packBuf +PROTO_LEN_OFFSET, &len, 4);
	packLen += 4;

	memcpy(packBuf +PROTO_DATA_OFFSET, data, len);
	packLen += len;

	packBuf[PROTO_DATA_OFFSET +len] = PROTO_TAIL;
	packLen += 1;

	*outlen = packLen;

	return 0;
}

int proto_analyPacket(uint8_t *pack, int packLen, uint8_t *seq, \
								uint8_t *cmd, int *len, uint8_t **data)
{

	if(pack==NULL || seq==NULL || cmd==NULL || len==NULL || data==NULL)
		return -1;

	if(packLen < PROTO_PACK_MIN_LEN)
		return -2;

	*seq = pack[PROTO_SEQ_OFFSET];

	*cmd = pack[PROTO_CMD_OFFSET];

	memcpy(len, pack +PROTO_LEN_OFFSET, 4);

	if(*len +PROTO_PACK_MIN_LEN != packLen)
		return -1;

	if(*len > 0)
		*data = pack + PROTO_DATA_OFFSET;

	return 0;
}

int proto_detectPack(struct ringbuffer *ringbuf, struct detect_info *detect, \
							uint8_t *proto_data, int size, int *proto_len)
{
	char buf[64];
	int len;
	char veri_buf[] = PROTO_VERIFY;
	int tmp_protoLen;
	uint8_t byte;

	if(ringbuf==NULL || proto_data==NULL || proto_len==NULL || size<PROTO_PACK_MIN_LEN)
		return -1;

	tmp_protoLen = *proto_len;

	/* get and check protocol head */
	if(!detect->head)
	{
		while(ringbuf_datalen(ringbuf) > 0)
		{
			ringbuf_read(ringbuf, &byte, 1);
			if(byte == PROTO_HEAD)
			{
				proto_data[0] = byte;
				tmp_protoLen = 1;
				detect->head = 1;
				//printf("********* detect head\n");
				break;
			}
		}
	}

	/* get and check verify code */
	if(detect->head && !detect->verify)
	{
		while(ringbuf_datalen(ringbuf) > 0)
		{
			ringbuf_read(ringbuf, &byte, 1);
			if(byte == veri_buf[tmp_protoLen-1])
			{
				proto_data[tmp_protoLen] = byte;
				tmp_protoLen ++;
				if(tmp_protoLen == 1+strlen(PROTO_VERIFY))
				{
					detect->verify = 1;
					//printf("********* detect verify\n");
					break;
				}
			}
			else
			{
				if(byte == PROTO_HEAD)
				{
					proto_data[0] = byte;
					tmp_protoLen = 1;
					detect->head = 1;
				}
				else
				{
					tmp_protoLen = 0;
					detect->head = 0;
				}
			}
		}
	}

	/* get other protocol data */
	if(detect->head && detect->verify)
	{
		while(ringbuf_datalen(ringbuf) > 0)
		{
			if(tmp_protoLen < PROTO_DATA_OFFSET)	// read data_len
			{
				len = ringbuf_read(ringbuf, buf, PROTO_DATA_OFFSET -tmp_protoLen);
				if(len > 0)
				{
					memcpy(proto_data +tmp_protoLen, buf, len);
					tmp_protoLen += len;
				}
				if(tmp_protoLen >= PROTO_DATA_OFFSET)
				{
					memcpy(&len, proto_data +PROTO_LEN_OFFSET, 4);
					detect->pack_len = PROTO_DATA_OFFSET +len +1;
				}
			}
			else	// read data
			{
				len = ringbuf_read(ringbuf, buf, detect->pack_len -tmp_protoLen);
				if(len > 0)
				{
					memcpy(proto_data +tmp_protoLen, buf, len);
					tmp_protoLen += len;
					if(tmp_protoLen == detect->pack_len)
					{
						if(proto_data[tmp_protoLen-1] != PROTO_TAIL)
						{
							//printf("%s : packet data error, no detect tail!\n", __FUNCTION__);
							memset(detect, 0, sizeof(struct detect_info));
							tmp_protoLen = 0;
							break;
						}
						*proto_len = tmp_protoLen;
						memset(detect, 0, sizeof(struct detect_info));
						//printf("%s : get complete protocol packet, len: %d\n", __FUNCTION__, *proto_len);
						return 0;
					}
				}
			}
		}
	}

	*proto_len = tmp_protoLen;

	return -1;
}





