# Enterprise Message API (EMA) - C++ Edition

The Enterprise Message API: This is an easy-to-use, performant, open source message layer API. The Enterprise Message API helps developers by allowing them to develop applications with significantly less code. It is new and will be enhanced by collaboration with customers (through GitHub) and Refinitiv based on customer feedback.

EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist.  

Copyright (C) 2018-2021 Refinitiv. All rights reserved.
  
# EMA C++ Documentation

- Installation Guide
- DevGuide
- RDMUsageGuide
- API_ConceptsGuide
- ConfigGuide
- EMACPP_Examples
- PerfToolsGuide

In addition, HTML documentation is available in Cpp-C/Ema/Docs. For addtional documentation, please refer to top level README.MD files.

# EMA Features and Functionality

## Common Features:

- TCP/IP Connectivity

- RMTES Decoder: EMA provides a built in RMTES decoder. If desired, application may cache RmtesBuffer objects and apply all the received changes to them.

- Data::toString(): All OMM containers, primitives and messages may simply be printed out to screen in a standardized output format.

- Data::getAsHex(): Applications may obtain binary representations of all OMM containers, primitives and messages.

- File Config: Enables applications to specify EMA configuration in an EmaConfig.xml file

- Parameters, reconnectAttemptLimit, reconnectMinDelay, reconnectMaxDelay, xmlTrace, MsgKeyInUpdates only can be configured on Consumer/IProvider/NiProvider instance level.

## Consumer Features:

- RSSL Encrypted and HTTP Connectivity

- Connection Failover: EMA can be configured to specify a list of failover servers via ChannelSet configuration.  In the event that the consumer's connection attempt fails, EMA will utilize the next channel in the ChannelSet list.

- Default Admin Domain Requests: EMA uses default login, directory and dictionary request while connecting to server. This provides minimum configuration for applications to get up and running.

- Configurable Admin Domain Requests:  EMA provides means for modifying the default admin domain requests. 
      
- Tunnel Streams: EMA supports private streams, with additional associated behaviors (e.g., end-to-end authentication, guaranteed delivery, and flow control).

- Batch Request: Application may use a single request message to specify interest in multiple items via the item list

- Dynamic View: Application may specify a subset of fields or elements of a particular item

- Optimized Pause and Resume: Application may request server to pause and resume item stream

- Single Open: EMA supports application selected single open functionality

- Programmatic Config: Enables application to programmatically specify and overwrite EMA configuration


## Non-Interactive Provider Features:

- Default Admin Domains: EMA uses default login and directory messages while connecting to server. This provides minimum configuration for applications to get up and running.
 
- Configurable Admin Domains: EMA provides means for modifying the default admin domain messages. 

- Programmatic Config: Enables application to programmatically specify and overwrite EMA configuration


## Interactive Provider Features:

- Default Admin Domains: EMA uses default directory messages while sending to the connected client. This provides minimum configuration for applications to get up and running.
 
- Configurable Admin Domains: EMA provides means for modifying the default admin domain messages. 

- Programmatic Config: Enables application to programmatically specify and overwrite EMA configuration

# EMA C++ Library and Version Information

This distribution contains several sets of libraries, intended to allow for ease of integration into both production and development environments.  

Both shared and static libraries are available for use.  All functionality is available with either option.  For information on using and deploying with the shared libraries, see below. 
  
Libraries in the Optimized subdirectory are built with optimizations.  These libraries are production ready and will offer the highest level of performance.

Libraries in the Debug subdirectory are built with optimizations, but also contain additional built in safety checks.  If a misuse is detected, an assertion containing additional information will be triggered.  These libraries can be deployed into production, however due to additional performance overhead of the safety checking, they are mainly intended for use during development phases.  

A Shared subdirectory, containing the shared libraries, is available within the Optimized and Debug directories. 

### Shared Libraries

Shared libraries are available for use and contain the same functionality as the static libraries.  

#### Windows

Shared library use is similar to static library use, however there are several key differences.  The shared library can be stored in a different location on the machine than the application using it. Ensure that the shared library location is present in the library search path (local directory, system path, etc.) being used by the application.  The library use can be confirmed by using a utility similar to Dependency Walker, available at www.dependencywalker.com.  This will show the shared library dependencies and where they are being resolved to.  

