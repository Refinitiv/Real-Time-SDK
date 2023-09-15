# Enterprise Message API (EMA) CSharp over .NET Core Edition

The Enterprise Message API: This is an easy-to-use, performant, open source message layer API. The Enterprise Message API helps developers by allowing them to develop applications with significantly less code. It is new and will be enhanced by collaboration with customers (through GitHub) and Refinitiv based on customer feedback.

EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist.  

Copyright (C) 2023 Refinitiv. All rights reserved.
  
# EMA C# Documentation

Documentation is forthcoming

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
		
## Consumer Features:
- Default Admin Domain Requests: EMA uses default login, directory and dictionary request while connecting to server. This provides minimum configuration for applications to get up and running.   

- Connection Failover: EMA can be configured to specify a list of failover servers via ChannelSet configuration.  In the event that the consumer's connection attempt fails, EMA will utilize the next channel in the ChannelSet list.

- Configurable Admin Domain Requests:  EMA provides means for modifying the default admin domain requests

- Batch Request: Application may use a single request message to specify interest in multiple items via the item list

- Dynamic View:	Application may specify a subset of fields or elements of a particular item

- Optimized Pause and Resume: Application may request server to pause and resume item stream

- Single Open: EMA supports application selected single open functionality

- Programmatic Config: Enables application to programmatically specify and overwrite EMA configuration

# EMA CSharp Library and Version Information

    Library Name            Package Version
    ------------            ----------------
    LSEG.Ema.Core.dll       ema3.0.3.L1  <pre-release version> 

# EMA CSharp Issues and Workarounds

    None
 
# Reference Information : TBD

    I-COS Questionnaire: 
    Refinitiv Item Number: N/A
    Product Name: Enterprise Message API - CSharp Edition
    Release Number:  
    Load Number: 
    Load ID:
        Supersedes: 
    Release Status:
    Release Type: 
    US ECCN: 
    EU ECCN:
    Export Code: 
    Security Compliance: Refinitiv Security Compliant
    Template Version Supported: v4.20.59_RealTimeDistributionSystem_23.71 for RWF and Marketfeed Record Templates

# Security

    The components in this package have been scanned using the below software and security scanning products:

    Black Duck by Synopsis, 2022.10.1.1048, https://www.blackducksoftware.com/
    SemGrep 1.2.1, https://semgrep.dev/

# Notes
- This package contains APIs that are subject to proprietary and open source licenses. Please make sure to read the top level README.md files for clarification.
