This is the change log of the Real-Time SDK (RTSDK), CSharp edition. RTSDK consists of Enterprise Message API (EMA) and Enterprise Transport API (ETA). This file contains history starting with initial release of CSharp. Note that RTSDK product version numbers start from 2.0.8 and EMA/ETA version numbers start from 3.0.0.

There are three types of RTSDK releases that append a letter directly followed by a number to the version number. 

"L" releases (e.g., 2.0.8.L1) are full RTSDK releases that are uploaded to 
    Customer Zone, Developer Community and GitHub. 
"G" releases (e.g., 2.0.8.G1) are releases that are only uploaded to GitHub. 
"E" releases (E-Loads) are emergency RTSDK releases that are uploaded to 
    Customer Zone and Developer Community but not to GitHub.
    Also note that emergency releases may only be partial (i.e., CSharp, Java or C++/C only).

----------------------------------------------------------------------------------------
CURRENT RELEASE HIGHLIGHTS - RTSDK CSharp 2.2.3.G2 aka EMA/ETA 3.3.1.G2 aka 3.3.1.1
----------------------------------------------------------------------------------------

This is a maintenance release which incorporates a GitHub Pull request.

Customer Issues Resolved
----------------------------------------------------------------------------------------
- [GitHub Pull Request #300, GitHub #301] - [RTSDK-9563] - GitHub Pull Request to fix DirectoryConfigMap property should be used instead of DictionaryConfigMap 

----------------------------------------------------------------------------------------
FULL CHANGELOG
----------------------------------------------------------------------------------------

---------------------------------------------
RTSDK CSharp Release 2.2.3.G2 (Mar 21, 2025)
---------------------------------------------

EMA CSharp 3.3.1.G2/3.3.1.1 Issues Resolved
-------------------------------------------
- [RTSDK-9563] - GitHub Pull Request to fix DirectoryConfigMap property should be used instead of DictionaryConfigMap [GitHub Pull Request #300, GitHub #301]

---------------------------------------------
RTSDK 2.2.3.G1 Release: No changes for CSharp
---------------------------------------------

---------------------------------------------
RTSDK CSharp Release 2.2.3.L1 (Dec 13, 2024)
---------------------------------------------

This is a maintenance release with support for Windows 11, customer issue fixes and other minor enhancements. Note that starting with this version, EMA configuration is validated/enforced using an embedded schema file.

EMA CSharp 3.3.1.L1 Issues Resolved
-----------------------------------
- [RTSDK-9169] - EMA C# Lack of thread safety when decoding complex types which is created by application
- [RTSDK-8524] - Enforce EmaConfig validation using embedded schema file

ETA CSharp 3.3.1.L1 Issues Resolved
-----------------------------------
- [RTSDK-7092] - Some unit tests are failing when executed as a batch on Windows platform
- [RTSDK-9109] - Unittest LSEG.Eta.Transports.Tests.TransportMessageTests.test7_encrypted failed
- [RTSDK-9142] - ETA C# RMTESToUTF8 does not work [GitHub #289]

Both ETA and EMA CSharp 3.3.1.L1 Issues Resolved
------------------------------------------------
- [RTSDK-8244] - Qualification on Windows 11 [Case Number: 13278230] 
- [RTSDK-8510] - Rebranding to LSEG in CSharp code: URLs, unit tests, etc.
- [RTSDK-9073] - Rebranding to LSEG in CSharp documentation
- [RTSDK-9198] - Update to C# dependencies: NLog, LZ4, Tokens, xunit
- [RTSDK-9306] - EMA C# | memory growth: Fix was to un-reference view handling objects from WlItemRequest and WlItemStream. Also it was to create m_PostIdToMsgTable and m_PostTimeoutInfoPool as needed instead of defaults in the WlStream class.

---------------------------------------------
RTSDK CSharp Release 2.2.2.L1 (Sep 30, 2024)
---------------------------------------------

This is a maintenance release with minor enhancements and fixes. This release introduces an EMA schema file to reflect Ema configuration. Note that the schema file is not enforced in this version.

EMA CSharp 3.3.0.L1 Issues Resolved 
-----------------------------------
- [RTSDK-8225] - EMACSharp should convey dispatch errors from Reactor via error callback to EMA applications
- [RTSDK-8352] - Create and ship a schema file that reflects Ema Configuration
- [RTSDK-8422] - Create QATool Series300Consumer360-MultiThreadViews-001
- [RTSDK-8570] - Added NIProvider examples: NIProv_250_MP_Perf & NIProv_251_MP_Perf_KeyByServiceId
- [RTSDK-8594] - Minor fix to ConsPerf's Usage
- [RTSDK-8615] - Fix for REST proxy parameters in file config
- [RTSDK-8654] - Added minimum timeout to avoid high CPU on Linux with API Dispatch
- [RTSDK-8852] - EMACSharp ReconnectMinDelay default is changed to 5000 from 1000

ETA CSharp 3.3.0.L1 Issues Resolved
-----------------------------------
- [RTSDK-7167] - Added ETA example readme to explain dependencies
- [RTSDK-8568] - Update solution to include build of AuthLock
- [RTSDK-9033] - CSharp Reactor.Watchlist throws NullReferenceException

Both ETA and EMA CSharp 3.3.0.L1 Issues Resolved
------------------------------------------------
- [RTSDK-6994] - Support for .NET Core 8.x
- [RTSDK-7213] - Qualify RTSDK C# on RedHat 9.X
- [RTSDK-8552] - Rebranding to LSEG in PDF documentation
- [RTSDK-8742] - Delivery of separate BinaryPack RRG archive and Library RRG archive
- [RTSDK-8745] - Rebrand references in code to LSEG
- [RTSDK-8883] - Change default build to net8.0 with update to Directory.Build.props
- [RTSDK-8992] - Support for Windows 2022 Server

---------------------------------------------
RTSDK CSharp Release 2.2.1.L1 -- SKIPPED for C# 
---------------------------------------------

---------------------------------------------
RTSDK CSharp Release 2.2.0.G1 (May 17, 2024)
---------------------------------------------

EMA CSharp 3.2.0.G1 Issues Resolved 
-----------------------------------
- [RTSDK-8578] - IndexOutOfRangeException when NIProvider submits very large source directory to ADH after the connection is recovered
- [RTSDK-8606] - ObjectDisposedException exception is thrown from the Socket.Select() method after the socket is disposed  

Both ETA and EMA CSharp 3.2.0.G1 Issues Resolved
------------------------------------------------
- [RTSDK-8579] - EMA C# Memory growth with multiple OmmConsumer with Initialization/Uninitialization [GitHub #278]

---------------------------------------------
RTSDK CSharp Release 2.2.0.L1 (Apr 30, 2024)
---------------------------------------------

This release introduces support for Enterprise Message API (EMA) Interactive and Non-Interative Providers. In addtion, several customer issues were addressed. Also included are the following features:
- Abilty to set proxy for REST requests separately from Reactor channel proxy
- Ability to add connection type to OmmConsumerConfig
- Ability to pass in a dictionary object into a newly created OMMConsumer
- Support for XmlTrace to file parameters in ETA & EMA C#
- Support for DirectWrite option in ETA Reactor and EMA
- Introduction of DACSLock library and AuthLock example

----------------------------------------------------------------------------------------

EMA CSharp 3.2.0.L1 New Features 
--------------------------------
- [RTSDK-7711] - OmmNiProvider Implementation: OmmNiProvider, ChannelInformation, IOCTL, Domain handlers, etc.
- [RTSDK-7728] - NIProvider Configuration (file, programmatic, function), Unit Testing
- [RTSDK-7731] - NIProvider Examples: 100 Series & QATools
- [RTSDK-7732] - NIProvider Examples: 200 Series & QATools
- [RTSDK-7733] - NIProvider Examples: 300 Series & QATools
- [RTSDK-7734] - NIProvider Examples: 400 Series & QATools
- [RTSDK-7735] - Create NIProvPerf
- [RTSDK-7901] - OmmIProvider Implementation: Configuration, OmmIProvider, ChannelInformation, IOCTL, Domain Handlers, etc.
- [RTSDK-7736] - IProvider Configuration (file, programmatic, function), Unit Testing
- [RTSDK-7911] - IProvider Series 100 Examples & QATools
- [RTSDK-7912] - IProvider Series 200 Examples & QATools
- [RTSDK-7913] - IProvider Series 300 Examples & QATools
- [RTSDK-7914] - IProvider Series 400 Examples & QATools
- [RTSDK-7915] - Create EMA ProvPerf Tool
- [RTSDK-8196] - Support Message Packing

EMA CSharp 3.2.0.L1 Issues Resolved
-----------------------------------
- [RTSDK-7746] - Add connection type to OmmConsumerConfig [GitHub #248]
- [RTSDK-7757] - Added toString implementation for the containers and messages in EMA C#
- [RTSDK-7763] - Enhancement to pass in a dictionary into OMMConsumer
- [RTSDK-8296] - Decoding issue with blank OmmArray in FieldEntry [GitHub #269] 
- [RTSDK-8324] - EMA: Make it possible to use logging options from NLOG configuration [GitHub #257]
- [RTSDK-8436] - Fixed OmmConsumerConfig.Clear() to reload Ema configuration file after clearing the existing configurations
- [RTSDK-8529] - Failed to call Unregister method on the OmmConsumer (fix to IOCtl) [GitHub #277] 

ETA CSharp 3.2.0.L1 Issues Resolved
-----------------------------------
- [RTSDK-7617] - Modified ETA examples to always set CredentialRenewal callback to demonstrate secure ways to provide credentials
- [RTSDK-7786] - Remove all references to the HTTP transport type in ETA C# (HTTP is not supported)
- [RTSDK-8553] - Client connected CompressionType does not follow Server's CompressionType when BindOptions.ForceCompression is set to true.

Both ETA and EMA CSharp 3.2.0.L1 Issues Resolved
------------------------------------------------
- [RTSDK-7008] - Introduction of DACSLock library and AuthLock example 
- [RTSDK-7594] - Enhancement for ability to specify proxy separately for REST requests versus reactor channels
- [RTSDK-7947] - Updated dependency versions: nlog, microsoft.identitymodel, microsoft.netcore, system.identitymodel, system.security
- [RTSDK-7977] - Support for XmlTrace to file parameters in ETA & EMA C#
- [RTSDK-7984] - ETA Reactor & EMA: Support DirectWrite option
- [RTSDK-8051] - Address issues found by Coverity scan
- [RTSDK-8179] - Update to ConsPerf applications to add commandline option to specify TLS version

---------------------------------------------
RTSDK CSharp Release 2.1.3.L1 (Nov 6, 2023)
---------------------------------------------

This release introduces Enterprise Tranport API (ETA) C# watchlist support and Enterprise Message API (EMA) C# client side implementation. Included is support for socket encrypted and unencrypted connections, session management feature for Real-Time - Optimized (RTO) connectivity, round trip latency monitoring, and features required to consume/contribute content on multiple domains: batch, view, snapshot/streaming, etc.

EMA CSharp 3.1.0.L1 
-------------------
- [RTSDK-7261] - EMA C# implementation
- [RTSDK-7509] - EMA C#: Create EMA ConsPerf Tool
- [RTSDK-7529] - EMA C#: Create EMA Examples: 100, 200, 300, 400 Series
- [RTSDK-7510] - EMA C#: Support session management feature with V2 authenticaiton and service discovery 
- [RTSDK-7613] - Create CSharp PDF documentation
- [RTSDK-7919] - Create EMA reference manual

ETA CSharp 3.1.0.L1 Issues Resolved
-----------------------------------
- [RTSDK-6752] - ETA C# watchlist implementation
- [RTSDK-7097] - ETA C# WL: Create watchlist consumer example
- [RTSDK-7193] - ETA C# WL: Update ConsPerf tool with -watchlist option
- [RTSDK-7194] - ETA C# WL: Performance optimizations
- [RTSDK-7278] - ETA C# WL: Refactor initial timeout timer implementation to avoid locking
- [RTSDK-7350] - Fix to WLConsumer and VAConsumer command lines to accept -tokenURLV2 instead of -tokenURL
- [RTSDK-7502] - Fix to ProvPerf to show error when MsgData.xml is not found
- [RTSDK-7520] - ETA C# EncodeIterator.RealignBuffer doesn't work as expected
- [RTSDK-7540] - Fix to C# VAProvider to avoid rejecting a second request for TRI where the 1st request was PRIVATE
- [RTSDK-7609] - Fix to VAProvider to change -key to -keyfile to align with other Provider examples
- [RTSDK-7630] - Fix to non-ASCII characters encoding/decoding in ETA C#

Both ETA and EMA CSharp 3.1.0.L1 Issues Resolved
------------------------------------------------
- [RTSDK-7342] - Support for TLS 1.3

---------------------------------------------
RTSDK CSharp Release 2.1.2.L1 (Sep 8, 2023)
---------------------------------------------

This is a maintenance release with fixes.

ETA CSharp 3.0.3.L1  
-------------------
- [RTSDK-6261] - Qualify RTSDK API on Ubuntu Kylin
- [RTSDK-7272] - Qualify RTSDK API on Ubuntu
- [RTSDK-7505] - ProvPerf example does not include name in message key for item refresh messages
- [RTSDK-7517] - Training example ConsMod5 decodes Source Directory incorrectly
- [RTSDK-7574] - VAProvider does NOT set Login Element for SupportEnhancedSymbolList

---------------------------------------------
RTSDK CSharp Release 2.1.1.L1 (Jun 9, 2023)
---------------------------------------------

This is a maintenance release with fixes. In addition, this release introduces a self-contained RRG package that contains external dependencies required for builds. This is in contrast to last release where dependencies were auto-downloaded from NuGet. Note that building GitHub source will continue to auto-download external dependencies. 

ETA CSharp 3.0.2.L1  
-------------------
- [RTSDK-7333] - Minor edits to Perftools Guide
- [RTSDK-7169] - Self-contained RRG Package for CSharp
- [RTSDK-7316] - Consumer application cannot decode ACK message on streamId=1
- [RTSDK-7352] - VAProvider sends STATUS msg with domainType MARKET_PRICE for SYMBOL_LIST
- [RTSDK-7421] - ConsPerf code cleanup
- [RTSDK-7495] - Modified Reactor to support setting JWK string for ReactorOAuthCredential.ClientJwk

---------------------------------------------
RTSDK CSharp Release 2.1.0.L1 (Mar 15, 2023)
---------------------------------------------

This release introduces client credentials with jwt authentication for connectivity to LSEG Real-Time Optimized. Ability to obtain service accounts to use this authentication mechanism is forthcoming.

ETA CSharp 3.0.1.L1  
-------------------
- [RTSDK-6441] - Support V2 authentication: Support client credential and client credential with JWT flow in ETA Reactor
- [RTSDK-6730] - V2 authentication: Add a separate string called "Audience" with ability to override value for JWT flow
- [RTSDK-6748] - Addition of ANSI page codec library 
- [RTSDK-7057] - ETA.NET: Added RDMUsage guide and made fixes to Install Guide

---------------------------------------------
RTSDK CSharp Release 2.0.8.L1 (Jan 23, 2023)
---------------------------------------------

New Features Added
------------------
This is the first official RTSDK CSharp release with support for Enterprise Transport API. This initial release includes transport and value add Reactor layers with both client and server side implemenation. The transport layer API supports TCP/IP transport, buffer management (such as read, write), fragmentation, packing, compression, a codec to implement open message model (OMM). Transport layer supports socket and encrypted socket connections. In addition, the value add layer handles adminitrative messages and implements a dispatching/callback mechanism to simplify the application. Also included in the API at Reactor Layer are Session Management (authentication) and Service Discovery (discovering host/port information based on Cloud region and type of connection) features. Documentation is available for all supported features in two formats: HTML (reference manuals) and PDF. 

ETA CSharp 3.0.0.L1  
-------------------
- BIZELEK-894 Design and implement ETA CSharp Transport and Value Add Reactor layers
- RTSDK-5865 ETACSharp, Transport: Implement flush buffer to channel
- RTSDK-5866 ETACSharp, Transport: Implement tracing to dump message form/to network
- RTSDK-5867 ETACSharp, Transport: Implement RIPC protocol for Fragmented data message layout
- RTSDK-5868 ETACSharp, Transport: Implement RIPC protocol for Compressed data message layout
- RTSDK-6020 ETACSharp, Transport: Implement RIPC protocol for packing messages into a buffer
- RTSDK-5871 ETACSharp, Transport: Server side implementation (accept)
- RTSDK-5872 ETACSharp, Transport: Implement Encrypted socket
- RTSDK-5894 ETACSharp: RMTES parser functionality
- RTSDK-5870 ETACSharp, Example: Create consumer example
- RTSDK-5877 ETACSharp, Example: Create provider example
- RTSDK-5878 ETACSharp, Example: Create non interactive provider example
- RTSDK-5874 ETACSharp, Example: Create TransportTest
- RTSDK-5875 ETACSharp, Example: Create TransportPerf
- RTSDK-6178 ETACSharp, Example: Create ETACSharp Consumer training examples
- RTSDK-6253 ETACSharp, Example: Create ETACSharp Provider training examples
- RTSDK-6313 ETACSharp, Example: Create ETACSharp Non-Interactive Provider training examples
- RTSDK-6173 ETACSharp, Example: Create ETACSharp ConsPerf test tool
- RTSDK-6174 ETACSharp, Example: Create ETACSharp ProvPerf test tool
- RTSDK-6175 ETACSharp, Example: Create ETACSharp NIProvPerf test tool
- RTSDK-6176 ETACSharp, Reactor: Design and implement connection handling in the Reactor component
- RTSDK-6342 ETACSharp, Reactor: Login Domain Handling
- RTSDK-6343 ETACSharp, Reactor: Source Directory Domain Handling
- RTSDK-6344 ETACSharp, Reactor: Dictionary Domain Handling
- RTSDK-6440 ETACSharp, Reactor: Implement Session Management for oAuthClientCredentials authentication
- RTSDK-6361 ETACSharp, Reactor, Example: Create VAConsumer example
- RTSDK-6362 ETACSharp, Reactor, Example: Create VAProvider example
- RTSDK-6424 ETACSharp, Reactor, Example: Create VANiProvider example
- RTSDK-6573 ETACSharp, Reactor, Example: Support -reactor option with PerfTools: ConsPerf, ProvPerf, NIProvPerf
- RTSDK-6464 ETACSharp: Generate documentation for CSharp Transport & Reactor
- RTSDK-5915 ETACSharp: Create Distribution files for RRG Package
