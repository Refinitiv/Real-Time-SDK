(C) Copyright 2015 Thomson Reuters Limited. All rights reserved,
Reuters Oak Brook, IL USA
 
CONTENTS
============================
 
1.    REFERENCE INFORMATION
2.    RELEASE OVERVIEW
3.    DOCUMENTATION
4.    INTEROPERABILITY
5.    INSTALLATION
6.    ISSUES AND WORKAROUNDS
7.    OBTAINING THE THOMSON REUTERS FIELD DICTIONARIES



1. REFERENCE INFORMATION
=============================

    I-COS Questionnaire: 6032
    Reuters Item Number: N/A
    Product Name: Elektron Message API - C++ Edition
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Thomson Reuters Security Compliant
    Hardware Platform: Intel or AMD 
    Operating System: 
       (linux)
          - Red Hat Enterprise Linux Advanced Server 
          - Oracle Linux 
          - CentOS
		   
       (windows)
          - Windows 8, Windows 7, Server 2012 & Server 2008 
		  
    Supersedes: n/a
    Template Version Supported: v4.20.19_TREP_15.61 RWF and Marketfeed Record
                                Templates



2. RELEASE OVERVIEW
===================

2.1 Release Purpose

    This is the initial release of the Elektron Message API (EMA).
	
    The EMA is an ease of use, viewable source, OMM API. EMA is designed to 
    provide clients rapid development of applications, minimizing lines of code
    and providing a broad range of flexibility. It provides flexible configuration
    with default values to simplify use and deployment. 
    
    EMA is written on top of the Elektron Transport API (ETA) utilizing 
    the Value Added Reactor and Watchlist.  NOTE that ETA is a re-brand of the Ultra
    Performance API (UPA).
	
	License information can be found in the included LICENSE.MD file.


    Features Supported:
    -------------------
    - Default Admin Domain Requests: EMA uses default login, directory and 
      dictionary request while connecting to server. This provides minimum 
      configuration for applications to get up and running. 
	  
    - Configurable Admin Domain Requests:  EMA provides means for modifying the
      default admin domain requests. 
      
    - Tunnel Streams: EMA supports private streams, with additional associated 
      behaviors (e.g., end-to-end authentication, guaranteed delivery, and
      flow control).
	  
    - Batch Request: Application may use a single request message to specify 
      interest in multiple items via the item list
	  
    - Dynamic View:	Application may specify a subset of fields or elements of a 
      particular item
	  
    - Optimized Pause and Resume: Application may request server to pause and 
      resume item stream
	
    - Single Open: EMA supports application selected single open functionality
	  
    - RMTES Decoder	EMA provides a built in RMTES decoder. IF desired, application
      may cache RmtesBuffer objects and apply all the received changes to them.
	
    - Data::toString()	All OMM containers, primitives and messages may simply be
      printed out to screen in a standardized output format. 
	
    - Data::getAsHex()	Applications may obtain binary representations of all OMM 
      containers, primitives and messages.
	
    - Programmatic Config	Enables application to programmatically specify and 
      overwrite EMA configuration
	
    - File Config:	Enables applications to specify EMA configuration in an 
      EmaConfig.xml file
		
		

2.2 Product Content

	- Makefile/Windows project files to build EMA library
	- EMA Examples
	- TREP Dictionary
	- libxml2 [binaries not included in GitHub distribution]
	- Documentation 
	
			
3.  DOCUMENTATION
============================

3.1 EMA Documentation

    - EMA Developers Guide
    - EMA Configuration Guide
    - EMA Reference Manual
    - EMA RDM Usage Guide
    - Readme (This File)
    - License File
    - Test Results
	
	
	
	
5.  INSTALLATION
============================
 
5.1 Hardware/OS Requirements

      (Linux)
      - HP Intel PC or AMD Opteron (64-bit)
      - AMD Opteron (64-bit)
      - Red Hat Enterprise Linux Advanced Server 6.0 64-bit 
      - Oracle Linux Server 6.0 64-bit (Qualified on RHAS 6.0)
      - Oracle Linux Server 7.0 64-bit
	  - CentOS 7 64-bit (Qualified on OL7)

      (Windows)
      - Intel compatible PC and AMD Opteron for 64-bit
      - CPUs must have high resolution timer frequencies greater than 1GHz.
      - Microsoft Windows Server 2008 (SP1 or greater) 64-bit 
      - Microsoft Windows 7 Professional 64-bit
      - Microsoft Windows 8 Professional 64-bit
      - Microsoft Windows 8.1 Professional 64-bit 



5.2 Software Requirements
	
      (Linux)
      - GCC compiler suite version 4.4.4 or higher for RHAS 6.0 (64-bit)
      - GCC compiler suite version 4.4.4 or higher for OLS 6.0 (64-bit)
      - GCC compiler suite version 4.8.2 or higher for OLS 7.0 (64-bit)
      - GCC compiler suite version 4.8.2 or higher for CentOS 7.0 (64-bit)

      (Windows)
      - Microsoft Visual C++ 10.0 64-bit (visual Studio 2010)
      - Microsoft Visual C++ 11.0 64-bit (Visual Studio 2012)
      - Microsoft Visual C++ 12.0 64-bit (Visual Studio 2013)
	  

5.3 Installation and Use

      See the top level Elektron-SDK README.md for details.


	  
6.  ISSUES AND WORKAROUNDS
============================
    - EMACPP-414: EMA Consumer needs to indicate channel disconnects with the login status message
    - EMACPP-273: RMTES decoding errors (eg. buffer too small are not passed correctly from underlying API)
    - EMACPP-354: Source directory reissue and genmsg on source directory is not currently supported.
    - EMACPP-100: Generic Message is not currently supported on login stream.
    - EMACPP-422: Tunnel Streams: If the provider messages exceeds the recvWindowSize no message are received.
    - EMACPP-407: Consumer does not exit on reaching reconnectAttemplLimit
    - EMACPP-412: Cannot pass decoded complex type objects into encoding methods of complex types.
	- EMACPP-444: Cannot open emaconfig.xml file from example project files on windows 

7. OBTAINING THE THOMSON REUTERS FIELD DICTIONARIES
===================================================

The Thomson Reuters `RDMFieldDictionary` and `enumtype.def` files are present in the GitHub repo under `Ema/Etc`.

In addition, the most current version can be downloaded from the Customer Zone from the following location.

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - General
- **Products**: TREP Templates Service Pack

Place the downloaded `enumtype.def` and `RDMFieldDictionary` under `/Ema/Etc` If these are not present when building some of the applications, their build will fail when they reach the step to copy these. The executable will still be built properly. 
