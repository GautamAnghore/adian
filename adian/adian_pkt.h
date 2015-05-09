#ifndef __protoname_pkt_h__
#define __protoname_pkt_h__
#include <packet.h>
#define HDR_PROTONAME_PKT(p) hdr_protoname_pkt::access(p)
struct hdr_protoname_pkt {
	nsaddr_t	pkt_src_;
	u_int16_t	pkt_len_;
	u_int8_t	pkt_seq_num_;
	inline 	nsaddr_t& 	pkt_src() { return pkt_src_; }
	inline 	u_int16_t& 	pkt_len() { return pkt_len_; }
	inline	u_int8_t&	pkt_seq_num() { return pkt_seq_num_; }
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static hdr_protoname_pkt* access(const Packet* p) {
		return (hdr_protoname_pkt*)p->access(offset_);
	}

};
#endif