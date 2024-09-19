EMA_NIProvPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the EMA,
by providing Level I Market Price content through a LSEG Real-Time
Distribution System. 

The non-interactive provider creates two types of threads:
- A main thread, which collects and records statistical information;
- Working threads, each of which handle a connection with ADH,
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

EMA_NIProvPerf

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

EMA_NIProvPerf
(runs with a default set of parameters. The full set of configured 
 parameters is printed to the screen. )

- EMA_NIProvPerf -? displays command 
  line options, with a brief description of each option.  

- Pressing the CTRL+C buttons terminates the program. 

- Default Configuration file, EmaConfig.xml, contains several sections for 
  running this performance tool in EmaConfig.xml

  Following is sample encrypted connection channel config to specify in EmaConfig.xml. 
  See EmaConfig.xml for additional examples of configuration:
  Note: Encrypted Websocket is not currently supported
     
    Encrypted Socket:
    <Channel>
        <Name value="Perf_NIP_Channel_Encr"/>
        <ChannelType value="ChannelType::RSSL_ENCRYPTED"/>
        <EncryptedProtocolType value="EncryptedProtocolType::RSSL_SOCKET"/>
        <CompressionType value="CompressionType::None"/>
        <GuaranteedOutputBuffers value="50000"/>
        <NumInputBuffers value="10000"/>
        <ConnectionPingTimeout value="30000"/>
        <TcpNodelay value="0"/>
        <DirectWrite value="1"/>
        <SysRecvBufSize value="65535"/>
        <SysSendBufSize value="65535"/>
        <Host value="adh-host"/>
        <Port value="adh-rssl-server-port"/>
    </Channel>

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

EMANIProvPerf.cs - The main file for the EmajNIProvPerf application.

EMANIProviderPerfConfig.cs - Provides configurable options for the application.

NIProviderThread.cs - Handles consumer connections and providing content.

NIProviderPerfClient.cs - Handles messages from the consumer application.
