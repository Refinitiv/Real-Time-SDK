# Enterprise Message API (EMA) Java Edition

The Enterprise Message API: This is an easy-to-use, performant, open source message layer API. The Enterprise Message API helps developers by allowing them to develop applications with significantly less code. It is new and will be enhanced by collaboration with customers (through GitHub) and Refinitiv based on customer feedback.

EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist.  

Copyright (C) 2019-2021 Refinitiv. All rights reserved.
  
# EMA Java Documentation

- Installation Guide
- DevGuide
- RDMUsageGuide
- ConfigGuide
- API_ConceptsGuide
- EMAJExamples

In addtion, HTML documentation is available in Java/Ema/Docs. For addtional documentation, please refer to top level README.MD files.

# EMA Features and Functionality

## Common Features:

- TCP/IP Connectivity

- Component Versioning: This feature sends information about itself to the connected component.

- RMTES Decoder	EMA provides a built in RMTES decoder. If desired, application may cache RmtesBuffer objects and apply all the received changes to them.

- Data::toString() All OMM containers, primitives and messages may simply be printed out to screen in a standardized output format. 

- Data::asHex()	Applications may obtain binary representations of all OMM containers, primitives and messages.

- File Config:	Enables applications to specify EMA configuration in an EmaConfig.xml file

- Direct Write setting on socket channel

- High Water Mark setting on socket channel
		
- Parameters, reconnectAttemptLimit, reconnectMinDelay, reconnectMaxDelay, xmlTraceEnable, MsgKeyInUpdates only can be configured on Consumer/IProvider/NiProvider instance level.
	
## Consumer Features:
- Default Admin Domain Requests: EMA uses default login, directory and dictionary request while connecting to server. This provides minimum configuration for applications to get up and running.   

- Connection Failover: EMA can be configured to specify a list of failover servers via ChannelSet configuration.  In the event that the consumer's connection attempt fails, EMA will utilize the next channel in the ChannelSet list.

- Configurable Admin Domain Requests:  EMA provides means for modifying the default admin domain requests

- Batch Request: Application may use a single request message to specify interest in multiple items via the item list

- Dynamic View:	Application may specify a subset of fields or elements of a particular item

- Optimized Pause and Resume: Application may request server to pause and resume item stream

- Single Open: EMA supports application selected single open functionality

- Programmatic Config: Enables application to programmatically specify and overwrite EMA configuration

## Non-Interactive Provider Features:

- Default Admin Domains: EMA uses default login and directory messages while connecting to server. This provides minimum configuration for applications to get up and running.

- Configurable Admin Domains: EMA provides means for modifying the default admin domain messages. 		

- Programmatic Config: Enables application to programmatically specify and overwrite EMA configuration

## Interactive Provider Features:

- Default Admin Domains: EMA uses default directory messages while sending to the connected client. This provides minimum configuration for applications to get up and running.
 
- Configurable Admin Domains:  EMA provides means for modifying the default admin domain messages. 

- Programmatic Config	Enables application to programmatically specify and overwrite EMA configuration

# EMA Java Library and Version Information

    Library Name            Package Version
    ------------            ----------------
    ema-3.6.2.0.jar         ema3.6.2.L1

# EMA Java Issues and Workarounds
 
- ESDK-475 When using DirectoryMsg.decode(), Source Directory FilterActions of Update may be overwritten with Set

- ESDK-312 Watchlist fans out Dictionary state of Open/Suspect state instead of Closed/Recover

- Users of encrypted tunneling connection type may encounter trust issues with DigiCert certificates. JRE8 update 91 and higher support DigiCert certificates. Users can upgrade to a higher JRE version if they encounter problems.

- EMA can not download dictionary from a Refinitiv Real-Time Distribution System over a Websocket connection using the tr_json2/rssl_json protocol. This is a limitation of the simplied JSON protocol.

- The RWF/JSON Converter library does not support groupID property of RWF message when using Websocket Transport with JSON data format.

# Reference Information

    I-COS Questionnaire: 6313
    Refinitiv Item Number: N/A
    Product Name: Enterprise Message API - Java Edition
    Release Number: 3.6.2
    Load Number: 1
    Load ID: ema3.6.2.L1.java
        Supersedes: ema3.6.1.L2.java
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

# Notes
- This package contains APIs that are subject to proprietary and opens source licenses.  Please make sure to read the top level README.md files for clarification.
- Java unit tests may use [Mockito](http://site.mockito.org/) for creation of mock objects. Mockito is distributed under the MIT license.
