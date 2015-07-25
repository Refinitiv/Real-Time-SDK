# Elektron Transport API
This is the **Elektron Transport API (ETA)**, the high performance, low latency, foundation of the Elektron SDK. This product allows applications to achieve the highest throughput, lowest latency, low memory utilization, and low CPU utilization when publishing or consuming content. All OMM content and domain models are available through the Elektron Transport API.  


The Transport API is the re-branding of the Ultra Performance API (UPA), which is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data.  All interfaces in ETA are the same as their corresponding interfaces in UPA (same name, same parameter sets) and the transport and codec are fully wire compatible.  


ETA contains both closed source and open source components.  The transport, decoder, encoder, and cache components are closed source and is proprietary to Thomson Reuters.  As a result, the the source code is not included on GitHub. Instead, source code for stub libraries are provided.  The stub libraries provide base implementation of the ETA closed source binaries, which are intended 
to allow users to build the open source portions hosted in this repository.  This 'stub' library will allow you to build and run, but will not provide implementation for connectivity or data handling. 
To get a fully functioning ETA library please see "Obtaining the Transort API Binary Package" below.


# Building the Transport API

This section assumes that the reader has obtained the source from this repository. 
It will contain all of the required source to the Transport API Reactor, its dependencies, and the ETA stub libraries.  
It also includes source code for all example applications, performance measurement applications, and training suite applications to help
users understand how to develop to this API.


####1) Build the Transport API 

**For Linux/Solaris**:

Navigate to `Eta/Impl` 
-	Run `makefile all` to build Stub libraries and Reactor and its dependencies
-	Run `makefile stubs` to build only the Stub libraries
-	Run `makefile rsslVA` to build only Reactor and its dependencies

This will build both static and shared versions of the libraries and will build Optimized libraries by default.  
If Optimized_Assert libraries are preferred, this can be modified from within the makefiles.

**For Windows**:

Navigate to `Eta/Impl` 
Select the `vcxproj` for the specific library you want to build, or use the provided solution (or `sln`) file to build in **Visual Studio**.  

When building via the solution, select the configuration combination you want (Static, Shared, Debug, Release, etc) and select `Build -> Build Solution` this will create both static and shared libraries for all targets.  

If you have the corresponding ETA Binary Package you will not need to build the stubs. Please see "Obtaining the ETA Binary Package" below.

**NOTE:** You must build the stub libraries prior to building the Reactor and its dependencies.

####2) Build the Transport API Examples

Navigate to `Eta/Applications`, locate the example, performance tool, or training suite you would like to build. Run the makefile or open and build the windows solution file (when applicable) or the vcxproj.

####3) Run the ETA Examples

If you have only built the 'stub' library from above, the examples run but fail.  
In order to get full functioning behaviour of ETA you will need to get the official binaries from the Thomson Reuters Customer Zone (http://customers.thomsonreuters.com). 
Please see "Obtaining the ETA Binary Package" below.


# Obtaining the ETA Binary Package

To get the full functionality of the Transport API, please get the official ETA libraries from the following.

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - API
- **Products**: Elektron SDK

Then select the following release, where `<platform>` is linux, win, or solaris based on your preferred platform.

    eta3.0.0.L1.<platform>-binaries.lib
	
Once you have downloaded these libraries, copy them to the corresponding directories under `Eta/Libs`
Additional information about the contents of the closed source ETA libraries are available in the README contained in that distribution package.

**NOTE:** If you are using shared libraries, you will need to run the LinuxSoLink or SolarisSoLink to properly soft link for versioned libraries.  

# Obtaining the Thomson Reuters Field Dictionaries

The Thomson Reuters `RDMFieldDictionary` and `enumtype.def` files are distributed with the ETA Binary Package.  
In addition, they can be downloaded from the Customer Zone from the following location.

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - General
- **Products**: TREP Templates Service Pack

Place the downloaded `enumtype.def` and `RDMFieldDictionary` under `/Eta/etc`
If these are not present when building some of the applications, their build will fail when they reach the step to copy these.  The executable will still be built properly.  

# Developing 

If you discover any issues with regards to this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](Elektron API Individual Contributor License Agreement.pdf)
- [Entity Contributor License Agreement](Elektron API Entity Contributor License Agreement.pdf)

