#ifndef __adian_pkt_h__
#define __adian_pkt_h__
#include <packet.h>
#define HDR_ADIAN_PKT(p) hdr_adian_pkt::access(p)
struct hdr_adian_pkt {
	nsaddr_t	pkt_src_;
	u_int16_t	pkt_len_;
	u_int8_t	pkt_seq_num_;
	inline 	nsaddr_t& 	pkt_src() { return pkt_src_; }
	inline 	u_int16_t& 	pkt_len() { return pkt_len_; }
	inline	u_int8_t&	pkt_seq_num() { return pkt_seq_num_; }
	static int offset_;
	inline static int& offset() { return offset_; }
	inline static hdr_adian_pkt* access(const Packet* p) {
		return (hdr_adian_pkt*)p->access(offset_);
	}

};
#endif