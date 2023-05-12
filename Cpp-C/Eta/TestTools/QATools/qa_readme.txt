Definition of QATools: The purpose of QATools is to test variations of examples (see Applications/Examples). For each example that is altered to run a test, there will be a directory here to represent that variation. Example: If value add consumer was altered 2 times to do 2 different tests, there will be directories here, such as, "vaconsumer-*01" and "vaconsumer-*02". In each directory are the files that have been altered. These directories contain ONLY the files that were altered for that example.

How to use QATools: For each QATool direcotry, user must copy or overlay the source files from that directory into the original location in Applications/Examples where the entire source for an example exists.  The user must re-build the code to run the altered example.

Disclaimer:  Please note that this is not a comprehensive list of all test variations used in test.

List of altered code directories:
-----------------------------------------------------------------------------------------

Module:  Consumer
-----------------

consumer-LoginReissue-001:  Alter Consumer to send a login reissue with PAUSE-ALL after some item updates and a
RESUME-ALL after receiving a source directory update.

consumer-Dict-001: Altered the code of the Consumer application to both read the dictionary from file
and download from the netowork. Once the dictionaries are loded they are compared to see if they are the same.
This is testing the encoding of dictionary messages as multi parts refreshes.

consumer-OMMSeq-001: Alter Consumer to have the following new options and implementation:
    Send Directory Request without Login (-sendDirReqWithoutLogin)
	Send Dictionary Request without Login (-sendDictReqWithoutLogin)
	Send Item Request without Login (-sendItemReqWithoutLogin)
	Send Reissue on Directory Stream with filter value specified with argument(-reissueDir 3)
	Send Reissue on Dictionary Stream(-reissueDict 1)
	Load Dictionary from file (-loadDictFromFile)
	Send Item Reissue with Qos (-reissueQos)
	Send Item Reissue with Worst Qos (-reissueQosAndWorstQos)
	Send Generic Msg On Login Stream (-sendGenericMsgOnLogin)
	Send Generic Msg On Directory Stream (-sendGenericMsgOnDirectory)
	Request Directory with filter value specified with argument after login refresh received(-requestDir 1)
	Request Dictionary with filter value specified with argument(-requestDict 15)
	Send Item Reissue with ServiceId change (-reissueServiceId)
	Set Private Stream Flag for Item Request(-privateStreamForItemReq)
    Two Market Price Item Request with same key and different streamId (-requestSameKeyWithDiffStreamId)

consumer-DebugDump-001: Alter Consumer to print debug dumps and add options -dumpIpcIn, -dumpIpcOut, -dumpIpcComp, -dumpIpcInit, -dumpRsslIn, -dumpRsslOut.

consumer-TracePing-001: Alter Consumer to trace Ping messages. Add option -xping, -x1, -xnowrite, -xnoread.


Module:  Value Add Consumer
---------------------------

vaconsumer-LoginReissue-001:  Alter VAConsumer to send a login reissue with PAUSE-ALL after some item updates 
and a RESUME-ALL after receiving a source directory update.

vaconsumer-TsFrag-001:  Alter VAConsumer to accept -bufSize and -fillSize as inputs.  The bufSize is the
size of buffer requested by getBuffer().  The fillSize is is the number of bytes written.  For example,
if bufSize is 1000 and fillSize is 900, then the VAConsumer gets a buffer of 1000 bytes but only writes
900 bytes before sending. The filled buffer is sent to the provider/ads in fragments if larger than max
fragment size.  This code change fills the buffer with "1, 2, 3....255" and repeats this pattern of data
up to the fillSize.  The provider will then need to check for the same pattern in the fully assembled
message and print "TEST PASSED".

vaconsumer-TsFrag-002:  Alter VAConsumer to accept -msgSize as an input.  This is the message size
  application gets and then writes data into.  Note that the message is a generic message with an
  opaque buffer as its data body.  Typically, the opaque buffer data body is 10 bytes less than the
  msgSize with the generic message header being the other 10 bytes.  This size of message is then sent
  to the provider/ads in fragments if larger than max fragment size.  This code change fills the opaque
  buffer data body with "1, 2, 3....255" and repeats this pattern of data.  The provider (common code)
  will then need to check for the same pattern in the fully assembled message and print "TEST PASSED".

vaconsumer-ChnlStats-001: Alters VAConsumer to set statisticFilter for testing ReactorChannelStatistics base on Filer READ, WRITE or PING.
  And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.

vaconsumer-ChnlStats-003: Alters VAConsumer to set statisticFilter for testing ReactorChannelStatistics base on Filer READ, WRITE or PING. 
  And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.
  The new configuration parameters introduce in version 1.4.0 for reactor to test 'restRequestTimeout', 'tokenReissueRatio', 'reissueTokenAttemptLimit' and 'reissueTokenAttemptInterval'.

