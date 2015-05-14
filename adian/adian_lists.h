/*
 * Defination of various lists which is used by agent to keep
 * various status data.
 */

#ifndef __adian_lists_h__
#define __adian_lists_h__

#include <map>		// lists implemented using hash tables
#include <list>		// for failed path list

typedef std::map<u_int32_t, double> seq_expire_t;
typedef std::map<int, double> uid_expire_t;

//-----------------------------Reply Route List--------------------------------//

typedef std::map<u_int32_t, nsaddr_t> route_list_t;

class Adian_Reply_Route_list {
// USE: When an agent recieves a hdr_adian_req type packet,
// it tries to find the path to the destination. When destination
// is reachable, agent needs to send hdr_adian_req_reply type packet.
// For sending this reply, Address of node which sent the request is needed.
// That address is stored in this list.

// |----------------------------|		|----------------------------|
// |  seq_num_   |  reply_to_   |       |  seq_num_  |    expire     |
// |----------------------------|       |----------------------------|

	route_list_t	rl_;	//route list
	seq_expire_t	el_;	//expire list

public:
	Adian_Reply_Route_list();
	void add_reply_route(u_int32_t, nsaddr_t, double);	// (request seq_num_, reply_to address, expire time)
	void rm_reply_route(u_int32_t);						// (request seq_num_)
	nsaddr_t lookup(u_int32_t);							// get the reply to address corrosponding to seq_num
	//currently not needed
	//double expire_time(u_int32_t);						// get the expire time of the entry
	void purge();										// remove the expired entries from table. 
};


//-----------------------------Data Source List---------------------------------//

typedef std::map<int, nsaddr_t> src_list_t;

class Adian_Data_Source_list {
// USE: This list works almost the same as Adian_Reply_Route list but
// Adian_Reply_Route is used when the packet is Adian Control Packet. 
// This list will be used when forwarding of data packets will fail for 
// allowed number of times and the agent needs to send an error packet
// back to its source node. If the attempt is successfull, the entry for that
// packet will be deleted by a schedule call.

// |-----------------------------|      |-----------------------------|
// |  packet uid_  |  src_addr_  |      |  packet uid_  |    expire   |
// |-----------------------------|      |-----------------------------|

	src_list_t		sl_;	//source list
	uid_expire_t	el_;	//expire list

public:
	Adian_Data_Source_list();
	void add_data_source(int, nsaddr_t, double);	// (packet unique id, source address, expire time)
	void rm_data_source(int);						// (packet unique id)
	nsaddr_t lookup(int);							// get the source for uid
	//double expire_time(u_int32_t);					// get the expire time
	void purge();									// remove the expired entries.
};


//----------------------------------Attempt List---------------------------------------//

typedef std::map<int, int> attempt_list_t;
class Adian_Attempt_list {
// USE: When forwarding a data packet fails, the agent will make an attempt
// to send the packet from another route available. These attempts are limited
// and are stored in this list. List entry deleted after a certain time.

// |---------------------------------|     |--------------------------|
// |  packet uid_  |  attempts_left  |     |  packet uid_  |  expire  | 
// |---------------------------------|     |--------------------------|

	attempt_list_t		al_;	//attempt list
	uid_expire_t		el_;	//expire list

public:
	Adian_Attempt_list();
	void add_entry(int, int, double);		// (packet unique id, max attempts)
	void rm_entry(int);						// (packet unique id)
	int decrease_attempts(int);				// after making a failed attempt, decrease attempts left (uid)
											// returns number of attempts left (so that no extra function for checking zero) 
	//double expire_time(u_int32_t);			// get the expire time
	void purge();							// remove the expired entries from table.
};


// ------------------------------Failed Path List----------------------------------------//

typedef struct {
	nsaddr_t	next_hop;
	nsaddr_t	daddr;
} failed_path;

typedef std::list<failed_path> pl_t;	//path list type
//NOTE : map with list
typedef std::map<int, pl_t> fpl_t;		//failed path list 

class Adian_Failed_Path_list {
// USE: When sending data packet on a path fails, it is saved in
// this list corrosponding to the packet uid so that while attempting
// another route, agent can identify which path already have been failed
// and not pick that up again. Expires after expire time.

//     Failed Path List
// |--------------------------|		|--------------------------|
// | packet_uid |  path_list  |     |  packet uid_  |  expire  | 
// |--------------------------|     |--------------------------|

//		   Path List
// |------------------------|
// |  next_hop  |   daddr   |  -> ...
// |------------------------|   

	fpl_t			fl_;	//failed list
	uid_expire_t	el_;	//expire list

	void add_new_packet(int, failed_path, double);		//Add a new entry in Failed Path List
														//(packet_uid, failed_path, expire time)
														//the uid is added in failed path list on first failure
														//failed_path is not list, have to be converte to.

public:
	Adian_Failed_Path_list();
	void rm_entry(int);									//(packet_uid)
	void add_failed_path(int, nsaddr_t, nsaddr_t);		//add failed path to existing path list
														//if entry do not exist calls add_new_packet
	int check_failed_path(int, nsaddr_t, nsaddr_t);		// 1 - exists   0 - not exists
														// (uid, next_hop, destination)
	void add_expire_time(int,double);					// add expiry time in existing time
	//double expire_time(int);							// get the expire time
	void purge();										// remove the expired entries from table. 
};

#endif