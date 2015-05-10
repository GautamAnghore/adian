#ifndef __adian_h__
#define __adian_h__
#include "adian_pkt.h"

//routing agents and alarm definiton

#include <agent.h>
#include <packet.h>
#include <trace.h>
#include <timer-handler.h>
#include <random.h>
#include <classifier-port.h>
#include <mobilenode.h> 
/*MobileNode class owns two functions we are interested in. First of all
is base stn(), which returns identifier of the base station the mobile node is
attached to. Second is set base stn() which is be able to establish suitable base
station for that mobile node*/

#define CURRENT_TIME  Scheduler::instance().clock()
#define JITTER  (Random::uniform()*0.5)

class Adian; // forward declaration
/* Timers */
class Adian_PktTimer : public TimerHandler {

	//Constructor
public:
	Adian_PktTimer(Adian* agent) : TimerHandler() {
		agent_ = agent;
	}

protected:	
	Adian* agent_;
	//MobileNode* node_;
	virtual void expire(Event* e);
};

/* Agents */
class Adian : public Agent {

/* Friends */
	friend class Adian_PktTimer;

/* Private members*/
	nsaddr_t  ra_addr_;
	adian_state  state_;
	adian_rtable  rtable_;
	int  accesible_var_;
	u_int8_t  seq_num_;
	
protected:
	PortClassifier*  dmux_;  // For passing packets up to agents.
	Trace*  logtarget_; // For logging.
	Adian_PktTimer pkt_timer_; // Timer for sending packets.

	inline nsaddr_t&  ra_addr() { return ra_addr_; }
	inline adian_state&  state() { return state_; }
	inline int&  accessible_var() { return accessible_var_; }


	void  forward_data(Packet*);
	void  recv_adian_pkt(Packet*);
	void  send_adian_pkt();
	void  reset_adian_pkt_timer();


public:
	Adian(nsaddr_t);
	int command(int, const char*const*);

	void recv(Packet*, Handler*);
};
#endif
