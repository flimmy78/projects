#ifndef _RADIONETWORK_H
#define _RADIONETWORK_H

#include "taskmanager.h"

enum RADIO_ROLE
{
	ROLE_MAIN = 0x1,
	ROLE_CHILD = 0x2
};

enum RADIO_COMMAND
{
    RD_DISCOVERY = 0x1,
	RD_DIS_REPLY = 0x2,
	RD_DIS_REPLY_ACK = 0x3,
	RD_COM = 0x50,
	RD_BIND = 0x80,
	RD_UNBIND = 0x88,
	RD_HEARTBEAT = 0xF0,
	RD_PACK_EXT = 0x8F
};

typedef struct _radio_packet
{
	uint8_t addr;
	uint8_t cmd;
	uint8_t data[32];
	uint8_t len;
	uint8_t rssi;
	uint8_t lqi;
}RADIO_PACKET;

void radionetwork_init(TASK *task, RADIO_NETWORK *radio);
uint8_t radionetwork_config(RADIO_NETWORK *radio);
uint8_t radionetwork_discovery(RADIO_NETWORK *radio);
void radionetwork_ondata(RADIO_NETWORK *radio,uint8_t *data, int size);
void radionetwork_free(TASK *task, RADIO_NETWORK *radio);

#endif