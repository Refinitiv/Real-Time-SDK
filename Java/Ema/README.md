

# Elektron Message API - Java Edition


The Elektron Message API: This is an easy-to-use, performant, open source message layer API. The Elektron Message API helps developers by allowing them to develop applications with significantly less code. It is new and will be enhanced by collaboration with customers (through GitHub) and Thomson Reuters based on customer feedback.

EMA is written on top of the Elektron Transport API (ETA) utilizing    the Value Added Reactor and Watchlist.  

(C) Copyright 2016 Thomson Reuters Limited. All rights reserved,
Reuters Oak Brook, IL USA
  


# Message API Features and Functionality

##Consumer Features:
- Default Admin Domain Requests: EMA uses default login, directory and 
      dictionary request while connecting to server. This provides minimum 
      configuration for applications to get up and running.   

- Connection Failover: EMA can be configured to specify a list of failover servers via ChannelSet configuration.  In the event that the consumer's connection attempt fails, EMA will utilize the next channel in the ChannelSet list.

- Configurable Admin Domain Requests:  EMA provides means for modifying the default admin domain requests

- Batch Request: Application may use a single request message to specify interest in multiple items via the item list

- Dynamic View:	Application may specify a subset of fields or elements of a particular item

- Optimized Pause and Resume: Application may request server to pause and resume item stream

- Single Open: EMA supports application selected single open functionality
	  



##Non-Interactive Provider Features:

- Default Admin Domains: EMA uses default login and directory messages while connecting to server. This provides minimum configuration for applications to get up and running.

- Configurable Admin Domains:  EMA provides means for modifying the default admin domain messages. 		

##Common Features:

- TCP/IP Connectivity

- Component Versioning: This feature sends information about itself to the connected component.

- RMTES Decoder	EMA provides a built in RMTES decoder. If desired, application may cache RmtesBuffer objects and apply all the received changes to them.

- Data::toString()	All OMM containers, primitives and messages may simply be printed out to screen in a standardized output format. 

- Data::asHex()	Applications may obtain binary representations of all OMM containers, primitives and messages.

- File Config:	Enables applications to specify EMA configuration in an      EmaConfig.xml file

- Direct Write setting on socket channel

- High Water Mark setting on socket channel
		


# Product Content

- EMA libraries [binaries not included in GitHub distribution]
- Ant script files to build EMA library
- EMA Examples
- TREP Dictionary
- Documentation 
- SLF4J logging API libraries
- Apache Commons Configuration libraries 
	
			
# Documentation

- API Concepts Guide
- EMA Developers Guide
- EMA Configuration Guide
- EMA Reference Manual
- EMA RDM Usage Guide
- EMA Examples Cross Reference
- HTML Documentation
- Readme (This File)
- License File
- Test Results
	


# Hardware/OS Requirements

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
      - Microsoft Windows 10 Professional 64-bit



# Software Requirements
	
    ----------------- 
    Core OS Platforms
    ----------------- 
    - Microsoft Windows Server 2008 (SP1 or greater) 64-bit
    - Microsoft Windows Server 2012 Standard 64-bit
    - Microsoft Windows 7 Professional 64-bit
    - Microsoft Windows 8 Professional 64-bit
    - Microsoft Windows 8.1 Professional 64-bit   
    - Microsoft Windows 10 Professional 64-bit   

    - Red Hat Enterprise Linux Advanced Server 6.0 (or grater) 64-bit 
    - Oracle Linux Server 6.0 (or greater) 64-bit 
    - Oracle Linux Server 7.0 (or greater) 64-bit
    - CentOS Linux 7.0 (or greater) 64-bit

    
	-------------
    Core Java VMs  
    -------------
    - Java SE 7 (JDK1.7)
    - Java SE 8 (JDK1.8)

		  
    ---------------------------------------------------------
    Enterprise Platform for Real-Time - RSSL/RWF connections
    ---------------------------------------------------------
    - ADS 2.4 or higher
    - ADH 2.4 or higher
	  
    --------
    EleKtron
    --------
    - EleKtron Deployed
    - EleKtron Hosted
      
      
# Installation and Use

See the top level Elektron-SDK README.md for details.


	  
# Issues and Workarounds
 
- EMA-9 Generic Message is not currently supported on login stream.
- EMA-401 Source directory reissue may internally request on invalid handle
- EMA-437 Batch Request status message itemName not correct.
- EMA-622 Invalid value of compressionThreshold give error and could not connect
- EMA-624 Invalid config for tcpNodelay and REconnectMaxDelay defaults to incorrect defaults.
- EMA-719 CaseId: 04883420 EMA Java throws ClassCastException when configured for HTTP Connection type
- EMA-720 CaseId: 04883420 Broken links in EMA Java ConfigGuide 

# Obtaining the Thomson Reuters Field Dictionaries


The Thomson Reuters `RDMFieldDictionary` and `enumtype.def` files are present in the GitHub repo under `Ema/Etc`.

In addition, the most current version can be downloaded from the Customer Zone from the following location.

https://customers.reuters.com/a/technicalsupport/softwaredownloads.aspx

- **Category**: MDS - General
- **Products**: TREP Templates Service Pack

Place the downloaded `enumtype.def` and `RDMFieldDictionary` under `/Ema/Etc` If these are not present when building some of the applications, their build will fail when they reach the step to copy these. The executable will still be built properly. 

# Contributing
Please see the top level **README.md** file for details.


# Reference Information

    I-COS Questionnaire: 6313
    Reuters Item Number: N/A
    Product Name: Elektron Message API - Java Edition
    US ECCN: EAR99
    EU ECCN: None
    Export Code: NL
    Security Compliance: Thomson Reuters Security Compliant
	  

# Notes
- Please make sure to review the **LICENSE.md** file.
