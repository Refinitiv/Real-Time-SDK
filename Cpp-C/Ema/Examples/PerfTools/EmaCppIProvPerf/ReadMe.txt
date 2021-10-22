EmaCppIProvPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the EMA,
by providing Level I Market Price content either directly to
an OMM consumer or through a Refinitiv Real-Time Distribution System. 

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
total system time (The CPU time is the total across all threads, and as such 
this number can be greater than 100% if multiple threads are busy).  

This application uses Libxml2, an open source  XML parser library.
See the readme in the provided Libxml2 source for more details.

-----------------
Application Name:
-----------------

EmaCppIProvPerf

------------------
Setup Environment:
------------------

The following configuration files are required:
- RDMFieldDictionary and enumtype.def, located in the etc directory.
- 350k.xml, located in PerfTools/Common.
- MsgData.xml, located in PerfTools/Common.
- EmaCppIProvPerf includes the EMA library. For shared builds, the location of that
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

EmaCppIProvPerf
(runs with a default set of parameters. The full set of configurable 
 parameters is printed to the screen. )

- EmaCppIProvPerf -? displays command line options, with a brief description
   of each option.  

- Pressing the CTRL+C buttons terminates the program.  

- Additional Note: Command line also takes "-providerName".   
      Sample Command "-providerName" Explanation: The value of -providerName is 
      used to specify the Provider to use from EmaConfig.xml

  Following is Sample Encrypted Server Config to specify in EmaConfig.xml. 
  Please see EMA Configuration Guide for details on each parameter for optimal tuning.
  See EmaConfig.xml for additional examples of configuration:

    Encrypted Socket or Websocket:
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
        <MaxFragmentSize value="6144"/>
        <NumInputBuffers value="10000"/>
        <SysRecvBufSize value="65535"/>
        <SysSendBufSize value="65535"/>
	<ServerCert value="mycert.crt"/>
	<ServerPrivateKey value="mykey.key"/>
    </Server>

    Sample ServerCert values: "/local/mycert.crt" or "C:\mycert.crt"
    Sample ServerPrivateKey values: "/local/mykey.key" or "C:\mykey.key"

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

EmaCppIProvPerf.cpp - The main file for the EmaCppIProvPerf application.

IProvPerfConfig.cpp - Provides configurable options for the application.

ProviderThread.cpp - Handles consumer connections and sends content.

ProviderPerfClient.cpp - Handles messages from the consumer application.

AppUtil.cpp - Utility for use by applications and/or common classes.

CtrlBreakHandler.cpp  - Provides Contol-C handling.
 
Statistics.cpp - Provides methods for collecting and calculating statistical  information.

PerfConfig.cpp  - Common configurable options across PerfTool applications.

Mutex.cpp   - Provides Mutex handling.

ThreadAffinity.cpp - Used for printout and determination of thread affinity binding.

ThreadBinding.h - Handles Thread binding.

XmlItemParser.cpp - Used for parsing Item file (350k.xml).

xmlPerfMsgDataParser.cpp - Used for parsing Message template file (MsgData.xml).