Please email a signed and scanned copy to sdkagreement@thomsonreuters.com.  If you require that a signed agreement has to be physically mailed to us, please email sdkagreement@thomsonreuters.com to request the mailing address.


# Transport API Features and Functionality

- 64-bit, C-based API
- Shared and static library deployments
- Thread safe and thread aware    
- Can consume and provide:
    - Any and all OMM primitives supported on Elektron, Enterprise Platform, and Direct Exchange Feeds.
    - All Domain Models, including those defined by Thomson Reuters as well as other user-defined models.
- Consists of:
    - A transport-level API allowing for connectivity using TCP, HTTP, HTTPS,
         sockets, reliable and unreliable UDP multicast, and Shared Memory.  
    - OMM Encoder and Decoders, allowing full use of all OMM constructs and messages.
	
	-*Transport and Encoder/Decoders are available in all Transport API distributions, with stub implementations available on GitHub.*
- RMTES Support.
      Several structures and functions can be used to process RMTES content 
      and convert to several Unicode formats for interpretation. 
	  
	  -*Available in all Transport API distributions as part of binary libraries.*
- Open Source performance tools:
      Allow users to measure the performance through their system.  Customers 
      can modify the tools to suit their specific needs.  These are found
      in the Value Add portion of this package.
	  
	  -*Available in the full Transport API distribution and on GitHub*
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
		representations of the OMM administrative domain models.  This Value Added Component contains structures that represent the 
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
		
	-*Available in the full Transport API distribution and on GitHub*	   
- DACS library for users to create custom locks for content publishing
	
	-*Available in the full Transport API distribution*
- ANSI library for users to process ANSI Page based content
	
	-*Available in the full Transport API distribution*

####General Capabilities
Transport API provides the following general capabilities independent of the type of application:
- ETA can internally fragment and reassemble large messages.
- ETA applications can pack multiple, small messages into the same 
      network buffer.
- ETA can internally perform data compression and decompression.
- ETA applications can choose their locking model based on need. Locking 
      can be enabled globally, within a connection, or disabled entirely, 
      allowing clients to develop single-threaded, multi-threaded thread safe, 
      or thread-aware solutions.
- ETA applications have full control over the number of message buffers 
      and can dynamically increase or decrease this quantity during runtime.
- ETA does not have configuration file, log file, or message file 
      dependencies: everything is programmatic.
- ETA allows users to write messages at different priority levels, 
      allowing higher priority messages to be sent before lower priority 
      messages.
- ETA applications can create and manage both standard and private 
      data streams.
- ETA Reactor applications can create and manage standard, private,
	  and tunnel streams.

#OMM Application Type Abilities

####Consumer Applications
Users can use Transport API to write consumer-based applications capable of the following:
- Make Streaming and Snapshot based subscription requests.
- Perform Batch, Views, and Symbol List requests to capable provider applications,
      including ADS.
- Pause and Resume active data streams open to the ADS.
- Send Post Messages to capable provider applications, including ADS
      (used for making Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.

####Provider Applications: Interactive
Users can use Transport API to write interactive providers capable of the following:
- Receive requests and respond to Streaming and Snapshot based Requests.
- Receive and respond to requests for Batch, Views, and Symbol Lists.
- Receive requests for Pause and Resume on active Data Streams.
- Receive and acknowledge Post Messages (used when receiving Consumer-based 
	  Publishing and Contributions).
- Send and receive Generic Messages.
- Accept multiple connections, or allow multiple consumers to connect to a provider.

####Provider Applications: Non-Interactive
Users can use Transport APi to write non-interactive applications that start up and begin publishing data to ADH.
- Connect to one or many ADH devices using TCP sockets or reliable UDP multicast, 
	  making only configuration changes. 

####Reactor Based Consumer and Provider Applications
- Reactor applications can take advantage of an event-driven 
      distribution model
- Reactor will manage ping heartbeats and ensure that user
      written content is flushed out as effectively as possible.
- Reactor applications can use the watchlist functionality for
      item recovery, like-request aggregation, fan out, and group status 
      handling.
- Reactor applications can leverage the tunnel streams capability, 
      allowing for a private stream with end-to-end flow control, 
      reliability, authentication, and (when communicating with a Queue
      Provider) persistent queue messaging.


# Notes:
- This package contains APIs that are subject to proprietary and opens source licenses.  Please make sure to read the README.md files within each package for clarification.
- Please make sure to review the LICENSE.md file.
