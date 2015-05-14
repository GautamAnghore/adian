#include "adian_rtable.h"
#include <vector>
#include "adian_lists.h"

//--------------- routing table functions ---------------------

// constructor
Adian_rtable::Adian_rtable() { }

//The print() function will dump the contents of the node’s routing table to the trace file.
void Adian_rtable::print(Trace* out) {
	
	sprintf(out->pt_->buffer(), "A\tdest\tnext_hop\tnn");
	out->pt_->dump();
	
	// Iterate over through the _nexthop_ map 
	rtable_nexthop_t::iterator it_hop;
	
	for (it_hop = rt_nexthop_.begin(); it_hop != rt_nexthop_.end(); it_hop++) {
  		
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
 		return result;	// not found
	}
 	else {
  		rtable_nn_t::iterator it_nn = rt_nn_.find(dest);
  		if(it_nn == rt_nn_.end()) {
  			result.next_hop = IP_BROADCAST;
			result.nn = 0;
	 		return result; // not found
  		}
  		else {
  			result.next_hop = (*it_hop).second;
  			result.nn = (*it_nn).second;
  			return result;	// found
  		}
 	}
 }


//------------------------neighbourhood table functions------------------------------

 //constructor
Adian_nbtable::Adian_nbtable(){ }

//print function will dump all enteries of  its neighbouring table to trace file
void Adian_nbtable::print(Trace* out) {
	sprintf(out->pt_->buffer(),"a\tnode\n");
	out->pt_->dump();

	//Iterate over through the _nb_ list
	nbtable_t::iterator it_nb;

	for(it_nb = nb_.begin(); it_nb != nb_.end(); it_nb++ ) {
		sprintf(out->pt_->buffer(), "A\t%d",*it_nb);
   		out->pt_->dump();
	}
}

// remove all the enteries in neighbourhood table
void Adian_nbtable::clear(){
	nb_.clear();
}

// remove a particular entry in neighbourhood table
void Adian_nbtable::rm_entry(nsaddr_t neighbour){
	nb_.erase(neighbour);
}

// add a new node to the neighbourhood 
void Adian_nbtable::add_entry(nsaddr_t neighbour){
	nb_.push_back(neighbour);// adds a entry from back of the list
}

//to get all the neighbour nodes from neighbourhood table
neighbour_entry Adian_nbtable::get_neighbours(){
	neighbour_entry neighbours_{std::begin(nb_),std::end(nb_)};//copies all the list to vector "neighbours_"
	return neighbours_;
}

int Adian_nbtable::lookup(nsaddr_t daddr) {
	nbtable_t::iterator it = nb_.begin();
	while(it != nb_.end())
	{
		if((*it) == daddr) {
			return 1;
		}
		it++;
	}

	return 0;
}

//vectors elements can be accesed directly with their index. (std::vector v = get_neighbours();)



//-------------------------------Belief Table functions----------------------------------

//constructor
Adian_btable::Adian_btable(){ }


//to print all content of belief table in trace file
void Adian_btable::print(Trace* out) {
	
	sprintf(out->pt_->buffer(), "A\tnext_hop\tdest\tnn\ttotal_trans\tsuccessful_trans\tbelief");
	out->pt_->dump();
	
	// Iterate over through the  map 
	btable_t::iterator it_bt;
	
	for (it_bt = bt_.begin(); it_bt != bt_.end(); it_bt++) {
   			sprintf(out->pt_->buffer(), "A\t%d\t%d\t%d\t%d\t%d\t%f",it_bt->next_hop,it_bt->daddr,it_bt->nn,it_bt->total,it_bt->success,it_bt->belief);
   			out->pt_->dump();
  	}
 }

 //clear complete belief table
 void Adian_btable::clear(){
	bt_.clear();
}

//to remove a particular entry
void Adian_btable::rm_entry(nsaddr_t next_hop, nsaddr_t dest_addr){
	btable_t::iterator it_bt;

	for(it_bt = bt_.begin(); it_bt != bt_.end();it_bt++) {
		if((it_bt->next_hop == next_hop)&&(it_bt->daddr == dest_addr)){
			it_bt = bt_.erase(it_bt);
		}
	}
}

//to add a new entry for a particular route
void Adian_btable::add_entry(nsaddr_t next_hop, nsaddr_t dest_addr,u_int8_t nn){
	//TODO : Check if entry already exists and add only if not
	btable_entry new_entry; //new struct btable_entry type object containing all values of new node.
	new_entry.next_hop = next_hop;
	new_entry.daddr = dest_addr;
	new_entry.nn = nn;
	new_entry.total = 0;
	new_entry.success = 0;
	new_entry.belief = 100.00;
	bt_.push_back(new_entry);//add that entry to the belief table in the end
}

//to add a success transaction to that path
void Adian_btable::add_success(nsaddr_t next_hop, nsaddr_t dest_addr) {
	btable_t::iterator it_bt;

	for(it_bt = bt_.begin(); it_bt != bt_.end();it_bt++) {
		if((it_bt->next_hop == next_hop)&&(it_bt->daddr == dest_addr)){
			it_bt->total+=1;
			it_bt->success+=1;
			it_bt->belief = ((float)(it_bt->success)/(it_bt->total))*100;
		}
	}
}

//to add a failure transaction to that path
void Adian_btable::add_failure(nsaddr_t next_hop, nsaddr_t dest_addr) {
	btable_t::iterator it_bt;

	for(it_bt = bt_.begin(); it_bt != bt_.end();it_bt++) {
		if((it_bt->next_hop == next_hop)&&(it_bt->daddr == dest_addr)){
			it_bt->total+=1;
			it_bt->belief = ((float)(it_bt->success)/(it_bt->total))*100;
		}
	}
}

//to select other paths if one path fails
btable_entry Adian_btable::get_path(nsaddr_t dest){
	float min_belief = 12.50;//initial minimum belief is set to 0; to compare
	btable_entry next_suitable_path = {0}; //another btable_entry type entry to return a complete next suitable path to follow if one fails
	btable_t::iterator it_bt;

	for(it_bt = bt_.begin(); it_bt != bt_.end();) {
		if(it_bt->belief <= min_belief){
			it_bt = bt_.erase(it_bt);
		}

		else if((it_bt->daddr == dest)&&(it_bt->belief > min_belief)) {
			if(!(Adian_Failed_Path_list::check_failed_path(uid, it_bt->next_hop, dest)) { //checks if that node does not exists in failure list
				min_belief = it_bt->belief;
				//next suitable path values will be equal to the entry values currently pointed by iterator
				next_suitable_path.next_hop = it_bt->next_hop;
				next_suitable_path.daddr = it_bt->daddr;
				next_suitable_path.nn = it_bt->nn;
				next_suitable_path.total = it_bt->total;
				next_suitable_path.success = it_bt->success;
				next_suitable_path.belief = it_bt->belief; 
			}
		}

		else{
			it_bt++;
		}
	}
	//if some other possible nod is found it will return next_suitable_path structure of btable_entry type except return null;
	if(next_suitable_path.next_hop == NULL){
		return NULL;
	}

	else {
		return next_suitable_path;
	}

}