vaconsumer-ChnlStats-002: Alters VAConsumer to create 2 connections with 1 reactor, each connection requests login using different uname and passwd, 
  in order to test ReactorChannelStatistic and sessionMgnt for 2 connections. And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.

vaconsumer-ChnlStats-004: Alters VAConsumer to create 2 connections with 1 reactor, each connection requests login using different uname and passwd, 
  in order to test ReactorChannelStatistic and sessionMgnt for 2 connections. And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.
  The new configuration parameters introduce in version 1.4.0 for reactor to test 'restRequestTimeout', 'tokenReissueRatio', 'reissueTokenAttemptLimit' and 'reissueTokenAttemptInterval'.

vaconsumer-DebugFunctions-001:  Alter VAConsumer to show how to use rsslSetDebugFunctions

vaconsumer-Ws-001: Alter VAConsumer to add test options for Websocket transport which are; -testCompressionZlib, -jsonExpandEnum

vaconsumer-Ws-002: Alter VAConsumer to post enum FIDs add test options for Websocket transport which are; -testCompressionZlib, -jsonExpandEnum

vaconsumer-Ws-003: Alter VAConsumer to send request as json packed buffer instead of batch request.

vaconsumer-Ws-004: Alter VAConsumer to test handle error case when it sends request as json packed buffer.

vaconsumer-Ws-005: Alter VAConsumer to send Post Key Attribute as FieldList and Domain Contribution.

vaconsumer-OAuthV2-001: Alter VAConsumer to create 1 reactor, 2 connections, both for OAuth V2 but can identify either same / diff credential.

vaconsumer-OAuthV2-002: Alter VAConsumer to create 1 reactor, 2 connections, 1st connection is for STS and 2nd connection is for OAuth V2.

vaconsumer-OAuthV2-003: Alter VAConsumer to create 1 reactor, 6 connections, 1st, 2nd and 3rd connection are for STS. And 4th, 5th, and 6th connection are for OAuth V2.

vaconsumer-Jwt-001: Alter VAConsumer to create 1 reactor, 2 connections, both for OAuth V2 JWT but can identify either same / diff credential

vaconsumer-CpuBind-001: Alter VAConsumer to add command-line options for testing bind CPU API.
 Command line options.
-mainThreadBindVer specifies the method of API call to bind main thread: 1 - use regular (default), 2 - use extended,
   i.e. when 1 - will call rsslBindThread() to bind CPU specified in "-mainThreadCPU" for the main thread of VAConsumer,
        when 2 - will call rsslBindThreadEx() to bind CPU specified in "-mainThreadCPU" for the main thread of VAConsumer;
-mainThreadCPU specifies the CPU to bind for main VAConsumer thread: Cpu core id or P:X C:Y T:Z format;
-workerThreadCPU specifies the CPU to bind for Reactor worker thread: Cpu core id or P:X C:Y T:Z format.

consumer-Ws-001: Alter Consumer to add test options for Websocket transport which are; -testCompressionZlib

Module:  Watchlist Consumer
---------------------------

wlconsumer-LoginReissue-001:  Alter WLConsumer to send a login reissue with PAUSE-ALL after some item updates 
and a RESUME-ALL after receiving a source directory update.

wlconsumer-Auth-001:  Alter WLConsumer to take a "-at2" input that permits user to specify a 
2nd token used in a token renewal.

wlconsumer-Auth-002:  Alter WLConsumer to send PAUSE_ALL after 5 update and a RESUME_ALL after
receiving a source directory update.  Also WLConsumer is altered to take a "-at2" input that 
permits user to specify a 2nd token used in a token renewal.

wlconsumer-Auth-003:  Alter WLConsumer to send item pause after 5 updates on stream id 5
and item pause after 10 updates on stream id 6. Do a item resume after 20 updates on stream id 6
Also WLConsumer is altered to take a "-at2" input that permits user to specify a
2nd token used in a token renewal.  NOTE:  This test must be done with two like items and one different item.
Example:  "-mp TRI -mp TRI -mp IBM"

wlconsumer-Auth-004:  Alter WLConsumer to send PAUSE_ALL after 5 update. Also WLConsumer 
is altered to take a "-at2" input that permits user to specify a 2nd token used in a token 
renewal. NOTE: This is very similar to wlconsumer-Auth-002 except that it does not resume
Targetted for the 3.1 release are two main features for which there will be code changes specific 
to that feature:
  Below, 02001 can be read as "02" and "001".  The "02" represents 2nd feature of the release.
  The "001" is the 1st code change for this feature. 

wlconsumer-TsFrag-001:  Identical to vaconsumer-TsFrag-001, except that this altered watchlist 
  consumer application. 

wlconsumer-TsFrag-002:  Identical to vaconsumer-TsFrag-002, except that this altered watchlist 
  consumer application. 

