ADIAN
=====
__A Distributed Intelligent Ad-hoc Network__

 ADIAN is a new routing protocol for Mobile Ad-hoc Networks. A Mobile Ad-hoc Network (MANET) is a network consisting of wireless devices that makes a self-configured network together.

 In ADIAN protocol, network nodes are considered as intelligent agents and the agents discover routes to deliver information. In this system, the routing overhead, which has an important impact on the performance of the MANETs, is aimed to be minimized.

 __Note:__ This repository contains the implementation of ADIAN protocol in Network Simulator ([NS2](http://nsnam.isi.edu/nsnam/index.php/User_Information)). Using this implementation in NS2, the actual performance of ADIAN can be studied and analysed.

####Implementation :

 Resource Reference : [Implementing new MANET protocol](http://www.cs.mun.ca/~yzchen/papers/papers/testing/nsrt-howto.pdf) 

 NS2 is implemented in c++ and tcl scripts can be used to simulate various networks. Hence ADIAN is implemented in c++. To integrate it in ns2, some files of ns2-codebase needs to be updated. Those files are in `ns-allinone-2.35/ns-2.35` directory.

##Setting up Local development enviornment/Testing

__Notes :__ 

 + Use a separate installation of ns2 for development/testing purpose. Some of the source files of ns2 are also modified and can lead to inconsistencies with your current ns2 installation.
 + Configurations steps are in accordance with _ubuntu 14.04_.
 + Implementation is based on `ns-allionone-2.35`.

####Instructions:

 + __Install dependencies :__ To compile ns2, these dependencies must be installed.
   <br/>
   ```
   sudo apt-get install tcl8.5-dev tk8.5-dev gcc-4.4 g++-4.4 build-essential autoconf automake perl xgraph libxt-dev libx11-dev libxmu-dev
   ```
 
 + __Get `ns-allinone-2.35`:__
   
   - Download `ns-allinone-2.35.tar.gz` file from [here](http://garr.dl.sourceforge.net/project/nsnam/allinone/ns-allinone-2.35/ns-allinone-2.35.tar.gz) .
   - Extract `ns-allinone-2.35.tar.gz`.
     <br/>`tar -xzvf ns-allinone-2.35.tar.gz` 
   - The path where you extracted the `ns-allionone-2.35.tar.gz` will be refered to as `<path/to/local/ns-allinone>`

 + __Clone the repo:__
   <br/>
   ```
   git clone https://github.com/GautamAnghore/adian adian-master
   ```
   The repository will be cloned to `adian-master` directory.

 + __Configure:__

   - `cd adian-master`
   - `./configure <path/to/local/ns-allinone>`
     <br/>( This will create hard links of the files in the repository to the `ns-allinone-2.35` directory. Now the changes in the files in repository will be reflected in ns2 source-code directory )

 + __Compile:__

   - `cd <path/to/local/ns-allinone>`
   - `./install`

 + __Test:__
   <br/>To run the simulation through this NS installation
   <br/>
   `<path/to/local/ns-allinone>/ns-2.35/ns <simulation.tcl>`

