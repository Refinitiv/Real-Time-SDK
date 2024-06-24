
EmaCppNIProvPerf Application Description

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
application.  The CPU usage is the total time using the CPU divided by the 
total system time (The CPU time is the total across all threads, and as such 
this number can be greater than 100% if multiple threads are busy).  

This application uses Libxml2, an open source  XML parser library.
See the readme in the provided Libxml2 source for more details.

-----------------
Application Name:
-----------------

EmaCppNIProvPerf

------------------
Setup Environment:
------------------

The following configuration files are required:
- RDMFieldDictionary and enumtype.def, located in the etc directory.
- 350k.xml, located in PerfTools/Common.
- MsgData.xml, located in PerfTools/Common.
- EmaCppNIProvPerf includes the EMA library. For shared builds, the location of that
  library must be included in the LD_LIBRARY_PATH. The library (libema.so) will be found in
  Ema/Libs/<env>/Optimized/Shared or Ema/Libs/<env>/Debug/Shared
  where <env> is your build directory (i.e., OL7_64_GCC482).
- EmaConfig.xml, located in the Ema directory.

-----------------
Compiling Source:
-----------------

Windows: Run CMake to generate solution files and build.
Linux: Run CMake to build.
See installation guide for instructions on how to build using CMake

-------------------
Command line usage:
-------------------  

EmaCppNIProvPerf
(runs with a default set of parameters. The full set of configurable 
 parameters is printed to the screen.)

- EmaCppNIProvPerf -? displays command line options, with a brief description
   of each option.  

- Pressing the CTRL+C buttons terminates the program.  

- Default Configuration file, EmaConfig.xml, contains several sections for 
  running this performance tool in EmaConfig.xml

- Additional Note: Command line also takes "-providerName".   
      Sample Command "-providerName" Explanation: The value of -providerName is 
      used to specify the NIProvider to use from EmaConfig.xml

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
        <TcpNodelay value="1"/>
        <SysRecvBufSize value="65535"/>
        <SysSendBufSize value="65535"/>
        <Host value="adh-host"/>
        <Port value="adh-rssl-server-port"/>
        <OpenSSLCAStore value="myCA.pem"/>
    </Channel>

    Sample OpenSSLCAStore values: "/local/myCA.pem" or "C:\myCA.pem"

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

EmaCppNIProvPerf.cpp - The main file for the EmaCppNIProvPerf application.

NIProvPerfConfig.cpp - Provides configurable options for the application.

NIProviderThread.cpp - Sends content.

NIProviderPerfClient.cpp - Handles messages from the consumer application.

AppUtil.cpp - Utility for use by applications and/or common classes.

CtrlBreakHandler.cpp  - Provides Contol-C handling.
 
Statistics.cpp - Provides methods for collecting and calculating statistical  information.

PerfConfig.cpp  - Common configurable options across PerfTool applications.

PerfMessageData.cpp - Provides the logic that fill up Refresh/Update messages by templates.

PerfUtils.cpp - Initializes a list of items that provider will publish.

Mutex.cpp   - Provides Mutex handling.

ThreadAffinity.cpp - Used for printout and determination of thread affinity binding.

ThreadBinding.h - Handles Thread binding.

XmlItemParser.cpp - Used for parsing Item file (350k.xml).

xmlPerfMsgDataParser.cpp - Used for parsing Message template file (MsgData.xml).
