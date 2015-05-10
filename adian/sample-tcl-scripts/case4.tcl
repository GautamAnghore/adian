set val(chan) Channel/WirelessChannel
set val(prop) Propagation/TwoRayGround
set val(netif) Phy/WirelessPhy
set val(mac) Mac/802_11
set val(ifq) Queue/DropTail/PriQueue
set val(ll) LL
set val(ant) Antenna/OmniAntenna
set val(ifqlen) 50
set val(nn) 5
set val(rp) AODV
set ns [new Simulator]

$ns use-newtrace
set tf [open simple4.tr w]
$ns trace-all $tf

set tf1 [open adHocNam1.nam w]
$ns namtrace-all-wireless $tf1 300 300

set topo [new Topography]
$topo load_flatgrid 700 700

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
-macTrace ON \
-movementTrace ON

set node0 [$ns node]
set node1 [$ns node]
set node2 [$ns node]
set node3 [$ns node]
set node4 [$ns node]


$node0 set X_ 50.0
$node0 set Y_ 50.0
$node0 set Z_ 0.0

$node1 set X_ 120.0
$node1 set Y_ 150.0
$node1 set Z_ 0.0

$node2 set X_ 220.0
$node2 set Y_ 110.0
$node2 set Z_ 0.0

$node3 set X_ 340.0
$node3 set Y_ 30.0
$node3 set Z_ 0.0

$node4 set X_ 420.0
$node4 set Y_ 120.0
$node4 set Z_ 0.0


$ns initial_node_pos $node0 10
$ns initial_node_pos $node1 10
$ns initial_node_pos $node2 10
$ns initial_node_pos $node3 10
$ns initial_node_pos $node4 10

Agent/TCP set packetSize_ 4000


set tcp1 [new Agent/TCP]
$ns attach-agent $node0 $tcp1

set ftp [new Application/FTP]
$ftp attach-agent $tcp1
$ftp set type_ FTP

set sink1 [new Agent/TCPSink]
$ns attach-agent $node4 $sink1

$ns connect $tcp1 $sink1

$ns at 0 "$node0 label ftp-source"
$ns at 0 "$node0 add-mark mark0 red circle"
$ns at 0 "$node2 label ftp-sink"
$ns at 0 "$node2 add-mark mark1 blue circle"
$ns at 0 "$node1 label intermediate-1"
$ns at 0 "$node1 add-mark mark2 green circle"
$ns at 0 "$node3 label intermediate-2"
$ns at 0 "$node3 add-mark mark2 green circle"
$ns at 0 "$node4 label intermediate-3"
$ns at 0 "$node4 add-mark mark2 green circle"

$ns at 3.0 "$node2 setdest 180.0 90.0 10.0"
$ns at 7.0 "$node4 setdest 370.0 110.0 10.0"

$ns at 0.0 "$ftp start"
$ns at 10.0 "$ftp stop"

$ns at 10.0 "finish"

proc finish {} {
global ns tf tf1
$ns flush-trace
close $tf
exit 0
}

$ns run

