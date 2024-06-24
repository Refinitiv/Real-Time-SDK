EmaCppConsPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the EMA,
in consuming Level I Market Price content directly from an
OMM provider or through a LSEG Real-Time Distribution System. 

The consumer creates two types of threads:
- A main thread, which collects and records statistical information
- Consumer threads, each of which create a connection to a provider and
request market data.

To measure latency, a timestamp is randomly placed in each burst of updates by 
the provider. The consumer then decodes the timestamp from the update to
determine the end-to-end latency. 

This application also measures memory and CPU usage. The memory usage measured 
is the 'resident set,' or the memory currently in physical use by the 
application. The CPU usage is the total time using the CPU divided by the 
total system time. That is, the CPU time is the total across all threads, and 
as such this number can be greater than 100% if multiple threads are busy.

For more detailed information on the performance applications, see the 
Performance Tools Guide.

This application uses Libxml2, an open source  XML parser library.
See the readme in the provided Libxml2 source for more details.

-----------------
Application Name:
-----------------

EmaCppConsPerf

------------------
Setup Environment:
------------------

The following files are required:
- RDMFieldDictionary and enumtype.def in Cpp-C/etc/
- 350k.xml in PerfTools/Common
- EmaConfig.xml in Cpp-C/Ema/ 
- EmaCppConsPerf includes the EMA library. For shared builds, the location of that
  library must be included in the LD_LIBRARY_PATH. The library (libema.so) will be found in
  Ema/Libs/<env>/Optimized/Shared or Ema/Libs/<env>/Debug/Shared
  where <env> is your build directory (i.e., OL7_64_GCC482)

-----------------
Compiling Source:
-----------------

Windows: Run CMake to generate solution files and build.
Linux: Run CMake to build. 
See installation guide for instructions on how to build using CMake

-------------------
Command line usage:
-------------------  

EmaCppConsPerf
(runs with a default set of parameters. The full set of configurable 
 parameters is printed to the screen. )

- EmaCppConsPerf -? displays command line options, with a brief description
   of each option.  

- Pressing the CTRL+C buttons terminates the program.  

- Default Configuration file, EmaConfig.xml, contains several sections for 
  running this performance tool. See section, "Performance tools consumers" 
  in EmaConfig.xml

  Following is Sample Encrypted Connection Channel Config to specify in EmaConfig.xml. 
  See EmaConfig.xml for additional examples of configuration:
     
    Encrypted Socket:
    <Channel>
    <Name value="Perf_Channel_Encr_1"/>
    <ChannelType value="ChannelType::RSSL_ENCRYPTED"/>
    <EncryptedProtocolType value="EncryptedProtocolType::RSSL_SOCKET"/>
    <CompressionType value="CompressionType::None"/>
    <GuaranteedOutputBuffers value="5000"/>
    <NumInputBuffers value="2048"/>
    <ConnectionPingTimeout value="30000"/>
    <TcpNodelay value="1"/>
    <Host value="localhost"/>
    <Port value="14002"/>
    <OpenSSLCAStore value="./RootCA.crt"/>
    </Channel>

    Encrypted Websocket:
    <Channel>
    <Name value="Perf_Channel_Encr_1"/>
    <ChannelType value="ChannelType::RSSL_ENCRYPTED"/>
    <EncryptedProtocolType value="EncryptedProtocolType::RSSL_WEBSOCKET"/>
    <CompressionType value="CompressionType::None"/>
    <GuaranteedOutputBuffers value="5000"/>
    <NumInputBuffers value="2048"/>
    <ConnectionPingTimeout value="30000"/>
    <TcpNodelay value="1"/>
    <WsMaxMsgSize value="61440"/>
    <WsProtocols value="rssl.json.v2"/>
    <Host value="localhost"/>
    <Port value="15000"/>
    <OpenSSLCAStore value="./RootCA.crt"/>
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

EmaCppConsPerf.cpp - The main file for the EmaCppConsPerf application.

ConsPerfConfig.cpp - Provides configurable options for the application.

ConsumerThread.cpp - Handles consumer connections and consuming content.

AppUtil.cpp - Utility for use by applications and/or common classes.

CtrlBreakHandler.cpp  - Provides Contol-C handling
 
Statistics.cpp - Provides methods for collecting and calculating statistical  information.

PerfConfig.cpp  - Common configurable options across PerfTool applications.

Mutex.cpp   - Provides Mutex handling.

ThreadAffinity.cpp  -Used for printout and determination of thread affinity binding

ThreadBinding.h - Handles Thread binding.

XmlItemParser.cpp  -Used for parsing Item file (350k.xml)

