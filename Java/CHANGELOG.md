This is the change log of the Refinitiv Real-Time SDK (RTSDK) for Java. RTSDK consists of Enterprise Message API (EMA) and Enterprise Transport API (ETA). This file contains history starting from version 1.2.0 which is when all components (EMA C++, EMA Java, ETA C, ETA Java) of RTSDK were fully open sourced. Note that RTSDK product version numbers start from 1.2.0 and EMA/ETA version numbers start from 3.2.0.

Rebranding NOTE: Refinitiv Real-Time SDK was formerly known as Elektron SDK or ESDK.

There are three types of RTSDK releases that append a letter directly followed by a number to the version number.

"L" releases (e.g., 1.2.0.L1) are full RTSDK releases that are uploaded to MyRefinitiv (formerly Customer Zone), Developer Community and GitHub.
"G" releases (e.g., 1.2.0.G1) are releases that are only uploaded to GitHub.
"E" releases (E-Loads) are emergency RTSDK releases that are uploaded to MyRefinitiv and Developer Community but not to GitHub. Also note that emergency releases may only be partial (i.e., Java or C++/C only).

----------------------------------------------------------------------------------------
CURRENT RELEASE HIGHLIGHTS - RTSDK Java 2.2.0.L1 aka EMA/ETA 3.8.0.L1 aka 3.8.0.0
----------------------------------------------------------------------------------------

This release introduces EMA message packing feature. In addition, a number of customer issues were addressed.

