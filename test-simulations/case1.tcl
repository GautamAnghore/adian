#simulation consisting of two nodes A and B

set val(chan)           Channel/WirelessChannel    ;# channel type
set val(prop)           Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)          Phy/WirelessPhy            ;# network interface type
set val(mac)            Mac/802_11                 ;# MAC type
set val(ifq)            Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)             LL                         ;# link layer type
set val(ant)            Antenna/OmniAntenna        ;# antenna model
set val(ifqlen)         50                         ;# max packet in ifq
set val(nn)             2                          ;# number of mobilenodes
set val(rp)             ADIAN                       ;# routing protocol


set ns		[new Simulator]
set tracefd     [open simple1.tr w]
$ns trace-all $tracefd

set namfd [open samplenam.nam w]
$ns namtrace-all-wireless $namfd 300 300 

# set up topography object
set topo       [new Topography]

$topo load_flatgrid 500 500
create-god $val(nn)

$ns node-config -adhocRouting $val(rp) \
			 -llType $val(ll) \
			 -macType $val(mac) \
			 -ifqType $val(ifq) \
			 -ifqLen $val(ifqlen) \
			 -antType $val(ant) \
			 -propType $val(prop) \
			 -phyType $val(netif) \
			 -channelType $val(chan) \
			 -topoInstance $topo \
			 -agentTrace ON \
			 -routerTrace ON \
			 -macTrace OFF \
			 -movementTrace ON	


for {set i 0} {$i < $val(nn) } {incr i} {
		set node($i) [$ns node]	
		$node($i) random-motion 0		;# disable random motion
	}


$node(0) set X_ 5.0
$node(0) set Y_ 2.0
$node(0) set Z_ 0.0

$node(1) set X_ 150.0
$node(1) set Y_ 125.0
$node(1) set Z_ 0.0

$ns at 50.0 "$node(1) setdest 25.0 20.0 15.0"
$ns at 10.0 "$node(0) setdest 20.0 18.0 1.0"

$ns at 100.0 "$node(1) setdest 490.0 480.0 15.0" 


set tcp [new Agent/TCP]
$tcp set class_ 2
set sink [new Agent/TCPSink]
$ns attach-agent $node(0) $tcp
$ns attach-agent $node(1) $sink
$ns connect $tcp $sink
set ftp [new Application/FTP]
$ftp attach-agent $tcp
$ns at 10.0 "$ftp start" 

for {set i 0} {$i < $val(nn) } {incr i} {
    $ns at 150.0 "$node($i) reset";
}
$ns at 150.0 "stop"
$ns at 150.01 "puts \"NS EXITING...\" ; $ns halt"
proc stop {} {
    global ns tracefd
    $ns flush-trace
    close $tracefd
    exit 0
}

puts "Starting Simulation..."
$ns run

