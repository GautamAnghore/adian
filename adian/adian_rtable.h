#ifndef __adian_rtable_h__
#define __adian_rtable_h__

#include <trace.h>
#include <map>			//We use a hash table (map) as the storage structure.
#include<common/ip.h>
#include<common/packet.h>
/*For each entry in routing table one might want to store destination addresses, next hop addresses, distances or cost
associated to the routes, sequence numbers, lifetimes.*/

 typedef std::map<nsaddr_t, nsaddr_t> rtable_t;

 class Adian_rtable {
 	rtable_t rt_;
 public:
  Adian_rtable();
  void print(Trace*);
  void clear();
  void rm_entry(nsaddr_t);
  void add_entry(nsaddr_t, nsaddr_t);
  nsaddr_t lookup(nsaddr_t);
  u_int32_t
 };
 #endif
