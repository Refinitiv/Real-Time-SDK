This is the change log of the Refinitiv Real-Time SDK (RTSDK), CSharp edition. RTSDK consists of both the Elektron Message API (EMA) and the Elektron Transport API (ETA). For CSharp ETA is delivered and EMA is forthcoming. This file contains history starting with initial release of CSharp. Note that RTSDK product version numbers start from 2.0.8 and EMA/ETA version numbers start from 3.0.0.

There are three types of RTSDK releases that append a letter directly followed by a number to the version number. 

"L" releases (e.g., 2.0.8.L1) are full RTSDK releases that are uploaded to 
    Customer Zone, Developer Community and GitHub. 
"G" releases (e.g., 2.0.8.G1) are releases that are only uploaded to GitHub. 
"E" releases (E-Loads) are emergency RTSDK releases that are uploaded to 
    Customer Zone and Developer Community but not to GitHub.
    Also note that emergency releases may only be partial (i.e., CSharp, Java or C++/C only).

----------------------------------------------------------------------------------------
CURRENT RELEASE HIGHLIGHTS - RTSDK CSharp 2.1.0.L1 aka EMA/ETA 3.0.1.L1 aka 3.0.1.0
----------------------------------------------------------------------------------------

New Features Added
------------------

This release introduces client credentials with jwt authentication for connectivity to Refinitiv Real-Time Optimized. Ability to obtain service accounts to use this authentication mechanism is forthcoming.

----------------------------------------------------------------------------------------
FULL CHANGELOG
----------------------------------------------------------------------------------------

---------------------------------------------
RTSDK CSharp Release 2.1.0.L1 (Mar 15, 2023)
---------------------------------------------

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
