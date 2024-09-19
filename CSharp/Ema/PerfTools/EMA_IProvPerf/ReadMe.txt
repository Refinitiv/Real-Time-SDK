EMA_IProvPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of EMA API,
by providing Level I Market Price content either directly to
an OMM consumer or through the LSEG Real-Time Distribution System. 

The interactive provider creates two types of threads:
- A main thread, which collects and records statistical information,
- Working threads, each of which handle a connection with a consumer,
provide refreshes and updates of market data.

To measure latency, a timestamp is randomly placed in each burst of updates by 
the provider. A performance consumer can, then, decode the timestamp from the update to
determine the end-to-end latency.

This application also measures memory and CPU usage. The memory usage measured 
is the 'resident set,' or the memory currently in physical use by the 
application. The CPU usage is the total time using the CPU divided by the 
total system time. That is, the CPU time is the total across all threads, and 
as such this number can be greater than 100% if multiple threads are busy.


-----------------
Application Name:
-----------------

EMA_IProvPerf

------------------
Setup Environment:
------------------

The following files are required:
- RDMFieldDictionary and enumtype.def in CSharp/etc/
- 350k.xml in CSharp/Ema/PerfTools/EmaPerfToolsCommon/ 
- MsgData.xml in CSharp/Ema/PerfTools/EmaPerfToolsCommon/
- EmaConfig.xml in CSharp/Ema/

-----------------
Compiling Source:
-----------------

To build and run performance tool. See installation 
guide for build instructions and command line usage below for run instructions.

-------------------
Command line usage:
-------------------  

EMA_IProvPerf
(runs with a default set of parameters. The full set of configured 
 parameters is printed to the screen. )

- EMA_IProvPerf -? displays command 
  line options, with a brief description of each option.  

- Pressing the CTRL+C buttons terminates the program. 

- Default Configuration file, EmaConfig.xml, contains several sections for 
  running this performance tool. See section, "Performance tools servers" 
  in EmaConfig.xml

  Following is Sample Encrypted Server Config to specify in EmaConfig.xml. 
  Please see EMA Configuration Guide for details on each parameter for optimal tuning.
  See EmaConfig.xml for additional examples of configuration:
     
    Encrypted Socket:
    <Server>
        <Name value="Perf_Server"/>
        <ServerType value="ServerType::RSSL_ENCRYPTED"/>
        <CompressionType value="CompressionType::None"/>
        <GuaranteedOutputBuffers value="5000"/>
        <ConnectionPingTimeout value="30000"/>
        <TcpNodelay value="1"/>
        <Port value="14002"/>
        <HighWaterMark value="6144"/>
        <InterfaceName value=""/>
        <DirectWrite value="1" />
        <MaxFragmentSize value="6144"/>
        <NumInputBuffers value="10000"/>
        <SysRecvBufSize value="65535"/>
        <SysSendBufSize value="65535"/>
    </Server>
 
----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

EmaIProvPerf.cs - The main file for the EmajProvPerf application.

EmaIProvPerfConfig.cs - Provides configurable options for the application.

IProviderThread.cs - Handles consumer connections and providing content.

ProviderPerfClient.cs - Handles messages from the consumer application.
