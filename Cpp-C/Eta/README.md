# Enterprise Transport API (ETA) C Edition

This is the **Enterprise Transport API (ETA)**, the high performance, low latency, foundation of the Refinitiv Real-Time SDK. This product allows applications to achieve the highest throughput, lowest latency, low memory utilization, and low CPU utilization when publishing or consuming content. All OMM content and domain models are available through the Enterprise Transport API.  

The Transport API is the re-branding of the Ultra Performance API (UPA), which is used by the Refinitiv Real-Time Distribution System and Refinitiv Real-Time for the optimal distribution of OMM/RWF data. All interfaces in ETA are the same as their corresponding interfaces in UPA (same name, same parameter sets) and the transport and codec are fully wire compatible.  

ETA contains open source components. The transport, decoder, encoder, value add reactor and watchlist features are open source. The reliable multicast transport and VA cache component are closed source.

ETA provides the necessary libraries and information to allow for OMM/RWF encoding and decoding along with all of the necessary Refinitiv transport implementations to connect to Refinitiv Real-Time Distribution System, Refinitiv Real-Time, and Refinitiv Data Feed Direct products.

This repository depends on a binary pack consisting of closed source dependent libraries. The BinaryPack is available in the [release section on GitHub](https://github.com/Refinitiv/Real-Time-SDK/releases) and is auto pulled by RTSDK build via CMake.

Copyright (C) 2019-2024 Refinitiv. All rights reserved.

# ETA C-Edition Documentation

- Installation Guide
- DevGuide
- ValueAddDevGuide
- RDMUsageGuide
- API_ConceptsGuide
- TrainingToolGuide
- PerfToolsGuide
- AnsiPageDevGuide
- DacsLibraryFunctions

In addtion, HTML documentation is available in Cpp-C/Eta/Docs. For addtional documentation, please refer to top level README.MD files.


# ETA Features and Functionality

The Enterprise Transport API is the foundation of the Refinitiv Real-Time SDK, offering the highest throughput most tunability, and lowest latency of any API in the SDK.  

The Enterprise Transport API fully supports all OMM constructs and messages. 

#### A List of Transport API Features

- 64-bit, C-based API
- Shared and static library deployments
- Thread safe and thread aware    

- Can consume and provide:

    - Any and all OMM primitives supported on Refinitiv Real-Time, Refinitiv Real-Time Distribution Systems and Direct Exchange Feeds.
    - All Domain Models, including those defined by Refinitiv as well as other user-defined models.

- Consists of:

    - A transport-level API allowing for connectivity using TCP sockets, HTTP, HTTPS, websockets, reliable and unreliable UDP multicast, and Shared Memory.

    - OMM Encoder and Decoders, allowing full use of all OMM constructs and messages sent over the wire in a binary data format called, Refinitiv Wire Format (RWF). Websocket transport also supports JSON data format which must adhere to Refinitiv [Websocket protocol specification](https://github.com/Refinitiv/websocket-api/blob/master/WebsocketAPI_ProtocolSpecification.pdf).
    
    - RMTES Support: Several structures and functions can be used to process RMTES content and convert to several Unicode formats for interpretation. 
    
- Open Source performance tools:

    - Allow users to measure the performance through their system. Customers can modify the tools to suit their specific needs. These are found in the Value Add portion of this package.
    
- Open Source value added helpers:

    - Reactor is a connection management and event processing component that can significantly reduce the amount of code an application must write to leverage OMM in their own applications and to connect to other OMM based devices. The Reactor can be used to create or enhance Consumer, Interactive Provider, and Non-Interactive Provider start-up processing, including user log in, source directory establishment, and dictionary download. The Reactor also allows for dispatching of events to user implemented callback functions. In addition, it handles flushing of user written content and manages network pings on the user's behalf. Value Added domain representations are coupled with the Reactor, allowing domain specific callbacks to be presented with their respective domain representation for easier, more logical access to content.

    - The Administration Domain Model Representations are RDM specific representations of the OMM administrative domain models. This Value Added Component contains structures that represent the messages within the Login, Source Directory, and Dictionary domains. This component also handles all encoding and decoding functionality for these domain models, so the application needs only to manipulate the message's structure members to send or receive this content. This not only significantly reduces the amount of code an application needs to interact with OMM devices (i.e., Refinitiv Real-Time Distribution System), but also ensures that encoding/decoding for these domain models follow OMM specified formatting rules. Applications can use this Value Added Component directly to help with encoding, decoding and representation of these domain models. When using the ETA Reactor, this component is embedded to manage and present callbacks with a domain specific representation of content.
    
    - Auto-conversion of JSON to RWF or vice versa by Reactor for Websocket Transport: Reactor does automatic conversion of JSON data from a Websocket connection, to RWF, and presents RWF to application layer. Please view documentation section for further details. 

- RWF/JSON conversion library for users to convert JSON messages to RWF and vice-versa from a Websocket connection: This exists as a separate shared library but built into rssl library for static version.

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

# OMM Application Types and Capabilities

#### Consumer Applications
Users can use Transport API to write consumer-based applications capable of the following:

- Make Streaming and Snapshot based subscription requests.
- Perform Batch, Views, and Symbol List requests to capable provider applications, including ADS.
- Pause and Resume active data streams open to the ADS.
- Send Post Messages to capable provider applications, including ADS 
(used for making Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.

#### Interactive Provider Applications
Users can use Transport API to write interactive providers capable of the following:

- Receive requests and respond to Streaming and Snapshot based Requests.
- Receive and respond to requests for Batch, Views, and Symbol Lists.
- Receive requests for Pause and Resume on active Data Streams.
- Receive and acknowledge Post Messages (used when receiving Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.
- Accept multiple connections, or allow multiple consumers to connect to a provider.

#### Non-Interactive Provider Applications
Users can use Transport API to write non-interactive applications that start up and begin publishing data to ADH.

- Connect to one or many ADH devices using TCP sockets or reliable UDP multicast, making only configuration changes. 

#### Reactor Based Consumer and Provider Applications

- Reactor applications can take advantage of an event-driven distribution model
- Reactor will manage ping heartbeats and ensure that user written content is flushed out as effectively as possible.
- Reactor applications can use the watchlist functionality for item recovery, like-request aggregation, fan out, and group status handling.
- Reactor applications can leverage the tunnel streams capability, allowing for a private stream with end-to-end flow control, reliability, authentication, and (when communicating with a Queue Provider) persistent queue messaging.

# ETA C-Edition Library and Version Information

This distribution contains several sets of libraries, intended to allow for ease of integration into both production and development environments.  

Both shared and static libraries are available for use. All functionality is available with either option. For information on using and deploying with the shared libraries, see below. 
  
Libraries in the Optimized subdirectory are built with optimizations. These libraries are production ready and will offer the highest level of performance.

Libraries in the Debug subdirectory are built in Debug mode. These libraries contain additional safety checking. If a misuse is detected, an assertion containing additional information will be triggered. These libraries are intended for use during development phases where debug C-runtime libraries are required. A Shared subdirectory, containing the shared libraries, is available within the Optimized and Debug directories. 

### Shared Libraries

Shared libraries are available for use and contain the same functionality as the static libraries.

#### Windows

Shared library use is similar to static library use, however there are several key differences.  The shared library can be stored in a different location on the machine than the application using it. Ensure that the shared library location is present in the library search path (local directory, system path, etc.) being used by the application.  The library use can be confirmed by using a utility similar to Dependency Walker, available at www.dependencywalker.com.  This will show the shared library dependencies and where they are being resolved to.

##### Static Library Manifest

    Library Name              Package Version
    ------------              ---------------
    librssl.lib*              eta3.8.0.L1
    librsslVA.lib             eta3.8.0.L1
    librsslVACache.lib        eta3.8.0.L1
    libansi.lib               eta3.8.0.L1
    libdacs.lib               eta3.8.0.L1

    *librssl.lib includes JsonConverter

##### Shared Library Manifest

    Library Name              Package Version
    -------------             ---------------
    librssl.dll               eta3.8.0.L1
    librssl.lib               eta3.8.0.L1
    librssl.pdb               eta3.8.0.L1
    librsslVA.dll             eta3.8.0.L1
    librsslVA.lib             eta3.8.0.L1
    librsslVA.pdb             eta3.8.0.L1
    librsslVACache.dll        eta3.8.0.L1
    librsslVACache.lib        eta3.8.0.L1
    librsslVACache.pdb        eta3.8.0.L1
    librsslJsonConverter.dll  eta3.8.0.L1
    librsslJsonConverter.lib  eta3.8.0.L1
    librsslJsonConverter.pdb  eta3.8.0.L1
    
#### Linux    
Shared library use is similar to static library use, however there are several key differences. The shared library can be stored in a different location on the machine than the application using it. Ensure that the shared library location is present in the LD_LIBRARY_PATH being used by the application. The library use can be confirmed by using the ldd command on the application. This will show the shared library dependencies and where they are being resolved to.  

In addition, several versions of a shared library can co-exist on the machine. This allows for easy upgrade of functionality by deploying a newer shared library. It is important to ensure that the application is using a version that is binary compatible to the library that it originally linked with.  

To help with this, Transport API provides several versioning mechanisms for its open source and closed source shared libraries. Each open source library is provided with its package version appended to the end. For example, librssl.so.1.2.3.L1. For closed source shared libraries, the binary version is appended to the name. For example, librsslVACache.so.3. Embedded in each library is a shared object name (soname) that conveys binary compatibility information. For example, assuming that the embedded soname is librssl.so.1, if binary compatibility were to change in ETA, this embedded soname would be updated to be librssl.so.2. This naming convention is intended to help protect applications from using a non-compatible version of the shared library. 

The Transport API provides a helpful script that will create soft links for the appropriate library names, allowing for applications to link against a consistent name, but still leverage product and binary compatibility versioning. For example, librssl.so.1.2.3.L1 is the file; librssl.so.1 and librssl.so are symlinks to librssl.so.1.2.3.L1. Similarly for closed source example, librsslVACache.so.1 is the file; librsslVACache.so.1.2.3.L1 and librsslVACache.so are symlinks to librsslVACache.so.1.  The following script located at the base level of the package, creates the appropriate symlinks, and can be run as follows: 

    ./LinuxSoLink
    

##### Static Library Manifest

    Library                            Package Version
    ------------                       ---------------
    librssl.a                          eta3.8.0.L1
    librsslVA.a                        eta3.8.0.L1
    librsslVACache.a                   eta3.8.0.L1
    libansi.lib                        eta3.8.0.L1
    libdacs.lib                        eta3.8.0.L1

##### Shared Library Manifest

    Library                            Binary Version                  Package Version
    -------------                      --------------                  ----------------
    librssl.so.3.8.0.0                 librssl.so.24                   eta3.8.0.L1
    librsslVA.so.3.8.0.0               librsslVA.so.25                 eta3.8.0.L1
    librsslJsonConverter.so.3.8.0.0    librsslJsonConverter.so.2       eta3.8.0.L1

    librsslVACache.so.3.8.0.0          librsslVACache.so.4             eta3.8.0.L1
    librsslRelMcast.so.3.8.0.0         librsslRelMcast.so.3            eta3.8.0.L1


# ETA C-Edition Issues and Workarounds

- Non-Interactive Provider with Multicast to ADH Packet Loss Under heavy throughput situations, the ADH may drop packets due to its inbound queue filling up. For more information on diagnosing and troubleshooting this potential problem, see the ADH 2.4 or later release notes and documentation.   

- The Reliable Multicast connection type makes use of SIGUSR1. If an application also handles or uses this signal, it may impact the functionality of this connection type.  

- The Watchlist accepts batch requests, however batch requests are not made on the wire to the provider. This will be addressed in a future release.  
      
- When using watchlist, if the application sets msgKey.serviceId in an RsslRequestMsg, any recovery for the stream will be made to services with the same Service ID. This may change in the future. Applications that connect to multiple different providers should consider using the pServiceName option when calling rsslReactorSubmitMsg to request items. 
 
- When using watchlist, the encDataBody member of an RsslRequestMsg is only used for retrieving data related to batch requests, view requests, and symbol list behaviour requests. No other payload is stored or forwarded for standard streams. The extendedHeader of an RsslRequestMsg is not used. When requesting with private streams, the encDataBody member of the RsslRequestMsg is stored and transmitted with the request.

- The RWF/JSON Converter library does not support groupID property of RWF message when using Websocket Transport with JSON data format.

- With certain Visual Studio compiler versions, input string issues generated by the compiler result in rsslJCTest.exe to fail in some test scenarios. Unit tests will be fixed in a future release.

- The ServerSharedSocket feature which permits multiple provider applications to reuse a port for load balancing is available only with certain patch levels on Linux 6. So, applications that intend to use this feature on Linux 6 must rebuild the RTSDK library (librssl) natively on a Linux 6 platform with the appropriate patch level that supports this feature.

            
# Reference Information

    I-COS Questionnaire: 6212
    Refinitiv Item Number: N/A
    Product Name: Enterprise Transport API - C Edition
    Release Number: 3.8.0
    Load Number: 1
    Windows Load ID: eta3.8.0.L1.win
        Supersedes: eta3.7.3.L2.win.rrg
    Linux Load ID: eta3.8.0.L1.linux
        Supersedes: eta3.7.3.L2.linux.rrg
    Release Status: RRG
    Release Type: RRG
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Refinitiv Security Compliant
    Template Version Supported: v4.20.62_RealTimeDistributionSystem_24.31 for RWF and Marketfeed Record Templates

# Security

    The components in this package have been scanned using the below software and security scanning products:

    Black Duck by Synopsis, 2023.3.0.1060, https://www.blackducksoftware.com/
    SemGrep 1.2.1, https://semgrep.dev/

# Notes:
- This package contains APIs that are subject to proprietary and open source licenses. Please make sure to read the top level README.md files for clarification.
