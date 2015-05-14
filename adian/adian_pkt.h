#ifndef __adian_pkt_h__
#define __adian_pkt_h__

#include <packet.h>
// #include <config.h>	// nsaddr_t, u_int16_t, u_int8_t data types definec
						// not needed to be included (why?)

// Packet Identifiers
#define ADIANTYPE_PING			0x01
#define ADIANTYPE_PING_REPLY	0x02
#define ADIANTYPE_REQ			0x04
#define ADIANTYPE_REQ_REPLY		0x08
#define ADAINTYPE_ERROR			0x10


// Packet accessing Macros
#define HDR_ADIAN(p) 				((struct hdr_adian*)hdr_adian::access(p))
#define HDR_ADIAN_PING(p) 			((struct hdr_adian_ping*)hdr_adian::access(p))
#define HDR_ADIAN_PING_REPLY(p)		((struct hdr_adian_ping_reply*)hdr_adian::access(p))
#define HDR_ADIAN_REQ(p)			((struct hdr_adian_req*)hdr_adian::access(p))
#define HDR_ADIAN_REQ_REPLY(p)		((struct hdr_adian_req_reply*)hdr_adian::access(p))
#define HDR_ADIAN_ERROR(p)			((struct hdr_adian_error*)hdr_adian::access(p))

//general header shared by all headers
struct hdr_adian {
	
	u_int8_t	h_type_;	//Type of header

	//header access functions
	static int offset_;	// required by Packet Header manager

	inline static int& offset() { return offset_; }
	inline static hdr_adian* access(const Packet* p) {
		return (hdr_adian*)p->access(offset_);
	}

};

// ping type header
// used to find the neighbours
struct hdr_adian_ping {

	u_int8_t	p_type_;		// Packet type.
								// when we define p_type_ using hdr_adian_ping and access it using
								// hdr_adian master structure, this p_type_ is read as h_type_ and 
								// in recvADIAN() function we can hence identify the type of packet.
								// When defining we do not need to add hdr_adian type header. Its kind
								// of shared header. This behaviour is obtained because of access function
								// which only see the offset_ in the bit pattern and type cast as our wish.

	u_int32_t	seq_num_;		// request sequence number
								// used to identify the reply

//	can directly use seq_num_
//	inline u_int32_t& seq_num() { return seq_num_; }

	inline int size() {
		int sz = 0;
		sz = 2*sizeof(u_int32_t);
		assert(sz>=0);
		return sz;
	}
};

// ping response type header
// used to reply to requests
struct hdr_adian_ping_reply {

	u_int32_t	seq_num_;
};

// request to send data to destination
// used for finding routes
struct hdr_adian_req {

	u_int32_t	seq_num_;		// request sequence number
	nsaddr_t	daddr_;			// destination address which is being searched
	nsaddr_t	rootaddr_;		// root address from which request is generated
	u_int8_t	hop_count_;		// number of hops the request has already travelled
								// USE: while recieving the request, check if hop_count_
								// is more than limit and drop if exceeds 

	inline int size() {
		int sz = 0;
		sz = 4*sizeof(u_int32_t);
		assert(sz>=0);
		return sz;
	}
};

struct hdr_adian_req_reply {

	u_int32_t	seq_num_;		// sequence number of request which is being replied
	nsaddr_t	daddr_;			// destination address which was being searched
								// NOTE: daddr is the destination address for which request
								// was generated and not the address to which this reply is
								// being sent
	nsaddr_t	rootaddr_;		// root address from which request was generated
	u_int8_t	hop_count_;		// number of hops, used to compare nodes for best path


	inline int size() {
		int sz = 0;
		sz = 4*sizeof(u_int32_t);
		assert(sz>=0);
		return sz;
	}
};

struct hdr_adian_error {

	u_int32_t	seq_num_;		// sequence number, not confirm needed or not
	nsaddr_t	daddr_;			// destination address to which data delivery failed
	
	inline int size() {
		int sz = 0;
		sz = 2*sizeof(u_int32_t);
		assert(sz>=0);
		return sz;
	}
};

// for size calculation of header-space
// use : line12 <adian.cc>
union hdr_all_adian {
	hdr_adian				gh;
	hdr_adian_ping			ping;
	hdr_adian_ping_reply 	pingreply;
	hdr_adian_req 			req;
	hdr_adian_req_reply 	rep;
	hdr_adian_error 		error;
};

#endif
