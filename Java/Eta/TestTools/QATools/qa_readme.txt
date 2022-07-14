Definition of QATools: The purpose of QATools is to test variations of examples (see Applications/Examples). For each example that is altered to run a test, there will be a directory here to represent that variation. Example: If VAConsumer was altered 2 times to do 2 different tests, there will be directories here, such as, "vaconsumer*01" and "vaconsumer*02". In each directory are the files that have been altered. These directories contain ONLY the files that were altered for that example.

How to use QATools: For each QATool direcotry, user must copy or overlay the source files from that directory into the original location in Applications/Examples where the entire source for an example exists.  The user must re-build the code to run the altered example. 

Disclaimer:  Please note that this is not a comprehensive list of all test variations used in test.

List of altered code directories:
-----------------------------------------------------------------------------------------

Module:  Consumer 
-----------------

consumer-LoginReissue-001:  Alter Consumer to send a login reissue with PAUSE-ALL after some item updates and a RESUME-ALL after receiving a source directory update.

consumer-Dict-001: Altered the code of the Consumer application to both read the dictionary from file and download from the network. Once the dictionaries are loded they are compared to see if they are the same.  This is testing the encoding of dictionary messages as multi parts refreshes.


consumer-Item-001: Alters consumer to decode Date/Time/DateTime and print using ISO 8601

consumer-HttpReconnect-001:  Alters consumer which calls chnl.channel().reconnectClient(error) when receive login response.

consumer-Ws-001: Alters Consumer to test compression, user can pass option -compressionType to either 'ZLib' or 'LZ4'.
consumer-Ws-002: Alters Consumer to test compression Zlib -testCompressionZlib.
					  This app also shows bytesRead and bytesWritten for compressed and uncompressed stats


Module:  Value Add Provider 
---------------------------

vaprovider-TsPost-001: Alters VAProvider to handle incoming tunnel stream Post message. Sends back Ack message if the flag "applyAck" has been set on Post message. Added additional command line options/arguments:
	-sendNack: sends negative acks if post message is received (defaults to false);
	-rejectLogin: rejects main login request (defaults to false);
	-rejectTsLogin: rejects the tunnel stream login request (defaults to false).

	Example: ./gradlew runVAProvider -PcommandLineArgs="-p 14002 -s DIRECT_FEED -sendNack -x"

vaprovider-TsFrag-001:  Alter VAProvider to validate incoming tunnel stream message content to have "1, 2, 3...255".  If the content is as expected, this code change prints a "TEST PASSED".  This code change works in conjunction with vaconsumer-TsFrag-001.

vaprovider-TsFrag-002:  Alter VAProvider to validate incoming tunnel stream message as a generic message with opaque buffer data body of "1, 2, 3....255" repeated.  If the content is as expected, this code change prints a "TEST PASSED".  This code change works in conjunction with vaconsumer-TsFrag-002.

vaprovider-GenM-001:   Alters VAProvider to send genericMsg on login, directory and market price streams. 
GenericMsg contains ElementList with one ElementEntry. 
Also added is the ability to decode genericMsg from a consumer. 
This VAProvider works in conjuction with the watchlist consumer code change from wlconsumer-GenM-001

vaprovider-ProvFunc-001: Adds command line arguments to turn set open window to specified value. Also altered to accept view requests.

vaprovider-ProvFunc-002: Market price refresh/updates were altered to send out negative FIDs 

vaprovider-Ws-001: Alters VAProvider to test compression, user can pass option -compressionType to either 'ZLib' or 'LZ4' and option -compressionLevel to 0-9.
vaprovider-Ws-002: Alters VAProvider to test sending a big one dictionary message without multi-part it into many parts, user can pass option -testCompressionZlib, -compressionLevel to 0-9, , -maxFragmentSize (i.e 65536).

Module:  Provider 
---------------------------

provider-Item-001:    Alters Provider which send TEMPORARY_REJECT on every 5th item request from consumer. Expand OPEN_LIMIT to 50000.
provider-Ws-001:      Alters Provider to test compression, user can pass option -compressionType to either 'ZLib' or 'LZ4' and option -compressionLevel to 0-9.
provider-Ws-002:      Alters Provider to test sending a big one dictionary message without multi-part it into many parts, 
					  And option to test compression Zlib, identify compressionLevel and maxMsgSize for websocket connection, user can pass option -testCompressionZlib, -compressionLevel to 0-9, -maxFragmentSize (i.e 65536).
					  This app also shows bytesRead and bytesWritten for compressed and uncompressed stats.
