EmajNIProvPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of the EMA,
by providing Level I Market Price content through a Refinitiv Real-Time
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

EmajNIProvPerf

------------------
Setup Environment:
------------------

The following files are required:
- RDMFieldDictionary and enumtype.def in Java/etc/
- 350k.xml in Java/Ema/PerfTools/ 
- MsgData.xml in Java/Ema/PerfTools/ 
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

EmajNIProvPerf
(runs with a default set of parameters. The full set of configured 
 parameters is printed to the screen. )

- com.refinitiv.ema.perftools.emajniprovperf.EmajNIProvPerf -? displays command 
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

- Run EmajNIProvPerf using java or gradlew

  - To run with java from <your install directory>/Java>:

    Set JAVA_HOME. Sample Cmd: export JAVA_HOME=/local/jdk1.11
    Set CLASSPATH. 
      Sample Cmd: export CLASSPATH="Ema/PerfTools/build/classes/java/main;Ema/build/libs/RTSDK-all.jar"
      To build RTSDK-all.jar: gradlew shadowJar
    Set JVM options and run. Sample Cmd: 
      $JAVA_HOME/bin/java -XX:+ForceTimeHighResolution -Xms2048m -Xmx2048m \
         com.refinitiv.ema.perftools.emajniprovperf.EmajNIProvPerf <command line arguments>

  - To run with gradlew:
    
    Set JAVA_HOME. Sample Cmd: export JAVA_HOME=/local/jdk1.11
    Default JVM options can be changed in build.gradle file present in here: <your install directory>/Java/Ema/PerfTools 
    Optionally JVM_OPTIONS explicitly. 
      Sample Cmd: export JVM_OPTIONS="-server -XX:+ForceTimeHighResolution -Xms3048m -Xmx3048m"
    Run EmajNIProvPerf. Sample Cmd: 
      ./gradlew runEMAPerfNIProvider --args="-providerName Perf_NIProvider_1 \
         -updateRate 100000 -itemCount 100000 -tickRate 1000 \
         -serviceName NI_PUB -runTime 300"

      Sample Command "-providerName" Explanation: The value of -providerName is 
      used to specify the provider to use from EmaConfig.xml
 

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

EmajNIProvPerf.java - The main file for the EmajNIProvPerf application.

NIProviderPerfConfig.java - Provides configurable options for the application.

NIProviderThread.java - Handles consumer connections and providing content.

NIProviderPerfClient.java - Handles messages from the consumer application.