##### Static Library Manifest

    Library Name              Package Version
    ------------              ---------------
    libema.lib                ema3.6.2.G3

##### Shared Library Manifest

    Library Name              Package Version
    -------------             ---------------
    libema.lib                ema3.6.2.G3
    libema.dll                ema3.6.2.G3

#### Linux
    
Shared library use is similar to static library use, however there are several key differences. The shared library can be stored in a different location on the machine than the application using it. Ensure that the shared library location is present in the LD_LIBRARY_PATH being used by the application. The library use can be confirmed by using the ldd command on the application. This will show the shared library dependencies and where they are being resolved to.  

In addition, several versions of a shared library can co-exist on the machine. This allows for easy upgrade of functionality by deploying a newer shared library. It is important to ensure that the application is using a version that is binary compatible to the library that it originally linked with.  

To help with this, the EMA API provides several versioning mechanisms for its shared libraries.  Each library is provided with its package version appended to the end.  For example, libema.so.3.1.0.0. Embedded in this library is a shared object name (soname) that conveys binary compatibility information. For example, assuming that the embedded soname is libema.so.1, if binary compatibility were to change in EMA, this embedded soname would be updated to be libema.so.2. This naming convention is intended to help protect applications from using a non-compatible version of the shared library.  

The API provides a helpful script that will create soft links for the appropriate library names, allowing for applications to link against a consistent name, but still leverage product and binary compatibility versioning. This script is provided at the base level of the package, and can be run as follows: 

	./LinuxSoLink
    
This will create all necessary soft links for example makefiles to link. It is suggested that any applications deployed using shared libraries follow a similar methodology to ensure proper versioning.

##### Static Library Manifest

    Library Name                Package Version  
    -------------               -------------- 
    libema.a                    ema3.6.2.G3
    
##### Shared Library Manifest

    Library Name                Binary Version       Package Version
    -------------               --------------       ----------------
    libema.so.3.6.2.G3          libema.so.12          ema3.6.2.G3
    
  
# EMA C++ Issues and Workarounds

- ESDK-421 need infinite timeout support for PostAckTimeout and RequestTimeout in EMA

- ESDK-385 ChannelSet with two multicast channels userQLimit set incorrectly 

- ESDK-395 NiProvider360 application uses 100% CPU when CTRL-C pressed while publishing data

- RTSDK-5119 EMACPP NIProvPerf has a limitation of 50000 watchlist size. RFA CPP used message packing to push the typical watchlist size of 100000 to ADH. Message packing is unavailable with EMA CPP.

- ESDK-361 When overriding admin messages using addAdminMessage and if the service is down at start-up, the dictionary will not be downloaded properly.

- EMA can not download dictionary from a Refinitiv Real-Time Distribution System over a Websocket connection using the tr_json2/rssl_json protocol. This is a limitation of the simplied JSON protocol.

- The RWF/JSON Converter library does not support groupID property of RWF message when using Websocket Transport with JSON data format.

- The ServerSharedSocket feature which permits multiple provider applications to reuse a port for load balancing is available only with certain patch levels on Linux 6. So, applications that intend to use this feature on Linux 6 must rebuild the RTSDK library (librssl) natively on a Linux 6 platform with the appropriate patch level that supports this feature. 


# Reference Information

    I-COS Questionnaire: 6032
    Refinitiv Item Number: N/A
    Product Name: Enterprise Message API - C++ Edition
    Release Number: 3.6.2
    Load Number: 1
    Windows Load ID: ema3.6.2.E1.win
        Supersedes: ema3.6.2.L1.win
    Linux Load ID: ema3.6.2.E1.linux
        Supersedes: ema3.6.2.L1.linux
    Release Status: RRG
    Release Type: RRG
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Refinitiv Security Compliant
    Template Version Supported: v4.20.48_RealTimeDistributionSystem_21.61 for RWF and Marketfeed Record Templates

# Security

    The components in this package have been scanned using the below software and security scanning products:

    Veracode, Refinitiv Standard v21, https://www.veracode.com/.
    Black Duck by Synopsis, 2020.12.0.808, https://www.blackducksoftware.com/.

# Notes:
- This package contains APIs that are subject to proprietary and opens source licenses.  Please make sure to read the top level README.md files for clarification.
