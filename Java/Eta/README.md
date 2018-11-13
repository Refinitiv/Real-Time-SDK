# Elektron Transport API
This is the **Elektron Transport API (ETA)**, the high performance, low latency, foundation of the Elektron SDK. This product allows applications to achieve the highest throughput, lowest latency, low memory utilization, and low CPU utilization when publishing or consuming content. All OMM content and domain models are available through the Elektron Transport API.  


The Transport API is the re-branding of the Ultra Performance API (UPA), which is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data.  All interfaces in ETA are the same as their corresponding interfaces in UPA (same name, same parameter sets) and the transport and codec are fully wire compatible.  


ETA Java contains open source components.  The transport, decoder, encoder, and cache components are open source. 
This repository depends on the `Elektron-SDK-BinaryPack` (http://www.github.com/thomsonreuters/Elektron-SDK-BinaryPack) repository and pulls ETA dependent libraries from that location.


####Supported Platforms
ETA Java can be used with a J2SE7 or J2SE8 compilant JVM.  
These can be downloaded from http://www.oracle.com 

At the current time, the JNI libraries support the following platform/compiler combinations:

- RedHat Advanced Server 6.X 64-bit (gcc4.4.4)
- Oracle Linux Server 6.X 64-bit (gcc4.4.4)
- Oracle Linux Server 7.X 64-bit (gcc4.8.2)
- CentOS 7.X 64-bit (gcc4.8.2)
- Windows 7 64-bit
- Windows 8 64-bit
- Windows 8.1 64-bit
- Windows 10 64-bit
- Windows Server 2008 64-bit
- Windows Server 2012 64-bit
- Windows Server 2016 64-bit

Compilers: Visual Studio 11 (2012), Visual Studio 12 (2013), Visual Studio 14 (2015), Visual Studio 15 (2017)

Users are welcome to migrate open source code to the platforms they prefer, however support for the included ETA libraries are only provided on platforms captured in the README file.

# Obtaining the Thomson Reuters Field Dictionaries

The Thomson Reuters `RDMFieldDictionary` and `enumtype.def` files are present in the GitHub repo under `Java/etc`.  
In addition, the most current version can be downloaded from the Customer Zone from the following location.

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - General
- **Products**: TREP Templates Service Pack

# Documentation

Elektron Transport API Documentation is available online at https://developers.thomsonreuters.com/elektron/elektron-sdk-java/docs 

These are also available as part of the full Elektron SDK package that can be downloaded from the the following locations. 


**Developer Community:**

https://developers.thomsonreuters.com/

Then select the following options:

- **APIs by Product**: Elektron
- **APIs in this Family**: Elektron SDK - Java Edition
- **Downloads**: ETA - Java - LATEST VERSION

**Customer Zone:**

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - API
- **Products**: Elektron SDK



# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](Elektron API Individual Contributor License Agreement.pdf)
- [Entity Contributor License Agreement](Elektron API Entity Contributor License Agreement.pdf)

Please email a signed and scanned copy to sdkagreement@thomsonreuters.com.  If you require that a signed agreement has to be physically mailed to us, please email sdkagreement@thomsonreuters.com to request the mailing address.


# Transport API Features and Functionality

- 64-bit, Java-based API
- Thread safe and thread aware    
- Can consume and provide:
    - Any and all OMM primitives supported on Elektron, Enterprise Platform, and Direct Exchange Feeds.
    - All Domain Models, including those defined by Thomson Reuters as well as other user-defined models.
- Consists of:
    - A transport-level API allowing for connectivity using TCP, HTTP, HTTPS,
         sockets, reliable and unreliable UDP multicast, and Shared Memory.  
    - OMM Encoder and Decoders, allowing full use of all OMM constructs and messages.
- RMTES Support.
	-	Several structures and functions can be used to process RMTES content and convert to several Unicode formats for interpretation. 
	  
	  -*Available in all Transport API distributions as part of binary libraries.*
- Open Source performance tools:
	-	Allow users to measure the performance through their system.  Customers can modify the tools to suit their specific needs.  These are found in the Value Add portion of this package.
	  
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
		representations of the OMM administrative domain models. This
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
		
	-*Available in the full Transport API distribution and on GitHub*	   
- DACS library for users to create custom locks for content publishing
	
	-*Available in the full Transport API distribution*
- ANSI library for users to process ANSI Page based content
	
	-*Available in the full Transport API distribution*

####General Capabilities
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

#OMM Application Type Abilities

####Consumer Applications
Users can use Transport API to write consumer-based applications capable of the following:
- Make Streaming and Snapshot based subscription requests.
- Perform Batch, Views, and Symbol List requests to capable provider applications, including ADS.
- Pause and Resume active data streams open to the ADS.
- Send Post Messages to capable provider applications, including ADS
(used for making Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.

####Provider Applications: Interactive
Users can use Transport API to write interactive providers capable of the following:
- Receive requests and respond to Streaming and Snapshot based Requests.
- Receive and respond to requests for Batch, Views, and Symbol Lists.
- Receive requests for Pause and Resume on active Data Streams.
- Receive and acknowledge Post Messages
(used when receiving Consumer-based Publishing and Contributions).
- Send and receive Generic Messages.
- Accept multiple connections, or allow multiple consumers to connect to a provider.

####Provider Applications: Non-Interactive
Users can use Transport API to write non-interactive applications that start up and begin publishing data to ADH.
- Connect to one or many ADH devices using TCP sockets or reliable UDP multicast, making only configuration changes.

####Reactor Based Consumer and Provider Applications
- Reactor applications can take advantage of an event-driven distribution model
- Reactor will manage ping heartbeats and ensure that user written content is flushed out as effectively as possible.
- Reactor applications can use the watchlist functionality for item recovery, like-request aggregation, fan out, and group status handling.
- Reactor applications can leverage the tunnel streams capability, allowing for a private stream with end-to-end flow control, reliability, authentication, and (when communicating with a Queue Provider) persistent queue messaging.


# Notes:
- This package contains APIs that are subject to proprietary and opens source licenses.  Please make sure to read the README.md files within each package for clarification.
- Please make sure to review the LICENSE.md file.
- Java unit tests may use Mockito (http://site.mockito.org/) for creation of mock objects. Mockito is distributed under the MIT license.
