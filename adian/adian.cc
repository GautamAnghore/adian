int ADIAN_pkt::offset_;
static class ADIANHeaderClass : public PacketHeaderClass 
{
  public:
   ADIANHeaderClass() : PacketHeaderClass("PacketHeader/ADIAN",sizeof(hdr_ADIAN_pkt))
 {
   bind_offset(&hdr_ADIAN_pkt::offset_);
 }
} class_rtADIAN_hdr;




static class ADIANClass : public TclClass 
{
 public:
 //class constructor and it merely calls the base class with the string “Agent/ADIAN” as an argument
  ADIANClass() : TclClass("Agent/ADIAN") {}
  TclObject* create(int argc, const char*const* argv) {	
 //create() which returns a new ADIAN instance as a TclObject

  assert(argc == 5);

   return (new ADIAN((nsaddr_t)Address::instance().str2addr(argv[4])));
   //we return a new ADIAN object with the identifier stated in argv[4]. We use the Address class to get a nsaddr t type from a string.

  }
 } class_rtADIAN;



  //about timers expire() method. Implementing this because we only want to send a new control packet and to reschedule the timer itself.
  void ADIAN_PktTimer::expire(Event* e) 
  {
   agent_->send_ADIAN_pkt();
   agent_->reset_ADIAN_pkt_timer();
  }



  /*calling the constructor for the base class passing PT_ADIAN as an argument.
 it is used to identify control packets sent and received by this routing agent.*/
  ADIAN::ADIAN(nsaddr_t id) : Agent(PT_ADIAN), pkt_timer_(this) 
  {
    bind_bool("accessible_var_", &accessible_var_);
    ra_addr_ = id;
    node_= (MobileNode*)Node::get_node_by_address(id);
  }




  //It consists of the implementation of the command() method that our agent inherites from the Agent class.
   int ADIAN::command(int argc, const char*const* argv) {
   if (argc == 2)
    {
     if (strcasecmp(argv[1], "start") == 0) 
     {
	pkt_timer_.resched(0.0); 
	return TCL_OK;
     }

   else if (strcasecmp(argv[1], "print_rtable") == 0) 
   {
    if (logtarget_ != 0) {
    sprintf(logtarget_->pt_->buffer(), "P %f _%d_ Routing Table",CURRENT_TIME,ra_addr());
    logtarget_->pt_->dump();
    rtable_.print(logtarget_);
   }

   else {
    fprintf(stdout, "%f _%d_ If you want to print this routing table ""you must create a trace file in your tcl script",CURRENT_TIME,ra_addr());
     }
     return TCL_OK;
   }

}

  else if (argc == 3) {
  // Obtains corresponding dmux to carry packets to upper layers
   if (strcmp(argv[1], "port-dmux") == 0) {
   dmux_ = (PortClassifier*)TclObject::lookup(argv[2]);
   if (dmux_ == 0) {
   fprintf(stderr, "%s: %s lookup of %s failed\n",__FILE__,argv[1],argv[2]);
   return TCL_ERROR;
   }
  return TCL_OK;
 }


   // Obtains corresponding tracer
  else if (strcmp(argv[1], "log-target") == 0 || strcmp(argv[1], "tracetarget") == 0) {
  logtarget_ = (Trace*)TclObject::lookup(argv[2]);
  if (logtarget_ == 0)
  return TCL_ERROR;
  return TCL_OK;
   }
  }
  // Pass the command to the base class
 return Agent::command(argc, argv);
 }




//recv() invoked when agent recieve the packet.

  void ADIAN::recv(Packet* p, Handler* h)
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
    recv_ADIAN_pkt(p);                           /*When the received packet is of type PT ADIAN then we will call
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
void ADIAN::recv_ADIAN_pkt(Packet* p) {
struct hdr_ip* ih= HDR_IP(p);
struct hdr_ADIAN_pkt* ph = HDR_ADIAN_PKT(p);
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
   void ADIAN::send_ADIAN_pkt() {
     Packet* p= allocpkt();		//allocation of packet,allocpkt() is defined for all agents.
    //we get common HDR and hdr IP
     struct hdr_cmn* ch= HDR_CMN(p);
     struct hdr_ip* ih= HDR_IP(p);
     struct hdr_ADIAN_pkt* ph = HDR_ADIAN_PKT(p);

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
  void ADIAN::reset_ADIAN_pkt_timer() 
  {
   pkt_timer_.resched((double)5.0);
  }




//Definition of forward data() function which decides whether a packet has to be delivered to the upper-layer agents or to be forwarded to  other node.
 void ADIAN::forward_data(Packet* p) 
 {
  struct hdr_cmn* ch = HDR_CMN(p);
  struct hdr_ip* ih = HDR_IP(p);
  ch->xmit_failure_= ADIAN_mac_failed_callback;
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
 }

   drop(p, DROP_RTR_NO_ROUTE);
   return;
   }
  else
   ch->next_hop() = next_hop;
  }
  Scheduler::instance().schedule(target_, p, 0.0);  //sending actual packet when no error
 }
}


//Functions below are Receiving Information from Layer-2 Protocols
static void ADIAN_mac_failed_callback(Packet *p, void *arg) {
((ADIAN*)arg)->mac_failed(p);
}
/*mac failed() depends very much on ADIAN specification. As an example, the next piece of code prints a debug
message and drops the packet.*/
 void ADIAN::mac_failed(Packet* p) {
struct hdr_ip* ih= HDR_IP(p);
struct hdr_cmn* ch = HDR_CMN(p);
debug("%f: Node %d MAC layer cannot send a packet to node %d\n",CURRENT_TIME,ra_addr(),ch->next_hop());
drop(p, DROP_RTR_MAC_CALLBACK);
/* ... do something ... */
 }

//code checks if the mobile node itself is a base station; and if not then it is assigned one dynamically.
if (node_->base_stn() == ra_addr()) {
printf("\nI’m a base station");
}
else {
printf("\nI’m not a base station);
node_->set_base_stn(base_stn_addr);
}
