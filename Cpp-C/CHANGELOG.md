This is the change log of the Refinitiv Real-Time SDK (RTSDK) for C++/C. RTSDK consists of Enterprise Message API (EMA) and Enterprise Transport API (ETA). This file contains history starting from version 1.2.0 which is when all components (EMA C++, EMA Java, ETA C, ETA Java) of RTSDK were fully open sourced. Note that RTSDK product version numbers start from 1.2.0 and EMA/ETA version numbers start from 3.2.0.

Rebranding NOTE: Refinitiv Real-Time SDK was formerly known as Elektron SDK or ESDK. 

There are three types of RTSDK releases that append a letter directly followed by a number to the version number. 

"L" releases (e.g., 1.2.0.L1) are full RTSDK releases that are uploaded to MyRefinitiv (formerly Customer Zone), Developer Community and GitHub. 
"G" releases (e.g., 1.2.0.G1) are releases that are only uploaded to GitHub. 
"E" releases (E-Loads) are emergency RTSDK releases that are uploaded to MyRefinitiv and Developer Community but not to GitHub. Also note that emergency releases may only be partial (i.e., Java or C++/C only).

----------------------------------------------------------------------------------------
CURRENT RELEASE HIGHLIGHTS - RTSDK C/CPP 2.0.8.L1 aka EMA/ETA 3.6.8.L1 aka 3.6.8.0
----------------------------------------------------------------------------------------

This is a maintenance release with fixes and support for Visual Studio 2022.

