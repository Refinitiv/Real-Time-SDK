EmajProvPerf Application Description

--------
Summary:
--------
 
The purpose of this application is to measure performance of EMA API,
by providing Level I Market Price content either directly to
an OMM consumer or through the Refinitiv Real-Time Distribution System. 

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

EmajProvPerf

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

EmajProvPerf
(runs with a default set of parameters. The full set of configured 
 parameters is printed to the screen. )

- com.refinitiv.ema.perftools.emaprovperf.EmajProvPerf -? displays command 
  line options, with a brief description of each option.  

- Pressing the CTRL+C buttons terminates the program. 

- Default Configuration file, EmaConfig.xml, contains several sections for 
  running this performance tool. See section, "Performance tools consumers" 
  in EmaConfig.xml

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
        <DirectWrite value="1" />
        <MaxFragmentSize value="6144"/>
        <NumInputBuffers value="10000"/>
        <SysRecvBufSize value="65535"/>
        <SysSendBufSize value="65535"/>
    </Server>

- Run EmajProvPerf using java or gradlew

  - To run with java from <your install directory>/Java>:

    Set JAVA_HOME. Sample Cmd: export JAVA_HOME=/local/jdk1.11
    Set CLASSPATH. 
      Sample Cmd: export CLASSPATH="Ema/PerfTools/build/classes/java/main;Ema/build/libs/RTSDK-all.jar"
      To build RTSDK-all.jar: gradlew shadowJar
    Set JVM options and run. Sample Cmd: 
      $JAVA_HOME/bin/java -XX:+ForceTimeHighResolution -Xms2048m -Xmx2048m \
         com.refinitiv.ema.perftools.emajprovperf.EmajProvPerf <command line arguments>

  - To run with gradlew:
    
    Set JAVA_HOME. Sample Cmd: export JAVA_HOME=/local/jdk1.11
    Default JVM options can be changed in build.gradle file present in here: <your install directory>/Java/Ema/PerfTools 
    Optionally JVM_OPTIONS explicitly. 
      Sample Cmd: export JVM_OPTIONS="-server -XX:+ForceTimeHighResolution -Xms3048m -Xmx3048m"
    Run EmajProvPerf. Sample Cmd: 
      ./gradlew runEMAPerfProvider --args="--args="-providerName Perf_Provider_1 \
         -tickRate 1000 -updateRate 100000 -latencyUpdateRate 1000 \
         -runTime 240"

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

EmajProvPerf.java - The main file for the EmajProvPerf application.

IProviderPerfConfig.java - Provides configurable options for the application.

IProviderThread.java - Handles consumer connections and providing content.

ProviderPerfClient.java - Handles messages from the consumer application.
