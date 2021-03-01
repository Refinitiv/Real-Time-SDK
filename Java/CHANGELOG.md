This is the change log of the Refinitiv Real-Time SDK (RTSDK) for Java. RTSDK consists of Enterprise Message API (EMA) and Enterprise Transport API (ETA). This file contains history starting from version 1.2.0 which is when all components (EMA C++, EMA Java, ETA C, ETA Java) of RTSDK were fully open sourced. Note that RTSDK product version numbers start from 1.2.0 and EMA/ETA version numbers start from 3.2.0.

Rebranding NOTE: Refinitiv Real-Time SDK was formerly known as Elekton SDK or ESDK.

There are three types of RTSDK releases that append a letter directly followed by a number to the version number.

"L" releases (e.g., 1.2.0.L1) are full RTSDK releases that are uploaded to MyRefinitiv (formerly Customer Zone), Developer Community and GitHub.
"G" releases (e.g., 1.2.0.G1) are releases that are only uploaded to GitHub.
"E" releases (E-Loads) are emergency RTSDK releases that are uploaded to MyRefinitiv and Developer Community but not to GitHub. Also note that emergency releases may only be partial (i.e., Java or C++/C only).

----------------------------------------------------------------------------------------
CURRENT RELEASE HIGHLIGHTS - RTSDK Java 2.0.1.L1 aka EMA 3.6.1.L1 and ETA 3.6.1.L1
----------------------------------------------------------------------------------------

New Features Added
------------------
This release introduces support for Websocket Transport in RTSDK with capabilities like compression, fragmentation and packing. With WS tranport, user can choose either JSON (rssl.json.v2 aka tr_json2; tr_json2 will be deprecated) or RWF (rssl.rwf) data formats to send over the wire. Application layer will continue to receive data in RWF data format. In addition, conversion from RWF to JSON and vice versa is also available as part of librssl and as a separate shared library. This release adds Server Side Encryption support in EMA and ETA.