Customer Issues Resolved
----------------------------------------------------------------------------------------
- [Case Number: 11639977] - [RTSDK-6655] - Example provider application with EMA C++ is crashing at exit
- [GitHub #205] - [RTSDK-6596] - RSSL_INIT_ENCODE_ITERATOR missing initializer
- [GitHub #208] - [RTSDK-6597] - Memory issue in ETA 3.6.6.L1 (valgrind)
- [GitHub #214] - [RTSDK-6793] - Removed unused parameter 'ReactorEventFdPort' from application
- [GitHub #215] - [RTSDK-6747] - GitHub #215: Erroneous documentation for Logger
- [GitHub #220] - [RTSDK-6834] - SEGV during reconnection in cold standby (ChannelSet)
- [GitHub #231] - [RTSDK-6607] - Update libxml2 version

----------------------------------------------------------------------------------------
FULL CHANGELOG
----------------------------------------------------------------------------------------

--------------------------------------------
RTSDK C++/C Release 2.0.8.L1 (Jan 6, 2023)
--------------------------------------------

EMA C++ 3.6.8.L1 Issues Resolved
--------------------------------
- [RTSDK-6655] - EMA C++ provider example is crashing at exit [Case Number: 11639977]
- [RTSDK-6747] - Documentation: Erroneous documentation for Logger [GitHub #215]
- [RTSDK-6793] - Removed unused parameter 'ReactorEventFdPort' from application [GitHub #214]

ETA C 3.6.8.L1 Issues Resolved
------------------------------
- [RTSDK-5924] - ETA C Reactor does not handle redirect when get HTTP response with redirect host (3XX) from TokenUrl
- [RTSDK-6596] - RSSL_INIT_ENCODE_ITERATOR missing initializer [GitHub #205]
- [RTSDK-6597] - Memory issue in ETA 3.6.6.L1 (valgrind) [GitHub #208]

Both ETA C and EMA C++ 3.6.8.L1 Issues Resolved
----------------------------------------------
- [RTSDK-5693] - Support latest version of CMake and update minimum CMake version to use for buiding RTSDK
- [RTSDK-5880] - EmaCppConsPerf gets Segmentation fault core dump during terminate at UDR=250K websocket RWF
- [RTSDK-6258] - Support Visual Studio 2022
- [RTSDK-6600] - Update CURL version
- [RTSDK-6606] - Update LZ4 version 
- [RTSDK-6607] - Update libxml2 version [GitHub #231]
- [RTSDK-6765] - Documentation: Correct default value of HighWaterMark from EMA C/J ConfigGuide and ETA C/J PerfToolsGuide
- [RTSDK-6834] - SEGV during reconnection in cold standby (ChannelSet) [GitHub #220]
- [RTSDK-6840] - Documentation: Clarify cmake build instructions in readme and installation guide
- [RTSDK-6874] - Segmentation fault with Websocket transport due to buffer overflow

--------------------------------------------
RTSDK C++/C Release 2.0.7.G2 (Nov 17, 2022)
--------------------------------------------
This is a maintenance release with a fix.

Both ETA C and EMA C++ 3.6.7.G2 Issues Resolved
----------------------------------------------
- [RTSDK-6828] - rssBindInitialize saves process affinity and restores it after analyzed CPU Topology. 

--------------------------------------------
RTSDK C++/C Release 2.0.7.L2 (Nov 11, 2022)
--------------------------------------------

This is a maintenance release with fixes.

EMA C++ 3.6.7.L2 Issues Resolved
--------------------------------
- [RTSDK-6739] - Out of memory error when not supplying an XML config file [GitHub #213]

Both ETA C and EMA C++ 3.6.7.L2 Issues Resolved
----------------------------------------------
- [RTSDK-6665] Crash when binding cores beyond limit [Case Number: 11677751] 

--------------------------------------------
RTSDK C++/C Release 2.0.7.L1 (Oct 7, 2022)
--------------------------------------------
This is a maintenance release with fixes

EMA C++ 3.6.7.L1 Issues Resolved
--------------------------------
- [RTSDK-5561] - EMA C++ exception due to prior random data in RMTES cache buffer
- [RTSDK-5686] - Add support DirectWrite to Ema configuration for IProvPerf and NIProvPerf
- [RTSDK-5938] - EMA Support: Reactor should bind threads to specified cores
- [RTSDK-6303] - Fix libxml2 utilities for encoding/decoding to check for flags wrt. counthint, updatetype, serviceID, etc.
- [RTSDK-6151] - Add ability to append to existing file upon application restart
- [RTSDK-6152] - EMA C++ : Log shows messages that should be "Info" as "Error"

ETA C 3.6.7.L1 Issues Resolved
------------------------------
- [RTSDK-5076] - ETAC ConsPerf websocket JSON cannot send Directory/Item request to ProvPerf running with inputs: -inputBufs >=35000 -outputBufs >=35000
- [RTSDK-6076] - WatchlistConsumer connect RTO via websocket sometimes gets select: Bad file descriptor on RH6 only
- [RTSDK-6108] - Service Discovery doesn't return an error when an incorrect password is provided [GitHub #194] 
- [RTSDK-6126] - ETA ConsPerf support for parsing JSON messages over Websocket connection with option to measure overhead of conversion to RWF
- [RTSDK-6241] - WatchlistConsumer does NOT connect to endpoint if servicediscovery returns 1 endpoint in the location
- [RTSDK-6287] - Fix for libxml2EncodeUtilTest failure
- [RTSDK-6352] - Fix supporting single digit month values in dates
- [RTSDK-6423] - Remove duplicate dumpOut
- [RTSDK-6557] - Add ipc dumpOut to include print debug after adding RIPC header

Both ETA C and EMA C++ 3.6.7.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-5116] - Update to LZ4 version (dependency)
- [RTSDK-5221] - Add support for RTO connectivity in consumer perf applications as a QATool
- [RTSDK-6305] - Enhance rsslBindThreadWithString() to take in an output string and populate with cpu array
- [RTSDK-6339] - Document curl/openSSL dependency in README [GitHub #199]
- [RTSDK-6358] - Update CURL version (dependency)
- [RTSDK-6396] ETAC Server sends wrong fin/opcode flags over websocket transport in boundary case when sending single fragement [Case Number: 11340239]
- [RTSDK-6479] - Update DACS libraries to 7.8

--------------------------------------------
RTSDK C++/C Release 2.0.6.L1 (Jun 20, 2022)
--------------------------------------------

EMA C++ 3.6.6.L1 Issues Resolved
--------------------------------
- [RTSDK-1486] - Fix to EMA examples to account for MapEntry key which can be Buffer, Ascii or Rmtes
- [RTSDK-6099] - Port is not released after destroying OMMProvider [GitHub #192]
- [RTSDK-6125] - Ensure that LibcurlName can be specified via programmatic config (removed from file config)
- [RTSDK-6150] - EMA_Config Guide shows duplicate definitions: NumberOfLogFiles, Filename, etc
- [RTSDK-6168] - InitializationTimeout does not take effect via programmatic config [GitHub #196]
- [RTSDK-6302] - EMAC++'s default Login request does not have the correct position on Windows
- [RTSDK-6290] - Cons470 Segmentation fault when connect to server doesn't support warmstandby and Ctrl+C

ETA C 3.6.6.L1 Issues Resolved
--------------------------------
- [RTSDK-892] - Additional error checking added to Date, Time & DateTime
- [RTSDK-305] - Enhance xml encode utilities to be able to configure buffer size
- [RTSDK-4416] - Missing initializer enumeration in ripch.h
- [RTSDK-5343] - Support binding Reactor threads to specified cores: logical and physical binding
- [RTSDK-5544] - High CPU usage with VAConsumer with session mgmt enabled
- [RTSDK-5577] - Provide debugging information per Reactor instance
- [RTSDK-5822] - Fix rwsReadWebSocket error output
- [RTSDK-5919] - RTT Generic messages are not handled resulting in "Received unexpected msg class 7 from login domain"
- [RTSDK-6016] - Add QATool for ETAC VAProvider which supports posting and can send ACK/NACK using comand line options
- [RTSDK-6059] - Fix curl to configuration to set library [GitHub #190]
- [RTSDK-6065] - Support ability to log JSON data differently from RWF per channel
- [RTSDK-6077] - VAConsumer example gets HTTP 400 trying to use encryptedWebSocket [GitHub #191]
- [RTSDK-6079] - Improvement REST logging mechanism; enhancement to RTSDK-5473
- [RTSDK-6288] - Provide an option to disable/enable REST callback logging when creating a Reactor instance
- [RTSDK-6304] - Fixed WarmStandby loginbase doesn't check if serviceID and serviceName mismatched.
- [RTSDK-6325] - Handle mismatched QoS values when moving from one warm standby group to another warm standby group.

Both ETA C and EMA C++ 3.6.6.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-5763] - Design Multi-Credential feature to associate different auth per channel or group of channels for channelset and warmstandby features
- [RTSDK-6080] - Update cJson version
- [RTSDK-6136] - Implement ETAC and EMA C++ changes for Multi-Credentials feature
- [RTSDK-6153] - Documentation: Correction to API Concepts Guide, section 2.4, for WarmStandBy support

--------------------------------------------
RTSDK C++/C Release 2.0.5.G1 (Mar 31, 2022)
--------------------------------------------

ETA C 3.6.5.G1 Issues Resolved
--------------------------------
- [RTSDK-6101] - Updated ZLIB library to version 1.2.12

--------------------------------------------
RTSDK C++/C Release 2.0.5.L1 (Mar 10, 2022)
--------------------------------------------

EMA C++ 3.6.5.L1 Issues Resolved
--------------------------------
- [RTSDK-4379] - Documentation update to clarify printing of RMTES strings with partial updates
- [RTSDK-5115] - EmaConfig.xml fix to remove references to bespoke dictionary files [Case Number: 09924064]
- [RTSDK-5717] - Fix for posting submit failure upon reconnect to ADS [Case Number: 10539465] 
- [RTSDK-5953] - Documentation clarification in EMA Dev Guide related to session management

ETA C 3.6.5.L1 Issues Resolved
--------------------------------
- [RTSDK-5039] - Added unit tests to cover additional simplified JSON-to-RWF conversions
- [RTSDK-5111] - Add ADS/Server-Side Websocket JSON->RWF conversion error checks on posted data to be in valid range
- [RTSDK-5610] - Added the missing lock around the sessionByNameAndClientIdHt hash when fails to create a reactor channel
- [RTSDK-5889] - Altered ETA C NIProvPerf to support encrypted connection
- [RTSDK-6011] - Doxygen generated does not include RWF/JSON Converter
- [RTSDK-6019] - Crash due to double-free or corruption in rwfToJsonBase::uninitializeIntToStringTable
- [RTSDK-6025] - Fixed misspelled for the RsslReactorWarmStandbyChannelInfo structure


Both ETA C and EMA C++ 3.6.5.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-4551] - Redo Service Discovery after X reconnection attempts
- [RTSDK-4994] - Support oAuthClientCredential authentication
- [RTSDK-5056] - Support Windows Server 2019
- [RTSDK-5098] - oAuthClientCredential support: Add in token timer for reconnection cases
- [RTSDK-5473] - Enhancement Request: Dynamically enable/disable REST interaction logging
- [RTSDK-5727] - oAuthClientCredential support: Update READMEs for examples with support
- [RTSDK-5855] - Rename TransportAPIC_RDMUsageGuide to TransportAPI_C_RDMUsageGuide
- [RTSDK-5948] - Unexpected characters in the Set-Cookie header causes websocket connection close with java clients
- [RTSDK-6013] - Enhancement Request: Reactor should bind worker thread to specified core (logical binding)

--------------------------------------------
RTSDK C++/C Release 2.0.4.L1 (Dec 17, 2021)
--------------------------------------------

This release introduces Warm Standby consumer feature in ETA-Reactor and EMA. It also include maintenance/fixes for issues. 

EMA C++ 3.6.4.L1 Issues Resolved
--------------------------------
- [RTSDK-5698] - Documentation: EMAJ and EMACPP Config Guides provide Server & Channel settings for socket/websocket encrypted/unencrypted connections
- [RTSDK-5713] - Implement the Warm Standby feature in EMA

ETA C 3.6.4.L1 Issues Resolved
--------------------------------
- [RTSDK-5626] - Error in conversion of a double-backslash or escaped backslash in JSON message key

Both ETA C and EMA C++ 3.6.4.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-3830] - Design Warm StandBy feature 
- [RTSDK-5364] - If reactor is shutting down, provide a return code/error to indicate this
- [RTSDK-5469] - Implement the Warm Standby feature in the Reactor component
- [RTSDK-5697] - Documentation: Fix content in PerfTools Guide related to Generic Messages and Posting for ETAC/ETAJ/EMAC++/EMAJ
- [RTSDK-5761] - Documentation for Warm Standby feature

--------------------------------------------
RTSDK C++/C Release 2.0.3.L2 (Oct 25, 2021)
--------------------------------------------

This is a maintenance release with fixes to performance tools, an updated 130-byte update size used for performance testing, and added ability to set system send and receive buffers dyamically in EMA. 

EMA C++ 3.6.3.L2 Issues Resolved
--------------------------------
- [RTSDK-5195] - EMA ConsPerf: Support post and generic messages
- [RTSDK-5619] - EMAC++ - Extend IOCTL to include SysSendBufSize/SysRecvBufferSize

ETA C 3.6.3.L2 Issues Resolved
--------------------------------
- [RTSDK-5596] - As a follow up to RTSDK-3843, made -serverSharedSocket available as a command line parameter for RSSL Provider example 

Both ETA C and EMA C++ 3.6.3.L2 Issues Resolved
-----------------------------------------------
- [RTSDK-5667] - Altered update message size to 130 Bytes for performance tools

--------------------------------------------
RTSDK C++/C Release 2.0.3.L1 (Sep 30, 2021)
--------------------------------------------

This is a maintenance release with fixes for customer issues and bugs

EMA C++ 3.6.3.L1 Issues Resolved
--------------------------------
- [RTSDK-3540] - Change EMA log files from \_pid.log to a rolling log file.  [GitHub #121]
- [RTSDK-5219] - Item name is missing in a NACK message for a post upon acknowledgement timeout [Case Number: 09915927] 
- [RTSDK-5412] - Documentation error in EMA Java Config guide [GitHub #179] 

ETA C 3.6.3.L1 Issues Resolved
--------------------------------
- [RTSDK-4464] - rsslVACacheTest core dumped Debug only
- [RTSDK-5031] - Json to Rwf converts unset Action of Vector entry to Unknown.
- [RTSDK-5109] - Error with Perm Data being overwritten when order is changed in MapEntry
- [RTSDK-5278] - ETAC rsslDecodeReal should handle INVALID_DATA to be compatible with ETAJ
- [RTSDK-5522] - The information in TransportAPIC_RDMUsageGuide.pdf for Dictionary Refresh is incorrect [GitHub #181] 

Both ETA C and EMA C++ 3.6.3.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-4417] - ETAC and EMAC++: Xml trace functionality is not stopping at file size limit when RSSL_TRACE_TO_MULTIPLE_FILES is not set [GitHub #162] 
- [RTSDK-4615] - Rebrand Change: ADSPOP is now RTC, Refinitiv Real-Time Connector
- [RTSDK-4862] - Update doxygen version used to generate documenation
- [RTSDK-5014] - CMake string(REPLACE ...) must always use quoted source [GitHub #115] 
- [RTSDK-5103] - Vulnerability in curl 7.63.0, CVE-2021-22890 -- upgrade to latest
- [RTSDK-5104] - Vulnerability in libxml2 2.9.9 -- upgrade to latest
- [RTSDK-5464] - Remove Support for VS2012 and VS2013

--------------------------------------------
RTSDK C++/C Release 2.0.2.G3 (Sep 24, 2021)
--------------------------------------------

This is a maintenance release with fixes

ETA C 3.6.2.G3 Issues Resolved
--------------------------------
- [RTSDK-5562] - Rssl socket transport fixes to handle FD value of 0
- [RTSDK-5569] - Websocket transport fix that waits to send open handshake till socket is ready for write in non blocking mode

--------------------------------------------
RTSDK C++/C Release 2.0.2.E1 (Sep 23, 2021)
--------------------------------------------

This is an emergency release with fixes

ETA C 3.6.2.E1 Issues Resolved
--------------------------------
- [RTSDK-5539] - Race condition from closed tunnelstream resulting in unhandled state in tunnelstream provider 
- [RTSDK-5599] - pReactorChannelImpl->tunnelDispatchEventQueued is not unset resulting in tunnel events not being dispatched 

--------------------------------------------
RTSDK C++/C Release 2.0.2.G2 (Aug 11, 2021)
--------------------------------------------

This is a maintenance release consisting of a fix to customer issue.

ETA C 3.6.2.G2 Issues Resolved
--------------------------------
- [RTSDK-5431] - Permitted RMTES partial updates to be sent without buffering 

EMA C++ 3.6.2.G2 Issues Resolved
--------------------------------
- [RTSDK-5450] - Altered EMA RDP applications (113 & 450) to take URL overrides for token URL and service discovery URL 

Both ETA C and EMA C++ 3.6.2.G2 Issues Resolved
-----------------------------------------------
- [RTSDK-5411] - Change to default location for RTO from us-east to us-east-1 due to addition of us-east-2
- [RTSDK-5440] - Update to documentation to change default location for RTO from us-east to us-east-1 due to addition of us-east-2

--------------------------------------------
RTSDK C++/C Release 2.0.2.G1 (Aug 6, 2021)
--------------------------------------------

This is a maintenance release consisting of a fix to customer issue.

ETA C 3.6.2.G1 Issues Resolved
--------------------------------
- [RTSDK-5370] - Added -compressionType and -compressionLevel to ProvPerf and -compressionType to ConsPerf
- [RTSDK-5404] - Memory leak with Provider application when providing dictionary over Websocket transport 

Both ETA C and EMA C++ 3.6.2.G1 Issues Resolved
-----------------------------------------------
- [RTSDK-5350] - Websocket Transport: Fix to transport to compress message and then fragment to address "Invalid Frame Header" error 

--------------------------------------------
RTSDK C++/C Release 2.0.2.L1 (Jun 23, 2021)
--------------------------------------------

This is a maintenance release consisting of fixes to customer issues, support for EMA C++ Interactive ProvPerf and EMA C++ NIProvPerf tools and changes to ConsPerf tool to provide overhead of RWF to JSON conversion in application and to improve statistics.

EMA C++ 3.6.2.L1 Issues Resolved
--------------------------------
- [RTSDK-1343] - EMA C++ ProvPerf Tool: Implementation Part I
- [RTSDK-4608] - Create a EMA Performance Tools Guide
- [RTSDK-4824] - EMA C++: Destructor of OmmConsumer freezes when interrupt is sent as soon as Consumer connects
- [RTSDK-4827] - EMA C++ ProvPerf Tool: Testing Story
- [RTSDK-4839] - EMA C++ NIProvPerf Tool: Implementation Part II
- [RTSDK-4972] - Documentation: OmmConsumerConfig.clientId() not mentioned in EMA ConfigGuide [GitHub #173]
- [RTSDK-5041] - Updated EMA ConsPerf Readme.txt file with build instructions, configuration and sample command lines
- [RTSDK-5075] - EMA ConfigGuide has incorrect default value of Server Group's SysRecvBufSize and SysSendBufSize

ETA C 3.6.2.L1 Issues Resolved
--------------------------------
- [RTSDK-4585] - Turning multi-line comments into single line comments [GitHub Pull Request #159]
- [RTSDK-4617] - Replace XERCES with Libxml2
- [RTSDK-4768] - Documentation: Clarify error scenarios for conversion utilities for RsslReal type
- [RTSDK-4843] - Unit tests added for handling of trailing zeros
- [RTSDK-5030] - Fix to ETAC ProvPerf to reset the error code upon failure to get output buffers for Webscoket Transport with JSON wireformat
- [RTSDK-5100] - Fix to Websocket message parsing to handle edge case of buffer containing one message and one byte of next message

Both ETA C and EMA C++ 3.6.2.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-4584] - Make destructor virtual because of polymorphic class object deletion [GitHub Pull Request #160]
- [RTSDK-4586] - Catch polymorphic type by reference, not by value [GitHub Pull Request #158]
- [RTSDK-4942] - Update to DACS RHEL8 libraries
- [RTSDK-5043] - PerfTools: Skewed steadystate latency calculations due to processing refreshes before updates
- [RTSDK-5050] - Source Directory update with additional entries in DictionariesProvided or DictionariesUsed causes crash [Case Number: 09850212]
- [RTSDK-5051] - Modify ConsPerf application to calculate overhead of conversion from RWF to JSON on Websocket Transport (ETA Transport layer only)
- [RTSDK-5084] - Documentation: Fix to definitions of DictionariesProvided vs. DictionariesUsed in RDM Usage Guide
- [RTSDK-5186] - Send tr_json2 on the wire but accept both rssl.json.v2 and tr_json2 from the wire

--------------------------------------------
RTSDK C++/C Release 2.0.1.G1 (Apr 21, 2021)
--------------------------------------------

EMA C++ 3.6.1.G1 Issues Resolved
--------------------------------
- [RTSDK-3884] - EMA C++: Destructor of OmmConsumer occasionally hangs forever in deadlock - [GitHub #138]
- [RTSDK-4291] - OmmConsumerImpl::registerClient implementation doesn't unlock mutex when exception is thrown - [GitHub #152]
- [RTSDK-4706] - EMA C++: configuration parameters ConnectionPingTimeout / ConnectionMinPingTimeout are applied as number of seconds, not milliseconds - [GitHub #168]
- [RTSDK-4719] - Support encryption connection type for EMAC consumer performance tool
- [RTSDK-4951] - EMA C++ Websocket example mismatch between Readme and source code - [GitHub #172]
- [RTSDK-4957] - EMA WebSocket example default port in EmaConfig.xml - [GitHub #171]
- [RTSDK-4954] - Correction to IProvider180 README file

ETA C 3.6.1.G1 Issues Resolved
--------------------------------
- [RTSDK-4371] - ETA compilation warning - [GitHub #161]
- [RTSDK-4382] - Change RDP 'scope' default value to trapi.streaming.pricing.read
- [RTSDK-4508] - JSON converter library fails to conform to UTF-8 when passing the JSON dictionary message over the network
- [RTSDK-4606] - TRCERouter Crash in ETA - [Case Number: 09514087]
- [RTSDK-4713] - ETAC: Websocket Transport should handle Sec-Websocket-Extensions with unsupported parameter: client_max_window_bits
- [RTSDK-4727] - rsslNumericStringToReal does not return error with an input like 1151194421449.10009766
- [RTSDK-4589] - In VAConsumer refresh message is received the channel is closed if the streamState is not open - [GitHub #164]
- [RTSDK-4941] - Buggy code in rsslRDMDictionaryMsg.c - [GitHub #170]


--------------------------------------------
RTSDK C++/C Release 2.0.1.L2 (Mar 31, 2021)
--------------------------------------------

Both ETA C and EMA C++ 3.6.1.L2 Issues Resolved
-----------------------------------------------
- [RTSDK-4860, RTSDK-4861] - Remove unused EtaJni files 

--------------------------------------------
RTSDK C++/C Release 2.0.1.L1 (Mar 4, 2021)
--------------------------------------------

EMA C++ 3.6.1.L1 Issues Resolved
--------------------------------
- [RTSDK-444] - Change Consumer270 to not use the hasMsgKey()
- [RTSDK-3775] - Add unit test EMA C++ [GitHub # 128]
- [RTSDK-3964] - Use after free in OmmServerBaseImpl.cpp [GitHub # 134]
- [RTSDK-4248] - Avoid empty catch(std::bad_alloc) blocks
- [RTSDK-4308] - Error in DirectoryHandler when using more than 5 services [GitHub # 153]
- [RTSDK-4402] - Support additional Source Directory attributes via EmaConfig [GitHub # 119]
- [RTSDK-4629] - EMACPP ServerType::RSSL_ENCRYPTED doesn't take WsProtocols

ETA C 3.6.1.L1 Issues Resolved
--------------------------------
- [RTSDK-214] - Consumer crashes when decoding using wrong field type [Case Number: 05224063]
- [RTSDK-3411] - Warnings when build with GCC 740
- [RTSDK-4171] - Linux shared memory server creation issue - remove resources incomplete, under non-root account
- [RTSDK-4295] - rsslTransportUnitTest fails when certificate and key are not passed in commandline args
- [RTSDK-3976] - RWF JSON Conversion Issue: Converting of a zero length message causes a crash
- [RTSDK-4544] - Add hashing algorithm, rsslHashingEntityId 

Both ETA C and EMA C++ 3.6.1.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-1761] - Remove malloc.h for clang; qualification with clang on RH8 [GitHub # 82]
- [RTSDK-3207] - Source directory request for invalid serviceId or serviceName give full source directory
- [RTSDK-4404] - Unreachable or dead code due to wrong behavior of some rsslEncode / rsslDecode functions
- [RTSDK-4421] - ESDK-Documentation: Copyright Notice link on Refman footer is linked to invalid page


--------------------------------------------
RTSDK C++/C Release 2.0.0.L1 (Oct 19, 2020)
--------------------------------------------

New Features Added
------------------
The primary object of this release is to completely rebrand RTSDK:  change namespace to com.refinitiv, change library names and alter documentation to reflect new branding. A new file explaining impact to customer, REBRAND.md was also added. In addition, there were a few fixes included. This release also introduces qualification on CentOS8.

EMA C++ 3.6.0.L1 Issues Resolved
--------------------------------
- [RTSDK-3222] - Multiple config files shipped with EmaCpp
- [RTSDK-3367] - EMA CPP Config Guide contradicts between the Section Headings and the title for tables for Channel parameter
- [RTSDK-3671] - Shorten EMA example names to avoid build errors on windows
- [RTSDK-4151] - Change namespace to refinitiv
- [RTSDK-4397] - EMA C++ offstream posting payload decode issue - [Case Number: 09179092]

ETA C 3.6.0.L1 Issues Resolved
--------------------------------
- [RTSDK-4253] - Add in thread naming for the reactor worker thread
- [RTSDK-4310] - Replace versioning variables in header files to rtsdk
- [RTSDK-4372] - Invalid conversion between char and char\* in rwfToJsonConverter.cpp - [GitHub # 157 ]
- [RTSDK-4373] - Extraneous space before REAL_SHA1_INIT - [GitHub # 156]
- [RTSDK-4380] - Move ripcVer definitions to just rsslSocketTransportImpl.c instead of in the header

Both ETA C and EMA C++ 3.6.0.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-4056] - Qualification & build with CentOS8
- [RTSDK-4243] - Documentation: change code snippets to new namespace in all documentation (PDF)
- [RTSDK-4244] - Documentation: change namespace references and references to previous product names (PDF)
- [RTSDK-4266] - Documentation: change references to prior product names in code comments
- [RTSDK-4274] - Replace dependent libraries to re-branded for C/C++: DACS
- [RTSDK-4285] - Change QA tools to adapt to changes to filenames
- [RTSDK-4288] - Create REBRAND.md with customer impacts and add any changes to product names
- [RTSDK-4395] - Rebrand: Differentiate between RTSDK product and ETA/EMA library versions

---------------------------------------------
RTSDK C++/C Release 1.5.1.L1 (Sept 4, 2020)
---------------------------------------------

New Features Added
------------------
This is a maintenance release which resolves customer issues, bugs and adds support for the following: ability to measure tunnel stream performance, VS2019 builds and RedHat 8.X builds. Included in this release are rebranding changes.

EMA C++ 3.5.1.L1 Issues Resolved
--------------------------------
- [RTSDK-3882] - EMA C++: Setting literal as status text with Login::LoginRefresh::state() is unsafe! [GitHub #135]
- [RTSDK-4132] - EmaConfigImpl.cpp uses xmlCleanupParser wrongly, can cause memory corruption in multithreaded programs [GitHub #147]
- [RTSDK-4086] - EMA must check both DictionaryUsed and DictionaryProvided to download dictionary from network
- [RTSDK-4099] - EMACPP does NOT set HAS_SERVICE_ID flag on onStream postMsg with if it sets serviceName

ETA C 3.5.1.L1 Issues Resolved
--------------------------------
- [RTSDK-767] - Example and Training code print statements contain mis-spelling of the word "Received"
- [RTSDK-1570] - Calling rsslCloseServer does not call the function assigned to the trans function, shutdown server, for Socket Type connections
- [RTSDK-3211] - Deprecate TLS1.0
- [RTSDK-3219] - XML output rsslDoubleToString issue
- [RTSDK-3241] - Fix invalid usage of pthread_mutex_init() [GitHub Pull Request #99]
- [RTSDK-3638] - WS Transport: Automatic Login by passing token credentials during the initial WebSocket connection to the ADS via HTTP Cookies
- [RTSDK-3639] - Provided the ability for applications to access HTTP headers for WS open handshake and handshake response
- [RTSDK-3689] - Enhance ETA Performance tools to support Tunnelstreams
- [RTSDK-3850] - VS110 and VS120 json conversion test issues
- [RTSDK-3861] - Provides a configurable option to enable curl debugging message in ETA
- [RTSDK-3891] - Replace magic values with constants from rwfNet.h
- [RTSDK-3950] - Add in a programmatic way to access JSON converter library version
- [RTSDK-3981] - catch(std::bad_alloc) for 2 sequential new() leads to memory leak
- [RTSDK-4079] - rsslReactorConnect returned 0 when using an invalid network interface [Case Number: 08784579]
- [RTSDK-4166] - Tunnel Stream Performance Issue with un-needed events?
- [RTSDK-4182] - Consumer app doesn't apply subprotocol for encrypted websocket
- [RTSDK-4249] - Provider and VAProvider cannot bind port if setting compressionType to LZ4 when setting subprotocol

Both ETA C and EMA C++ 3.5.1.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-3646] - VS2019 Support: add build machine, add build support
- [RTSDK-3665] - Add DACSLock code snippet for ETA/EMA C++ into documentation
- [RTSDK-3697] - Build and ship libraries using RedHat 8.X
- [RTSDK-3902] - Document lsb_release requirement for cmake
- [RTSDK-3956] - Readme and text files have spelling errors
- [RTSDK-4090] - Rebranding: Change code references to "Refinitiv" in unit tests, examples, etc.
- [RTSDK-4091] - Support a configurable debug parameters to show REST interactions (that do not print credentials)
- [RTSDK-4165] - Rebranding: Change references in Code Comments and READMEs
- [RTSDK-4177] - Rebranding: Change references to RTSDK in Cmake build

---------------------------------------------
RTSDK C++/C Release 1.5.0.G1 (Jun 30, 2020)
---------------------------------------------

New Features Added
------------------
This is a maintenance GitHub push which resolves customer issus, bugs and adds support for the following: ability for providers to get round trip latency measurements, provider support for posting, permit server side socket to be reused and ability to configure takeExclusiveSignOn in RDP connectivity.

EMA C++ 3.5.0.G1 Issues Resolved
--------------------------------
- [RTSDK-504] Support Posting in EMA Providers [GitHub # 117]
- [RTSDK-1440] Rename EMAC++ Unit Test input files to be more descriptive
- [RTSDK-2587] Cons170 memory leak reported from valgrind
- [RTSDK-3292] Dictionary.entry(int fieldId) returns the same DictionaryEntry instance [Case Number: 07697024] [GitHub # 141]
- [RTSDK-3843] Support SO_REUSEADDR to permit server side socket to be reused for loadbalancing
- [RTSDK-3907] Ema Cons113 Example does NOT work with EncryptedProtocolType::RSSL_WEBSOCKET
- [RTSDK-3908] Support EMA RDP Websocket encrypted connection example 
- [RTSDK-3933] Support Round Trip Latency Monitoring
- [RTSDK-3988] Change EMA RDP example to take RIC as an input

ETA C 3.5.0.G1 Issues Resolved
--------------------------------
- [RTSDK-1650] rsslDoubleToReal conversion UPA C API lib function doesn't work as we expected [Case Number: 06708565]
- [RTSDK-3441] ETA Reactor API persistently retains memory and is not released until shutdown [Case Number: 07823520]
- [RTSDK-3819] Support Round Trip Latency Monitoring
- [RTSDK-3850] VS110 and VS120 json conversion test issues
- [RTSDK-3897] Access Violation Closing Reactor Tunnel (over SSL) [Github #139]
- [RTSDK-3963] Add ability to catch WSAEWOULDBLOCK  error
- [RTSDK-4069] Tunnel stream must notify application when login timeout occurs for authenticating a tunnel stream

Both ETA C and EMA C++ 3.5.0.G1 Issues Resolved
-----------------------------------------------
- [RTSDK-3859] JSON to RWF conversion passing thru errors in Time
- [RTSDK-3860] Invalid conversion of UINT64 when value on wire is -1
- [RTSDK-3903] Provide the ability to configure the takeExclusiveSignOnControl parameter for the password grant type
- [RTSDK-4084] EMA should not set compression threshold unless explicitly configured by application 

---------------------------------------------
RTSDK C++/C Release 1.5.0.L1 (Mar 31, 2020)
---------------------------------------------

New Features Added
------------------
This release introduces support for Websocket Transport in RTSDK with capabilities like compression, fragmentation and packing. With WS transport, user can choose either JSON (rssl.json.v2 aka tr_json2; tr_json2 will be deprecated) or RWF (rssl_rwf) data formats to send over the wire. Application layer will continue to receive data in RWF data format. In addition, conversion from RWF to JSON and vice versa is also available as part of librssl and as a separate shared library.

EMA C++ 3.5.0.L1 Issues Resolved
--------------------------------
- [RTSDK-3244] Catch polymorphic type by reference, not by value [GitHub Pull Request #97]
- [RTSDK-3274] EMAC++ 'OmmInvalidUsageException', Text='The Field name STOCK_TYPE does not exist in the field dictionary' [Case Number: 07645599]

ETA C 3.5.0.L1 Issues Resolved
--------------------------------
- [RTSDK-3616] Watchlist example issue: With service down, posting is still attempted
- [RTSDK-3805] Integrate rsslReactorQueryServiceDiscovery() method with centralized token management to reuse token when using same credentials 

Both ETA C and EMA C++ 3.5.0.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-3419] RTSDKC Websocket Transport Support 
- [RTSDK-3437] Add port info into OmmConsumerEvent::getChannelInformation [GitHub # 113]
- [RTSDK-3475] RTSDK C RWF<->JSON conversion
- [RTSDK-3818] Support for SNI (server name indication) in TLS communication in client side encryption with openSSL
- [RTSDK-3834] Update default token service URL to use verison v1

---------------------------------------------
RTSDK C++/C Release 1.4.0.G1 (Jan 10, 2020)
---------------------------------------------

EMA C++ 3.4.0.G1 Issues Resolved
--------------------------------
- [RTSDK-3472] Added support for NoWait dispatch timeout in Consumer and NiProvider [GitHub # 110] 
- [RTSDK-3596] Removed TLS 1.0 and 1.1 enumerations from EMA interface

ETA C 3.4.0.G1 Issues Resolved
--------------------------------
- [RTSDK-126] Fix to Ansi library build and runtime issues [Case Number: 04035985 GitHub #124]

Both ETA C and EMA C++ 3.4.0.G1 Issues Resolved
-----------------------------------------------
- [RTSDK-3594] Documentation changes with addtional rebranding changes

---------------------------------------------
RTSDK C++/C Release 1.4.0.L1 (Nov 15, 2019)
---------------------------------------------

New Features Added
------------------
This release adds Server Side Encryption support in EMA and ETA.

EMA C++ 3.4.0.L1 Issues Resolved
--------------------------------
- [RTSDK-3294] Enhancement Request: Added ability to dynamically increase number of allocated output buffers for handling "out of buffers" error [Case Number: 07652023]
- [RTSDK-3417] Documentation Issue: Specify in EMA Config guide, the precedence of configuration vectors
- [RTSDK-3495] Memory leak in C++/EMA (in OmmConsumer/OmmLoggerClient) [Case Number: 08003411 GitHub # 118]
- [RTSDK-3535] Inconsistency contents in default and description of ReissueTokenAttemptInterval and ReissueTokenAttemptLimit parameter [GitHub # 120]

ETA C 3.4.0.L1 Issues Resolved
--------------------------------
- [RTSDK-755] Trying to establish more than 2 encrypted connections fails returning an error, when using ETAC provider
- [RTSDK-3503] Memory Growth with RTSDK 1.3.X: Remove error struct from event to avoid memory usage
- [RTSDK-3504] Add option for maxOutputBuffer on ProvPerf
- [RTSDK-3505] Add ability of upacTransportPerf to support compressionType lz4 as the input argument
- [RTSDK-3538] Uptick version on non-open source libs only when binary version changes

Both ETA C and EMA C++ 3.4.0.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-3181] RTSDKC Server side encryption support
- [RTSDK-3202] RTSDK Documentation: Remove classic portal, refer to VARefman
- [RTSDK-3204] RTSDK Documentation: Fix copyright link after removal of classic portal from esdk repository
- [RTSDK-3500] Enhancement Request: Add ability to retrieve number of tunnel stream buffers in use
- [RTSDK-3340] Rebrand RTSDK documentation 

---------------------------------------------
RTSDK C++/C Release 1.3.1.G2 (Oct 18, 2019)
---------------------------------------------

Both ETA C and EMA C++ 3.3.1.G2 Issues Resolved
-----------------------------------------------
- [RTSDK-3265] Sporadic crash in RTSDK consumers with service interruptions [Case Number: 07573905,08051848 GitHub # 103,108]
- [RTSDK-2562] Shared pool buffers actively queued upon client disconnection are not cleaned up correctly [Case Number: 07010347]

---------------------------------------------
RTSDK C++/C Release 1.3.1.G1 (Sept 25, 2019)
---------------------------------------------

EMA C++ 3.3.1.G1 Issues Resolved
--------------------------------
- [RTSDK-3440] Provider fails to accept client connections with dispatch timeOut=NoWaitEnum [GitHub #110]

ETA C 3.3.1.G1 Issues Resolved
--------------------------------
- [RTSDK-3442] ipcWaitProxyAck should return RIPC_CONN_IN_PROGRESS for all success cases 
- [RTSDK-3453] ReactorChannel event -3 causing reactor to shutdown

Both ETA C and EMA C++ 3.3.1.G1 Issues Resolved
-----------------------------------------------
- [RTSDK-3468] Add RDP Auth proactive token renewal with password grant prior to refresh token expiration

---------------------------------------------
RTSDK C++/C Release 1.3.1.L1 (July 31, 2019)
---------------------------------------------

New Features Added
------------------
This release adds ability in EMA to clone and copy messages in order to decode payload outside of message callbacks. This release enables Realtime Cloud users to centralized session management per OAuth user shared between multiple connections. Please note that client_id is now a mandatory input for Cloud connectivity. RTSDK C/C++ now supports GCC 7.4.0. 

EMA C++ 3.3.1.L1 Issues Resolved
--------------------------------
- [RTSDK-509] Add InitializationTimeout to EMA Config at Channel Level
- [RTSDK-633] EmaCppConsPerf does not reach steady state occasionally [Case Number: 05594510]
- [RTSDK-900] Change documentation to reflect user-defined Config filename
- [RTSDK-1750] Clone/copy facility for message payload to decode it outside of onUpdateMsg() [Case Number: 06854285,5201994]
- [RTSDK-3238] Incorrect spelling in interface name in EmaCpp
- [RTSDK-3251] Fix extendedHeader typo where Status is returned instead of Close
- [RTSDK-3252] Please restore version headers, eg EmaVersion.h [GitHub #105]

ETA C 3.3.1.L1 Issues Resolved
--------------------------------
- [RTSDK-3176] Windows build warning
- [RTSDK-3182] Documentation, ETAJ Dev Guide: Fix "UPA" in Figure 36 to "Transport API Consumer App"
- [RTSDK-3184] Warning when building rsslTransportUnitTest on RH6
- [RTSDK-3185] Warning rsslRestClientImpl.c about RsslRestCurlHandleSumFunction
- [RTSDK-3202] RTSDK Documentation: Remove links to "Transport API Value Added Components" in html and refer to VARefman
- [RTSDK-3243] Fix application ID length in Eta Examples [GitHub #100]
- [RTSDK-3253] Compile Error: rsslTransport.h:1343:82: error: too many initializers for ‘RsslWriteInArgs’
- [RTSDK-3255] Enhance the Reactor to specify OAuth token credentials in rsslReactorOmmConsumerRole
- [RTSDK-3258] The rsslCreateReactor() method crashes when the RsslCreateReactorOptions.serviceDiscoveryURL is empty
- [RTSDK-3267] Expose ping stats and rsslReadEx in reactor
- [RTSDK-3269] Enhance the Reactor for applications to specify the password for OAuth via the callback method
- [RTSDK-3278] Enhance the Reactor error handling wrt to session mgmt POST
- [RTSDK-3285] Add ability to turn on debugging for encrypted connections to diagnose TLS issues on incoming packets
- [RTSDK-3295] Crash in reactor when multiple reactor channels are used with session management
- [RTSDK-3296] Provides centralized location to keep the OAuth tokens to share between multiple connections using the same OAuth credential

Both ETA C and EMA C++ 3.3.1.L1 Issues Resolved
-----------------------------------------------
- [RTSDK-3249] Support 32-bit builds with RTSDK 
- [RTSDK-3260] EMA log files exist in RTSDK AMI
- [RTSDK-3266] Add 32-bit DACS libraries into BinaryPack and open source ANSI
- [RTSDK-3272] Qualify RTSDK with GCC Version 7.4.0
- [RTSDK-3410] Removed extra "/" to service discovery URL to get an Elektron cloud endpoint

---------------------------------------------
RTSDK C++/C Release 1.3.0.G1 (April 16, 2019)
---------------------------------------------

EMA/ETA C/C++ 3.3.0.G1 Issues Resolved
--------------------------------
- [RTSDK-3194] Documentation improvements for RDP examples [GitHub #98]
- [RTSDK-3239] CMake fix for build of cjson libraries

---------------------------------------------
RTSDK C++/C Release 1.3.0.L1 (March 26, 2019)
---------------------------------------------

New Features Added
------------------
This RTSDK release provides support for RDP Session management (token renewal) and Service Discovery (discovering host/port information based on Cloud region and type of connection ). Also available is added support for encrypted transport using openSSL versions 1.0.X and 1.1.X on Windows and Linux for EMA C++ and ETA C. Also in this release, all external dependencies such as libxml2, zlib, lz2 rely on associated external distribution locations and incorporated into build using cmake. 

EMA C++ 3.3.0.L1 Issues Resolved
--------------------------------
- [RTSDK-484] EMA Consumer application that requests a streaming source directory does not receive source directory updates. [ Case 05257390 ]
- [RTSDK-619] RMTES Partial updates are not processed correctly if OmmRmtes.toString() is called before OmmRmtes.apply() is called [Case Number: 05533464, GitHub #74]
- [RTSDK-1245] Qualify Linux GCC 4.8.5
- [RTSDK-1480] Default CMAKE option in GSG package to be cmake -DUSE_PREBUILT_ETA_EMA_LIBRARIES=ON
- [RTSDK-1565] Turn on OpenSSL support for Windows Client connections
- [RTSDK-1611] Client side encryption
- [RTSDK-1622] Elektron-SDK-BinaryPack should be optional, client can't download external resources via git [Case Number: 06643952]
- [RTSDK-1626] Update OpenSSL usage to support both 1.0.X and 1.1.X interfaces at run-time
- [RTSDK-1687] Use Cmake to obtain Zlib from GitHub
- [RTSDK-1688] Use Cmake to obtain Libxml2 from GitHub
- [RTSDK-1714] Provides interface design and implementation for EMACPP to support session managment from the Reactor
- [RTSDK-1760] Fix uname program name in cmake setup [GitHub #81]
- [RTSDK-2599] Require a new utility or interface similar to asHexString that shows raw hex output [Case Number: 07023993]
- [RTSDK-2678] Expose initializationTimeout configuration and make default to higher value for Encrypted

ETA C 3.3.0.L1 Issues Resolved
--------------------------------
- [RTSDK-132] ETAC WL consumer example with encrypted connection is crashing when channel initialization fails
- [RTSDK-627] Remove references to UPA Developers Guide [Case Number: 05543578]
- [RTSDK-212] Incorrect syntax for command line argument example with upacTransportPerf example
- [RTSDK-1245] Qualify Linux GCC 4.8.5
- [RTSDK-1565] Turn on OpenSSL support for Windows Client connections
- [RTSDK-1611] Client side encryption
- [RTSDK-1626] Update OpenSSL usage to support both 1.0.X and 1.1.X interfaces at run-time
- [RTSDK-1628] ETAC: Extend OpenSSL usage to verify the certificate
- [RTSDK-1687] Use Cmake to obtain Zlib from GitHub
- [RTSDK-1688] Use Cmake to obtain Libxml2 from GitHub
- [RTSDK-1710] Provides HTTP requests for blocking and non-blocking call for ETAC
- [RTSDK-1716] Implements RDP service discovery and token management for ETAC reactor
- [RTSDK-1746] Update ETA examples to connection using HTTPS connection type with/without a proxy
- [RTSDK-1747] Fix Cpp-C ANSI and DACS Guide links in reference manual
- [RTSDK-2603] CMake changes for new add external project cmake modules
- [RTSDK-2605] Remove references to TS1 Parser

---------------------------------------------
RTSDK C++/C Release 1.2.2.L1 (November 15, 2018)
---------------------------------------------

New Features Added
------------------
Provides the functionality for Non-interactive, Interactive, and Consumer applications to get channel information from the EMA's callback methods via OmmProviderEvent and OmmConsumerEvent classes

EMA C++ 3.2.2.L1 Issues Resolved
--------------------------------
- [RTSDK-632] Elektron SDK EmaCppConsPerf latencyFile doesn't create log file [Case Number: 05541113]
- [RTSDK-1125] EMA ConsPerf applications do not use specified username in Login Request [Case Number: 05958811]
- [RTSDK-1517] Unable to exit with EMAC multithread app
- [RTSDK-1601] Provide channel information in EMA's callback methods [Case Number: 06611113]
- [RTSDK-1751] Remove undefined increment operator behavior [GitHub Pull Request #80]
- [RTSDK-1753] Add support for WindowsServer2016
- [RTSDK-1723] IProvider application with UserDispatch has 100% cpu
- [RTSDK-2543] Change to EMA Devlopers Guide to accurately show map encoding follow up to RTSDK-1323

ETA C 3.2.2.L1 Issues Resolved
--------------------------------
- [RTSDK-647] EMAJ or ETAJ consumer sends duplicate FIDs in a snapshot view request
- [RTSDK-1753] Add support for WindowsServer2016 
- [RTSDK-2550] ETA RDM Usage guide section 6.2.4 shows market price update instead of status [Developer Community]

---------------------------------------------
RTSDK C++/C Release 1.2.1.L1 (August 15, 2018)
---------------------------------------------

New Features Added
------------------
Programmatic configuration for EMA IProvider and NIProvider.

EMA C++ 3.2.1.L1 Issues Resolved
--------------------------------
- [RTSDK-380] If CompressionType is set to "None", the CompressionThreshold range check still occurs
- [RTSDK-398] XMLTrace may not flush all information to trace file
- [RTSDK-405] Example 421 is not using the Dictionary_1 and Logger_1 defined in the code [Case Number: 04296327]
- [RTSDK-415] Clarify parent handle usage in EMA interface [Case Number: 05109877]
- [RTSDK-430, RTSDK-1323, RTSDK-1552] EMA C++ crashes when encoding a large Map [Case Numbers: 05354708, 06292070, GitHub #54]
- [RTSDK-635] EMA C++ Compiler warnings [Case Number: 05830919]
- [RTSDK-1496] Double login reissue & Exception with EMA C++ NIProvider (430)
- [RTSDK-1529] Ema Example Cons100 valgrind errors when EmaConfig.xml is present
- [RTSDK-1548] Update RDMUsageGuide to include information on the required filters to mark a service back Up [Case Number: 06538048]
- [RTSDK-1556] Update Doxygen for OmmDateTime, OmmDate and OmmTime [GitHub #55]
- [RTSDK-1560] Provide ability to modify the configuration programmatically for IProvider [Case Number: 06548186]
- [RTSDK-1593] Migration Guide Issues with CMake Elektron SDK 1.2
- [RTSDK-1595] Calling toString on a newly created message throws Access Violation Exception [Case Number: 06484891]
- [RTSDK-1624] Can't build Elektron-SDK1.2.0.win.rrg on MS Windows [Case Number: 06612117]
- [RTSDK-1644] Fix README content Github to have change log (without duplicating information)

ETA C 3.2.1.L1 Issues Resolved
------------------------------
- [RTSDK-380] If CompressionType is set to "None", the CompressionThreshold range check still occurs
- [RTSDK-398] XMLTrace may not flush all information to trace file
- [RTSDK-1423] Warnings ( 240 ) when doing build all
- [RTSDK-1574] Check for empty string instead of null pointer [GitHub #61]
- [RTSDK-1593] Migration Guide Issues with CMake Elektron SDK 1.2
- [RTSDK-1624] Can't build Elektron-SDK1.2.0.win.rrg on MS Windows [Case Number: 06612117]
- [RTSDK-1635] ETA should not have EDF/Queue examples
- [RTSDK-1636] Consumer Module_2_Login training example does not properly fall through
- [RTSDK-1644] Fix README content Github to have change log (without duplicating information)
- [RTSDK-1659] ETA Consumer reserves too little space for AuthenticationToken

------------------------------------------
RTSDK C++/C Release 1.2.0.G1 (May 31, 2018)
------------------------------------------

EMA C++ 3.2.0.G1 Issues Resolved
--------------------------------
- [RTSDK-1572] IProvider application hits 100% CPU in API dispatch mode after Consumer disconnects [Case Number: 06564982]

ETA C 3.2.0.G1 Issues Resolved
------------------------------
- [RTSDK-1573] rsslNumericStringToReal() conversion error [GitHub #62]

--------------------------------------------
RTSDK C++/C Release 1.2.0.L1 (April 27, 2018)
--------------------------------------------

New Features Added
------------------
RTSDK C/C++ now utilizes a CMake build environment. Refer to the RTSDK C/C++ Migration Guide for detailed instructions regarding how to build EMA C++ with CMake.

RTSDK C/C++ now supports Visual Studio 2017.

EMA now supports encrypted connection type.

ETA C is now fully open sourced except for reliable multicast transport and VA cache. Open source transports include TCP, HTTP, HTTP encrypted, shared memory and sequenced multicast transport types. The OMM encoder and decoder have also been open sourced.

Note that the memory footprint has increased this release due to the following:

Around 20 MB is introduced by changing the container type for handling message fragmentation. The hash table is initialized when a rsslChannelImpl is created which ETA allocates 10 of them upfront for this first initialization of Rssl library.

Around 4.6 MB is introduced by the new functionality of RsslDataDictionary to look up
RsslDictionaryEntry by name (rsslDictionaryGetEntryByFieldName).

EMA C++ 3.2.0.L1 Issues Resolved
--------------------------------
- [RTSDK-487] EMA throws OmmInvalidUsageException if an empty Map is encoded [Case No. 05338640 and GitHub #28]
- [RTSDK-813] Date/Time/DateTime to string and from string conversions should support ISO 8601 format.
- [RTSDK-907] EMA can't handle a SERVICE_DIRECTORY refresh that contains a lot of services [Case No 05896732, 06042281 and 06443659]
- [RTSDK-1145] Add const to EMAString
- [RTSDK-1194] Expose encrypted connection support through EMACPP
- [RTSDK-1280] Remove duplicated assignments [GitHub pull request #45]
- [RTSDK-1290] ripc sslName cryptoName copy limits to 8 bytes
- [RTSDK-1359] Add VS2017 to RTSDK

ETA C 3.2.0.L1 Issues Resolved
------------------------------
- [RTSDK-709] No genericmsg be fan out to the client on directory domain stream
- [RTSDK-901] EMA does not honor the filters on the directory request message [Case No. 05881972]
- [RTSDK-1262] Fix bigBufferPoolCleanup for loop [GitHub Pull Request #43]
- [RTSDK-1280] Remove duplicated assignments [GitHub pull request #45]
