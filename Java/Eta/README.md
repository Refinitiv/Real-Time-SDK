# Enterprise Transport API (ETA) Java Edition

This is the **Enterprise Transport API (ETA)**, the high performance, low latency, foundation of the Refinitiv Real-Time SDK. This product allows applications to achieve the highest throughput, lowest latency, low memory utilization, and low CPU utilization when publishing or consuming content. All OMM content and domain models are available in Enterprise Transport API.  

The Transport API is the re-branding of the Ultra Performance API (UPA), which is used by Refinitiv Real-Time Distribution Systems and Refinitiv Real-Time for the optimal distribution of OMM/RWF data. All interfaces in ETA are the same as their corresponding interfaces in UPA (same name, same parameter sets) and the transport and codec are fully wire compatible. Starting with verison RTSDK 2.0, there are changes to namespace and jar file names: please see [REBRAND.md](https://github.com/Refinitiv/Real-Time-SDK/blob/master/REBRAND.md) for details on how to adapt a UPA application or a ETA application written to prior versions of the library to see what must be changed. 

ETA Java contains open source components. The transport, decoder, encoder, and cache components are open source. 

ETA provides the necessary libraries and information to allow for OMM/RWF encoding and decoding along with all of the necessary Refinitiv transport implementations to connect to Refinitiv Real-Time Distribution System, Refinitiv Real-Time, and Refinitiv Data Feed Direct products.

This repository depends on a binary pack consisting of closed source dependent libraries. The BinaryPack is available in the [release section on GitHub](https://github.com/Refinitiv/Real-Time-SDK/releases) and is auto pulled by RTSDK Gradle build.

Copyright (C) 2019-2021 Refinitiv. All rights reserved.

# ETA Java Documentation

- Installation Guide
- DevGuide
- ValueAddDevGuide
- RDMUsageGuide
- API_ConceptsGuide
- TrainingToolGuide
- PerfToolsGuide
- AnsiPageDevGuide
- DacsLibraryFunctions

In addtion, HTML documentation is available in Java/Eta/Docs. For addtional documentation, please refer to top level README.MD files.

# ETA Features and Functionality

- 64-bit, Java-based API

- Shared JNI deployments

- Thread safe and thread aware

- Can consume and provide:

   - Any and all OMM primitives supported on Refinitiv Real-Time Distribution System, Refinitiv Real-Time, and Refinitiv Data Feed Direct 

   - All Domain Models, including those defined by Refinitiv as well as other user-defined models.

- Consists of:

   - A transport-level API allowing for connectivity using TCP, HTTP, HTTPS, sockets, websockets, reliable and unreliable UDP multicast, and Shared Memory.

   - OMM Encoder and Decoders, allowing full use of all OMM constructs and messages. Websocket transport also supports JSON data format which must adhere to Refinitiv [Websocket protocol specification](https://github.com/Refinitiv/websocket-api/blob/master/WebsocketAPI_ProtocolSpecification.pdf).

- RMTES Support: Several classes and methods can be used to process RMTES content and convert to several Unicode formats for interpretation.

- Open Source performance tools: Allow users to measure the performance through their system.  Customers can modify the tools to suit their specific needs.  These are found in the Applications portion of this package.  The performance tools use the open source XML Pull Parser. It's full license document is provided in the location with the tools.

- Open Source value added helpers:

   - Reactor is a connection management and event processing component that can significantly reduce the amount of code an application must write to leverage OMM in their own applications and to connect to other OMM based devices.  The Reactor can be used to create or enhance Consumer, Interactive Provider, and Non-Interactive Provider start-up processing, including user log in, source directory establishment, and dictionary download.  The Reactor also allows for dispatching of events to user implemented callback functions.  In addition, it handles flushing of user written content and manages network pings on the user's behalf.  Value Added domain representations are coupled with the Reactor, allowing domain specific callbacks to be presented with their respective domain representation for easier, more logical access to content. Reactor also provides opportunity in-box support of RTT monitoring for consumer applications.

   - The Administration Domain Model Representations are RDM specific amount of code an application needs to interact with OMM devices (i.e., Refinitiv Real-Time Distribution System), but also ensures that encoding/decoding for these domain models follow OMM specified formatting rules.  Applications can use this Value Added Component directly to help with encoding, decoding and representation of these domain models.  When using the ETA Reactor, this component is embedded to manage and present callbacks with a domain specific representation of content.

    - Auto-conversion of JSON to RWF or vice versa by Reactor for Websocket Transport: Reactor does automatic conversion of JSON data from a Websocket connection, to RWF, and presents RWF to application layer. Please view documentation section for further details. 

- RWF/JSON conversion library for users to convert JSON messages to RWF and vice-versa from a Websocket connection: This exists as a separate library.

- DACS library for users to create custom locks for content publishing

- ANSI library for users to process ANSI Page based content


#### General Capabilities

Transport API provides the following general capabilities independent of the type of application:

- ETA can internally fragment and reassemble large messages.
- ETA applications can pack multiple, small messages into the same network buffer.
- ETA can internally perform data compression and decompression.
- ETA applications can choose their locking model based on need. Locking can be enabled globally, within a connection, or disabled entirely, allowing clients to develop single-threaded, multi-threaded thread safe, or thread-aware solutions.
- ETA applications have full control over the number of message buffers and can dynamically increase or decrease this quantity during runtime.
- ETA does not have configuration file, log file, or message file dependencies: everything is programmatic.
- ETA allows users to write messages at different priority levels, allowing higher priority messages to be sent before lower priority messages.
- ETA applications can create and manage both standard and private data streams.
- ETA Reactor applications can create and manage standard, private, and tunnel streams.

# OMM Application Type Abilities

#### Consumer Applications

Users can use Transport API to write consumer-based applications capable of the following:

- Make Streaming and Snapshot based subscription requests.
- Perform Batch, Views, and Symbol List requests to capable provider applications, including ADS.
- Pause and Resume active data streams open to the ADS.
- Send Post Messages to capable provider applications, including ADS (used for making Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.

#### Provider Applications: Interactive

Users can use Transport API to write interactive providers capable of the following:

- Receive requests and respond to Streaming and Snapshot based Requests.
- Receive and respond to requests for Batch, Views, and Symbol Lists.
- Receive requests for Pause and Resume on active Data Streams.
- Receive and acknowledge Post Messages (used when receiving Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.
- Accept multiple connections, or allow multiple consumers to connect to a provider.

#### Provider Applications: Non-Interactive
Users can use Transport API to write non-interactive applications that start up and begin publishing data to ADH.

- Connect to one or many ADH devices using TCP sockets or reliable UDP multicast, making only configuration changes.

#### Reactor Based Consumer and Provider Applications

- Reactor applications can take advantage of an event-driven distribution model
- Reactor will manage ping heartbeats and ensure that user written content is flushed out as effectively as possible.
- Reactor applications can use the watchlist functionality for item recovery, like-request aggregation, fan out, and group status handling.
- Reactor applications can leverage the tunnel streams capability, allowing for a private stream with end-to-end flow control, reliability, authentication, and (when communicating with a Queue Provider) persistent queue messaging.
- Reactor allows for consumer based applications to measure and monitor Round Trip Latency during message exchanging.

# ETA Java Library and Version Information

The distribution contains several JAR files and other non-Java libraries, intended to allow for ease of integration into both production and development environments.

    Library Name                  Package Version   Description
    ------------                  ----------------  -----------
    eta-3.6.2.2.jar               eta3.6.2.G2       The ETA - Java Edition library.  Includes
                                                    the ETA transport package and the RWF codec.

    etaValueAdd-3.6.2.2.jar       eta3.6.2.G2       The Value Add library for ETA Java Edition.
                                                    Includes the ETA Value Add Reactor and
                                                    Administration Domain Model Representations.

    etaValueAddCache-3.6.2.2.jar  eta3.6.2.G2       The Value Add payload cache library for ETA
                                                    Java Edition.

    etajConverter-3.6.2.2.jar     eta3.6.2.G2       The RWF/JSON Converter library.

    jDacsEtalib.jar               dacs7.7           The ETA Java DACS library.

    ansipage-3.6.2.2.jar          eta3.6.2.G2       The ANSI decoders and encoders.
                  

    ETAC/ETA/RSSL JNI Libs        eta3.6.2.G2       The JNI libraries for Reliable Multicast
                                                    Transport and Shared Memory Transport. These
                                                    are native libraries for each supported
                                                    platform. The DLL files must be included
                                                    in the "path" in order to use the Windows
                                                    platform. Shared object files must be present
                                                    in the LD_LIBRARY_PATH for the Linux platform.

    Apache                        4.5.13            The Apache libraries in the ApacheClient
                                                    directory. These are used for proxy
                                                    authentication.

# ETA Java Issues and Workarounds

- Although the examples use float and double for simplicity, these data types should never be used for precise values like currency. Use java.math.BigDecimal instead. Refer to the float and double section in: http://docs.oracle.com/javase/tutorial/java/nutsandbolts/datatypes.html

- ETA Java uses the Oracle JDK OperatingSystemMXBean for Performance Tool CPU and Memory usage statistics. The Oracle JDK OperatingSystemMXBean is incompatible with the some alternate JDK implementations.

- ESDK-307 When using DirectoryMsg.decode(), Source Directory FilterActions of Update may be overwritten with Set

- ESDK-312 Watchlist fans out Dictionary state of Open/Suspect state instead of Closed/Recover

- Users of encrypted tunneling connection type may encounter trust issues with DigiCert certificates. JRE8 update 91 and higher support DigiCert certificates.  Users can upgrade to a higher JRE version if they encounter problems.

- ETA can not download dictionary from a Refinitiv Real-Time Distribution System over a Websocket connection using the tr\_json2/rssl\_json protocol. This is a limitation of the simplied JSON protocol.

- The RWF/JSON Converter library does not support groupID property of RWF message when using Websocket Transport with JSON data format.

- ETA Java ValueAdd cache code which uses JNI is not compatible with the following JDK versions on Windows: Open JDK 1.11, JDK 1.11, Open JDK 1.8, JDK 1.8 update 261 or higher. Please note that this is not an issue on Linux.

- ETA PerfTools applications print out final statistics when interrupted using Ctrl+C only if launched manually, Gradle environment doesn't support this behavior.  



# Reference Information

    I-COS Questionnaire: 6314
    Refinitiv Item Number: N/A
    Product Name: Enterprise Transport API - Java Edition
    Release Number: 3.6.2
    Load Number: 1
    Load ID: etaj3.6.2.L1.all
        Supersedes: etaj3.6.1.L2.all.rrg
    Release Status: RRG
    Release Type: RRG
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Refinitiv Security Compliant
    Template Version Supported: v4.20.48_RealTimeDistributionSystem_21.51 for RWF and Marketfeed Record Templates

# Security

    The components in this package have been scanned using the below software and security scanning products:

    Veracode, Refinitiv Standard v21, https://www.veracode.com/.
    Black Duck by Synopsis, 2020.12.0.808, https://www.blackducksoftware.com/.

# Notes:
- This package contains APIs that are subject to proprietary and opens source licenses.  Please make sure to read the README.md files within each package for clarification.
- Java unit tests may use [Mockito](http://site.mockito.org/) for creation of mock objects. Mockito is distributed under the MIT license.
