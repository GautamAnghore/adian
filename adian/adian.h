#ifndef __adian_h__
#define __adian_h__

// #include "adian_pkt.h"	// not needed here, included in adian.cc
#include "adian_rtable.h"
#include "adian_lists.h"

#include <agent.h>
#include <packet.h>
#include <trace.h>
#include <cmu-trace.h>
#include <timer-handler.h>
#include <random.h>
#include <classifier/classifier-port.h>

#define CURRENT_TIME  Scheduler::instance().clock()
#define JITTER  (Random::uniform()*0.5)

#define NB_UPDATE_INTERVAL		2		//Interval after which nb_table is flushed
#define LIST_UPDATE_INTERVAL	2		//Interval after which expired values are removed from all lists

class ADIAN; // forward declaration for timer definations

//---------------------------- Timers --------------------------------//

// Using TimerHandler as a base class instead of Handler
// Timer for Rebuilding Neighbourhood table
class Neighbour_timer : public TimerHandler {
private:
	ADIAN*	agent;		// Agent with which this timer will be associated
public:
	Neighbour_timer(ADIAN *a): agent(a) {}
protected:
	virtual void expire(Event*);	// Function called when this timer expires
									// Truncate the Neighbourhood table and call 
									// the function to rebuild the neighbourhood table 
};

// Instead of using different timers classes and handling different events for each list
// updation, we can use single timer class and update each list.
class List_timer : public TimerHandler {
private:
	ADIAN*	agent;
public:
	List_timer(ADIAN *a): agent(a) {}
protected:
	virtual void expire(Event*);	// remove expired entries from each list
									// Reply_Route_list, Data_Source_list, Attempt_list
									// and Failure_Path_list
};


//-------------------------------- The Routing Agent ----------------------------------//

class ADIAN : public Agent {

	// Friend List
	friend class Neighbour_timer;
	friend class List_timer;

protected:
	//(why protected? why not private)

	// Node Attributes/Properties
	nsaddr_t  					ra_addr_;			// Address of this node

	// Agent Tables 
	Adian_rtable 				routing_table_;
	Adian_nbtable				neighbour_table_;
	Adian_btable 				belief_table_;

	// Agent Lists
	Adian_Reply_Route_list		reply_route_list_;
	Adian_Data_Source_list		data_source_list_;
	Adian_Attempt_list			attempt_list_;
	Adian_Failed_Path_list 		failed_path_list_;

	// Agent Timers
	Neighbour_timer				nbtimer_;
	List_timer					ltimer_;

	// Accessibility Variables
	PortClassifier*  			dmux_;  			// For passing packets up to agents.
	Trace*  					logtarget_; 		// For logging.


public:

	//----------------------- Constructor ------------------------------------
	ADIAN(nsaddr_t);									//(id)

	//------------------ Packet Receiving functions --------------------------
protected:
	// Master Recieve Method for ADIAN packets
	void	recvADIAN(Packet*);					// If the packet type is PT_ADIAN,
												// recv will call this funcition
												// It will check the packet type 
												// and call the respective reciever

	// Different Packet specific recievers
	void	recv_ping(Packet*);
	void	recv_ping_reply(Packet*);
	void	recv_req(Packet*);
	void	recv_req_reply(Packet*);
	void	recv_error(Packet*);

public:
	// Master Recieve Method for ALL packets
	void	recv(Packet*, Handler*);			// On recieving a packet, this method is
												// is called. It checks various possiblities
												// and if the packet type is PT_ADIAN, calls
												// master method for adian pkt `recvADIAN`

	//------------------- Packet Transmission functions ----------------------
protected:
	// Master Trasmission Function
	void	forward_data(Packet*);				// Called by recv() function, checks if the packet
												// is destined for this node and accepts it or
												// checks if it needs to be forwarded or replied
												// with another new packet.
	// Different Packet specific Transmission functions
	// (TODO: Parameters need to be decided during implementations)
	void	send_ping();
	void	send_ping_reply();
	void	send_req();
	void	send_req_reply();
	void	send_error();

	//--------------------- TCL Binding helper function -----------------------
	int 	command(int, const char*const*);	// defines various commands and their implementations
												// that can be used from TCL interface. 

	//---------------- Unique Sequence Number for ADIAN Packets ---------------
private:
	static u_int32_t	sequence_num_;			// Initialised with 1
public:
	inline static u_int32_t	get_next_seq_num() {
 	// returns new seq num by adding 1
 			return sequence_num_++;		
	}
};

#endif
