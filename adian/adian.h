//routing agents and alarm definiton

#ifndef __adian_h__
#define __adian_h__

#include "adian_pkt.h"
#include <agent.h>
#include <packet.h>
#include <trace.h>
#include <timer-handler.h>
#include <random.h>
#include <classifier-port.h>

#define CURRENT_TIME	Scheduler::instance().clock()
		//Q: what's the use of this jitter?
#define JITTER			(Random::uniform()*0.5)

class adian; // forward declaration
/* Timers */


class adian_PktTimer : public TimerHandler {
	public:
	adian_PktTimer(adian* agent) : TimerHandler() {
		agent_ = agent;
	}
	protected:
	adian* agent_;
	virtual void expire(Event* e);
};

/* Agent */
class adian : public Agent {
/* Friends */
	friend class adian_PktTimer;
/* Private members*/
nsaddr_t			ra_addr_;
adian_state			state_;
adian_rtable		rtable_;
int 				accesible_var_;
u_int8_t			seq_num_;


protected:
PortClassifier*	dmux_;
// For passing packets up to agents.
Trace*	logtarget_;
// For logging.
adian_PktTimer pkt_timer_; // Timer for sending packets.
inline nsaddr_t& ra_addr() { return ra_addr_; }

inline adian_state& state() { return state_; }

inline int& accessible_var() { return accessible_var_; }

void forward_data(Packet*);
void recv_adian_pkt(Packet*);
void send_adian_pkt();
void reset_adian_pkt_timer();


public:
adian(nsaddr_t);
int command(int, const char*const*);
	void recv(Packet*, Handler*);
};
#endif