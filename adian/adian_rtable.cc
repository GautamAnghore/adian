ADIAN_rtable::ADIAN_rtable() { }

//The print() function will dump the contents of the node’s routing table to the trace file.
 void ADIAN_rtable::print(Trace* out) {
 sprintf(out->pt_->buffer(), "P\tdest\tnext");
 out->pt_->dump();
 for (rtable_t::iterator it = rt_.begin(); it != rt_.end(); it++) 
  {
   sprintf(out->pt_->buffer(), "P\t%d\t%d",(*it).first,(*it).second);
   out->pt_->dump();
  }
 }


//function removes all entries in routing table.
 void ADIAN_rtable::clear() {
  rt_.clear();
 }


//To remove an entry given its destination address we implement the rm entry() function.
 void ADIAN_rtable::rm_entry(nsaddr_t dest) {
 rt_.erase(dest);
 }


//add_entry() is used to add a new entry in the routing table given its destination and next hop addresses.
 void ADIAN_rtable::add_entry(nsaddr_t dest, nsaddr_t next) {
 rt_[dest] = next;
 }


/*definition of lookup().
Lookup() returns the next hop address of an entry given its destination address. If such an entry doesn’t exist, (that is, there is no route for that destination) the function returns IP BROADCAST. Of course we include common/ip.h in order to use this constant.*/
 nsaddr_t ADIAN_rtable::lookup(nsaddr_t dest) {
 rtable_t::iterator it = rt_.find(dest);
 if (it == rt_.end())
 return IP_BROADCAST;
 else
  return (*it).second;
 }


//size() returns the number of entries in the routing table.
 u_int32_t  ADIAN_rtable::size() {
 return rt_.size();
 }
