/*
 * Defination of various lists which is used by agent to keep
 * various status data.
 */

#ifndef __adian_lists_h__
#define __adian_lists_h__

#include <map>		// lists implemented using hash tables

typedef std::map<u_int32_t, nsaddr_t> route_list_t;

class Adian_Reply_Route_list {
// USE: When an agent recieves a hdr_adian_req type packet,
// it tries to find the path to the destination. When destination
// is reachable, agent needs to send hdr_adian_req_reply type packet.
// For sending this reply, Address of node which sent the request is needed.
// That address is stored in this list.

// |----------------------------|
// |  seq_num_   |  reply_to_   |
// |----------------------------|

	route_list_t	rl_;

public:
	Adian_Reply_Route_list();
	void add_reply_route(u_int32_t, nsaddr_t);	// (request seq_num_, reply_to address)
	void rm_reply_route(u_int32_t);				// (request seq_num_)
	nsaddr_t lookup(u_int32_t);					// get the reply to address corrosponding to seq_num
};

typedef std::map<int, nsaddr_t> src_list_t;
class Adian_Data_Source_list {
// USE: This list works almost the same as Adian_Reply_Route list but
// Adian_Reply_Route is used when the packet is Adian Control Packet. 
// This list will be used when forwarding of data packets will fail for 
// allowed number of times and the agent needs to send an error packet
// back to its source node. If the attempt is successfull, the entry for that
// packet will be deleted by a schedule call.

// |-----------------------------|
// |  packet uid_  |  src_addr_  |
// |-----------------------------|

	src_list_t		sl_;

public:
	Adian_Data_Source_list();
	void add_data_source(int, nsaddr_t);	// (packet unique id, source address)
	void rm_data_source(int);				// (packet unique id)
	nsaddr_t lookup(int);					// get the source for uid
};

typedef std::map<int, int> attempt_list_t;
class Adian_Attempt_list {
// USE: When forwarding a data packet fails, the agent will make an attempt
// to send the packet from another route available. These attempts are limited
// and are stored in this list. List entry deleted after a certain time.

// |---------------------------------|
// |  packet uid_  |  attempts_left  |
// |---------------------------------|

	attempt_list_t		al_;

public:
	Adian_Attempt_list();
	void add_entry(int, int);		// (packet unique id, max attempts)
	void rm_entry(int);				// (packet unique id)
	int decrease_attempts(int);		// after making a failed attempt, decrease attempts left (uid)
									// returns number of attempts left (so that no extra function for checking zero) 

};

typedef struct {

	nsaddr_t	next_hop;
	nsaddr_t	daddr;
} failed_path;

typedef std::list<failed_path> fpl_t;
class Adian_Failed_Path_list {

};

#endif