provider-Ws-003:      Alters Provider to test sending multi-part dictionary, 
					  And option to test compression Zlib, identify compressionLevel and maxMsgSize for websocket connection, user can pass option -testCompressionZlib, -compressionLevel to 0-9, -maxFragmentSize (i.e 65536).
					  This app also shows bytesRead and bytesWritten for compressed and uncompressed stats.
provider-Ws-004:      Alters Provider to test sending a big one dictionary message without multi-part it into many parts, 
					  And option to test compression Zlib, identify compressionLevel and maxMsgSize for websocket connection, user can pass option -testCompressionZlib, -compressionLevel to 0-9, -maxFragmentSize (i.e 65536).
					  This app also shows bytesRead and bytesWritten for compressed and uncompressed stats.
					  This app modified ProviderSession to expand guaranteedOutpuBuffer.

Module:  Value Add Consumer 
---------------------------

vaconsumer-LoginReissue-001:  Alter VAConsumer to send a login reissue with PAUSE-ALL after some item updates and a RESUME-ALL after receiving a source directory update.

vaconsumer-TsFrag-001:  Alter VAConsumer to accept -bufSize and -fillSize as inputs.  The bufSize is the size of buffer requested by getBuffer().  The fillSize is is the number of bytes written.  For example, if bufSize is 1000 and fillSize is 900, then the VAConsumer gets a buffer of 1000 bytes but only writes 900 bytes before sending. The filled buffer is sent to the provider/ads in fragments if larger than max fragment size.  This code change fills the buffer with "1, 2, 3....255" and repeats this pattern of data up to the fillSize.  The provider will then need to check for the same pattern in the fully assembled message and print "TEST PASSED".

vaconsumer-TsFrag-002:  Alter VAConsumer to accept -msgSize as an input.  This is the message size application gets and then writes data into.  Note that the message is a generic message with an opaque buffer as its data body.  Typically, the opaque buffer data body is 10 bytes less than the msgSize with the generic message header being the other 10 bytes.  This size of message is then sent to the provider/ads in fragments if larger than max fragment size.  This code change fills the opaque buffer data body with "1, 2, 3....255" and repeats this pattern of data.
The provider will then need to check for the same pattern in the fully assembled message and print "TEST PASSED".

vaconsumer-Tunneling-001:  Alters VAConsumer to set tcpNoDelay to true for http and encrypted type connections. 
This adds the following into Consumer.java:
chnlInfo.connectOptions.connectionList().get(0).connectOptions().tcpOpts().tcpNoDelay(true)

vaconsumer-ChnlStats-001: Alters VAConsumer to set statisticFilter for testing ReactorChannelStatistics base on Filer READ, WRITE or PING.
And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.

vaconsumer-ChnlStats-002: Alters VAConsumer to create 2 connections with 1 reactor, each connection requests login using different uname and passwd, 
in order to test ReactorChannelStatistic and sessionMgnt for 2 connections. And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.

vaconsumer-ChnlStats-003: Alters VAConsumer to set statisticFilter for testing ReactorChannelStatistics base on Filer READ, WRITE or PING. 
And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.
The new configuration parameters introduce in version 1.4.0 for reactor to test 'restRequestTimeout', 'tokenReissueRatio', 'reissueTokenAttemptLimit' and 'reissueTokenAttemptInterval'.

vaconsumer-ChnlStats-004: Alters VAConsumer to create 2 connections with 1 reactor, each connection requests login using different uname and passwd, 
in order to test ReactorChannelStatistic and sessionMgnt for 2 connections. And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.
The new configuration parameters introduce in version 1.4.0 for reactor to test 'restRequestTimeout', 'tokenReissueRatio', 'reissueTokenAttemptLimit' and 'reissueTokenAttemptInterval'.

vaconsumer-ChnlStats-005: Alters VAConsumer to create 5 connections with 1 reactor, each connection requests login using either different or same uname and passwd, 
in order to test ReactorChannelStatistic and sessionMgnt for 5 connections same username and password share AccessToken, this behavior introduced in Java version 1.5.1.
And testing reactor options to identify tokenServiceUrl, serviceDiscoveryUrl.
The new configuration parameters introduce in version 1.4.0 for reactor to test 'restRequestTimeout', 'tokenReissueRatio', 'reissueTokenAttemptLimit' and 'reissueTokenAttemptInterval'.

vaconsumer-Ws-001: Alters VAConsumer to test compression, user can pass option -compressionType to either 'ZLib' or 'LZ4'.

vaconsumer-OAuthV2-001: Alter VAConsumer to create 1 reactor, 2 connections, both for OAuth V2 but can identify either same / diff credential.

vaconsumer-OAuthV2-002: Alter VAConsumer to create 1 reactor, 2 connections, 1st connection is for STS and 2nd connection is for OAuth V2.

