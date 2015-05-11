#include "adian_rtable.h"

//--------------- routing table functions ---------------------

// constructor
Adian_rtable::Adian_rtable() { }

//The print() function will dump the contents of the node’s routing table to the trace file.
void Adian_rtable::print(Trace* out) {
	
	sprintf(out->pt_->buffer(), "A\tdest\tnext_hop\tnn");
	out->pt_->dump();
	
	// Iterate over through the _nexthop_ map 
	rtable_nexthop_t::iterator it_hop;
	
	for (it_hop = rt_nexthop_.begin(),; it_hop != rt_nexthop_.end(); it_hop++) {
  		
  		// foreach destination -> (*it_hop).first, find the corrosponding number of nodes
  		rtable_nn_t::iterator it_nn = rt_nn_.find((*it_hop).first);
  		if (it_nn == rt_nn_.end()) {
  			// number of nodes entry not exists for destination node
  			// that means inconsistent data
  			sprintf(stderr, "Inconsistent Data, number of nodes entry not exists for %d node\n", (*it_hop).first);
  		}
  		else {
  			//found all data, dump it
   			sprintf(out->pt_->buffer(), "A\t%d\t%d\t%d",(*it_hop).first,(*it_hop).second,(*it_nn).second);
   			out->pt_->dump();
  		}
  	}
 }

//function removes all entries in routing table
void Adian_rtable::clear() {
 	rt_nexthop_.clear();
 	rt_nn_.clear();
}


//function removes entry given the destination address
void Adian_rtable::rm_entry(nsaddr_t dest) {
	rt_nexthop_.erase(dest);
	rt_nn_.erase(dest);
}


//function adds a new entry in the routing table given its destination,next hop address and nn
void Adian_rtable::add_entry(nsaddr_t dest, nsaddr_t next, u_int8_t nn) {
	rt_nexthop_[dest] = next;
	rt_nn_[dest] = nn;
}

//function updates the next_hop and number of nodes corrosponding to dest entry
void Adian_rtable::update_entry(nsaddr_t dest, nsaddr_t nexthop, u_int8_t nn) {
	//update operation
	rt_nexthop_[dest] = nexthop;
	rt_nn_[dest] = nn;
}

//function returns rtable_t object containing the next hop address and number of nodes
//if there exists an entry for destination in the routing table
rtable_entry Adian_rtable::lookup(nsaddr_t dest) {

	// to return the result
	rtable_entry result;
	result.daddr = dest;

	rtable_nexthop_t::iterator it_hop = rt_nexthop_.find(dest);
	if (it_hop == rt_nexthop_.end()) {
		result.next_hop = IP_BROADCAST;
		result.nn = 0;
 		return result;
	}
 	else {
  		rtable_nn_t::iterator it_nn = rt_nn_.find(dest);
  		if(it_nn == rt_nn_.end()) {
  			result.next_hop = IP_BROADCAST;
			result.nn = 0;
	 		return result; 
  		}
  		else {
  			result.next_hop = (*it_hop).second;
  			result.nn = (*it_nn).second;
  		}
 	}
 }
