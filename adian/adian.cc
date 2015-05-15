#include "adian.h"
#include "adian_pkt.h"
// #include "adian_lists.h"
// #include "adian_rtable.h"    // both already included in adian.h

// ------------------------------ TCL Hooks --------------------------------------------//

int hdr_adian::offset_;                 // declaration needed for static member

static class ADIANHeaderClass : public PacketHeaderClass {
public:
    AdianHeaderClass() : PacketHeaderClass("PacketHeader/ADIAN", sizeof(hdr_all_adian)) {
        bind_offset(&hdr_adian::offset_);
    }
} class_rtProtoADIAN_hdr;


static class ADIANClass : public TclClass {
public:
    
	AdianClass() : TclClass("Agent/ADIAN") {}          //It merely calls the base class with the 
                                                        //string “Agent/ADIAN” as an argument

  	TclObject* create(int argc, const char*const* argv) {	
    //returns a new ADIAN instance as a TclObject
    	
        assert(argc == 5);
        // We return a new Adian object with the identifier stated in argv[4].
        // We use the Address class to get a nsaddr_t type from a string.
		return (new ADIAN((nsaddr_t)Address::instance().str2addr(argv[4])));
    }

} class_rtProtoADIAN;


//----------------------- Timer Function Implementations --------------------------------//

void Neighbour_timer::expire(Event* e) {

    //flush the neighbour table
    agent->neighbour_table_.clear();
    //initiate rebuilding
    agent->send_ping();                                  // function broadcasts ping packet and on recieving
                                                         // response, adds them to neighbourhood table
    //reschedule the timer 
    //this is a friend class, so nbtimer_ is accessible
    agent->nbtimer_.resched((double)NB_UPDATE_INTERVAL); // first schedule is done when "start" command is
                                                         // invoked from tcl interface

}

void List_timer::expire(Event* e) {

    // clear every list : remove expired entries
    agent->reply_route_list_.purge();
    agent->data_source_list_.purge();
    agent->attempt_list_.purge();
    agent->failed_path_list_.purge();

    // reschedule the timer
    agent->ltimer_.resched((double)LIST_UPDATE_INTERVAL);
}


//------------------- ADIAN Agent Class Functions Implementations------------------------//

/*
 *Constructor -> 
 */
// PT_ADIAN passed to base class to identify the control packets sent and recieved
ADIAN::ADIAN(nsaddr_t id) : Agent(PT_ADIAN), nbtimer_(this), ltimer_(this) {
    //to bind any variable to tcl interface
    //bind_bool("accessible_var_", &accessible_var_);
    ra_addr_ = id;
    logtarget_ = 0;         //As it will be used to check whether trace file was connected 
                            //successfully or not
}

/*
 * command()
 */
// Command method is inherited from Agent class. These commands are passed from Tcl interface.
int ADIAN::command(int argc, const char*const* argv) {

    // Two arguements passed
    if (argc == 2) {
        
        // "start" command - (mendatory operation)
        // command used to start the functioning of agent
        if (strcasecmp(argv[1], "start") == 0) {
            // start the timer for neighbour table updation
            nbtimer_.resched(0.0);      // now the agent will start pinging
            // start the timer for lists updation
            ltimer_.resched(0.5);       // 0.5 - to avoid syncing of both timers
                                        // and no need to update lists initially 

            return TCL_OK;
        }
        // "print_rtable" command 
        // prints the routing table to trace file
        else if (strcasecmp(argv[1], "print_rtable") == 0) {
            
            if (logtarget_ != 0) {
                sprintf(logtarget_->pt_->buffer(), "A %f _%d_ Routing Table",CURRENT_TIME,ra_addr_);
                logtarget_->pt_->dump();
                routing_table_.print(logtarget_);
            }

            else {
                fprintf(stdout, "%f _%d_ If you want to print this routing table"
                " you must create a trace file in your tcl script", CURRENT_TIME, ra_addr_);
            }
            return TCL_OK;
        }
        //TODO: Add commands for printing all the tables and all the lists
    }
    // Three Arguements Passed
    else if (argc == 3) {
        
        // "port-dmux" command -> (mendatory operation)
        // obtains corresponding dmux to carry packets to upper layers
        if (strcmp(argv[1], "port-dmux") == 0) {
            
            //look the object specified in the list of tcl objects
            dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
            
            if (dmux_ == 0) {
                fprintf(stderr, "%s: %s lookup of %s failed\n", __FILE__, argv[1], argv[2]);
                return TCL_ERROR;
            }
            return TCL_OK;
        }
        // "log-target" or "tracetarget" command -> (mendatory operation)
        // obtains corresponding tracer object and initialises logtarget_
        else if (strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0) {
            
            logtarget_ = (Trace*)TclObject::lookup(argv[2]);
            
            if (logtarget_ == 0) {
                return TCL_ERROR;
            }
            return TCL_OK;
        }
    }

    // If no commands matches the above conditions, 
    // pass the command to be handled by base class
    return Agent::command(argc, argv);
}

