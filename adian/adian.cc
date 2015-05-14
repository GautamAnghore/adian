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

// Constructor -> 
// PT_ADIAN passed to base class to identify the control packets sent and recieved
ADIAN::ADIAN(nsaddr_t id) : Agent(PT_ADIAN), nbtimer_(this), ltimer_(this) {
    //to bind any variable to tcl interface
    //bind_bool("accessible_var_", &accessible_var_);
    ra_addr_ = id;
    logtarget_ = 0;         //As it will be used to check whether trace file was connected 
                            //successfully or not
}

// command()
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




//recv() invoked when agent recieve the packet.

  void Adian::recv(Packet* p, Handler* h)
  {
          struct hdr_cmn* ch = HDR_CMN(p);  //every packet has common header,accessed by HDR_CMN(included in common/packet.h)
          struct hdr_ip* ih= HDR_IP(p);     //to get IP add. of packet included in ip.h
           if (ih->saddr() == ra_addr()) {
            // If there exists a loop, must drop the packet
             if (ch->num_forwards() > 0) {
                    drop(p, DROP_RTR_ROUTE_LOOP);     //if packet that we send is recieved by us also then we drop that packet.
                    return;
   }
      // else if this is a packet I am originating, must add IP header
          else if (ch->num_forwards() == 0)
          ch->size() += IP_HDR_LEN;                    /*if the packet has been generated within the node (by
                                                       upper layers of the node) we should add to packet’s length the overhead that
                                                       the routing protocol is adding (in bytes).*/
  }
   // If it is a ADIAN packet, must process it
    if (ch->ptype() == PT_ADIAN)
    recv_adian_pkt(p);                           /*When the received packet is of type PT ADIAN then we will call
					          recv ADIAN pkt() to process it (lines 18-19). If it is a data packet then
					          we should forward it (if it is destined to other node) or to deliver it to upper
					          layers (if it was a broadcast packet or was destined to ourself), unless TTL 3
                                                  reached zero.*/

    // Otherwise, must forward the packet (unless TTL has reached zero)
    else
    {
     ih->ttl_--;
     if (ih->ttl_ == 0) {
     drop(p, DROP_RTR_TTL);
     return;
    }
    forward_data(p);
   }
 }



// definition of recv_ADIAN_pkt()
void Adian::recv_adian_pkt(Packet* p) {
	struct hdr_ip* ih= HDR_IP(p);
	struct hdr_adian_pkt* ph = HDR_ADIAN_PKT(p);
// All routing messages are sent from and to port RT_PORT, so we check it.

 /*next 2 lines get IP header and ADIAN packet header as usual. After that we make sure source and destination ports are RT PORT. This
  constant is defined in common/packet.h and it equals 255. This port is reserved to attach the routing agent.*/
	assert(ih->sport() == RT_PORT);
	assert(ih->dport() == RT_PORT);

/* ... processing of ADIAN packet ... */
// Release resources acquired by packet.
	Packet::free(p);
}




  //definition of send_ADIAN_pkt()
void Adian::send_adian_pkt() {
    Packet* p= allocpkt();		//allocation of packet,allocpkt() is defined for all agents.
    //we get common HDR and hdr IP
    struct hdr_cmn* ch= HDR_CMN(p);
    struct hdr_ip* ih= HDR_IP(p);
    struct hdr_adian_pkt* ph = HDR_ADIAN_PKT(p);

    //we are assigning values to packet header attributes.
    ph->pkt_src()= ra_addr();
    ph->pkt_len()= 7;
    ph->pkt_seq_num()= seq_num_++;
    
//The common header in NS as used below.Configuration of packet header. 
    ch->ptype()= PT_ADIAN;
    ch->direction()=hdr_cmn::DOWN;
    ch->size()=IP_HDR_LEN + ph->pkt_len(); //size calculated in bytes and for ns2 calculations such as propogation delay.
    ch->error()=0;				//to check whether any error in transmission or not
    ch->next_hop()=IP_BROADCAST;	/*assigns the next hop to which the packet must be sent to.
  					  it is established as IP BROADCAST because we want all
                                          of the neighboring nodes to receive this control packet.Included in ip.h. */
  
	ch->addr_type()=NS_AF_INET;		/*The last field we fill is the address type.We choose NS AF INET because we are implementing
                                          an Internet protocol.Included in packet.h*/

  	ch->xmit_failure_= ADIAN_mac_failed_callback;
  	ch->xmit_failure_data_ = (void*)this;


//configuration of ip header     
     ih->saddr()=ra_addr();
     ih->daddr()=IP_BROADCAST;
     ih->sport()=RT_PORT;
     ih->dport()=RT_PORT;
     ih->ttl()=IP_DEF_TTL;	/*new constant called IP DEF TTL which
                                  is defined in common/ip.h and represents the default TTL value for IP packets.*/
   
   	Scheduler::instance().schedule(target_, p, JITTER); /*sending a packet is equivalent to schedule it at a certain time.
						         The Packet class inherits from the Connector class, which has a
						         reference to a TclObject called target . This is the handler which will process
						         the event, and is passed as an argument to the schedule() function.*/
}




//definition of reset_ADIAN_pkt_timer
//Our packet sending timer performs another callback to reschedule itself. pkt timer is rescheduled to expire five seconds later.
void Adian::reset_adian_pkt_timer() 
{
	pkt_timer_.resched((double)5.0);
}




//Definition of forward data() function which decides whether a packet has to be delivered to the upper-layer agents or to be forwarded to  other node.
void Adian::forward_data(Packet* p) 
{
	struct hdr_cmn* ch = HDR_CMN(p);
	struct hdr_ip* ih = HDR_IP(p);
	ch->xmit_failure_= adian_mac_failed_callback;
	ch->xmit_failure_data_ = (void*)this;

//a packet has to be delivered to the upper-layer agents
  	if (ch->direction() == hdr_cmn::UP && ((u_int32_t)ih->daddr() == IP_BROADCAST || ih->daddr() == ra_addr())) 
   	{
    	dmux_->recv(p, 0.0);
    	return;
   	}

/*When it is an incoming packet and destination address is the node itself or broadcast, then we use the node’s dmux
  (if we remember it is a PortClassifier object) to accept the incoming packet.*/
  	else {
   		ch->direction() = hdr_cmn::DOWN;
   		ch->addr_type() = NS_AF_INET;
   		if ((u_int32_t)ih->daddr() == IP_BROADCAST)
   			ch->next_hop() = IP_BROADCAST;


/*If the packet is a broadcast one, then next hop will be filled accordingly. If not, we make use of our routing table to find out the next hop.
  Our implementation returns IP BROADCAST when there is no route to destination address. In such a case we print a debug message and drop the  packet. If everything goes fine then we will send the packet .*/
  		else {
    		nsaddr_t next_hop = rtable_.lookup(ih->daddr());
    		if (next_hop == IP_BROADCAST) {
    			debug("%f: Agent %d can not forward a packet destined to %d\n",CURRENT_TIME,ra_addr(),ih->daddr());
    			drop(p, DROP_RTR_NO_ROUTE);
				return;
	   		}
  		else
   			ch->next_hop() = next_hop;
  		}
  		Scheduler::instance().schedule(target_, p, 0.0);  //sending actual packet when no error

 	}
}