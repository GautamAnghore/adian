#ifndef __ADIAN_h__
#define __ADIAN_h__
#include "ADIAN_pkt.h"
#include <agent.h>
#include <packet.h>
#include <trace.h>
#include <timer-handler.h>
#include <random.h>
#include <classifier-port.h>
#include <mobilenode.h> /*MobileNode class owns two functions we are interested in. First of all
is base stn(), which returns identifier of the base station the mobile node is
attached to. Second is set base stn() which is be able to establish suitable base
station for that mobile node*/

#define CURRENT_TIME
#define JITTER Scheduler::instance().clock()
(Random::uniform()*0.5)
class ADIAN; // forward declaration
/* Timers */
class ADIAN_PktTimer : public TimerHandler {
public:
ADIAN_PktTimer(ADIAN* agent) : TimerHandler() {
agent_ = agent;
}
protected:
ADIAN* agent_;
MobileNode* node_;

class ADIAN : public Agent {
/* ... */
public:
ADIAN(nsaddr_t);
int command(int, const char*const*);
void recv(Packet*, Handler*);
void mac_failed(Packet*);
};
#endif
