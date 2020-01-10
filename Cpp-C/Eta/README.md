# Elektron Transport API (ETA) C Edition

This is the **Elektron Transport API (ETA)**, the high performance, low latency, foundation of the Elektron SDK. This product allows applications to achieve the highest throughput, lowest latency, low memory utilization, and low CPU utilization when publishing or consuming content. All OMM content and domain models are available through the Elektron Transport API.  

The Transport API is the re-branding of the Ultra Performance API (UPA), which is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data.  All interfaces in ETA are the same as their corresponding interfaces in UPA (same name, same parameter sets) and the transport and codec are fully wire compatible.  

ETA contains open source components. The transport, decoder and encoder components are open source. The reliable multicast transport and VA cache component are closed source.

This repository depends on the `Elektron-SDK-BinaryPack` (http://www.github.com/Refinitiv/Elektron-SDK-BinaryPack) repository. The BinaryPack is also available in the [release section on GitHub](https://github.com/Refinitiv/Elektron-SDK/releases) and is auto pulled by ESDK build via CMake. 

This release provides the necessary libraries and information to allow for OMM/RWF encoding and decoding along with all of the necessary Refinitiv transport implementations to connect to Enterprise Platform, Elektron, and the Data Feed Direct products.

Copyright (C) 2019 Refinitiv. All rights reserved,

# ETA C-Edition Documentation

- Installation Guide
- DevGuide
- ValueAddDevGuide
- RDMUsageGuide
- API_ConceptsGuide
- TrainingToolGuide
- PerfToolsGuide

In addtion, HTML documentation is available in Cpp-C/Eta/Docs. For addtional documentation, please refer to top level README.MD files.


# ETA Features and Functionality

The Elektron Transport API, formerly known as Ultra Performance API (UPA) is the foundation of the Elektron SDK, offering the highest throughput most tunability, and lowest latency of any API in the SDK.  

The Elektron Transport API fully supports all OMM constructs and messages. 

#### A List of Transport API Features

- 64-bit, C-based API
- Shared and static library deployments
- Thread safe and thread aware    

- Can consume and provide:

    - Any and all OMM primitives supported on Elektron, Enterprise Platform, and Direct Exchange Feeds.
    - All Domain Models, including those defined by Refinitiv as well as other user-defined models.

- Consists of:

    - A transport-level API allowing for connectivity using TCP, HTTP, HTTPS,
         sockets, reliable and unreliable UDP multicast, and Shared Memory.  
    - OMM Encoder and Decoders, allowing full use of all OMM constructs and messages.
    
    - RMTES Support.
      Several structures and functions can be used to process RMTES content 
      and convert to several Unicode formats for interpretation. 
    
- Open Source performance tools:

    - Allow users to measure the performance through their system.  Customers can modify the tools to suit their specific needs.  These are found in the Value Add portion of this package.
    
- Open Source value added helpers:

    - Reactor is a connection management and event processing
        component that can significantly reduce the amount of code an 
        application must write to leverage OMM in their own applications
        and to connect to other OMM based devices.  The Reactor can be
        used to create or enhance Consumer, Interactive Provider, and
        Non-Interactive Provider start-up processing, including user log
        in, source directory establishment, and dictionary download.  The
        Reactor also allows for dispatching of events to user implemented
        callback functions.  In addition, it handles flushing of user
        written content and manages network pings on the user's behalf.
        Value Added domain representations are coupled with the Reactor,
        allowing domain specific callbacks to be presented with their
        respective domain representation for easier, more logical 
        access to content.

    - The Administration Domain Model Representations are RDM specific
        representations of the OMM administrative domain models.  This
        Value Added Component contains structures that represent the 
        messages within the Login, Source Directory, and Dictionary 
        domains.  This component also handles all encoding and decoding
        functionality for these domain models, so the application needs
        only to manipulate the message's structure members to send or
        receive this content.  This not only significantly reduces the
        amount of code an application needs to interact with OMM devices
        (i.e., Enterprise Platform for Real-time), but also ensures that
        encoding/decoding for these domain models follow OMM specified
        formatting rules.  Applications can use this Value Added 
        Component directly to help with encoding, decoding and
        representation of these domain models.  When using the UPA
        Reactor, this component is embedded to manage and present
        callbacks with a domain specific representation of content.
    
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
- Receive and acknowledge Post Messages
(used when receiving Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.
- Accept multiple connections, or allow multiple consumers to connect to a provider.

#### Non-Interactive Provider Applications
Users can use Transport APi to write non-interactive applications that start up and begin publishing data to ADH.

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
    librssl.lib               eta3.4.0.G1
    librsslVA.lib             eta3.4.0.G1
    librsslVACache.lib        eta3.4.0.G1
    libansi.lib               eta3.4.0.G1
    libdacs.lib               eta3.4.0.G1

##### Shared Library Manifest

    Library Name              Package Version
    -------------             ---------------
    librssl.dll               eta3.4.0.G1
    librssl.lib               eta3.4.0.G1
    librssl.pdb               eta3.4.0.G1
    librsslVA.dll             eta3.4.0.G1
    librsslVA.lib             eta3.4.0.G1
    librsslVA.pdb             eta3.4.0.G1
    librsslVACache.dll        eta3.4.0.G1
    librsslVACache.lib        eta3.4.0.G1
    librsslVACache.pdb        eta3.4.0.G1
    
#### Linux    
Shared library use is similar to static library use, however there are several key differences. The shared library can be stored in a different location on the machine than the application using it. Ensure that the shared library location is present in the LD_LIBRARY_PATH being used by the application. The library use can be confirmed by using the ldd command on the application. This will show the shared library dependencies and where they are being resolved to.  

In addition, several versions of a shared library can co-exist on the machine. This allows for easy upgrade of functionality by deploying a newer shared library. It is important to ensure that the application is using a version that is binary compatible to the library that it originally linked with.  

To help with this, Transport API provides several versioning mechanisms for its open source and closed source shared libraries. Each open source library is provided with its package version appended to the end. For example, librssl.so.3.4.0.L1. For closed source shared libraries, the binary version is appended to the name. For example, librsslVACache.so.3. Embedded in each library is a shared object name (soname) that conveys binary compatibility information. For example, assuming that the embedded soname is librssl.so.1, if binary compatibility were to change in UPA, this embedded soname would be updated to be librssl.so.2. This naming convention is intended to help protect applications from using a non-compatible version of the shared library. 

The Transport API provides a helpful script that will create soft links for the appropriate library names, allowing for applications to link against a consistent name, but still leverage product and binary compatibility versioning. For example, librssl.so.3.4.0.L1 is the file; librssl.so.1 and librssl.so are symlinks to librssl.so.3.4.0.L1. Similarly for closed source example, librsslVACache.so.1 is the file; librsslVACache.so.3.4.0.L1 and librsslVACache.so are symlinks to librsslVACache.so.1.  The following script located at the base level of the package, creates the appropriate symlinks, and can be run as follows: 

	./LinuxSoLink
    

##### Static Library Manifest

    Library                            Package Version
    ------------                       ---------------
    librssl.a                          eta3.4.0.G1
    librsslVA.a                        eta3.4.0.G1
    librsslVACache.a                   eta3.4.0.G1
    libansi.lib                        eta3.4.0.G1
    libdacs.lib                        eta3.4.0.G1

##### Shared Library Manifest

    Library                            Binary Version       Package Version
    -------------                      --------------       ----------------
    librssl.so.3.4.0.1                 librssl.so.10         eta3.4.0.G1
    librsslVA.so.3.4.0.1               librsslVA.so.13       eta3.4.0.G1

    librsslVACache.so.3.4.0.1          librsslVACache.so.3   eta3.4.0.G1
    librsslRelMcast.so.3.4.0.l         librsslRelMcast.so.1  eta3.4.0.G1


# ETA C-Edition Issues and Workarounds

- Non-Interactive Provider with Multicast to ADH Packet Loss Under heavy throughput situations, the ADH may drop packets due to its inbound queue filling up. For more information on diagnosing and troubleshooting this potential problem, see the ADH 2.4 or later release notes and documentation.   

- The Reliable Multicast connection type makes use of SIGUSR1. If an application also handles or uses this signal, it may impact the functionality of this connection type.  

- The Watchlist accepts batch requests, however batch requests are not made on the wire to the provider.  This will be addressed in a future release.  
      
- When using watchlist, if the application sets msgKey.serviceId in an RsslRequestMsg, any recovery for the stream will be made to services with the same Service ID. This may change in the future. Applications that connect to multiple different providers should consider using the pServiceName option when calling rsslReactorSubmitMsg to request items. 
 
- When using watchlist, the encDataBody member of an RsslRequestMsg is only used for retrieving data related to batch requests, view requests, and symbol list behaviour requests. No other payload is stored or forwarded for standard streams. The extendedHeader of an RsslRequestMsg is not used. When requesting with private streams, the encDataBody member of the RsslRequestMsg is stored and transmitted with the request.
            
# Reference Information

    I-COS Questionnaire: 6211
    Refinitiv Item Number: N/A
    Product Name: Elektron Transport API - C Edition
    Release Number: 3.4.0
    Load Number: 1
    Windows Load ID: eta3.4.0.L1.win
        Supersedes: eta3.3.1.L1.win.rrg
    Linux Load ID: eta3.4.0.L1.linux
        Supersedes: eta3.3.1.L1.linux.rrg
    Release Status: RRG
    Release Type: RRG
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Refinitiv Security Compliant
    Template Version Supported: v4.20.39_TREP_20.01 for RWF and Marketfeed Record Templates

# Notes:
- This package contains APIs that are subject to proprietary and opens source licenses.  Please make sure to read the top level README.md files for clarification.
