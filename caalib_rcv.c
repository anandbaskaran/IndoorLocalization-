#include "contiki.h"
#include "lib/list.h"
#include "lib/memb.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include <stdio.h>

#define MAX_ANCHOR_NODES 3
#define MAX_CALIB_PKTS 500

/* This is the structure of calibration messages. */
struct calib_msg {
	uint8_t type;
	uint8_t seqno;
};

/* These are the types of calibration messages. */
enum {
	CALIB_PING,
	CALIB_PONG
};

/* This structure holds information about calibration packets. */
struct calib_pkt {
	/* The ->next pointer is needed since we are placing these on a
     Contiki list. */
	struct calib_pkt *next;

	/* The ->addr field holds the Rime address of the neighbor. */
	//	linkaddr_t addr;

	/* The ->rssi field holds the Received Signal
     Strength Indicator (RSSI) value of the incoming calibration packet. */
	uint16_t rssi;

	/* Each calibration packet contains a sequence number (seqno). The
     ->seqno field holds this sequence number. */
	uint8_t seqno;
};

const linkaddr_t anchor1_addr;
const linkaddr_t anchor2_addr;
const linkaddr_t anchor3_addr;

char anchor1_req_sent = 0, anchor2_req_sent = 0, anchor3_req_sent = 0;
char anchor1_done = 0, anchor2_done = 0, anchor3_done = 0;
char calib_done = 0;

/* This MEMB() definition defines a memory pool from which we allocate
   calib pkt entries. */
//MEMB(anchor_node_memb, struct anchor_node, MAX_ANCHOR_NODES);
MEMB(calib_pkt_memb, struct calib_pkt, MAX_ANCHOR_NODES*MAX_CALIB_PKTS);

/* The calib_pkt_anchor*_list lists are Contiki lists that holds the calibration packets received. */
//LIST(anchor_node_list);
LIST(calib_pkt_anchor1_list);
LIST(calib_pkt_anchor2_list);
LIST(calib_pkt_anchor3_list);

/* This holds the calib structure. */
static struct unicast_conn calib;

/* We first declare our calibration process. */
PROCESS(calib_process, "Calibration process");

/* The AUTOSTART_PROCESSES() definition specifies what processes to
   start when this module is loaded. We put our process here. */
AUTOSTART_PROCESSES(&calib_process);

/* This function is called for every incoming calib packet. */
static void calib_rcv(struct unicast_conn *c, const linkaddr_t *from)
{
	struct calib_msg *msg;
	struct calib_pkt *pkt;

	/* Grab the pointer to the incoming data. */
	msg = packetbuf_dataptr();

	/* We have two message types, CALIB_PING and
	   CALIB_PONG. If we receive a CALIB_PONG message, we
	   print out a message. */
	if(msg->type == CALIB_PONG) {
		printf("Calibration packet received from %d.%d with seqno %d, RSSI %u\n",
				from->u8[0], from->u8[1],
				msg->seqno,
				packetbuf_attr(PACKETBUF_ATTR_RSSI));

		/* We allocate a new struct calib pkt from the calib pkt memb memory
	     pool. */
		pkt = memb_alloc(&calib_pkt_memb);

		/* If we could not allocate a new calib pkt entry, we give up. */
		if(pkt == NULL) {
			return;
		}

		/* Initialize the fields. */
		pkt->seqno = msg->seqno;

		/* Place the calib pkt on the calib pkt list. */
		if(linkaddr_cmp(anchor1_addr, from)) {
			list_add(calib_pkt_anchor1_list, pkt);
			if(pkt->seqno == MAX_CALIB_PKTS) {
				anchor1_done = 1;
			}
		}
		else if(linkaddr_cmp(anchor2_addr, from)) {
			list_add(calib_pkt_anchor2_list, pkt);
			if(pkt->seqno == MAX_CALIB_PKTS) {
				anchor2_done = 1;
			}
		}
		else if(linkaddr_cmp(anchor3_addr, from)) {
			list_add(calib_pkt_anchor3_list, pkt);
			if(pkt->seqno == MAX_CALIB_PKTS) {
				anchor3_done = 1;
			}
		}

		/* We can now fill in the fields in our calib pkt entry. */
		pkt->rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);
	}
}

/* This is where we define what function to be called when a unicast
   is received. We pass a pointer to this structure in the
   unicast_open() call below. */
static const struct unicast_callbacks calib_callbacks = {calib_rcv};

PROCESS_THREAD(calib_process, ev, data)
{
	static uint8_t seqno;
	struct calib_msg msg;
	struct calib_pkt *pkt;

	PROCESS_EXITHANDLER(unicast_close(&calib);)

	PROCESS_BEGIN();

	unicast_open(&calib, 146, &calib_callbacks);

	/* Wait for calibration packets from anchor nodes */
	while(1) {
		if(!anchor1_done) {
			if (!anchor1_req_sent) {
				/* Send CALIB_PING packet. */
				msg.seqno = 1;
				msg.type = CALIB_PING;
				packetbuf_copyfrom(&msg, sizeof(struct calib_msg));
				unicast_send(&calib, anchor1_addr);
				printf("Sending calibration request to %d.%d\n", anchor1_addr.u8[0], anchor1_addr.u8[1]);
				anchor1_req_sent = 1;
			}
		}
		else if(!anchor2_done) {
			if (!anchor2_req_sent) {
				/* Send CALIB_PING packet. */
				msg.seqno = 1;
				msg.type = CALIB_PING;
				packetbuf_copyfrom(&msg, sizeof(struct calib_msg));
				unicast_send(&calib, anchor2_addr);
				printf("Sending calibration request to %d.%d\n", anchor2_addr.u8[0], anchor2_addr.u8[1]);
				anchor2_req_sent = 1;
			}
		}
		else if(!anchor3_done) {
			if (!anchor3_req_sent) {
				/* Send CALIB_PING packet. */
				msg.seqno = 1;
				msg.type = CALIB_PING;
				packetbuf_copyfrom(&msg, sizeof(struct calib_msg));
				unicast_send(&calib, anchor3_addr);
				printf("Sending calibration request to %d.%d\n", anchor3_addr.u8[0], anchor3_addr.u8[1]);
				anchor3_req_sent = 1;
			}
		}
	}

	PROCESS_END();
}