Customer Issues Resolved
------------------
[Github # 96] - [RTSDK-2707] - Java, UPA: IndexOutOfBoundsException thrown when applying RMTES to cache 
[GitHub # 131] - [RTSDK-3813] - Duplicate FID in EMA Java View request returns all fields in Refresh Response message
[GitHub # 123] - [RTSDK-3965] - Avoid incorrect pattern of use of ReentrantLock.tryLock()
[GitHub # 146] - [RTSDK-4114] - Nullpointer Exception in OmmConsumerImpl
[GitHub # 148] - [RTSDK-4175] -  NPE in Server.accept if connection is not available
[GitHub # 149] - [RTSDK-4187] -  Unable to reset view to get all fields
[GitHub # 151] - [RTSDK-4289] - Unable to resume a stream that was opened in a paused state

----------------------------------------------------------------------------------------
FULL CHANGELOG
----------------------------------------------------------------------------------------

--------------------------------------------
RTSDK Java Release 2.0.1.L1 (March 4, 2021)
--------------------------------------------

New Features Added
------------------
This release introduces support for Websocket Transport in RTSDK with capabilities like compression, fragmentation and packing. With WS tranport, user can choose either JSON (rssl.json.v2 aka tr_json2; tr_json2 will be deprecated) or RWF (rssl_rwf) data formats to send over the wire. Application layer will continue to receive data in RWF data format. In addition, conversion from RWF to JSON and vice versa is also available as part of librssl and as a separate shared library. This release adds Server Side Encryption support in EMA and ETA.

EMA Java 3.6.1.L1 Issues Resolved
---------------------------------
[RTSDK-3813] - Duplicate FID in EMA Java View request returns all fields in Refresh Response message [GitHub # 131]
[RTSDK-3965] - Avoid incorrect pattern of use of ReentrantLock.tryLock() [GitHub # 123]
[RTSDK-4013] - EMAJ does NOT set HAS_SERVICE_ID flag on onStream postMsg with if it sets serviceName
[RTSDK-4114] - Nullpointer Exception in OmmConsumerImpl [GitHub # 146]
[RTSDK-4418] - Documentation: Change Readme to fix "Consumer" to "Custom" for 320 example

ETA Java 3.6.1.L1 Issues Resolved
---------------------------------
[RTSDK-2707] - Java, UPA: IndexOutOfBoundsException thrown when applying RMTES to cache [Github # 96]
[RTSDK-4175] -  NPE in Server.accept if connection is not available [GitHub # 148]
[RTSDK-4383] - ETAJ issue with xml dumping blank data
[RTSDK-4707] - ETAJ RMTES Unit Test does not build on Java 9 or above

Both ETA Java and EMA Java 3.6.1.L1 Issues Resolved
---------------------------------------------------
[RTSDK-1752] - ESDKJava Websocket Transport Support - Design
[RTSDK-3418] - ESDKJ Server Side Encryption
[RTSDK-4110] - ETAJ Websocket Transport: Support XML tracing for the JSON2 protocol
[RTSDK-4127] - Server Side Encryption, Implementation: Extend ETAJ to handle server-side encrypted interactions
[RTSDK-4129] - Server Side Encryption, Interface: Extend the ETAJ server bind interface to handle all of the encrypted configuration
[RTSDK-4138] - JSON <-> RWF converters
[RTSDK-4139] - ESDKJ RWF/JSON Conversion: ESDK simple data JSON->RWF converters
[RTSDK-4140] - ESDKJ RWF/JSON Conversion: ESDK messages JSON->RWF converters
[RTSDK-4141] - ESDKJ RWF/JSON Conversion: Dictionary integration to support enumerated fields
[RTSDK-4142] - ESDKJ RWF/JSON Conversion: ESDK containers JSON->RWF converters
[RTSDK-4144] - ESDKJ RWF/JSON Conversion: Java primitive types to byte[] converters
[RTSDK-4145] - ESDKJ RWF/JSON Conversion: ESDK simple data RWF-JSON converters
[RTSDK-4146] - ESDKJ RWF/JSON Conversion: ESDK messages RWF->JSON converters
[RTSDK-4147] - ESDKJ RWF/JSON Conversion: ESDK containers RWF->JSON converters
[RTSDK-4187] -  Unable to reset view to get all fields [GitHub # 149]
[RTSDK-4190] - Websocket Transport: Implement proxy and encryption support for WebSocket connection type.
[RTSDK-4289] - Unable to resume a stream that was opened in a paused state [GitHub # 151]
[RTSDK-4312] - Implement server side encryption support for WebSocket connection type.
[RTSDK-4316] - ESDKJ RWF/JSON Conversion: Introduce RsslEncode/DecodeJsonMsgOptions
[RTSDK-4319] - ESDKJ RWF/JSON Conversion: Json Messages converters ping/pong/error
[RTSDK-4384] - Websocket Transport: Modify transportTest to support websocket transport and perform integration testing
[RTSDK-4385] - ESDKJ RWF/JSON Conversion: Create unit tests for JSON -> RWF container converters
[RTSDK-4386] - ESDKJ RWF/JSON Conversion: Create unit tests for Messages
[RTSDK-4421] - ESDK-Documentation: Copyright Notice link on Refman footer is linked to invalid page
[RTSDK-4422] - ESDKJ RWF/JSON Conversion: Support an array of Json messages supplied to parseJson method
[RTSDK-4423] - ESDKJ RWF/JSON Conversion: Unit tests for messages encoded in containers (sunny cases)
[RTSDK-4424] - ESDKJ RWF/JSON Conversion: RWF -> JSON Negative case scenarios for containers
[RTSDK-4425] - ESDKJ RWF/JSON Conversion: RWF -> JSON Negative case scenarios for messages
[RTSDK-4426] - ESDKJ RWF/JSON Conversion: JSON -> RWF Negative case scenarios for containers
[RTSDK-4427] - ESDKJ RWF/JSON Conversion: JSON -> RWF Negative case scenarios for messages
[RTSDK-4428] - ESDKJ RWF/JSON Conversion: Add factory for creating options
[RTSDK-4429] - Add auto conversion feature between RWF and JSON to the Reactor library
[RTSDK-4431] - Websocket Transport: Modify ETAJ Provider application to support the websocket transport
[RTSDK-4432] - Websocket Transport: Modify ETAJ Consumer application to support the websocket transport
[RTSDK-4433] - Modify ETAJ VAConsumer and WatchlistConsumer applications to support the websocket transport
[RTSDK-4434] - Modify ETAJ VAProvider application to support the websocket transport
[RTSDK-4442] - ESDKJ RWF/JSON Conversion: Converter library cleanup
[RTSDK-4448] - ESDKJ RWF/JSON Conversion: Refactor library to create converter Builder by ConverterFactory class
[RTSDK-4461] - ESDKJ RWF/JSON Conversion: Add method for getting failed json message
[RTSDK-4474] - Enhance the ETAJ TransportPerf tool to support websocket transport
[RTSDK-4476] - Enhance the EMAJ library to support the websocket connection type
[RTSDK-4477] - EMAJ: Provides additional examples for IProvider and Consumer to support the websocket connection type.
[RTSDK-4478] - Modify the Cons113 and Cons450 to support the websocket connection type.
[RTSDK-4492] - Update gradle-wrapper.jar
[RTSDK-4545] - Add hashing algorithm, rsslHashingEntityId
[RTSDK-4597] - Update RTSDK Apache HTTPCOMPONENT to 4.5.13

--------------------------------------------
RTSDK Java Release 2.0.0.L1 (Oct 19, 2020)
--------------------------------------------

New Features Added
------------------
The primary object of this release is to complete rebranding of RTSDK: change namespace to com.refinitiv, change library names and alter documentation to reflect new branding. A new file explaining impact to customer, REBRAND.md was also added. In addition, there were a few fixes included.

EMA Java 3.6.0.L1 Issues Resolved
---------------------------------
- [RTSDK-473] - Incorrect reference to a doc in the Perf Example ReadMe
- [RTSDK-3825] - EMAJava - Multiple config files shipped with EmaCpp
- [RTSDK-4311] - Application commandline usage/ readme / PDF should NOT make keyfile a mandatory parameter.

ETA Java 3.6.0.L1 Issues Resolved
---------------------------------
- [RTSDK-58] - Create training guide for ETA Java
- [RTSDK-3220] - Using gradlew with example runconsumer113 fails - [Case Number: 07508738]
- [RTSDK-4245] - Change references to UPA or "Ultra Performance API"
- [RTSDK-4282] - Received is mis-spelled

Both ETA Java and EMA Java 3.6.0.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-3220] - Using gradlew with example runconsumer113 fails - [Case Number: 07508738]
- [RTSDK-4151] - Change namespace to refinitiv
- [RTSDK-4243] - Documentation: change code snippets to new namespace in all documentation (PDF)
- [RTSDK-4244] - Documentation: change namespace references and references to previous product names
- [RTSDK-4266] - Documentation: change references to prior product names in code comments 
- [RTSDK-4274] - Replace dependent libraries to re-branded for DACS (updated versin of DACS also)
- [RTSDK-4283] - Maven Central: register for new namespace
- [RTSDK-4285] - Change QA tools to adapt to changes to filenames
- [RTSDK-4288] - Create REBRAND.md with customer impacts and add any changes to product names
- [RTSDK-4395] - Rebrand: Differentiate between RTSDK product and ETA/EMA library versions

--------------------------------------------
RTSDK Java Release 1.5.1.L1 (Sept 4, 2020)
--------------------------------------------

New Features Added
------------------
This is a maintenance release which resolves customer issus, bugs and adds support for the following: ability to measure tunnel stream performance. Included in this release are rebranding changes.

EMA Java 3.5.1.L1 Issues Resolved
---------------------------------
- [RTSDK-475] - EMAJ value add fails to decode message using the DirectoryMsg.decode() method
- [RTSDK-1558] - EMAJ: ema.access.DateTimeStringFormatImpl Observation [GitHub #57]
- [RTSDK-1633] - Allow EmaConfig.xml to be loaded from jar's resource [GitHub #69, GitHub #126]
- [RTSDK-3205] - RDP examples have no text error says could not open keystore file when identify wrong path/file.
- [RTSDK-3415] - Incorrect dependency for junit and commons-logging [GitHub # 111]
- [RTSDK-3416] - Drop commons-codec dependency [GitHub # 112]
- [RTSDK-3831] - IndexOutOfBoundsException when trying to access Cloned RefreshMsg EMA Java 1.4 [GitHub #128]
- [RTSDK-3835] - EMA ADS connection recovery failure [Case Number: 08431393]
- [RTSDK-4092] - EMAJ must check both DictionaryUsed and DictionaryProvided to download dictionary from network
- [RTSDK-4178] - Cloning issue with update message when name is included [Case Number: 09046247]

ETA Java 3.5.1.L1 Issues Resolved
---------------------------------
- [RTSDK-5] - [Training Examples] Request is for all Training examples to close all open streams and clean up connection upon runtime expiration
- [RTSDK-152] - [ Java Training Examples, Cosmetic text changes request ] Consumers show some unnecessary and inconsistent (with C-Edition) text
- [RTSDK-307] - ETAJ value add fails to decode message using the DirectoryMsg.decode() method
- [RTSDK-310] - No command line in training javadoc
- [RTSDK-767] - Example and Training code print statements contain mis-spelling of the word "Received"
- [RTSDK-2553] - recvWindowSize (from ETA Java ValueAdd TunnelStream ClassOfService) default value doesn't work as described in the document [Case Number: 07012810]
- [RTSDK-3270] - Enhance the Reactor for applications to specify the password for OAuth via the callback method
- [RTSDK-3310] - Provides centralize location to keep the OAuth tokens to share between multiple connections using the same OAuth credential
- [RTSDK-3473] - ReactorChannel event (Unknown channel event type -3) causing reactor to shutdown
- [RTSDK-3773] - Enhance ETAJ Performance tools to support Tunnelstreams
- [RTSDK-3865] - ETAJ Client Side Encryption: Add in host name validation, limit TLS version to 1.2, and enable SNI matching in ETAJ
- [RTSDK-4083] - Reactor prematurely enters CHANNEL_READY on reconnect [Case Number: 08873087]

Both ETA Java and EMA Java 3.5.1.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-3646] - VS2019 Support for JNI
- [RTSDK-3672] - Add DACSLock code snippet for ETA/EMA Java into documentation
- [RTSDK-3811] - Inefficient java package metadata extract by loop over Package.getPackages() [GitHub #130]
- [RTSDK-3828] - Integrate rsslReactorQueryServiceDiscovery() method with centralized token management to reuse token when using same credentials
- [RTSDK-3866] - RTSDKJ Client Side Encryption: Set default Certificate Authority keystore location in Java so a jks file isn't required
- [RTSDK-3867] - RTSDKJ Client Side Encryption: Split out encryption and HTTP(S) functionality
- [RTSDK-3991] - Provide the ability to configure the takeExclusiveSignOnControl parameter for the password grant type
- [RTSDK-4070] - Support a configurable debug parameters to show REST interactions (that do not print credentials)
- [RTSDK-4082] - Add tokenScope as a configuration for RDP connectivity
- [RTSDK-4090] - Rebranding: Change code references to new product and company name in unit tests, examples, etc.
- [RTSDK-4165] - Rebranding: Change references in READMEs, Code Comments,

--------------------------------------------
RTSDK Java Release 1.5.0.G1 (Jun 30, 2020)
--------------------------------------------

New Features Added
------------------
This is a maintenance GitHub push which resolves customer issus, bugs and adds support for the following: ability for providers to get round trip latency measurements, provider support for posting, and, openJDK 1.11.

EMA Java 3.5.0.G1 Issues Resolved
---------------------------------
- [RTSDK-3844] Network outage related race condition between oauth2 token renewal and login to infra [Case Number: 08453636]
- [RTSDK-3854] Support Posting in EMAJ Providers [GitHub #117]
- [RTSDK-3883] Documentation correction to specify that SysSendBufSize is applied as number of bytes, not KB [GitHub #137]
- [RTSDK-3909] EMA Java indexOutOfBoundsException [Case Number: 08600487]
- [RTSDK-3948] Support Round Trip Latency Monitoring
- [RTSDK-3988] Change EMA RDP example to take RIC as an input

ETA Java 3.5.0.G1 Issues Resolved
---------------------------------
- [RTSDK-773] ETAJ Training Provider Module 5 fails to connect to infra
- [RTSDK-1650] rsslDoubleToReal conversion  function doesn't work as expected [Case Number: 06708565]
- [RTSDK-3618] Dictionary.entry(int fieldId) returns the same DictionaryEntry instance [Case Number: 07697024 and GitHub # 141]
- [RTSDK-3823] Support release of memory used by reactor events by adding maxEventsInPool
- [RTSDK-3847] ETA Build warnings using JDK1.11
- [RTSDK-3918] ETAJ+Reactor: Suppport Round Trip Latency Monitoring

Both ETA Java and EMA Java 3.5.0.G1 Issues Resolved
---------------------------------------------------
- [RTSDK-3696] OpenJDK 1.11 qualification
- [RTSDK-4084] EMA should not set compression threshold unless explicitly configured by application 

--------------------------------------------
RTSDK Java Release 1.5.0.L1 (Mar 31, 2020)
--------------------------------------------

EMA Java 3.5.0.L1 Issues Resolved
---------------------------------
- [RTSDK-3614] EMAJ 'OmmInvalidUsageException', Text='The Field name STOCK_TYPE does not exist in the field dictionary' [Case Number: 07645599]
- [RTSDK-3615] Add port info into OmmConsumerEvent::getChannelInformation [GitHub # 113]
- [RTSDK-3677] Update the example in the EMA Java config guide to set a default for NumInputBuffers

ETA Java 3.5.0.L1 Issues Resolved
---------------------------------
- [RTSDK-45] Watchlist example issue: With service down, posting is still attempted

Both ETA Java and EMA Java 3.5.0.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-3834] Update default token service URL to use verison v1

--------------------------------------------
RTSDK Java Release 1.4.0.G1 (Jan 10, 2020)
--------------------------------------------

EMA Java 3.4.0.G1 Issues Resolved
---------------------------------
- [RTSDK-3472] Added support for NoWait dispatch timeout in Consumer and NiProvider [GitHub # 110] 

Both ETA Java and EMA Java 3.4.0.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-3594] Documentation changes with addtional rebranding changes

--------------------------------------------
RTSDK Java Release 1.4.0.L1 (Nov 15, 2019)
--------------------------------------------

EMA Java 3.4.0.L1 Issues Resolved
---------------------------------
- [RTSDK-3294] Enhancement Request: Added ability to dynamically increase number of allocated output buffers for handling "out of buffers" error [Case Number: 07652023]
- [RTSDK-3535] Inconsistency contents in default and description of ReissueTokenAttemptInterval and ReissueTokenAttemptLimit parameter [GitHub #120]

ETA Java 3.4.0.L1 Issues Resolved
---------------------------------
- [RTSDK-3204] RTSDK Documentation: Fix copyright link after removal of classic portal from esdk repository
- [RTSDK-3504] Add option for maxOutputBuffer on ProvPerf
- [RTSDK-3506] ETAJ VAConsumer with enable sessionMgnt fails to failover to backup connection on different location

Both ETA Java and EMA Java 3.4.0.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-3340] Rebrand RTSDK documentation
- [RTSDK-3476] Upgrade httpcomponents-core dependency [GitHub # 114]
- [RTSDK-3500] Enhancement Request: Add ability to retrieve number of tunnel stream buffers in use

--------------------------------------------
RTSDK Java Release 1.3.1.G2 (Oct 18, 2019)
--------------------------------------------

N/A; Changes are limited to RTSDK C/C++ ONLY

--------------------------------------------
RTSDK Java Release 1.3.1.G1 (Sept 25, 2019)
--------------------------------------------

EMA Java 3.3.1.G1 Issues Resolved
---------------------------------
- [RTSDK-3440] Provider fails to accept client connections with dispatch timeOut=NoWaitEnum [GitHub #110] 

ETA Java 3.3.1.G1 Issues Resolved
---------------------------------
- [RTSDK-3488] WlLoginHandler does not clear the state of login request for RDP connection
- [RTSDK-3423] ETAJ fails to renew token when user specifies non-default token service URL

Both ETA Java and EMA Java 3.3.1.G1 Issues Resolved
---------------------------------------------------
- [RTSDK-3430] Add error messages when RDP token request/renewal fails  
- [RTSDK-3431] Add TokenReissueRatio, ReissueTokenAttemptLimit and ReissueTokenAttemptLimit into RTSDK Java 
- [RTSDK-3433] Support configuring RestRequestTimeOut, ReissueTokenAttemptLimit, ReissueTokenAttemptInterval, TokenReissueRatio 
- [RTSDK-3468] Add RDP Auth proactive token renewal with password grant prior to refresh token expiration

--------------------------------------------
RTSDK Java Release 1.3.1.L1 (July 31, 2019)
--------------------------------------------

New Features Added
------------------
This release adds ability in EMA to clone and copy messages in order to decode payload outside of message callbacks. Please note that client_id is now a manadatory input for Cloud connectivity. RTSDK Java now supports JDK 1.11 and OpenJDK 1.8 with an upgrade in gradle version to 5.X. RTSDK Java no longer supports JDK1.7.

EMA Java 3.3.1.L1 Issues Resolved
---------------------------------
- [RTSDK-1750] Clone/copy facility for message payload to decode it outside of onUpdateMsg() [Case Number: 06854285, 5201994]

ETA Java 3.3.1.L1 Issues Resolved
---------------------------------
- [RTSDK-509] Add InitializationTimeout to EMA Config at Channel Level
- [RTSDK-900] Change documentation to reflect user-defined Config filename
- [RTSDK-1332] Update Consumer340 to increment postId
- [RTSDK-2625] Remove Mockito compile dependency from ETAJ [GitHub #94]
- [RTSDK-3182] Documentation, ETAJ Dev Guide: Fix "UPA" in Figure 36 to "Transport API Consumer App"
- [RTSDK-3183] ETAJ RDP should not perform service discovery lookup when address and port is set by user
- [RTSDK-3202] RTSDK Documentation: Remove links to "Transport API Value Added Components" in html and refer to VARefman
- [RTSDK-3268] Expose ping stats and rsslReadEx in reactor
- [RTSDK-3338] Require clientId parameter in ETAJ Reactor

Both ETA Java and EMA Java 3.3.1.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-2631] Support JDK 1.11 & Upgrade to Gradle version 5.4.1
- [RTSDK-2651] Support OpenJDK 1.8
- [RTSDK-3248] With new gradle version, 5.4.1, update documentation to include the -PcommandLineArgs with --args
- [RTSDK-3321] Update httpclient and other RTSDK jars from 4.X.X to recent version
- [RTSDK-3410] Removed extra "/" to service discovery URL to get an Elektron cloud endpoint

--------------------------------------------
RTSDK Java Release 1.3.0.G1 (April 16, 2019)
--------------------------------------------

EMA Java 3.3.0.G1 Issues Resolved
---------------------------------
- [RTSDK-3194] Documentation improvements for RDP examples [GitHub #98]

ETA Java 3.3.0.L1 Issues Resolved
---------------------------------
- [RTSDK-3198] Unsolicited Refresh is not fanned out to first stream when two streams are aggregated by Reactor Watchlist when second stream is a full view. 
- [RTSDK-3196] Fix to Watchlist consumer for -tunnel 

--------------------------------------------
RTSDK Java Release 1.3.0.L1 (March 26, 2019)
--------------------------------------------

New Features Added
------------------
This RTSDK release provides support for RDP Session management (token renewal) and Service Discovery (discovering host/port information based on Cloud region and type of connection ). 

EMA Java 3.3.0.L1 Issues Resolved
---------------------------------
- [RTSDK-484] EMA Consumer application that requests a streaming source directory does not receive source directory updates. [Case Number: 05257390]
- [RTSDK-619] RMTES Partial updates are not processed correctly if OmmRmtes.toString() is called before OmmRmtes.apply() is called [Case Number: 05533464] 
- [RTSDK-1713] Provides interface design and implementation for EMAJ to support session managment from the Reactor
- [RTSDK-1766] EMA returns messages to incorrect appClient when OmmConsumer is shared between multiple threads [Case Number: 06902288]
- [RTSDK-2597] EMA Consumer application running in User_Dispatch mode sometimes hangs when the upstream source disconnects 
- [RTSDK-2599] Require a new utility or interface similar to asHexString that shows raw hex output [Case Number: 07023993]
- [RTSDK-2609] Support JDK1.9
- [RTSDK-2633] EMA batch requests consume way more memory than the individual requests. [Case Number: 07244571]
- [RTSDK-2652] EMAJ Lack of item recovery if service was brought up after consumer was up
- [RTSDK-2678] Expose initializationTimeout configuration and make default to higher value for Encrypted

ETA Java 3.3.0.L1 Issues Resolved
---------------------------------
- [RTSDK-627] Remove references to UPA in ETA C and ETA Java Developers Guide [Case Number: 05543578]
- [RTSDK-1709] Provides HTTP requests for blocking and non-blocking call for ETAJ
- [RTSDK-1715] Implements RDP service discovery and token management for ETAJ reactor
- [RTSDK-2609] Support JDK1.9
- [RTSDK-2624] DateTime is incorrectly implemented as local time rather than GMT [Case Number: 07232265]

--------------------------------------------
RTSDK Java Release 1.2.2.L1 (November 15, 2018)
--------------------------------------------

New Features Added
------------------
Provides the functionality for Non-interactive, Interactive, and Consumer applications to get channel information from the EMA's callback methods via OmmProviderEvent and OmmConsumerEvent classes

EMA Java 3.2.2.L1 Issues Resolved
---------------------------------
- [RTSDK-800] EMA returns info of all services instead of the requested service [Case Number: 06453247]
- [RTSDK-1125] EMA ConsPerf applications do not use specified username in Login Request [Case Number: 05958811]
- [RTSDK-1601] Provide channel information in EMA's callback methods [Case Number: 06611113]
- [RTSDK-1723] IProvider application with UserDispatch has 100% cpu 
- [RTSDK-1724] Fixed a concurrent modification exception in ETAJ Reactor service handling after a disconnect. [GitHub #76] 
- [RTSDK-1753] Add support for WindowsServer2016 
- [RTSDK-2528] Fix headings syntax in readme [GitHub Pull Request #89]
- [RTSDK-2555] Include latest jdacsUpalib.jar in RTSDK [Case Number:  06042800 and 06041830]

ETA Java 3.2.2.L1 Issues Resolved
---------------------------------
- [RTSDK-647] EMAJ or ETAJ consumer sends duplicate FIDs in a snapshot view request
- [RTSDK-1651] UPAJConsPerf throws IllegalArgumentException while connecting when numerous threads attempt to initialize the channel at the same time
- [RTSDK-1745] File Descriptor leak open a SocketChannel but never closed [GitHub #78]
- [RTSDK-1753] Add support for WindowsServer2016 
- [RTSDK-2550] ETA RDM Usage guide section 6.2.4 shows market price update instead of status
- [RTSDK-2555] Include latest jdacsUpalib.jar in RTSDK [Case Number:  06042800 and 06041830]

--------------------------------------------
RTSDK Java Release 1.2.1.L1 (August 15, 2018)
--------------------------------------------

New Features Added
------------------
Programmatic configuration for EMA Consumer, IProvider and NIProvider.

EMA Java 3.2.1.L1 Issues Resolved
---------------------------------
- [RTSDK-380] If CompressionType is set to "None", the CompressionThreshold range check still occurs
- [RTSDK-415] Clarify parent handle usage in EMA interface [Case Number: 05109877]
- [RTSDK-677] Improve reference manual documentation for OmmArray.fixedwidth [Case Number: 05630733]
- [RTSDK-705] Programmatic configuration for EMA Java [Case Number: 06558057]
- [RTSDK-1513] Eliminate RTSDK Java Warnings
- [RTSDK-1548] Update RDMUsageGuide to include information on the required filters to mark a service back Up [Case Number: 06538048]
- [RTSDK-1587] Deprecate unused StatusCodes [GitHub #65]
- [RTSDK-1556] Update JavaDoc for OmmDateTime, OmmDate and OmmTime [GitHub #55]
- [RTSDK-1590] Gradlew Build Failed for Elektron SDK Java 1.2.0 package downloaded from the Developer Portal [Case Number: 06591880]
- [RTSDK-1596] Data from two different services are mixed up when an item is requested from different services using the same OmmConsumer [Case Number: 06604413]
- [RTSDK-1602] Fix EMA/ETA Java Gradle warnings
- [RTSDK-1603] toString on a newly created ElementList throws NullPointerException [Case Number: 06589180]
- [RTSDK-1640, RTSDK-1646] Handle leak in OmmBaseImpl that results in issues while reconnecting [Case Number: 06669026 and GitHub #72]
- [RTSDK-1644] Fix README content Github to have change log (without duplicating information)

ETA Java 3.2.1.L1 Issues Resolved
---------------------------------
- [RTSDK-380] If CompressionType is set to "None", the CompressionThreshold range check still occurs
- [RTSDK-1513] Eliminate RTSDK Java Warnings
- [RTSDK-1590] Gradlew Build Failed for Elektron SDK Java 1.2.0 package downloaded from the Developer Portal [Case Number: 06591880]
- [RTSDK-1602] Fix EMA/ETA Java Gradle warnings
- [RTSDK-1635] ETA should not have EDF/Queue examples
- [RTSDK-1636] Consumer Module_2_Login training example does not properly fall through
- [RTSDK-1644] Fix README content Github to have change log (without duplicating information)

-----------------------------------------
RTSDK Java Release 1.2.0.G1 (May 31, 2018)
-----------------------------------------

EMA Java 3.2.0.G1 Issues Resolved
---------------------------------
- [RTSDK-1309] EMA Java does not receive NAK when Posting timeout occurs [Case Number: 06037235]
- [RTSDK-1446] EMA Java examples missing RMTES data type decoding in decode() function [Case Number: 06453670]
- [RTSDK-1481, RTSDK-1538] EMA hangs when uninitializing the OmmConsumer [Case Numbers: 06469347, 06466793]
- [[RTSDK-1543] Gradle build script must not rely on working dir [GitHub #52]
- [RTSDK-1488] EMA Java does not decode the Buffer in a MapEntry key correctly [Case Number: 06486004]

ETA Java 3.2.0.G1 Issues Resolved
---------------------------------
- [RTSDK-1415] Incorrect usage of NaN in RealImpl.java [GitHub #51]
- [RTSDK-1543] Gradle build script must not rely on working dir [GitHub #52]
- [RTSDK-1545] Patterns created for each instance rather than once [GitHub Pull Request #53]

-------------------------------------------
RTSDK Java Release 1.2.0.L1 (April 27, 2018)
-------------------------------------------

New Features Added
------------------
RTSDK Java now utilizes a Gradle build environment. Refer to the RTSDK Java Migration Guide for detailed instructions regarding how to build EMA Java with Gradle.

EMA now supports encrypted connection type.

EMA Java 3.2.0.L1 Issues Resolved
---------------------------------
- [RTSDK-487] EMA throws OmmInvalidUsageException if an empty Map is encoded [Case Number: 05338640 and GitHub #28]
- [RTSDK-907] EMA can't handle a SERVICE_DIRECTORY refresh that contains a lot of services [Case Number: 05896732, 06042281 and 06443659]
- [RTSDK-929] EMA Java application does not receive updates after received the Status Message with no State information [Case  Number: 06436262 and GitHub #37]
- [RTSDK-943] Update EMAJ OmmBuffer documentation
- [RTSDK-417] Expose encrypted connection support through EMA Java
- [RTSDK-813] Date/Time/DateTime to string and from string conversions should support ISO 8601 format.
- [RTSDK-1120] ConcurrentModificationException in EMA [GitHub #34]
- [RTSDK-1218] EMA application uses 100% CPU and hangs when it makes a lot of requests and the monitor shows a large number of instances of WorkerEvent [Case  Number: 06393015 and GitHub #41]
- [RTSDK-1220] ConcurrentModificationException while invoking dispatch() [Case Number: 06107164]
- [RTSDK-1307] ETAJ connected component could not set for HTTP and HTTPS type connections
- [RTSDK-1360] EMA Java Tunnel Stream sub-stream with stream id gets NullPointerException
- [RTSDK-1364] AckMsgImpl toString() method typo
- [RTSDK-1365] ItemCallbackClient NullPointerException when rsslMsg state is null
- [RTSDK-1388] Mavenize the build [GitHub #21]
- [RTSDK-1431] EMA Java cannot handle negative FIDs in View request [Case Number: 06444345]

ETA Java 3.2.0.L1 Issues Resolved
---------------------------------
- [RTSDK-676] EMA Java batch consumer encounters "Reactor is shutdown, submit aborted" error status [Case Number: 05633341]
- [RTSDK-907] EMA can't handle a SERVICE_DIRECTORY refresh that contains a lot of services [Case Number: 05896732, 06042281 and 06443659]
- [RTSDK-929] EMA Java application does not receive updates after received the Status Message with no State information [Case Number: 06436262 and GitHub #37]
- [RTSDK-1062] EMA Java encountered java.lang.NullPointerException [Case Number: 06107469 and 05958461]
- [RTSDK-1064] Exception when set http with proxy using Consumer
- [RTSDK-1218] EMA application uses 100% CPU and hangs when it makes a lot of requests and the monitor shows a large number of instances of WorkerEvent [Case Number: 06393015 and GitHub #41]
- [RTSDK-1227] Exception reactor.ReactorChannel.releaseBuffer(ReactorChannel.java:658)
- [RTSDK-1300] Review/refactor ETAJ Watchlist Recovery & Cleanup Code
- [RTSDK-1385] Watchlist consumer returns a ‘NullPointerException” when same item is used with private, private view & snapshot
- [RTSDK-1388] Mavenize the build [GitHub #21]
- [RTSDK-1489] ETA Java adds payload when container type is set to NO_DATA [Case Number: 06475782]
