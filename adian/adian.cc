//binding our packet to TCL interface
//for that we use offset

int adian_pkt::offset_;

//Our header class inherits Packet's header class
static class AdianHeaderClass : public PacketHeaderClass {

public:
	//constructor
	//we are passing values to the parent class constructor 
	AdianHeaderClass() : PacketHeaderClass("PacketHeader/Adian", sizeof(hdr_adian_pkt)) {
		//Q: don't know what this bind_offset does
		bind_offset(&hdr_adian_pkt::offset_);
	}

}class_rtProtoAdian_hdr;