/*
 * Packet Receiving Routines
 */

/*
 * Master Routine to recv() packets
 */
// recv() invoked when agent recieves any packet
void ADIAN::recv(Packet* p, Handler* h) {

    struct hdr_cmn* ch = HDR_CMN(p);            // get the common header (packet.h)
    struct hdr_ip* ih= HDR_IP(p);               // get the ip header (ip.h)
    
    if (ih->saddr() == ra_addr_) {
        
        // packet is revieved after forwards, i.e. there is a loop in the route
        // drop the packet
        if (ch->num_forwards() > 0) {
            drop(p, DROP_RTR_ROUTE_LOOP);
            return;
        }
        // else if this is a packet I am originating, must add IP header
        else if (ch->num_forwards() == 0) {
            // if the packet has been generated within the node (by upper layers
            // of the node) we should add to packet’s size the ip header length
            // But if packet is TCP or ACK pkt, IP header is already added
            // so check and add
            if(ch->ptype() != PT_TCP && ch->ptype() != PT_ACK) {
                ch->size() += IP_HDR_LEN;    
            }                   
        }
    }
    
    // If it is an ADIAN packet, must process it
    if (ch->ptype() == PT_ADIAN) {
        ih->ttl_ -= 1;
        // call the master reciever for adian packets
        recvADIAN(p);
        return;
    }                           
    // otherwise, must forward the packet (unless TTL has reached zero)
    else {
        
        ih->ttl_--;
        // if ttl reached zero, drop packet
        if (ih->ttl_ == 0) {
            drop(p, DROP_RTR_TTL);
            return;
        }
    }
    // if packet is not recieved nor dropped, packet is to be fwded
    forward_data(p);
}

/*
 *  Master Routine to recieve PT_ADIAN packets
 */
// this routine identifies the packet type(adian's packet type)
// and then calls the appropriate reciever routine for that packet
void ADIAN::recvADIAN(Packet* p) {

    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian *ah = HDR_ADIAN(p);

    // RT_PORT = 255 // all routing packets are sent and recieved using this port
    // check if this is true
    assert(ih->sport() == RT_PORT);
    assert(ih->dport() == RT_PORT);

    // call the packet specific routine based on their header type
    switch(ah->h_type_) {
        case ADIANTYPE_PING:
            recv_ping(p);
            break;
        case ADIANTYPE_PING_REPLY:
            recv_ping_reply(p);
            break;
        case ADIANTYPE_REQ:
            recv_req(p);
            break;
        case ADIANTYPE_REQ_REPLY:
            recv_req_reply(p);
            break;
        case ADAINTYPE_ERROR:
            recv_error(p);
            break;
        default: 
            fprintf(stderr, "Invalid ADIAN packet type (%x)\n", ah->h_type_);
            exit(1);
    }
}

/*
 * Routine for Recieving Ping Packet recv_ping(p) 
 */
void ADIAN::recv_ping(Packet* p) {

    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian_ping *ah = HDR_ADIAN_PING(p);

    // ping recieved -
    // - entry in neighbour table 
    // - should be responded with a ping reply
    neighbour_table_.add_entry(ih->saddr());
    // with sequence number same as the ping request
    // destination address = ih->saddr()
    // seq_num_ = ah->seq_num_
    send_ping_reply(ih->saddr(), ah->seq_num_);

    // Packet's role is over, so free the memory
    Packet::free(p);
}

/*
 * Routine for Recieving Ping Packet Reply recv_ping_reply(p)
 */
void ADIAN::recv_ping_reply(Packet* p) {

    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian_ping_reply *ah = HDR_ADIAN_PING_REPLY(p);

    // ping reply recieved
    // - TODO: check the sequence number in a new list, if the entry is not there
    //          that means entry's time has expired, so drop the reply 
    // - entry in neighbour table
    neighbour_table_.add_entry(ih->saddr());

    Packet::free(p);
}

/*
 * Routine for Recieving Request Packet recv_req(p)
 */