wlconsumer-ConsFunc-001:  Altered WLConsumer to accept new command line arguments.
MP Items can now be requested as snapshot, private stream, and/or view.
Example: "-mp IBM:SPV3" is IBM with all three and viewId 3,
"-mp GOOG:V5P" requests GOOG with viewId 5 and private stream
(Order of SPV is not significant as long as viewId number immediatly follows the 'V').
If neither snapshot, private stream, or view are needed, it can be written as "-mp IBM"
List of MP Items in command line are treated as if indexed(starting at index 0),
and can be specified to be requested after a certain number of events.
(3 events are defined: -e1 is for source directory updates, -e2 is for item update messages,
-e3 is for channel_down_reconnecting events)
Example: doing "-mp IBM -mp GOOG:PV3 -mp TRI:S -mp NFLX:PSV7 -AAPL -e2 7::1:2 -e2 9::4 -e3 8::3 "
will initially only request IBM, and after 7 update messages,
it will request mp items indexed 1 through 2, so GOOG and TRI.
After 8 channel-down-reconnecting events, it will request item indexed 3 only
since there is no end index given, it is not treated a a range (so just NFLX)
Similarily, after 9 update events, it will request item indexed 4 (AAPL)
Initial requests can be delayed until source dir refresh by specifying '-delayIntialRequest'
Single open and allow suspect data can be specified by setting '-singleOpen 0(or 1)'
and '-allowSuspect 0(or 1)' (default set to 1)
Note: Only single digit viewId's are supported;
Note: Up to 9 mp arguments can be given when used with the event specified delay request
(so, if no -e1/e2/e3 etc arguments are given, then more than 9 mp items are fine)

wlconsumer-ConsFunc-002:  Altered WLConsumer to accept tokenServiceUrl, serviceDiscoveryUrl 
for testing sessionMgnt, it fixes tokenReissueRatio to 0.1 (10 seconds)

wlconsumer-GenM-001:  Alters WLConsumer to send genericMsg on login, directory and 
market price streams. 
GenericMsg contains ElementList with one ElementEntry. 
Also added is the ability to decode genericMsg from a provider. 
This works in conjuction with the VAProvider code change from vaprovider-GenM-001 

wlconsumer-ConnRec-001: Alters WLConsumer to be able configure multiple providers for connection recovery using 
the following command line inputs.
  Command Line Arguments:
    "-h2" input that permits user to specify second host to connect to
    "-p2" input that permits user to specify second port to connect to
    "-h3" input that permits user to specify third host to connect to
    "-p3" input that permits user to specify third port to connect to
    "-attemptLimit"  input that permits user to specify ReconnectAttemptLimit
    "-numConnections"  input that permits user to specify number of connections
    "-minDelay"  input that permits user to specify ReconnectMinDelay
    "-maxDelay"  input that permits user to specify ReconnectMaxDelay
Sample usage: -h2 localhost -p2 14025 -h3 localhost -p3 14026 -attempLimit -1 -numConnections 3 -minDelay 5000 -maxDelay 30000

wlconsumer-RtoRecovery-001: Alters WLConsumer to get use the RTSDK apigateway test tool to delay the response of the token reissue at least 5 seconds
before calling the rsslReactorQueryServiceDiscovery() method to establish a reactor channel after closing the previous one.

Module:  Provider 
-----------------

provider-Login-001: Alters Provider to not respond with login refresh when a login request is received

provider-Src-001: Alters Provider to not respond with source directory refresh when source directory request is received

provider-Src-002: Alters Provider to respond with a source directory refresh containing 100 services 
                  when source directory is requested.

provider-Item-001: Alters Provider to not send a market price refresh when market price request is received     
provider-Item-002: Alters Provider to send only 1 refresh and 10 updates when market price request received. 

provider-Item-003: Alters Provider to not send RSSL_DMT_SYMBOL_LIST capability in source directory refresh

provider-Item-004: Alters Provider to send AcceptingRequests false in function initServiceStateInfo

provider-Dict-001: Alters Provider to not return failure if it receives a source directory request and
not send dictionary reject if it receives an invalid dictionary request. This is done by commenting out
sendDictionaryRequestReject in rsslDictionaryProvider.c.

provider-Multi-001: Alters Provider to enable serverSharedSocket = RSSL_TRUE.

provider-Ws-001: Alter Provider to add test options for Websocket transport which are; -testCompressionZlib, -compressionLevel

provider-DebugDump-001: Alter Provider to print debug dumps and add options -dumpIpcIn, -dumpIpcOut, -dumpIpcComp, -dumpIpcInit, -dumpRsslIn, -dumpRsslOut.
	Sample usage for sending source directory response.
	./Provider -pl "rssl.json.v2,rssl.rwf" -dumpRsslOut
	./Consumer -c websocket -pl "rssl.json.v2"

