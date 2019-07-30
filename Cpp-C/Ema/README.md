# Elektron Message API (EMA) - C++ Edition


The Elektron Message API: This is an easy-to-use, performant, open source message layer API. The Elektron Message API helps developers by allowing them to develop applications with significantly less code. It is new and will be enhanced by collaboration with customers (through GitHub) and Thomson Reuters based on customer feedback.

EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist.  
This release provides the necessary libraries and information to allow for OMM/RWF encoding and decoding along with all of the necessary Thomson Reuters transport implementations to connect to Enterprise Platform, Elektron, and the Data Feed Direct products.

(C) Copyright 2018 - 2019 Thomson Reuters Limited. All rights reserved,
Reuters Oak Brook, IL USA
  
# EMA C++ Documentation

- ElektronMessageAPI Overview
- Installation Guide
- DevGuide
- RDMUsageGuide
- API_ConceptsGuide
- ConfigGuide
- EMACPP_Examples

In addition, HTML documentation is available in Cpp-C/Ema/Docs. For addtional documentation, please refer to top level README.MD files.

# EMA Features and Functionality

## Common Features:

- TCP/IP Connectivity

- RMTES Decoder: EMA provides a built in RMTES decoder. IF desired, application may cache RmtesBuffer objects and apply all the received changes to them.

- Data::toString(): All OMM containers, primitives and messages may simply be printed out to screen in a standardized output format.

- Data::getAsHex(): Applications may obtain binary representations of all OMM containers, primitives and messages.

- File Config: Enables applications to specify EMA configuration in an EmaConfig.xml file

- Removing one deprecation. Now reconnectAttemptLimit,reconnectMinDelay,reconnectMaxDelay,xmlTraceXXX,MsgKeyInUpdates only can be configured
  on Consumer/IProvider/NiProvider instance level.

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
    libema.lib                ema3.3.1.L1

##### Shared Library Manifest

    Library Name              Package Version
    -------------             ---------------
    libema.lib                ema3.3.1.L1
    libema.dll                ema3.3.1.L1

#### Linux
    
Shared library use is similar to static library use, however there are several key differences.  The shared library can be stored in a different location on the machine than the application using it. Ensure that the shared library location is present in the LD_LIBRARY_PATH being used by the application.  The library use can be confirmed by using the ldd command on the application.  This will show the shared library dependencies and where they are being resolved to.  

In addition, several versions of a shared library can co-exist on the machine.  This allows for easy upgrade of functionality by deploying a newer shared library.  It is important to ensure that the application is using a version that is binary compatible to the library that it originally linked with.  

To help with this, the EMA API provides several versioning mechanisms for its shared libraries.  Each library is provided with its package version appended to the end.  For example, libema.so.3.1.  Embedded in this library is a shared object name (soname) that conveys binary compatibility information. (For example, assuming that the embedded soname is libema.so.1.  If binary compatibility were to change in EMA, this embedded soname would be updated to be libema.so.2.) This naming convention is intended to help protect applications from using a non-compatible version of the shared library.  

The API provides a helpful script that will create soft links for the appropriate library names, allowing for applications to link against a consistent name, but still leverage product and binary compatibility versioning. This script is provided at the base level of the package, and can be run as follows: 

	./LinuxSoLink
    
This will create all necessary soft links for example makefiles to link.  It is suggested that any applications deployed using shared libraries follow a similar methodology to ensure proper versioning.  Please see the LinuxSoLink script and the example makefiles for a reference. 

##### Static Library Manifest

    Library Name                Package Version  
    -------------               -------------- 
    libema.a                    ema3.3.1.L1
    
##### Shared Library Manifest

    Library Name                Binary Version       Package Version
    -------------               --------------       ----------------
    libema.so.3.3.1.0           libema.so.7          ema3.3.1.L1
    
  
# EMA C++ Issues and Workarounds

- ESDK-421 need infinite timeout support for PostAckTimeout and RequestTimeout in EMA

- ESDK-385 ChannelSet with two multicast channels userQLimit set incorrectly 

- ESDK-395 NiProvider360 application uses 100% CPU when CTRL-C pressed while publishing data

- ESDK-361 When overriding admin messages using addAdminMessage and if the service is down at start-up, the dictionary will not be downloaded properly.

# Reference Information

    I-COS Questionnaire: 6032
    Reuters Item Number: N/A
    Product Name: Elektron Message API - C++ Edition
    Release Number: 3.3.1
    Load Number: 1
    Windows Load ID: ema3.3.1.L1.win
        Supersedes: ema3.3.0.L1.win
    Linux Load ID: ema3.3.1.L1.linux
        Supersedes: ema3.3.0.L1.linux
    Release Status: RRG
    Release Type: RRG
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Thomson Reuters Security Compliant
    Template Version Supported: v4.20.37_TREP_19.61 for RWF and Marketfeed Record Templates

# Notes:
- This package contains APIs that are subject to proprietary and opens source licenses.  Please make sure to read the top level README.md files for clarification.