// This type of packet is recieved when a route is being constructed
// Request - a request for sending data to destination node
void ADIAN::recv_req(Packet* p) {

    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian_req *ah = HDR_ADIAN_REQ(p);

    // Algo
    // search neighbourhood table for requested destination
        // if yes send reply back saying yes
    // search routing table
        // if yes send reply back saying yes

    // before sending request to neighbours, update the Route Reply list 
    //              with respect to sequence number
    // request neighbours with same seq number, same destination, same root, ttl-1

    if(neighbour_table_.lookup(ah->daddr_)) {
        // found in neighbour table
        send_req_reply(ih->saddr(), ah->daddr_, ah->rootaddr_, ah->seq_num_, ah->hop_count_+1, IP_DEF_TTL);
    }
    else {
        rtable_entry look = routing_table_.lookup(ah->daddr_);
        if(look.next_hop != IP_BROADCAST) {
            // route found
            send_req_reply(ih->saddr(), ah->daddr_, ah->rootaddr_, ah->seq_num_, ah->hop_count_+look.nn, IP_DEF_TTL);
        }
        else {
            //(seq num, reply to addr, expire)
            reply_route_list_.add_reply_route(ah->seq_num_, ih->saddr(), CURRENT_TIME+REPLY_ROUTE_LIST_LIFE);
            send_req(ah->daddr_, ah->rootaddr_, ah->seq_num_, ih->ttl_-1);
        }
    }
    Packet::free(p);
}

/*
 * Routine for Recieving Request Packet Reply recv_req_reply(p)
 */
// This type of packet is recieved only if the request sent for finding route to destination
// is successful. 
void ADIAN::recv_req_reply(Packet* p) {

    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian_req_reply *ah = HDR_ADIAN_REQ_REPLY(p);

    // Algo
    // insert into Belief table, the entry with (ih->saddr() as nexthop and ah->daddr as destination)
    // check the entry in routing table and compare based on number of nodes and update if this
    //      route is good
    // check if this node is root
        // if yes, free the packet (if possible, invoke resending of packet as route found)
        // else, check the route reply list. if entry for seq number exists, 
        //          send a req_reply type packet back to source node.

    belief_table_.add_entry(ih->saddr(), ah->daddr_, ah->hop_count_);

    rtable_entry look = routing_table_.lookup(ah->daddr_);
    if(look.next_hop == IP_BROADCAST || ah->hop_count_ < look.nn)
    {
        //(destination address, next_hop, hop count)
        routing_table_.update_entry(ah->daddr_, ih->saddr(), ah->hop_count_);
    }

    if(ah->rootaddr_ != ra_addr_) {
        if(reply_route_list_.lookup(ah->seq_num_))
            send_req_reply(reply_route_list_.lookup(ah->seq_num_), ah->daddr_, ah->rootaddr_, ah->seq_num_, ah->hop_count_ + 1, ih->ttl_-1);
    }

    Packet::free(p);
}

/*
 * Routine for Recieving Error packet
 */
// This type of packet is recieved only if the data fwding was failed by next hop
void ADIAN::recv_error(Packet* p) {
    //algo

}

/*
 * Packet Sending Routines 
 */

/*
 * Routine for Sending Ping packet - send_ping()
 */
void ADIAN::send_ping() {

    Packet *p = allocpkt();     // allocpkt() is defined for all agents

    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian_ping *ah = HDR_ADIAN_PING(p);

    // adian header
    ah->p_type_ = ADIANTYPE_PING;
    ah->seq_num_ = get_next_seq_num();

    // TODO: make an entry of sequence number in a list with an expiry time
    // if the ping reply is recieved after expiry time, discard it.

    // common header
    ch->ptype() = PT_ADIAN;
    ch->size() = IP_HDR_LEN + ah->size();
    ch->error() = 0;
    ch->addr_type() = NS_AF_INET;
    ch->direction() = hdr_cmn::DOWN;
    ch->next_hop() = IP_BROADCAST;

    ih->saddr() = ra_addr_;
    ih->daddr() = IP_BROADCAST;     // ping will be broadcasted
    ih->sport() = RT_PORT;
    ih->dport() = RT_PORT;
    ih->ttl_ = 1;                   // since ping must be accepted by single hop

    Scheduler::instance().schedule(target_, p, 0.0);    // send immediatly
}

/*
 * Routine for Sending Ping Reply packet - send_ping_reply()
 */
void ADIAN::send_ping_reply(nsaddr_t daddr, u_int32_t seq_num) {

    Packet *p = allocpkt();     // allocpkt() is defined for all agents

    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian_ping_reply *ah = HDR_ADIAN_PING_REPLY(p);

    // adian header
    ah->p_type_ = ADIANTYPE_PING_REPLY;
    ah->seq_num_ = seq_num;

    // common header
    ch->ptype() = PT_ADIAN;
    ch->size() = IP_HDR_LEN + ah->size();
    ch->error() = 0;
    ch->addr_type() = NS_AF_INET;
    ch->direction() = hdr_cmn::DOWN;
    ch->next_hop() = daddr;

    ih->saddr() = ra_addr_;
    ih->daddr() = daddr;             // ping back only to requesting node 
    ih->sport() = RT_PORT;
    ih->dport() = RT_PORT;
    ih->ttl_ = 1;                   // since ping must be accepted by single hop

    Scheduler::instance().schedule(target_, p, 0.0);    // send immediatly
}