provider-TracePing-001: Alters Provider to trace Ping messages. Add option -xping, -x1, -xnowrite, -xnoread.


Module:  VA Provider 
-----------------

vaprovider-TsPost-001:
	1. Alters VAProvider to process a generic message with a nested market price request and respond with a generic messagae which contains a market price refresh message. Altered code is in simpleTunnelMsgHandler.c. However, all files are saved to build a standalone VAProvider. 
	2. Alters VAProvider to handle incoming tunnel stream Post message. Sends back Ack message if the flag "applyAck" has been set on Post message.Added additional command line options/arguments: 
		-sendNack: sends negative acks if post message is received (defaults to false);
		-rejectLogin: rejects main login request (defaults to false);
		-rejectTsLogin: rejects the tunnel stream login request (defaults to false).
		Example: ./VAProvider -p 14002 -s DIRECT_FEED -sendNack -x

vaprovider-GenM-001:  Alters VAProvider to send genericMsg on login, directory and 
market price streams. 
GenericMsg contains ElementList with one ElementEntry. 
Also added is the ability to decode genericMsg from a consumer. 
This VAProvider works in conjuction with the watchlist consumer code change from 
wlconsumer-GenM-001 

vaprovider-LoginDelay-001:  Alters VAProvider to delay sending of a login refresh message on the number of main-loop cycles (added test option "-delaySendLoginRefresh <count>").
It allows sending and handling Generic messages before VAProvider sends a login refresh message.
Sample usage: VAProvider -rtt -delaySendLoginRefresh 2
VAConsumer -rtt; Consumer -rtt

vaprovider-TsFrag-001:  Alter VAProvider to validate incoming tunnel stream message content to have "1, 2, 3...255".  If the content is as expected, this code change prints a "TEST PASSED".  This code change works in conjunction with vaconsumer-TsFrag-001.

vaprovider-TsFrag-002:  Alter VAProvider to validate incoming tunnel stream message as a generic message with opaque buffer data body of "1, 2, 3....255" repeated.  If the content is as expected, this code change prints a "TEST PASSED".  This code change works in conjunction with vaconsumer-TsFrag-002.

vaprovider-Ws-001: Alter VAProvider to add test options for Websocket transport which are; -testCompressionZlib, -compressionLevel, -sendGenericMessage and -jsonExpandEnum

vaprovider-Ws-002: Alter VAProvider to add test options for Websocket transport -catchUnknownJsonKeys and decode FieldList data

Module:  NIProvider 
-----------------

niprovider-Item-001: Alters NIProvider to send only 1 refresh and 2 updates for each item specified on command line 


Module: Reactor
-----------------

etareactor-001: Alters reactor worker thread to print calculated reconnect delay for connection recovery

etacconsperf-Rto-001
    Performance tool with ability to connect to RTO. Requests one item by default; this item is the 1st one in the list specified in 350k.xml
    Alters consPerfConfig.c, consPerfConfig.h, consumerThreads.c to connect to RTO, requires CLI credentials.
    Run ConsPerf. Sample Cmd:

    # ETA session management with reactor socket no proxy 
    ConsPerf -serviceName ELEKTRON_DD \
         -uname <username> -password <password> -clientId <clientId> \
         -steadyStateTime 300 -sendBufSize 65000 -recvBufSize 65000 -inputBufs 2048 \
         -itemFile 350k.xml -outputBufs 5000  \
         -connType encrypted -encryptedConnectionType socket \
         -sessionMgnt -reactor

    If -sessionMgnt is enabled should set -reactor or -watchlist

Module: ETA
-----------------

eta-rsslWrite-001: Alters ETA methods rsslWrite and rsslWriteEx to display that the parameter writeFlags contains RSSL_WRITE_DIRECT_SOCKET_WRITE or not.
	rsslWrite/rsslWriteEx will print info about RSSL_WRITE_DIRECT_SOCKET_WRITE:
	- on the first usage of the method;
	- every time from call to call when the writeFlags parameter changes its value.


Module: Provider Performance tool
-----------------

provperf-DebugDump-001: Alter ProvPerf to print debug dumps and add options -dumpIpcIn, -dumpIpcOut, -dumpIpcComp, -dumpIpcInit, -dumpRsslIn, -dumpRsslOut.
	Sample usage for sending Websocket+JSON packed messages:
	./ProvPerf -pl "rssl.json.v2,rssl.rwf" -tickRate 1 -updateRate 3 -latencyUpdateRate 0 -maxPackCount 3 -dumpRsslOut
	./ConsPerf -connType websocket -pl "rssl.json.v2" -tickRate 1 -itemCount 3

provperf-InitCpuIdLib-001: Alter ProvPerf control initialization CpuId library and add option -shouldInitCpuIdLib.
