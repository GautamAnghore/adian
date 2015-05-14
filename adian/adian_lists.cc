#include "adian_list.h"

#define CURRENT_TIME Scheduler::instance().clock()

//---------------------------Failed Path List Implementation------------------------//

Adian_Failed_Path_list::Adian_Failed_Path_list() {}

void Adian_Failed_Path_list::add_new_packet(int uid, failed_path fp, double expire) {
	// It is already confirmed that entry for uid do not exit in the map
	
	// fp is not in list form
	pl_t newlist;
	newlist.push_back(fp);
	
	// insert path
	fl_[uid] = newlist;

	// insert expiry time
	el_[uid] = expire;
}

void Adian_Failed_Path_list::add_failed_path(int uid, nsaddr_t next_hop, nsaddr_t daddr) {

	// convert path to structure
	failed_path newpath;
	newpath.next_hop = next_hop;
	newpath.daddr = daddr;

	// First check if entry exists for uid
	fpl_t::iterator it = fl_.find(uid);

	if(it == fl_.end()) {
		// Entry does not exists
		add_new_packet(uid, newpath, CURRENT_TIME+FAILED_PATH_EXPIRE_TIME);
	}
	else {
		// Entry corrosponding to uid exists
		// push the path to the list of failed paths
		fl_[uid].push_back(newpath);
		// recently accessed, so add expiry time
		add_expire_time(uid, FAILED_PATH_EXPIRE_TIME);
	}
}

void Adian_Failed_Path_list::rm_entry(int uid) {
	// add check for entry exists or not
	fl_.erase(uid);
	el_.erase(uid);
}

int Adian_Failed_Path_list::check_failed_path(int uid, nsaddr_t next_hop, nsaddr_t daddr) {

	// first check if entry exists
	fpl_t::iterator it = fl_.find(uid);

	if(it == fl_.end()) {
		// entry does not exists
		return 0;	// false - entry do not exist
	}
	else {
		// entry for uid exists
		// now check if next_hop and daddr exists in the list
		pl_t::iterator list_it;
		for(list_it = fl_[uid].begin(); list_it != fl_[uid].end(); list_it++ ) {
			// iterate over whole list and check values
			if ((*list_it).next_hop == next_hop && (*list_it).daddr == daddr) {
				// path found
				return 1;
			}
		}
		return 0;
	}
}

void Adian_Failed_Path_list::add_expire_time(int uid, double extra_time) {
	//find the entry
	uid_expire_t::iterator it = el_.find(uid);
	if(it == el_.end()) {
		// uid not in list
		return;
	}
	else {
		(*it).second += extra_time;
	}
}

/*
double Adian_Failed_Path_list::expire_time(int uid) {
	// check if exists
	uid_expire_t::iterator it = el_.find(uid);

	if(it == el_.end()) {
		// uid not in list
		return 0.0;
	}
	else {
		return (*it).second;
	}
}
*/

void Adian_Failed_Path_list::purge() {
// remove the expired entries

	//loop through the map and check for expired entries
	fpl_t::iterator it;
	uid_expire_t::iterator it_expire;
	
	for(it = fl_.begin(); it != fl_.end();) {
		
		it_expire = el_.find(it->first);
		if(it_expire == el_.end()) {
		// if the entry in expire list for this uid do not exist, invalid entry, so remove
			
			// This may cause error because of version of c++ being used
			// Following code works for c++11
			// http://stackoverflow.com/questions/263945/what-happens-if-you-call-erase-on-a-map-element-while-iterating-from-begin-to
			
			it = fl_.erase(it);		// to make iterator consistent `it =`
			// fl_.erase(it++);		// for c++03

		}
		else if(it_expire->second <= CURRENT_TIME) {
		// or if entry exists and it has expired, then also remove	
			it = fl_.erase(it);
			it_expire = el_.erase(it_expire);
		}
		else {
			it++;
		}	
	}

}



//-------------------------------Reply route list implementation ------------------------

//constructor
Adian_Reply_Route_list::Adian_Reply_Route_list(){ }

//to add a new reply route to the list
void Adian_Reply_Route_list::add_reply_route(u_int32_t seq_no, nsaddr_t reply_to, double expire_time){
	rl_[seq_no] = reply_to; //adds reply_to in route list corresponding to the seq_no
	el_[seq_no] = expire_time; //adds expire_time in expire list corresponding to the seq_no
}

void Adian_Reply_Route_list::rm_reply_route(u_int32_t seq_no){
	seq_expire_t::iterator el_it;
	route_list_t::iterator rl_it;

//find the expiry time and reply to nodde corresponding to that seq_no and remove it
	el_it = el_.find(seq_no);
	rl_it = rl_.find(seq_no);

	el_.erase(el_it);
	rl_.erase(rl_it);
}

//return return to node with this seq_no
nsaddr_t Adian_Reply_Route_list::lookup(u_int32_t seq_no){
	route_list_t::iterator rl_it;

	rl_it = rl_.find(seq_no);
	return rl_it->second;
}

//remove already expired enteries
void Adian_Reply_Route_list::purge(){
	seq_expire_t::iterator el_it;
	route_list_t::iterator rl_it;

	for (rl_it = rl_.begin(); rl_it !=rl_.end();)
	{
		el_it = el_.find(rl_it->first) //if entry is not in expire list it will delete it from router list also.
		if(el_it = el_.end()){
			rl_it = rl_.erase(rl_it);
		}
		else if(el_it->second <= CURRENT_TIME){ //if expire time is les than current time it will delete that entry from both
			rl_it = rl_.erase(rl_it);
			el_it = el_.erase(el_it);
		}
		else{
			rl_it++;
		}
	}
}