vaconsumer-OAuthV2-003: Alter VAConsumer to create 1 reactor, 6 connections, 1st, 2nd and 3rd connection are for STS. And 4th, 5th, and 6th connection are for OAuth V2.

Module:  Watchlist Consumer 
---------------------------

wlconsumer-LoginReissue-001:  Alter WLConsumer to send a login reissue with PAUSE-ALL after some item updates and a RESUME-ALL after receiving a source directory update.

wlconsumer-Auth-001:  Alter WLConsumer to take a "-at2" input that permits user to specify a
2nd token used in a token renewal.

wlconsumer-Auth-002:  Alter WLConsumer to send PAUSE_ALL after 5 update and a RESUME_ALL after receiving a source directory update. Also WLConsumer is altered to take a "-at2" input that permits user to specify a 2nd token used in a token renewal.

wlconsumer-Auth-003:  Alter WLConsumer to send item pause after 5 updates on stream id 5 and item pause after 10 updates on stream id 6. Altered code does an item resume after 20 updates on stream id 6. Also WLConsumer is altered to take a "-at2" input that permits user to specify a 2nd token used in a token renewal.  NOTE:  This test must be done with two like items and one different item. Example:  "-mp TRI -mp TRI -mp IBM"

wlconsumer-Auth-004:  Alter WLConsumer to send PAUSE_ALL after 5 updates. Also WLConsumer is altered to take a "-at2" input that permits user to specify a 2nd token used in a token renewal. NOTE: This is a slight variation on wlconsumer-Auth-001 with the RESUME-ALL code removed.

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
Further extended to add event "-e4", for reissue on an item expressing View change, pause, resume.
 NOTE: On -e4 event 4, Please read the following on how the event 4 is used:
 Example arguments: -h 48.22.4.1 -p 14002 -s ELEKTRON_DD  -mp GOOG.O -mp TRI.N  -e2 5::1 -e4 10::1:1,P -x
  Notes: Make sure that -e2 event is prior to using -e4 because the way the counter works is unless an -e2
         event opens up a new item the re-issue will not work.
   In the above example "-e4 10::1:1,P" would mean after -e2 opened the item "TRI.N" the "-e4 10::1:1,P" means:
      -e4 -Reissue event
      10::1:1,P send reissue after 10 updates on item indexed 1 which is 'TRI.N' in the above example. (because end index is also 1 '1:1'.
      ',' after comma the reissue action V3 -View change V3, P pause, R resume. In the above example reissue action is P i.e pause.

wlconsumer-GenM-001:  Alters WLConsumer to send genericMsg on login, directory and market price streams. 
GenericMsg contains ElementList with one ElementEntry. 
Also added is the ability to decode genericMsg from a provider. 
This works in conjuction with the VAProvider code change from vaprovider-GenM-001

wlconsumer-MutiThreaded-001: Alter WLConsumer which is multi-thread, this application can send 
requests to multiple servers
      New command line arguments 
	  -server ${CONS_HOST1}:${PROV_PORT1}:${PROV_SERVICE1} -server ${CONS_HOST2}:${PROV_PORT2}:${PROV_SERVICE2} -itemFile 20k.xml -itemCount 10000
The example needs the XmlItemInfoList object in the perftools for loading items. For Gradle, two lines must be added to the Examples gradle.build:
	compile project(':Eta:Applications:Perftools').sourceSets.main.output
	compile group: 'xpp3', name: 'xpp3', version: '1.1.4c'
A gradle.build file is also included in this example's files.

wlconsumer-ConnRec-001: Alters WLConsumer to be able configure multiple providers for connection recovery using the following command line inputs.
  Also alters reactor worker thread to print calculated reconnect delay for connection recovery
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

etajconsperf-Rto-001
    Performance tool with ability to connect to RTO. Requests one item by default; this item is the 1st one in the list specified in 350k.xml
    Alters ConsPerfConfig.java, ConsumerThread.java to connect to RTO, requires CLI credentials.
    Run etajConsPerf. Sample Cmd:

    # ETA session management with reactor socket no proxy 
    ./gradlew runETAPerfConsumer --args="-serviceName ELEKTRON_DD \
         -uname <username> -password <password> -clientId <clientId> \
         -steadyStateTime 300 -sendBufSize 65000 -recvBufSize 65000 -inputBufs 2048 \
         -itemFile 350k.xml -outputBufs 5000  \
         -threads 1 -connType encrypted -encryptedConnectionType socket \
         -sessionMgnt -reactor"

    If -sessionMgnt is enabled should set -reactor or -watchlist
