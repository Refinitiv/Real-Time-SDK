EMA_ConsPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of EMA API,
in consuming Level I Market Price content directly
from an OMM provider or through the Refinitiv Real-Time Distribution System. 

The consumer creates two types of threads:
- A main thread, which collects and records statistical information
- Consumer threads, each of which create a connection to a provider and
request market data.

To measure latency, a timestamp is randomly placed in each burst of updates by 
the provider.  The consumer then decodes the timestamp from the update to
determine the end-to-end latency.

This application also measures memory and CPU usage. The memory usage measured 
is the 'resident set,' or the memory currently in physical use by the 
application. The CPU usage is the total time using the CPU divided by the 
total system time. That is, the CPU time is the total across all threads, and 
as such this number can be greater than 100% if multiple threads are busy.


-----------------
Application Name:
-----------------

EMA_ConsPerf

------------------
Setup Environment:
------------------

The following files are required:
- RDMFieldDictionary and enumtype.def in CSharp/etc/
- 350k.xml in CSharp/Ema/PerfTools/EmaPerfToolsCommon/ 
- EmaConfig.xml in CSharp/Ema/

-----------------
Compiling Source:
-----------------

To build and run performance tool. See installation 
guide for build instructions and command line usage below for run instructions.

-------------------
Command line usage:
-------------------  

EMA_ConsPerf
(runs with a default set of parameters. The full set of configured 
 parameters is printed to the screen. )

- EMA_ConsPerf -? displays command 
  line options, with a brief description of each option.  

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

EmaConsPerf.cs - The main file for the emajConsPerf application.

EmaConsPerfConfig.cs - Provides configurable options for the application.

ConsumerThread.cs - Handles consumer connections and consuming content.

ItemRequest.cs - Provides functions for caching and retrieving item information
for use in request message bursts.

EmaDirectoryHandler.cs - Provides functions for sending the source directory
request to a provider and processing the response.

MarketPriceDecoder.cs - Decodes Market Price content.

TimeRecord.cs - Stores time information for calculating latency.

ValueStatistics.cs - Provides methods for collecting and calculating statistical 
information.

XmlItemInfoList.cs - Loads the list of sample items that the consumer may 
request from an XML file.
