int protoname_pkt::offset_;

static class ProtonameHeaderClass : public PacketHeaderClass {
	public:
	ProtonameHeaderClass() : PacketHeaderClass("PacketHeader/Adian",
	sizeof(hdr_protoname_pkt)) {
		bind_offset(&hdr_protoname_pkt::offset_);
	}
} class_rtProtoProtoname_hdr;