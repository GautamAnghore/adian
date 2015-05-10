#ifndef __adian_rtable_h__
#define __adian_rtable_h__

#include <trace.h>
#include <map>		// for hash table
#include <list>		// for double linked list


//routing table structure
typedef struct {

	//routing table is implemented using two maps
	//routing table contains only one route for every destination
	//best route for every destination is stored
// |--------------------------------------------------|
// | destination_address | next_hop | number of nodes |
// |--------------------------------------------------|

	nsaddr_t	daddr;		//destination address
	nsaddr_t	next_hop;	//next hop address
	u_int8_t	nn;			//number of nodes

} rtable_entry;

//routing table implementation data types
// < key: destination_address, value: next_hop >
typedef std::map<nsaddr_t, nsaddr_t> rtable_nexthop_t;
// < key: destination_address, value: number of nodes >
typedef std::map<nsaddr_t, u_int8_t> rtable_nn_t;


// neighbourhood table
// implemented using double linked list
typedef std::list<nsaddr_t> nbtable_t;


// belief table structure
typedef struct {
	
	// belief table is implemented using double linked list
	// belief table contains the belief on the next_hop for sending the data
	// belief table comes into play when the existing route fails in sending the data
// |-----------------------------------------------------------------------------------------------------------|
// | next_hop | destination_address | number of nodes | total_transactions | successful_transactions | belief  |	
// |-----------------------------------------------------------------------------------------------------------|

	nsaddr_t	next_hop;	//next hop address
	nsaddr_t	daddr;		//destination address
	u_int8_t	nn;			//number of nodes
	u_int32_t	total;		//total number of requests for sending data to daddr
	u_int32_t	success;	//successful transections
	float		belief;		//belief degree

} btable_entry;

// belief table data type
typedef std::list<belief_entry> btable_t;

// ADIAN routing table
class Adian_rtable {

	//destination address will be the key for both maps
	rtable_nexthop_t	rt_nexthop_;
	rtable_nn_t 		rt_nn_;

public:

	Adian_rtable();
	void print(Trace*);								//print the routing table
	void clear();									//clear the table
	void rm_entry(nsaddr_t);						//remove entry(destination address)
	void add_entry(nsaddr_t, nsaddr_t, u_int8_t);	//add entry(destination address, next hop, numberofnodes)
	void update_entry(nsaddr_t, nsaddr_t, u_int8_t); // update entry corrosponding to destination address
	rtable_t lookup(nsaddr_t);						//find entry(destination address)

};

// ADIAN neighbourhood table
class Adian_nbtable {

	nbtable_t nb_;

public:

	Adian_nbtable();
	void print(Trace*);			//print the neighbourhood table
	void clear();				//clear table
	void rm_entry(nsaddr_t);	//remove neighbour
	void add_entry(nsaddr_t);	//add neighbour
	nsaddr_t* get_neighbours(); //returns all neighbour addresses

};

// ADIAN belief table
class Adian_btable {

	btable_t bt_;

public:

	Adian_btable();
	void print(Trace*);								//print the table
	void clear();
	void rm_entry(nsaddr_t, nsaddr_t);				//remove entry( next_hop, destination_address)
	void add_entry(nsaddr_t, nsaddr_t, u_int8_t);	//add entry(next hop,destination_address, number of nodes)
													//initially, when the entry is being added
													//total number of trasactions = 0, success= 0, belief = 100.00
	void add_success(nsaddr_t, nsaddr_t);			//add a successful transaction to this path
	void add_failure(nsaddr_t, nsaddr_t);			//add a failed transaction to this path
	btable_t lookup(nsaddr_t, nsaddr_t);			//look for a path to daddr using next_hop

};


#endif
