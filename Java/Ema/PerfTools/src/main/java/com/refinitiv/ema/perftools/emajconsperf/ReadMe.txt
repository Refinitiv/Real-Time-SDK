EmajConsPerf Application Description

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

emajConsPerf

------------------
Setup Environment:
------------------

The following files are required:
- RDMFieldDictionary and enumtype.def in Java/etc/
- 350k.xml in Java/Ema/PerfTools/ 
- EmaConfig.xml in Java/Ema/ 
- EMA library must be included in CLASSPATH
- XML parser library xpp3-<version>.jar in RTSDK-BinaryPack/Java/Eta/Libs/

-----------------
Compiling Source:
-----------------

To build and run performance tool, use gradlew or javac. See installation 
guide for build instructions and command line usage below for run instructions.

-------------------
Command line usage:
-------------------  

emajConsPerf
(runs with a default set of parameters. The full set of configured 
 parameters is printed to the screen. )

- com.refinitiv.ema.perftools.emajconsperf.emajConsPerf -? displays command 
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

    Encrypted Websocket:
    <Channel>
    <Name value="Perf_Channel_Encr_1"/>
    <ChannelType value="ChannelType::RSSL_ENCRYPTED"/>
    <EncryptedProtocolType value="EncryptedProtocolType::RSSL_WESOCKET"/>
    <CompressionType value="CompressionType::None"/>
    <GuaranteedOutputBuffers value="5000"/>
    <NumInputBuffers value="2048"/>
    <ConnectionPingTimeout value="30000"/>
    <TcpNodelay value="1"/>
    <WsMaxMsgSize value="61440"/>
    <WsProtocols value="rssl.json.v2"/>
    <Host value="localhost"/>
    <Port value="15000"/>
    </Channel>

- Run emajConsPerf using java or gradlew

  - To run with java from <your install directory>/Java>:

    Set JAVA_HOME. Sample Cmd: export JAVA_HOME=/local/jdk1.11
    Set CLASSPATH. 
      Sample Cmd: export CLASSPATH="Ema/PerfTools/build/classes/java/main;Ema/build/libs/RTSDK-all.jar"
      To build RTSDK-all.jar: gradlew shadowJar
    Set JVM options and run. Sample Cmd: 
      $JAVA_HOME/bin/java -XX:+ForceTimeHighResolution -Xms2048m -Xmx2048m \
         com.refinitiv.ema.perftools.emajconsperf.emajConsPerf <command line arguments>

  - To run with gradlew:
    
    Set JAVA_HOME. Sample Cmd: export JAVA_HOME=/local/jdk1.11
    Default JVM options can be changed in build.gradle file present in here: <your install directory>/Java/Ema/PerfTools 
    Optionally JVM_OPTIONS explicitly. 
      Sample Cmd: export JVM_OPTIONS="-server -XX:+ForceTimeHighResolution -Xms3048m -Xmx3048m"
    Run emajConsPerf. Sample Cmd: 
      ./gradlew runEMAPerfConsumer --args="-serviceName DIRECT_FEED \
         -tickRate 1000 -itemCount 100000 -steadyStateTime 300 \
         -itemFile 350k.xml -consumerName Perf_Consumer_1"

      Sample Command "-consumerName" Explanation: The value of -consumerName is 
      used to specify the consumer to use from EmaConfig.xml
 

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

emajConsPerf.java - The main file for the emajConsPerf application.

ConsPerfConfig.java - Provides configurable options for the application.

ConsumerThread.java - Handles consumer connections and consuming content.

ItemRequest.java - Provides functions for caching and retrieving item information
for use in request message bursts.

DirectoryHandler.java - Provides functions for sending the source directory
request to a provider and processing the response.

MarketPriceDecoder.java - Decodes Market Price content.

TimeRecord.java - Stores time information for calculating latency.

ValueStatistics.java - Provides methods for collecting and calculating statistical 
information.

XmlItemInfoList.java - Loads the list of sample items that the consumer may 
request from an XML file.
