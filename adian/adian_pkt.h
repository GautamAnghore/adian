#ifndef __ADIAN_pkt_h__
#define __ADIAN_pkt_h__
#include <packet.h>
#include<config.h>
#define HDR_ADIAN_PKT(p) hdr_ADIAN_pkt::access(p)
struct hdr_ADIAN_pkt {
nsaddr_t pkt_src_;
// Node which originated this packet
u_int16_t pkt_len_;
// Packet length (in bytes)

u_int8_t pkt_seq_num_; // Packet sequence number
inline nsaddr_t& pkt_src()
{ return pkt_src_; }

inline u_int16_t& pkt_len()
{ return pkt_len_; }

inline u_int8_t& pkt_seq_num() { return pkt_seq_num_; }

static int offset_;

inline static int& offset() { return offset_; }

inline static hdr_ADIAN_pkt* access(const Packet* p) {

return (hdr_ADIAN_pkt*)p->access(offset_);

}

 };

 #endif
