#include "radionetwork.h"
#include "hardware.h"
#include "packet.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "libxml/xmlmemory.h"
#include "libxml/parser.h"

#define RADIO_CONFIG_FILE "radio.xml"

#define REG_PKTCTRL1 0x7

static void radionetwork_loadconfig(RADIO_NETWORK *radio)
{
    xmlDocPtr doc;
	xmlNodePtr cur;
	
	doc = xmlParseFile(RADIO_CONFIG_FILE);
	if (doc == NULL) return;

	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		xmlFreeDoc(doc);
		return;
	}

	if (!xmlStrEqual(cur->name, BAD_CAST"radio")){
		xmlFreeDoc(doc);
		return;
	}

	if (xmlHasProp(cur, BAD_CAST" state")){
		radio->state = atoi((char *)xmlGetProp(cur, BAD_CAST"state"));
	}
	if (xmlHasProp(cur, BAD_CAST"addr")){
		radio->addr = atoi((char *)xmlGetProp(cur, BAD_CAST"addr"));
	}
	if (xmlHasProp(cur, BAD_CAST"channel")){
		radio->channel = atoi((char *)xmlGetProp(cur, BAD_CAST"channel"));
	}
	if (xmlHasProp(cur, BAD_CAST"role")){
		radio->role = atoi((char *)xmlGetProp(cur, BAD_CAST"role"));
	}


	RADIO_CHILD *cl = radio->child;
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		if (xmlStrEqual(cur->name, BAD_CAST"child")){
			if (cl == NULL){
				cl = radio->child = malloc(sizeof(RADIO_CHILD));
			}
			else{
				cl = cl->next = malloc(sizeof(RADIO_CHILD));
			}

			cl->next = NULL;

			if (xmlHasProp(cur, BAD_CAST"addr")){
				cl->addr = atoi((char *)xmlGetProp(cur, BAD_CAST"addr"));
			}
			if (xmlHasProp(cur, BAD_CAST"channel")){
				cl->channel = atoi((char *)xmlGetProp(cur, BAD_CAST"channel"));
			}
			if (xmlHasProp(cur, BAD_CAST"role")){
				cl->role = atoi((char *)xmlGetProp(cur, BAD_CAST"role"));
			}
			if (xmlHasProp(cur, BAD_CAST"recv_rssi")){
				cl->recv_rssi = atoi((char *)xmlGetProp(cur, BAD_CAST"recv_rssi"));
			}
			if (xmlHasProp(cur, BAD_CAST"send_rssi")){
				cl->send_rssi = atoi((char *)xmlGetProp(cur, BAD_CAST"send_rssi"));
			}
		}
		cur = cur->next;
	}

	xmlFreeDoc(doc);
}


static void radionetwork_createNewXml(RADIO_NETWORK *radio)
{
	xmlDocPtr doc;
	xmlNodePtr curNode = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	curNode = xmlNewNode(NULL, BAD_CAST "radio");
	char temp[64];
	sprintf(temp,"%d",radio->state);
	xmlSetProp(curNode, BAD_CAST"state", BAD_CAST temp);
	sprintf(temp,"%d",radio->addr);
	xmlSetProp(curNode, BAD_CAST"addr", BAD_CAST temp);
	sprintf(temp,"%d",radio->channel);
	xmlSetProp(curNode, BAD_CAST"channel", BAD_CAST temp);
	sprintf(temp,"%d",radio->role);
	xmlSetProp(curNode, BAD_CAST"role", BAD_CAST temp);
	xmlDocSetRootElement(doc, curNode);

	xmlSaveFile(RADIO_CONFIG_FILE, doc);
	xmlFreeDoc(doc);
}

static void radionetwork_saveXml(RADIO_NETWORK *radio)
{
	xmlDocPtr doc;              
	xmlNodePtr curNode;

	doc = xmlParseFile(RADIO_CONFIG_FILE);  
	if (doc == NULL)
	{
		radionetwork_createNewXml(radio);
		return;
	}

	curNode = xmlDocGetRootElement(doc);       
	if (curNode == NULL)
	{
		xmlFreeDoc(doc);       
		remove(RADIO_CONFIG_FILE);
		radionetwork_createNewXml(radio);
		return;
	}

	if (xmlStrcmp(curNode->name, (const xmlChar*)"radio"))  
	{
		xmlFreeDoc(doc);
		remove(RADIO_CONFIG_FILE);
		radionetwork_createNewXml(radio);
		return;
	}
	char temp[64];
	sprintf(temp,"%d",radio->state);
	xmlSetProp(curNode, (const xmlChar *)"state", (const xmlChar *)temp);
	sprintf(temp,"%d",radio->addr);
	xmlSetProp(curNode, (const xmlChar *)"addr", (const xmlChar *)temp);
	sprintf(temp,"%d",radio->channel);
	xmlSetProp(curNode, (const xmlChar *)"channel", (const xmlChar *)temp);
	sprintf(temp,"%d",radio->role);
	xmlSetProp(curNode, (const xmlChar *)"role", (const xmlChar *)temp);

	xmlSaveFile(RADIO_CONFIG_FILE, doc);
	xmlFreeDoc(doc);
}