/*
 * Routine for Sending Request Packet send_req()
 */
// This routine is called after checking the neighbour table and routing table.
// If no path exists to daddr, this request will find a path
void ADIAN::send_req(nsaddr_t route_daddr, nsaddr_t root, u_int32_t seq_num, int ttl) {
   // Algo
    // get all the neighbours from neighbour table
    // for each
        // create a packet
        // set daddr as neighbour address
        // set passed values
        // send packet
    neighbour_entry neighbours = neighbour_table_.get_neighbours();
    neighbour_entry::iterator it;

    for(it = neighbours.begin(); it != neighbours.end(); it++) {
        Packet *p = allocpkt();     // allocpkt() is defined for all agents

        struct hdr_cmn *ch = HDR_CMN(p);
        struct hdr_ip *ih = HDR_IP(p);
        struct hdr_adian_req *ah = HDR_ADIAN_REQ(p);

        // adian header
        ah->p_type_ = ADIANTYPE_REQ;
        ah->seq_num_ = seq_num;
        ah->daddr_ = route_daddr;
        ah->rootaddr_ = root;
        ah->hop_count_ = 0;

        // common header
        ch->ptype() = PT_ADIAN;
        ch->size() = IP_HDR_LEN + ah->size();
        ch->error() = 0;
        ch->addr_type() = NS_AF_INET;
        ch->direction() = hdr_cmn::DOWN;
        ch->next_hop() = IP_BROADCAST;

        ih->saddr() = ra_addr_;
        ih->daddr() = (*it);
        ih->sport() = RT_PORT;
        ih->dport() = RT_PORT;
        ih->ttl_ = ttl;       

        Scheduler::instance().schedule(target_, p, 0.0);    // send immediatly
    }
}

/*
 * Routine for Sending Request Reply Packet send_req()
 */
// This routine is called after checking the neighbour table and routing table,
// or after recieving a route reply and node is not root node.
void ADIAN::send_req_reply(nsaddr_t daddr, nsaddr_t route_daddr, nsaddr_t root, u_int32_t seq_num, u_int32_t nn, int ttl) {
   // Algo
    // create a packet
    // set passed values
    // send packet
    Packet *p = allocpkt();     // allocpkt() is defined for all agents

    struct hdr_cmn *ch = HDR_CMN(p);
    struct hdr_ip *ih = HDR_IP(p);
    struct hdr_adian_req_reply *ah = HDR_ADIAN_REQ_REPLY(p);

    // adian header
    ah->p_type_ = ADIANTYPE_REQ_REPLY;
    ah->seq_num_ = seq_num;
    ah->daddr_ = route_daddr;
    ah->rootaddr_ = root;
    ah->hop_count_ = nn;

    // common header
    ch->ptype() = PT_ADIAN;
    ch->size() = IP_HDR_LEN + ah->size();
    ch->error() = 0;
    ch->addr_type() = NS_AF_INET;
    ch->direction() = hdr_cmn::DOWN;
    ch->next_hop() = IP_BROADCAST;

    ih->saddr() = ra_addr_;
    ih->daddr() = daddr;
    ih->sport() = RT_PORT;
    ih->dport() = RT_PORT;
    ih->ttl_ = ttl;       

    Scheduler::instance().schedule(target_, p, 0.0);    // send immediatly
}

/*
 * Routine for Sending Error Packet
 */
// This routine is called after the allowed attempts are over and packet sending is not successfull
void ADIAN::send_error() {

}

/*
 * Routine for forwarding data based on type of data
 */
void ADIAN::forward_data(Packet* p) 
{
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ip* ih = HDR_IP(p);

  	if (ch->direction() == hdr_cmn::UP && ((u_int32_t)ih->daddr() == IP_BROADCAST || ih->daddr() == ra_addr_) 
   	{
    	dmux_->recv(p, 0.0);
    	return;
   	}
  	else {
   		ch->direction() = hdr_cmn::DOWN;
   		ch->addr_type() = NS_AF_INET;
   		if ((u_int32_t)ih->daddr() == IP_BROADCAST)
   			ch->next_hop() = IP_BROADCAST;
		else {
            // get the next hop for destination from routing table
    		nsaddr_t next_hop = routing_table_.lookup(ih->daddr()).next_hop;
    		if (next_hop == IP_BROADCAST) {
    			debug("%f: Agent %d can not forward a packet destined to %d\n",CURRENT_TIME,ra_addr(),ih->daddr());
                // start building path
                send_req(ih->daddr(), ra_addr_, get_next_seq_num(), IP_DEF_TTL);
				drop(p, DROP_RTR_NO_ROUTE);
                return;
	   		}
  		else
   			ch->next_hop() = next_hop;
  		}
  		Scheduler::instance().schedule(target_, p, 0.0);  //sending actual packet when no error

 	}
}