Customer Issues Resolved
----------------------------------------------------------------------------------------
- [Case Number: 06589180] - [RTSDK-1643] - Added toString implementation for the containers and messages in EMA Java
- [GitHub #239] - [RTSDK-7624] - EMAJ Concurrency Issue for Views causing NPE on reissue/unregister
- [GitHub #248] - [RTSDK-7746] - Add connection type to OmmConsumerConfig
- [GitHub #272] - [RTSDK-8361] - CryptoHelper.java not able to detect socket close due to which application hangs infinitely

----------------------------------------------------------------------------------------
FULL CHANGELOG
----------------------------------------------------------------------------------------

--------------------------------------------
RTSDK Java Release 2.2.0.L1 (Apr 30, 2024)
--------------------------------------------

This release introduces EMA message packing feature. In addition, a number of customer issues were addressed.

EMA Java 3.8.0.L1 Issues Resolved
---------------------------------
- [RTSDK-434] - Message packing for EMA Providers
- [RTSDK-1643] - Added toString implementation for the containers and messages in EMA Java [Case Number: 06589180]
- [RTSDK-7624] - EMAJ Concurrency Issue for Views causing NPE on reissue/unregister [GitHub #239]
- [RTSDK-7746] - Add connection type to OmmConsumerConfig [GitHub #248] 
- [RTSDK-8361] - CryptoHelper.java not able to detect socket close due to which application hangs infinitely [GitHub #272]
- [RTSDK-8128] - Update to EMA C++ ConsPerf to add commandline option to specify TLS version

ETA Java 3.8.0.L1 Issues Resolved
---------------------------------
- [RTSDK-6989] - Fix to Reactor getBuffer to return null upon channel down to avoid NullPointerException
- [RTSDK-7515] - Fix to error string when setting tokenURLV2 to invalid content
- [RTSDK-7580] - Update to VAConsumer to add commandline option to specify location/region for service discovery when connecting to RTO
- [RTSDK-7618] - Modified ETA examples to always set CredentialRenewal callback to demonstrate secure ways to provide credentials
- [RTSDK-8091] - Update to ETAJ ConsPerf to add commandline option to specify TLS version

Both ETA Java and EMA Java 3.8.0.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-7410] - Update DACS libraries to 7.12
- [RTSDK-8101] - Updated commons-lang to 3.3.14.0, commons-logging to 1.3.0, slf4j to 2.0.11, jose4j to 0.9.4, jackson-\* 2.16.1
- [RTSDK-8107] - Avoid double unlock in EMA OmmBaseImpl by checking for unlock

--------------------------------------------
RTSDK Java Release 2.1.3.L1 (Dec 22, 2023)
--------------------------------------------

In this release, support for TLS 1.3 is introduced along with updates to dependent jars due to vulnerabilities and fixes to customers issues.

EMA Java 3.7.3.L1 Issues Resolved
---------------------------------
- [RTSDK-6733] - EmajNIProvPerf fixes for images sent to ADH in a performance test
- [RTSDK-7810] - Minor change to RMTES Cons310 example to request different item: NFCP_UBMS
- [RTSDK-7944] - Increase to NumInputBuffers from 10 to 100
- [RTSDK-8058] - EMAJ ProgrammaticConfig issue: Cons451 does NOT send REST request when ChannelType::RSSL_SOCKET with proxy specified

ETA Java 3.7.3.L1 Issues Resolved
---------------------------------
- [RTSDK-180] - Fixes to Performance Tool Guide to correct list of arguments to TransportPerf
- [RTSDK-7604] - Fix to send Generic message on login message callback
- [RTSDK-8073] - VAConsumer tool added to support implicit/auto service discovery

Both ETA Java and EMA Java 3.7.3.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-7343] - Support for TLS 1.3
- [RTSDK-7524] - EMA RsslSocketChannel::fillGatheringByteArray infinite loop [GitHub #249]
- [RTSDK-7593] - Enhancement to add ability to specify proxy separately for REST requests versus reactor channels
- [RTSDK-7764] - Poor performance in EMAJ/ETAJ consumer unregister [GitHub #247]
- [RTSDK-7780] - Update Java dependencies due to vulnerabilities: commons-configuration, commons-lang3, commons-text, commons-codec, httpcore, slf4j, jose4j 

--------------------------------------------
RTSDK Java Release 2.1.2.L1 (Sep 8, 2023)
--------------------------------------------

EMA Java 3.7.2.L1 Issues Resolved
---------------------------------
- [RTSDK-7311] - Performance improvement to EMA Java decoding with an added, more efficient iterator by reference that avoids java collection creation 

ETA Java 3.7.2.L1 Issues Resolved
---------------------------------
- [RTSDK-7034] - Concurrence issue in SelectableBiDirectionalQueue [GitHub #223]
- [RTSDK-7505] - ProvPerf example does not include name in message key for item refresh messages
- [RTSDK-7761] - WatchlistConsumer example error with servicediscovery using clientId and clientSecret (OAuth2_V2)

Both ETA Java and EMA Java 3.7.2.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-6242] - Support for XmlTrace to file parameters in ETA & EMA Java
- [RTSDK-6261] - Qualify RTSDK API on Ubuntu Kylin 
- [RTSDK-7272] - Qualify RTSDK API on Ubuntu
- [RTSDK-7440] - Update to Java Dependencies to address vulnerabilities

--------------------------------------------
RTSDK Java Release 2.1.1.L1 (Jun 9, 2023)
--------------------------------------------
This is a maintenance release with fixes for customer issues.

EMA Java 3.7.1.L1 Issues Resolved
---------------------------------
- [RTSDK-6357] - Thread executor main loop is broken by unchecked exceptions [GitHub #198, Case Number:  12360452]
- [RTSDK-7506] - Error in rrtviewer that JWK file is missing upon specifying service credentials

ETA Java 3.7.1.L1 Issues Resolved
---------------------------------
- [RTSDK-6846] - ETAJ code duplication in Reactor
- [RTSDK-7060] - Update to ETAJ documentation about JKS keystore file [GitHub #234]

Both ETA Java and EMA Java 3.7.1.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-7239] - Change default subprotocol to be socket instead of HTTP on encrypted connections

--------------------------------------------
RTSDK Java Release 2.1.0.L1 (Mar 15, 2023)
--------------------------------------------

This release introduces client credentials with jwt authentication for connectivity to Refinitiv Real-Time Optimized. Ability to obtain service accounts to use this authentication mechanism is forthcoming. In addition, this release serves as a maintenance release with fixes.

EMA Java 3.7.0.L1 Issues Resolved
---------------------------------
- [RTSDK-5352] - Update to RT Viewer to support V2 authentication for JWT flow
- [RTSDK-5931] - Enhancement to pass in Dictionary object upon OMMConsumer creation [Case Number: 10894282]
- [RTSDK-6839, RTSDK-7200] - Flags from request are not copied into response resulting in invalid snapshot response [GitHub #221] 
- [RTSDK-7042] - EMAJ NIProvPerf Example: missing latency information during startup time
- [RTSDK-7074] - EMA Deadlock fix: OmmConsumers stop wroking without any error message [Case Number: 11979617] 
- [RTSDK-7111] - EMA Java DataDictionaryImpl memory leak issue [Case Number: 12263752]

ETA Java 3.7.0.L1 Issues Resolved
---------------------------------
- [RTSDK-7113] - ETAC server using protocol-type=2 crashes upon incoming socket+json connection

Both ETA Java and EMA Java 3.7.0.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-4986] - Support V2 authentication: Support client credential and client credential with JWT flow
- [RTSDK-6728] - V2 authentication: Add a separate string called "Audience" with ability to override value for JWT flow
- [RTSDK-7026] - Update jackson-core to version 2.14.1

--------------------------------------------
RTSDK Java Release 2.0.8.L1 (Jan 6, 2023)
--------------------------------------------

This is a maintenance release with fixes and support for Visual Studio 2022 (JNI).

EMA Java 3.6.8.L1 Issues Resolved
---------------------------------
- [RTSDK-6585] - Documentation: EMA Java Config Guide global parameters
- [RTSDK-6794, RTSDK-6894, RTSDK-6902] - EMA Java | Deadlock in OmmConsumer [GitHub #218, Case Number: 11919895, Case Number: 11919122]
- [RTSDK-6896] - Fix to EMAJ Provider PerfTools to send message burst in allotted time slice

ETA Java 3.6.8.L1 Issues Resolved
---------------------------------
- [RTSDK-6416] - Json Converter provides ill formatted Json message when invalid fieldID (FID) is provided

Both ETA Java and EMA Java 3.6.8.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-6258] - Support Visual Studio 2022
- [RTSDK-6765] - Documentaion: Correct default value of HighWaterMark from EMA C/J ConfigGuide and ETA C/J PerfToolsGuide

--------------------------------------------
RTSDK Java Release 2.0.7.G2 (Nov 17, 2022)
--------------------------------------------
This is a maintenance release with a fix.

EMA Java 3.6.7.G2 Issues Resolved
---------------------------------
- [RTSDK-6835] - Fix to gradle build to address specifying ema dependencies and target javadoc jar directory 

--------------------------------------------
RTSDK Java Release 2.0.7.L2 (Nov 11, 2022)
--------------------------------------------

This is a maintenance release with fixes.

EMA Java 3.6.7.L2 Issues Resolved
---------------------------------
- [RTSDK-6614] - Ability to create test utilities jar using "gradlew packageTests"; also available on Maven Central [GitHub #197]

ETA Java 3.6.7.L2 Issues Resolved
---------------------------------
- [RTSDK-5853] - Fix msgCount increment in Reactor.dispatchAll method

Both ETA Java and EMA Java 3.6.7.L2 Issues Resolved
---------------------------------------------------
- [RTSDK-6672] - Update jackson-databind, commons-text, commons-codec jars with fixes to vulnerabilities [Case Number: 11755349]
- [RTSDK-6756] - Gradle build failure

--------------------------------------------
RTSDK Java Release 2.0.7.G1 (Oct 24, 2022)
--------------------------------------------
This is a rapid release with a fix for maven central upload due to gradle version update. 

Both ETA Java and EMA Java 3.6.7.G1 Issues Resolved
---------------------------------------------------
- [RTSDK-6693] - Maven upload of 2.0.7.L1 shows dependencies as "runtime" instead of "compile" 

--------------------------------------------
RTSDK Java Release 2.0.7.L1 (Oct 7, 2022)
--------------------------------------------
This release introduces Warm Standby consumer feature in ETA-Reactor and EMA. It also include maintenance/fixes for issues. 

EMA Java 3.6.7.L1 Issues Resolved
---------------------------------
- [RTSDK-2523] - EMA does not allow to publish Directory if serviceName is not NI_PUB and serviceId 0
- [RTSDK-6040] - Added EMAJ Consumer Example 470 for Warm Standby feature
- [RTSDK-6042] - Support for Java WarmStandby feature in EMAJ: programmatic and file config and implementation
- [RTSDK-6412] - Documentation: Fix EMAJ configuration guide, section 4.5.3: add innerElementList.clear() [GitHub #204] 
- [RTSDK-6417] - EMA log shows messages that should be "Info" as "Error"
- [RTSDK-6601] - EmaConfig.xml fix to remove duplicate Consumer_5 and Consumer_6

ETA Java 3.6.7.L1 Issues Resolved
---------------------------------
- [RTSDK-5276] - Added edge case testing for TCP transport
- [RTSDK-6127] - ETA ConsPerf support for parsing JSON messages over Websocket connection with option to measure overhead of conversion to RWF
- [RTSDK-6241] - WatchlistConsumer does not connect to endpoint if service discovery returns 1 endpoint in the location
- [RTSDK-6403] - ConsPerf WS JSON gets ClassCastException upon connecting to RTDS/RTC
- [RTSDK-6404] - Java provider training example fix to copy dictionary name into response
- [RTSDK-6044] - Support for Java WarmStandby feature in ETAJ: login based and service based standby
- [RTSDK-6568] - ETAJ Consperf exception when connect to RTDS/RTC

Both ETA Java and EMA Java 3.6.7.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-6167] - Uncaught NPE in EMA due to race condition in cleanup after timeout [Case Number: 11147646]
- [RTSDK-6259] - Support JDK and Oracle OpenJDK 1.17
- [RTSDK-6347] - Update to Jackson jar due to vulnerabilities detected
- [RTSDK-6395] - Apache commons-configuration2 version 2.7 vulnerability [GitHub #201]
- [RTSDK-6411] - Memory leak upon repeated OMMConsumer initialize and un-intialize [Case Number: 11444081] 
- [RTSDK-6479] - Update to DACS 7.8
- [RTSDK-6531] - JSON Converter unexpected value with date of 2022-11 [GitHub #207]

--------------------------------------------
RTSDK Java Release 2.0.6.G1 (Sep 16, 2022)
--------------------------------------------
This is a rapid release with a critical fix. 

EMA Java 3.6.6.G1 Issues Resolved
---------------------------------
- [RTSDK-6411] - Memory leak upon repeated init and un-init of OMMConsumer due to a growing global pool [Case Number: 11444081] 

--------------------------------------------
RTSDK Java Release 2.0.6.L1 (Jun 20, 2022)
--------------------------------------------
This is a maintenance release with fixes.

EMA Java 3.6.6.L1 Issues Resolved
---------------------------------
- [RTSDK-362] - EMAJ allows a null AppClient set for registerClient
- [RTSDK-1486] - Discrepancy between Provider and Consumer examples with MapEntry key
- [RTSDK-4415] - EMA Java offstream posting payload decode issue
- [RTSDK-5244] - EMAJ treats corrupted Real data as blank
- [RTSDK-6150] - EMA_Config Guide shows duplicate definitions: NumberOfLogFiles, Filename, etc
- [RTSDK-6297] - Add comment to Consumer310 to clarify RMTES partial updates handling [Case Number: 11264942]


ETA Java 3.6.6.L1 Issues Resolved
---------------------------------
- [RTSDK-189] - Issue with Packed messages sent by server over HTTP connection
- [RTSDK-4633] - Java Provider doesn't bind to localhost interface correctly
- [RTSDK-5430] - Permitted RMTES partial updates to be sent without buffering
- [RTSDK-5574] - Add ADS/Server-Side Websocket JSON->RWF conversion error checks on posted data to be in valid range
- [RTSDK-5576] - Provides debugging information per Reactor instance
- [RTSDK-5741] - Additional unit test added for conversion of a double-backslash or escaped backslash in JSON msg key
- [RTSDK-5901] - NullPointerException when processing directory domain message [Case Number: 10825196]
- [RTSDK-6036] - Add QATool for ETAC VAProvider which supports posting and can send ACK/NACK using comand line options


Both ETA Java and EMA Java 3.6.6.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-5324] - Nack Msg should contain Name and ServiceId attributes
- [RTSDK-5338] - Redo Service Discovery after X reconnection attempts
- [RTSDK-5694] - Qualify RTSDK Java with Amazon Corretto 8 & 11
- [RTSDK-6133] - Documentation: Correction to API Concepts Guide, section 2.4 with Websocket transport
- [RTSDK-6153] - Documentation: Correction to API Concepts Guide, section 2.4, for WarmStandBy support
- [RTSDK-6235] - Documentation: Fixes to EMAJ Configuration Guide


--------------------------------------------
RTSDK Java Release 2.0.5.L1 (Mar 10, 2022)
--------------------------------------------

This release introduces oAuthClientCredentials authentication in Early Access. This feature is available for preview only with ability to obtain credentials and use it, forthcoming. In addition, this release serves as a maintenance release with fixes to customer issues.

EMA Java 3.6.5.L1 Issues Resolved
---------------------------------
- [RTSDK-4379] - Documentation update to clarify printing of RMTES strings with partial updates
- [RTSDK-5096] - oAuthClientCredential support added to RRTViewer tool 
- [RTSDK-5115] - EmaConfig.xml fix to remove references to bespoke dictionary files [Case Number: 09924064]
- [RTSDK-5521] - Altered ETA Java NIProvPerf to support encrypted connection
- [RTSDK-5953] - Documentation clarification in EMA Dev Guides related to session management

ETA Java 3.6.5.L1 Issues Resolved
---------------------------------
- [RTSDK-4191] - Dictionary download support added for NIProvider role
- [RTSDK-5229] - Improvements to ReactorWatchlistJUnit
- [RTSDK-5593] - Added unit tests to cover additional simplified JSON-to-RWF conversions
- [RTSDK-5715] - ConcurrentModificationException when attempting to log REST interactions [Case Number: 10624185]
- [RTSDK-5854] - Remove extra copies of Dictionaries and fixed associated unit tests
- [RTSDK-6010] - Fixed exception to OmmJsonConverterException upon receiving FID not in dictionary with JSON over Websocket

Both ETA Java and EMA Java 3.6.5.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-5056] - Support Windows Server 2019
- [RTSDK-5098] - oAuthClientCredential support: Add in token timer for reconnection cases
- [RTSDK-5181] - Support oAuthClientCredential authentication
- [RTSDK-5604] - Enhancement Request: add asBigDecimal() to OmmReal interface [GitHub # 182]
- [RTSDK-5727] - oAuthClientCredential support: Update READMEs for examples with support
- [RTSDK-6051] - EMA throws a null pointer exception when making multiple batch requests [Case Number: 10982169]
- [RTSDK-6069] - Fixed a null pointer exception with request timeout [Case Number: 11101002]

--------------------------------------------
RTSDK Java Release 2.0.4.L1 (Dec 17, 2021)
--------------------------------------------

EMA Java 3.6.4.L1 Issues Resolved
---------------------------------
- [RTSDK-5698] - Documentation: EMAJ and EMACPP Config Guides provide Server & Channel settings for socket/websocket encrypted/unencrypted connections

ETA Java 3.6.4.L1 Issues Resolved
---------------------------------
- [RTSDK-5401] - ETAJ - If reactor is shutting down, provide a return code/error to indicate this

Both ETA Java and EMA Java 3.6.4.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-5220] - Add support for RTO connectivity in consumer perf applications published as QATools
- [RTSDK-5697] - Documentation: Fix content in PerfTools Guide related to Generic Messages and Posting for ETAC/ETAJ/EMAC++/EMAJ

--------------------------------------------
RTSDK Java Release 2.0.3.L2 (Oct 25, 2021)
--------------------------------------------

This is a maintenance release with addition of EMA Java provider performance tools, optimizations to Java encryption, an updated 130-byte update size used for performance testing, and added ability to set system send and receive buffers dyamically in EMA. 

EMA Java 3.6.3.L2 Issues Resolved
---------------------------------
- [RTSDK-527] - Addition of EMA Java Non-Interactive Provider Performance Tool
- [RTSDK-3986] - Fix to SysSendBufSize/SysRecvBufferSize being overwritten to 64K value
- [RTSDK-4938] - EMA Performance Tools Guide updates 
- [RTSDK-5214] - EMAJ ConsPerf: Support post and generic messages
- [RTSDK-5277] - Addition of EMA Java Interactive Provider Performance Tool

ETA Java 3.6.3.L2 Issues Resolved
---------------------------------
- [RTSDK-5596] - As a follow up to RTSDK-3966, made -serverSharedSocket available as a command line parameter for RSSL Provider example 

Both ETA Java and EMA Java 3.6.3.L2 Issues Resolved
---------------------------------------------------
- [RTSDK-5368] - Optimization to EMA/ETA Java encrypted connections 
- [RTSDK-5477] - Remove additional copying to \_appRecvBuffer during decryption
- [RTSDK-5545] - Fix to delaySteadyStateCalc in Java ConsPerf tools
- [RTSDK-5667] - Altered update message size to 130 Bytes for performance tools

--------------------------------------------
RTSDK Java Release 2.0.3.L1 (Sep 30, 2021)
--------------------------------------------

This is a maintenance release with fixes for customer issues and bugs

EMA Java 3.6.3.L1 Issues Resolved
---------------------------------
- [RTSDK-4780] - Programmatic config should set ChannelType by default
- [RTSDK-5188] - Provide a jar for the RRTViewer tool as part of the build
- [RTSDK-5342] - Desktop RRTViewer fails if using the EMA Configuration file [ GitHub #178] 
- [RTSDK-5412] - Documentation error in EMA Java Config guide [GitHub #179] 

ETA Java 3.6.3.L1 Issues Resolved
---------------------------------
- [RTSDK-4374] - Consumer crashes when decoding using wrong field type
- [RTSDK-4781] - ETAJ ValueAddCache JNI EXCEPTION_ACCESS_VIOLATION (0xc0000005) on Windows with JDK 1.8 version newer than 251
- [RTSDK-5026] - WSA:Posting Series with empty entries Entries:[{}] results in misleading error
- [RTSDK-5036] - Error Handling does NOT detect when sending Vector : null
- [RTSDK-5205] - ETAJ Json to Rwf converts unset Action of Vector entry to Unknown.

Both ETA Java and EMA Java 3.6.3.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-2661, RTSDK-5320] - Enhance ETA to not depend on the OS Character encoding for displaying non-ascii enum values [Case Number: 07311614, 10089325] 
- [RTSDK-3966] - Support SO_REUSEADDR to permit server side socket to be reused for loadbalancing
- [RTSDK-4615] - Rebrand Change: ADSPOP is now RTC, Refinitiv Real-Time Connector
- [RTSDK-5065] - Modify Java Perftools to print summary output if stopped before completing runtime
- [RTSDK-5216] - Update Java open source components versions
- [RTSDK-5253] - ETAJ Reference Manuals: Missing sections and link to converter
- [RTSDK-5351] - Websocket Transport: Compression implementation (with fragmentation) not compliant with Websocket RFC
- [RTSDK-5464] - Remove Support for VS2012 and VS2013

--------------------------------------------
RTSDK Java Release 2.0.2.G2 (Aug 11, 2021)
--------------------------------------------

This is a maintenance release consisting of fixes.

EMA Java 3.6.2.G2 Issues Resolved
---------------------------------
- [RTSDK-5450] - Altered EMA RDP applications (113 & 450) to take URL overrides for token URL and service discovery URL 

Both ETA Java and EMA Java 3.6.2.G2 Issues Resolved
---------------------------------------------------
- [RTSDK-5411] - Change to default location for RTO from us-east to us-east-1 due to addition of us-east-2
- [RTSDK-5440] - Update to documentation to change default location for RTO from us-east to us-east-1 due to addition of us-east-2

--------------------------------------------
RTSDK Java Release 2.0.2.G1 (Aug 6, 2021)
--------------------------------------------

This is a maintenance release consisting of a fix to customer issue.

Both ETA Java and EMA Java 3.6.2.G1 Issues Resolved
---------------------------------------------------
- [RTSDK-5292] - NullPointerException in EMA library due to improper login request initialization upon re-submit (submit fails under load) with session mgmt enabled [Case Number: 10057447]
- [RTSDK-5337] - NullPointerException occurs in rare race condition upon lost connection and ChannelInfo request

--------------------------------------------
RTSDK Java Release 2.0.2.L1 (Jun 23, 2021)
--------------------------------------------

New Features Added
------------------
This is a maintenance release consisting of fixes to customer issues, fixes for the Refinitiv Real-Time Market Data Viewer (RRTViewer) and changes to ConsPerf tool to provide overhead of RWF to JSON conversion in application and to improve statistics. 

EMA Java 3.6.2.L1 Issues Resolved
---------------------------------
- [RTSDK-4399] - EMAJ Enhancement Request: Support additional Source Directory attributes via EMAConfig
- [RTSDK-4750] - EMAJ RRTViewer: Ability to resize fields, Support ChannelSet for RDP connection, etc. 
- [RTSDK-4871] - RRTViewer: Implement "Back" button to return back to Connect UI
- [RTSDK-4974] - RRTViewer: Load from specified EmaConfig.xml file
- [RTSDK-4975] - RRTViewer: On 'Request UI', specify choices made by user: 'Specify Config', ConnectionType, Host, Port
- [RTSDK-4976] - RRTViewer: Label changes and optional parameter indicators
- [RTSDK-4984] - RRTViewer: Expand input fields to show content
- [RTSDK-5002] - RRTViewer: Fix such that Order of endpoint selection for discovered endpoint determines failover order
- [RTSDK-5041] - Updated EMA ConsPerf Readme.txt file with build instructions, configuration and sample command lines
- [RTSDK-5054] - RRTViewer - Multiple clicks on Refresh button results in multiple instances of snapshot data getting displayed
- [RTSDK-5072] - Added Round-Trip Time (RTT) stats in RRTViewer
- [RTSDK-5081] - EMA IProv421 prints Error about EMA Config param is not correct [GitHub #177]
- [RTSDK-5106] - EMA incomplete login refresh issue when doing RTO token renewal via application [Case Number: 09919129]
- [RTSDK-5121] - RRTViewer doesn't use the specified custom token service URL when making a connection with the selected endpoint(s)


ETA Java 3.6.2.L1 Issues Resolved
---------------------------------
- [RTSDK-4074] - ETAJ Tunnel stream must notify application when login timeout occurs for authenticating a tunnel stream


Both ETA Java and EMA Java 3.6.2.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-3984] - reissue() with initialImage attribute doesn't trigger a new RefreshMsg in Java EMA [GitHub #143]
- [RTSDK-4812] - Add jackson parser jar into BinaryPack
- [RTSDK-4942] - Update to DACS RHEL8 libraries 
- [RTSDK-4972] - OmmConsumerConfig.clientId() not mentioned in EMA ConfigGuide [GitHub #173]
- [RTSDK-5043] - PerfTools: Skewed steadystate latency calculations due to processing refreshes before updates
- [RTSDK-5055] - Modify ConsPerf application to calculate overhead of conversion from RWF to JSON on Websocket Transport (ETA Transport layer only)
- [RTSDK-5084] - Documentation: Change wording about DictionariesProvided vs. DictionariesUsed
- [RTSDK-5186] - Send tr_json2 on the wire but accept both rssl.json.v2 and tr_json2 on the wire
- [RTSDK-5191] - HTTP parse issue with HTTP Chunk Footer resulting in ChannelDown with EMA Error: Error Id 0 [Case Number: 09958339]
- [RTSDK-5192] - HTTP read issue resulting in EMA Error text CompressorException: Malformed input at 3 [Case Number: 09958326]


--------------------------------------------
RTSDK Java Release 2.0.1.G1 (April 21, 2021)
--------------------------------------------

New Features Added
------------------
This is a maintenance release which also introduces the Refinitiv Real-Time Market Data Viewer (RRTViewer), a OMM Consumer application with support for several connection types and connectivity to Advanced Distribution Server, Refinitiv Real-Time - Optimized (RRTO) or an OMM Provider.

EMA Java 3.6.1.G1 Issues Resolved
---------------------------------
- [RTSDK-4454] - Add EMAJ Refinitiv Real-Time Market Data Viewer GUI application
- [RTSDK-4720] - Support encryption connection type for EMAJ consumer performance tool
- [RTSDK-4954] - Correction to IProvider180 README file
- [RTSDK-4957] - EMA WebSocket example default port in EmaConfig.xml - [GitHub #171] 

ETA Java 3.6.1.G1 Issues Resolved
---------------------------------
- [RTSDK-4382] - Change RDP 'scope' default value to trapi.streaming.pricing.read
- [RTSDK-4742] - ETAJ Real.toString truncates trailing zeroes
- [RTSDK-4766] - Added unit tests for rsslNumericStringToReal with additional inputs such as 1151194421449.10009766 
- [RTSDK-4809] - ETA Providers cannot provide dictionary to Consumer when compression is enabled over websocket using rssl.rwf
- [RTSDK-4810] - OMMInvalidUsageException with RMTES Field when requesting 0005.HK MarketPrice instrument - [Case Number: 09700273]

Both ETA Java and EMA Java 3.6.1.G1 Issues Resolved
---------------------------------------------------
- [RTSDK-4502] - Compression resulting in additional fragmentation (specific corner case) with download dictionary causes channel down
- [RTSDK-4728] - Memory cleanup upon uninitialization - [Case Number: 09614140]
- [RTSDK-4813] - EMAJ/ETAJ:  Update Readme.md file to add Jackson parser as an external dependency

--------------------------------------------
RTSDK Java Release 2.0.1.L1 (March 4, 2021)
--------------------------------------------

New Features Added
------------------
This release introduces support for Websocket Transport in RTSDK with capabilities like compression, fragmentation and packing. With WS transport, user can choose either JSON (rssl.json.v2 aka tr_json2; tr_json2 will be deprecated) or RWF (rssl_rwf) data formats to send over the wire. Application layer will continue to receive data in RWF data format. In addition, conversion from RWF to JSON and vice versa is also available as part of librssl and as a separate shared library. This release adds Server Side Encryption support in EMA and ETA.

EMA Java 3.6.1.L1 Issues Resolved
---------------------------------
- [RTSDK-3813] - Duplicate FID in EMA Java View request returns all fields in Refresh Response message [GitHub # 131]
- [RTSDK-3965] - Avoid incorrect pattern of use of ReentrantLock.tryLock() [GitHub # 123]
- [RTSDK-4013] - EMAJ does NOT set HAS_SERVICE_ID flag on onStream postMsg with if it sets serviceName
- [RTSDK-4114] - Nullpointer Exception in OmmConsumerImpl [GitHub # 146]
- [RTSDK-4418] - Documentation: Change Readme to fix "Consumer" to "Custom" for 320 example

ETA Java 3.6.1.L1 Issues Resolved
---------------------------------
- [RTSDK-2707] - Java, UPA: IndexOutOfBoundsException thrown when applying RMTES to cache [Github # 96]
- [RTSDK-4175] - NPE in Server.accept if connection is not available [GitHub # 148]
- [RTSDK-4383] - ETAJ issue with xml dumping blank data
- [RTSDK-4707] - ETAJ RMTES Unit Test does not build on Java 9 or above

Both ETA Java and EMA Java 3.6.1.L1 Issues Resolved
---------------------------------------------------
- [RTSDK-1752] - ESDKJava Websocket Transport Support - Design
- [RTSDK-3418] - ESDKJ Server Side Encryption
- [RTSDK-4110] - ETAJ Websocket Transport: Support XML tracing for the JSON2 protocol
- [RTSDK-4127] - Server Side Encryption, Implementation: Extend ETAJ to handle server-side encrypted interactions
- [RTSDK-4129] - Server Side Encryption, Interface: Extend the ETAJ server bind interface to handle all of the encrypted configuration
- [RTSDK-4138] - JSON <-> RWF converters
- [RTSDK-4139] - ESDKJ RWF/JSON Conversion: ESDK simple data JSON->RWF converters
- [RTSDK-4140] - ESDKJ RWF/JSON Conversion: ESDK messages JSON->RWF converters
- [RTSDK-4141] - ESDKJ RWF/JSON Conversion: Dictionary integration to support enumerated fields
- [RTSDK-4142] - ESDKJ RWF/JSON Conversion: ESDK containers JSON->RWF converters
- [RTSDK-4144] - ESDKJ RWF/JSON Conversion: Java primitive types to byte[] converters
- [RTSDK-4145] - ESDKJ RWF/JSON Conversion: ESDK simple data RWF-JSON converters
- [RTSDK-4146] - ESDKJ RWF/JSON Conversion: ESDK messages RWF->JSON converters
- [RTSDK-4147] - ESDKJ RWF/JSON Conversion: ESDK containers RWF->JSON converters
- [RTSDK-4187] - Unable to reset view to get all fields [GitHub # 149]
- [RTSDK-4190] - Websocket Transport: Implement proxy and encryption support for WebSocket connection type.
- [RTSDK-4289] - Unable to resume a stream that was opened in a paused state [GitHub # 151]
- [RTSDK-4312] - Implement server side encryption support for WebSocket connection type.
- [RTSDK-4316] - ESDKJ RWF/JSON Conversion: Introduce RsslEncode/DecodeJsonMsgOptions
- [RTSDK-4319] - ESDKJ RWF/JSON Conversion: Json Messages converters ping/pong/error
- [RTSDK-4384] - Websocket Transport: Modify transportTest to support websocket transport and perform integration testing
- [RTSDK-4385] - ESDKJ RWF/JSON Conversion: Create unit tests for JSON -> RWF container converters
- [RTSDK-4386] - ESDKJ RWF/JSON Conversion: Create unit tests for Messages
- [RTSDK-4421] - ESDK-Documentation: Copyright Notice link on Refman footer is linked to invalid page
- [RTSDK-4422] - ESDKJ RWF/JSON Conversion: Support an array of Json messages supplied to parseJson method
- [RTSDK-4423] - ESDKJ RWF/JSON Conversion: Unit tests for messages encoded in containers (sunny cases)
- [RTSDK-4424] - ESDKJ RWF/JSON Conversion: RWF -> JSON Negative case scenarios for containers
- [RTSDK-4425] - ESDKJ RWF/JSON Conversion: RWF -> JSON Negative case scenarios for messages
- [RTSDK-4426] - ESDKJ RWF/JSON Conversion: JSON -> RWF Negative case scenarios for containers
- [RTSDK-4427] - ESDKJ RWF/JSON Conversion: JSON -> RWF Negative case scenarios for messages
- [RTSDK-4428] - ESDKJ RWF/JSON Conversion: Add factory for creating options
- [RTSDK-4429] - Add auto conversion feature between RWF and JSON to the Reactor library
- [RTSDK-4431] - Websocket Transport: Modify ETAJ Provider application to support the websocket transport
- [RTSDK-4432] - Websocket Transport: Modify ETAJ Consumer application to support the websocket transport
- [RTSDK-4433] - Modify ETAJ VAConsumer and WatchlistConsumer applications to support the websocket transport
- [RTSDK-4434] - Modify ETAJ VAProvider application to support the websocket transport
- [RTSDK-4442] - ESDKJ RWF/JSON Conversion: Converter library cleanup
- [RTSDK-4448] - ESDKJ RWF/JSON Conversion: Refactor library to create converter Builder by ConverterFactory class
- [RTSDK-4461] - ESDKJ RWF/JSON Conversion: Add method for getting failed json message
- [RTSDK-4474] - Enhance the ETAJ TransportPerf tool to support websocket transport
- [RTSDK-4476] - Enhance the EMAJ library to support the websocket connection type
- [RTSDK-4477] - EMAJ: Provides additional examples for IProvider and Consumer to support the websocket connection type.
- [RTSDK-4478] - Modify the Cons113 and Cons450 to support the websocket connection type.
- [RTSDK-4492] - Update gradle-wrapper.jar
- [RTSDK-4545] - Add hashing algorithm, rsslHashingEntityId
- [RTSDK-4597] - Update RTSDK Apache HTTPCOMPONENT to 4.5.13

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
- [RTSDK-3847] ETA Build warnings using JDK1.11
- [RTSDK-3918] ETAJ+Reactor: Support Round Trip Latency Monitoring

Both ETA Java and EMA Java 3.5.0.G1 Issues Resolved
---------------------------------------------------
- [RTSDK-3696] OpenJDK 1.11 qualification
- [RTSDK-3823] Support release of memory used by reactor events by adding maxEventsInPool
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