static void radionetwork_setaddr_freq(uint8_t role,uint8_t addr, uint8_t channel)
{
	uint8_t temp[260] = {0};
	uint8_t data[12] = {0};
	uint8_t *enc = data;
	
	*enc++= 0x65;
	*enc++= SYSTEM_SET;
	*enc++= CC1101;
	*enc++= SYSTEM_WIRTE;
	*enc++= REG_PKTCTRL1; //start addr
	*enc++= 0x4; //count
	*enc++= role == ROLE_MAIN ? 0x6 : 0x5;  //Address check and 0 (0x00) broadcast/Address check, no broadcast
	*enc++= 0x5;
	*enc++= addr;
	*enc++= channel;
	
	enc = packet_to_hardware(temp, data, enc - data);
	hardware_send(temp, enc - temp);
}

static void radionetwork_send(RADIO_PACKET *pkt)
{
    uint8_t temp[260] = {0};
	uint8_t data[36] = {0};
	uint8_t *enc = data;
	
	*enc++= 0x65;
	*enc++= CC1101;
	memcpy(enc, pkt->data, pkt->len);
	enc += pkt->len;
	
	enc = packet_to_hardware(temp, data, enc - data);
	hardware_send(temp, enc - temp);
}


static void radionetwork_discovery_send(RADIO_NETWORK *radio)
{
	uint8_t *enc;
	
	RADIO_PACKET pkt= {0};
	pkt.addr = 0x0;
	pkt.cmd  = RD_DISCOVERY;

	enc = pkt.data;
	*enc++= ++radio->dis_count;
	*enc++= radio->channel;
	enc = packet_writeInt16(enc, 100);
    pkt.len = enc - pkt.data;
	
	radionetwork_send(&pkt);
}

static void radionetwork_callback(void *radio)
{
	
}

void radionetwork_init(TASK *task, RADIO_NETWORK *radio)
{
	radio->state = 0x1;
	radio->addr = 0x0;
	radio->channel = 0x0;
	radio->role = 0x0;
	radio->child = NULL;
	radio->radiotask = NULL;
	radio->dis_state = 0x0;

	radionetwork_loadconfig(radio);

	if(radio->state == 0x1 && radio->role > 0x0){
	   //set headbeart
	   radio->radiotask = malloc(sizeof(TASK_TIME));
	   radio->radiotask->arg = radio;
	   radio->radiotask->callback = radionetwork_callback;
	   taskmanager_add_timer(task, radio->radiotask, 30000);
	}
}

uint8_t radionetwork_config(RADIO_NETWORK *radio)
{
	radionetwork_saveXml(radio);
	return 0x1;
}

uint8_t radionetwork_discovery(RADIO_NETWORK *radio)
{
	if(radio->dis_state == 0){
	   radio->dis_state = 0x2;
	   radio->dis_count = 0x0;
	
	   radionetwork_setaddr_freq(ROLE_MAIN, radio->dis_addr, radio->dis_channel);
       radionetwork_discovery_send(radio);
	}

	return radio->dis_state;
}

void radionetwork_onpacket(RADIO_NETWORK *radio, RADIO_PACKET *pkt)
{
	switch(pkt->cmd)
	{
		case RD_DISCOVERY:
			
			break;
		case RD_DIS_REPLY:
			break;
		case RD_DIS_REPLY_ACK:
			break;
		case RD_COM:
			break;
		case RD_BIND:
			break;
		case RD_UNBIND:
			break;
		case RD_HEARTBEAT:
			break;
		case RD_PACK_EXT:
			break;	
	}
}

void radionetwork_ondata(RADIO_NETWORK *radio,uint8_t *data, int size)
{
	uint8_t *enc = data+2;

	RADIO_PACKET pkt = {0};
    
	pkt.addr = *enc++;
	pkt.cmd  = *enc++;
	memcpy(pkt.data, enc, size - 4);
	enc+=size - 4;
	pkt.rssi = *enc++;
	pkt.lqi  = *enc++;

	radionetwork_onpacket(radio, &pkt);
}

void radionetwork_free(TASK *task, RADIO_NETWORK *radio)
{
   RADIO_CHILD *cl;
   if(radio->radiotask != NULL){
	   taskmanager_remove_timer(radio->radiotask);
	   free(radio->radiotask);
	   radio->radiotask = NULL;
   }

   cl = radio->child;
   while(cl != NULL){
	   RADIO_CHILD *swp = cl;	   
	   cl = swp->next;
	   free(swp);
   }

   radio->child = NULL;

   xmlCleanupParser();
}