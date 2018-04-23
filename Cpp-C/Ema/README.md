

# Elektron Message API


The Elektron Message API: This is an easy-to-use, performant, open source message layer API. The Elektron Message API helps developers by allowing them to develop applications with significantly less code. It is new and will be enhanced by collaboration with customers (through GitHub) and Thomson Reuters based on customer feedback.

EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist.  

(C) Copyright 2018 Thomson Reuters Limited. All rights reserved,
Reuters Oak Brook, IL USA
  


# Message API Features and Functionality



##Consumer Features:

- ADS Multicast: Applications can connect to the ADS Multicast component by specifying the connection type RSSL_RELIABLE_MCAST.  
	  
- RSSL Encrypted and HTTP Connectivity

- Connection Failover: EMA can be configured to specify a list of failover servers via ChannelSet configuration.  In the event that the consumer's connection attempt fails, EMA will utilize the next channel in the ChannelSet list.

- Default Admin Domain Requests: EMA uses default login, directory and dictionary request while connecting to server. This provides minimum configuration for applications to get up and running.
	  
- Configurable Admin Domain Requests:  EMA provides means for modifying the default admin domain requests. 
      
- Tunnel Streams: EMA supports private streams, with additional associated behaviors (e.g., end-to-end authentication, guaranteed delivery, and flow control).
	  
- Batch Request: Application may use a single request message to specify interest in multiple items via the item list
	  
- Dynamic View:	Application may specify a subset of fields or elements of a particular item
	  
- Optimized Pause and Resume: Application may request server to pause and resume item stream
	
- Single Open: EMA supports application selected single open functionality
	  
	
- Programmatic Config	Enables application to programmatically specify and overwrite EMA configuration
	

		
		
##Non-Interactive Provider Features:

- Default Admin Domains: EMA uses default login and directory messages while connecting to server. This provides minimum configuration for applications to get up and running.
 
- Configurable Admin Domains:  EMA provides means for modifying the default admin domain messages. 	


##Interactive Provider Features:

- Default Admin Domains: EMA uses default directory messages while sending to the connected client. This provides minimum configuration for applications to get up and running.
 
- Configurable Admin Domains:  EMA provides means for modifying the default admin domain messages. 	




##Common Features:

- TCP/IP Connectivity

- RMTES Decoder	EMA provides a built in RMTES decoder. IF desired, application may cache RmtesBuffer objects and apply all the received changes to them.
	
- Data::toString()	All OMM containers, primitives and messages may simply be printed out to screen in a standardized output format.
	
- Data::getAsHex()	Applications may obtain binary representations of all OMM containers, primitives and messages.

- File Config:	Enables applications to specify EMA configuration in an EmaConfig.xml file


# Product Content

- CMake files to build EMA library
- EMA Examples
- TREP Dictionary
- Documentation 
	
			
# Documentation

- EMA Developers Guide
- EMA Configuration Guide
- EMA Reference Manual
- EMA RDM Usage Guide
- EMA Examples Cross Reference
- Readme (This File)
- License File
- Test Results
- ESDK C/C++ Migration Guide
	
Elektron Message API Documentation is also available online at https://customers.reuters.com/a/ODL/EMA_C/3.0/HTML_Documentation/index.html 


# Hardware/OS Requirements

      (Linux)
      - HP Intel PC or AMD Opteron (64-bit)
      - AMD Opteron (64-bit)
      - Red Hat Enterprise Linux Advanced Server 6.0 64-bit 
      - Oracle Linux Server 6.0 64-bit (Qualified on RHAS 6.0) **
      - Oracle Linux Server 7.0 64-bit
	  - CentOS 7 64-bit (Qualified on OL7) **

** EMA is qualified on Oracle Linux Server 6.0 64-bit and CentOS 7 64-bit. Qualification was established by building on Red Hat AS 6.0 64-bit and Oracle Linux 7.0 64-bit, and by running the output on OL6 and CentOS 7, respectively. If you want to build source code on OL6 or CentOS 7, refer to the information in ETA’s readme and replicate the OL6 or CentOS 7 build steps from the ETA Makefiles for the EMA components you want to build. 

      (Windows)
      - Intel compatible PC and AMD Opteron for 64-bit
      - CPUs must have high resolution timer frequencies greater than 1GHz.
      - Microsoft Windows Server 2008 (SP1 or greater) 64-bit 
      - Microsoft Windows 7 Professional 64-bit
      - Microsoft Windows 8 Professional 64-bit
      - Microsoft Windows 8.1 Professional 64-bit 
	  - Microsoft Windows 10 Professional 64-bit



# Software Requirements
	
      (Linux)
      - GCC compiler suite version 4.4.4 or higher for RHAS 6.0 (64-bit)
      - GCC compiler suite version 4.4.4 or higher for OLS 6.0 (64-bit)
      - GCC compiler suite version 4.8.2 or higher for OLS 7.0 (64-bit)
      - GCC compiler suite version 4.8.2 or higher for CentOS 7.0 (64-bit)

      (Windows)
      - Microsoft Visual C++ 11.0 64-bit (Visual Studio 2012)
      - Microsoft Visual C++ 12.0 64-bit (Visual Studio 2013)
	  - Microsoft Visual C++ 14.0 64-bit (Visual Studio 2015)
	  - Microsoft Visual C++ 15.0 64-bit (Visual Studio 2017)
	  
      ---------------------------------------------------------
      Enterprise Platform for Real-Time - RSSL/RWF connections
      ---------------------------------------------------------
      - ADS 2.5 or higher
      - ADH 2.5 or higher
	  
       --------
       Elektron
       --------
      - Elektron Deployed
      - Elektron Hosted
      
# Installation and Use

See the top level Elektron-SDK README.md for details.


	  
# Issues and Workarounds

- ESDK-380 If CompressionType is set to "None", the CompressionThreshold range check still occurs
- ESDK-421 need infinite timeout support for PostAckTimeout and RequestTimeout in EMA
- ESDK-455: Shared windows solution file doesn't build ema if the libxml2 library in eta doesn't already exist. 
- ESDK-398 XMLTrace may not flush all information to trace file 
- ESDK-385 ChannelSet with two multicast channels userQLimit set incorrectly 
- ESDK-395 NiProvider360 application uses 100% CPU when CTRL-C pressed while publishing data
- ESDK-374 Random exit issue with NiProvider, application does not exit.
 

# Obtaining the Thomson Reuters Field Dictionaries


The Thomson Reuters `RDMFieldDictionary` and `enumtype.def` files are present in the GitHub repo under `Cpp-C/etc`.

In addition, the most current version can be downloaded from the Customer Zone from the following location.

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - General
- **Products**: TREP Templates Service Pack

# Contributing
Please see the top level **README.md** file for details.


# Reference Information

    I-COS Questionnaire: 6032
    Reuters Item Number: N/A
    Product Name: Elektron Message API - C++ Edition
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Thomson Reuters Security Compliant
	  

# Notes
- Please make sure to review the **LICENSE.md** file.
