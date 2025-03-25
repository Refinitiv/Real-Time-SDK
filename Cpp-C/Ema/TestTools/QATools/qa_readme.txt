Definition QATools: The purpose of QATools is to test variations of examples (see Applications/Examples). For each example that is altered to run a test, there will be a directory here to represent that variation. Example: If VAConsumer was altered 2 times to do 2 different tests, there will be directories here, such as, "VAConsumer-*01" and "VAConsumer*02". In each directory are the files that have been altered. These directories contain ONLY the files that were altered for that example.

How to use QATools: For each QATool direcotry, user must copy or overlay the source files from that directory into the original location in Applications/Examples where the entire source for an example exists.  The user must re-build the code to run the altered example. 

Disclaimer:  Please note that this is not a comprehensive list of all test variations used in test.

List of altered code directories:
--------------------------------------------------------------------------------
Module:  Series100Consumer100
-----------------
Series100Consumer100-ConsFunc-002
    Alters consumer that sleep for 1 second before requesting item, calls uninitialize() 
    right after requesting item

Series100Consumer100-ConsFunc-003
	Alters consumer to be slow consumer when dispatch refreshMsg and updateMsg.

Module:  Series100Consumer110 
-----------------

Series100Consumer110-LoginReissue-001
    Alters consumer to send a Login reissue with NameType 3

Series100Consumer110-SrcReissue-001
    Alters consumer to send a directory request for specific service with serviceID along
    with an item request. After receiving one item refresh, this code sends 
    a source directory reissue with a specific serviceID. 

Series100Consumer110-Dict-001
    Alters consumer to send a streaming dictionary request (no service name specified) 
    using addAdminMsg.

Series100Consumer110-Dict-002
    Alters consumer to send a streaming dictionary request with a specific serviceName 
    using addAdminMsg.

Series100Consumer110-ConsFunc-001
    Alters consumer to accept these options: -m <1-99> -user <username>
    Explanation of -m: Each value passed as a argument to this option will run a different test.  
    Below is a list of tests:
    -m 1: Requests these market price items: TRI.N(with view) and TRI.N.
          After .6 seconds unregister handle for item TRI.N
    -m 2: Requests these market price items: TRI.N(view), TRI.N(view), and IBM.N
    -m 3: Requests these market price items: TRI.N(view1), TRI.N(view2), IBM.N and TRI.N(view3)
    -m 4: Requests these market price items: TRI.N(view1), TRI.N(view2), IBM.N and TRI.N(snapshot)
    -m 5: Requests these market price items: TRI.N(view1). 
          After .6 seconds send item reissue with no-refresh flag
    -m 6: Requests these market price items:  CSCO.O(view1), SPOT. 
          After .6 seconds, this code sends item reissue to pause CSCO.O and SPOT. 
          After 5.6 seconds it sends reissue to resume CSCO.O with initial image 
          set to false and sends reissue to resume SPOT with inital image set to true.
    -m 7: Requests these items with custom domain of 200: TALK (view)
          Reissue request without view specified after 1.6 seconds
    -m 8: Requests these market price items: Snapshot TRI.N, Streaming View TRI.N (FIDS 22,25) 
          After 1.6 seconds this requests snapshot view TRI.N(6,25).  
    -m 9: Sends request for two TRI.N market price items on different handles 
          Then this sends item PAUSE on first handle and PAUSE second handle. 
          Then this sends item RESUME on both handles with initial image set to false
    -m 10: Send request for two TRI.N market price items on different handles
          Then this sends item PAUSE on first handle and PAUSE second handle. 
          Then this sends item RESUME on first handle with initial image set to true
          Then this sends item RESUME on second handle with initial image set to false
    -m 11: Sends request for two TRI.N market price items on different handles 
    -m 12: Sends request for two TRI.N market price items on different handles. 
           Then this code unregisters second handle after 10 seconds
    -m 13: Sends request for RES-DS. On first handle request RES-DS. 
           On second handle, request RES-DS as a private stream.
           Extra NOTE: For this code change to be used, user must run a Provider 
           that support RES-DS (private stream re-direct)
    -m 16: Sends request for items in multiple domains and default domain (domain not specified)
           Requests sent on default domain are for both streaming and snapshot.
    -m 17: Requests these market price items: TRI.N, A.N (snapshot), TRI.N 
    -m 19: Requests these market price items: TRI.N, A.N (snapshot), TRI.N 
           After a delay, it un-registers 3rd handle requesting TRI.N
    -m 20: Requests these market price items: TRI.N, A.N (snapshot), TRI.N
           After a delay, it sends reissue on first handle (TRI.N) with initial image set to true 
    -m 21: Requests these items using domain 8: TRI.N, TRI.N
           After a delay, it sends reissue on first handle (TRI.N) with initial image set to true 
    -m 24: Requests market price item.  After 6 seconds, sends Login reissue with PAUSE.
           After, 20 seconds, sends resume.
    -m 27: Requests market price item. After a delay, this un-registers item handle and then login handle.  
    -m 28: Registers 2 login handles and requests market price item. Both handles get data from this item request.
           After delay, un registers second login handle (first should still get data).
           After delay, un registers first login handle.
    -m 30: Requests these market price items: TRI.N with view.
           After 2 seconds, sends login reissue with PAUSE flag.
           After 5 seconds, sends login reissue to resume data.
    -m 99: Requests a market price item with 99 view FIDS.


Series100Consumer110-ConsFunc-002
    Alters consumer to request items ALFA.ST and BOL.ST. 

Series100Consumer110-ConsFunc-003
    Alters consumer request item N2_UBMS. 

Series100Consumer110-ConsFunc-004
    Alters consumer to request items SPOT and TIME.

Series100Consumer110-ConsFunc-005
    Alters consumer to decode FieldLists, ElementLists, and Maps; request items TRI.N and IBM.N. 
    This tool is re-uses a request message to open a second item when service id is set only in initial request message.

Series100Consumer110-ConsFunc-006
    Alters consumer to request snapshot batch of the following 10 items: 
    IBM.N, MSFT.O, GOOG.O, TRI.N, GAZP.MM, 09IY, .TRXFLDESP, A3M.MC, ABE.MC, ACS.MC.

Series100Consumer110-ConsFunc-007
    Alters consumer to request two streaming TRI.N items in a batch request.
    Then it sends a request for five TRI.N snapshots in a batch request. 
    This change also comments out consumer code to process updates. 

Series100Consumer110-ConsFunc-008
    Alters consumer to request an item with qos ReqMsg::BestTimelinessEnum, ReqMsg::BestRateEnum. 

Module:  Series100Consumer112
-----------------------------

Series100Consumer112-ConsFunc-001
    Alters consumer makes additional item requests
    MarketByOrder item MBO and MarketByPrice item MBP.

Series100Consumer112-ConsFunc-002
    Alters consumer added addition argument -objectname to set
    tunnelingObjectName

Series100Consumer112-PConfig-001
    Alters consumer to specify programatic config for parameters related to EncryptedType connection: 
    Host,Port,ProxyHost,ProxyPort and ObjectName

Module:  Series100Consumer113
-----------------------------

Series100Consumer113-ConsFunc-001
   Alters consumer added addition arguments -tokenServiceUrl and -serviceDiscoveryUrl to set config.tokenServiceUrl or config.serviceDiscoveryUrl. 
   
Series100Consumer113-ConsFunc-002
	Alters consumer to request about 240K rics from Asian region to reproduce customer issue RTSDK-5292.
	Cons113.exe -username <> -password <> -clientId <> -fileName preload_ubs1

Series100Consumer113-RestLogCallback-001
   Alters consumer to test a user callback that receive REST logging messages.
   Added command line options:
   -restLogCallback: Enable REST logging callback.
   -restLogFilename: File name for printing the REST logging messages.
   ./Cons113 -username <> -password <> -clientId <> -restLogCallback -restLogFilename "filename.log"

Series100Consumer113-RestProxy-001
   Alters consumer to test proxy connection for Rest requests.
   Added command line options for check "functional" way:
   -restProxyHost, -restProxyPort, -restProxyUserName, -restProxyPasswd, -restProxyDomain.
   Added command line options for check "programmatic" way:
   -prestProxyHost, -prestProxyPort.

Series100Consumer113-ApiDispatch-001
	Alters consumer ApiDispatch mode with implementing OMMConsumerErrorClient onJsonConverter.

Series100Consumer113-UserDispatch-001
	Alters consumer UserDispatch mode without implementing OMMConsumerErrorClient onJsonConverter.

Series100Consumer113-UserDispatch-002
	Alters consumer UserDispatch mode with implementing OMMConsumerErrorClient onJsonConverter.
	
Module:  Series100Consumer140
-----------------------------

Series100Consumer140-ConsFunc-002
     Alters consumer to clone and decode these messages: RefreshMsg and UpdateMsg containing MarketByOrder data 

Module:  Series100Consumer170
-----------------------------

Series100Consumer170-ConsFunc-001
     Alters consumer to test Channel Information ESDK-1601 

Series100Consumer170-ConsFunc-002
     Alters consumer to test OMMErrorClient and modifyIOCtl ESDK-3294 

Module:  Series200Consumer200
-----------------------------

Series200Consumer200-ConsFunc-001
     Alters consumer to decode Date/Time/DateTime and print using ISO 8601


Module:  Series200Consumer280
-----------------------------

Series200Consumer280-ConsFunc-001
     Alters consumer which remove set host with api call OmmConsumerConfig() in file Consumer.cpp ,also alter to specify consumerName("Consumer_3") 

Module:  Series300Consumer310
---------------------------
Series300Consumer310-Rmtes-001
   Alters consumer to use new function rmtesBuffer.getAsEmaBuffer() and encode it as a FieldEntry of FieldList.

Module:  Series300Consumer331
---------------------------
Series300Consumer331-GenM-001
   Alters consumer send genericMsg with connection status on directory stream, should proccess
   genericMsg from provider. Need to run with Series300Provider320-GenM-001 provider tool.

Series300Consumer331-SrcReissue-001
    Alters consumer to create a directory handle with an invalid serviceId. 
    Then this code attempts to send a reissue on the invalid handle.

Series300Consumer331-ConsFunc-000
    Alters consumer to request directory with several options 
    -f <filter>:  User may choose a value in decimal between 0 and 63
       inclusive to pick up one or more filters. Here are filter definitions
       -f 1  SERVICE_INFO_FILTER 0x01
       -f 2  SERVICE_STATE_FILTER 0x02
       -f 4  SERVICE_GROUP_FILTER 0x04
       -f 8  SERVICE_LOAD_FILTER 0x08
       -f 16  SERVICE_DATA_FILTER 0x10
       -f 32  SERVICE_LINK_FILTER 0x10
       User may mix and match different filters:
       -f 5  INFO & GROUP 4+1 
       -f 63 INFO, STATE, GROUP, LOAD, DATA 7 LINK (1+2+4+8+16+32)
       etc.
    -m <option>: 
       -m 0: Request source directory without serviceName or ID and a filter
       if specified
       (uses default filter)
       -m 1: Request source directory with serviceName and filter if specified 
       -m 2: Request source directory with serviceName and filter if specified 
             Also request item using serviceName after a sleep if sleeptime is
             specified
       -m 3: Request source diretory with serviceId and filter if specified 
       -m 4: Request source diretory with serviceId and filter if specified 
             Also request item using serviceId after a sleep if sleeptime is
             specified
       -m 5: Request source directory without serviceName or ID and a filter if specified. 
             Also request item usin serviceName after a sleep if specified
    -s <sleeptime>:  Amount of time specified in seconds to wait before making
             an item request. This only applies to -m 2 -m 4

Series300Consumer331-ConsFunc-001
    Alters consumer to request directory using info filter without a serviceName. 
    Also, alters consumer to not send out item requests.

Series300Consumer331-ConsFunc-002
    Alters consumer to request directory filters with -directoryFilter argument. 
                 -directoryFilter 1 (SERVICE_INFO_FILTER) 
                 -directoryFilter 2 (SERVICE_STATE_FILTER)
                 -directoryFilter 4 (SERVICE_GROUP_FILTER)
                 -directoryFilter 8 (SERVICE_LOAD_FILTER)
                 -directoryFilter 16 (SERVICE_DATA_FILTER)
                 -directoryFilter 32 (SERVICE_LINK_FILTER)
                 -directoryFilter 3 (SERVICE_INFO_FILTER | SERVICE_STATE_FILTER)
                 -directoryFilter 5 (SERVICE_INFO_FILTER | SERVICE_GROUP_FILTER)
                 -directoryFilter 63 (SERVICE_INFO_FILTER | SERVICE_GROUP_FILTER | SERVICE_STATE_FILTER | SERVICE_LOAD_FILTER | SERVICE_DATA_FILTER | SERVICE_LINK_FILTER)

Series300Consumer331-ConsFunc-002
    Alters consumer to request directory filters with -directoryFilter argument. 
                 -directoryFilter 1 (SERVICE_INFO_FILTER) 
                 -directoryFilter 2 (SERVICE_STATE_FILTER)
                 -directoryFilter 4 (SERVICE_GROUP_FILTER)
                 -directoryFilter 8 (SERVICE_LOAD_FILTER)
                 -directoryFilter 16 (SERVICE_DATA_FILTER)
                 -directoryFilter 32 (SERVICE_LINK_FILTER)
                 -directoryFilter 3 (SERVICE_INFO_FILTER | SERVICE_STATE_FILTER)
                 -directoryFilter 5 (SERVICE_INFO_FILTER | SERVICE_GROUP_FILTER)
                 -directoryFilter 63 (SERVICE_INFO_FILTER | SERVICE_GROUP_FILTER | SERVICE_STATE_FILTER | SERVICE_LOAD_FILTER | SERVICE_DATA_FILTER | SERVICE_LINK_FILTER)

Series300Consumer331-ConsFunc-003
    Alters consumer to request directory with interestAfterRefresh(false) 

Module:  Series300Consumer332
---------------------------

Series300Consumer332-Dict-001
    Alters consumer to request dictionary using serviceName.

Series300Consumer332-Dict-002
    Alters consumer to request the dictionary using 'INFO' filter using serviceName.

Series300Consumer332-Dict-003
    Alters consumer to request the dictionary using 'MINIMAL' filter using serviceName.

Series300Consumer332-Dict-004
    Alters consumer to request the dictionary using 'NORMAL' filter using serviceName.

Series300Consumer332-Dict-005
    Alters consumer to request the dictionary only, with no item requests.

Series300Consumer332-Dict-007
    Alters consumer to create multiple threads that call DataDictionary.entry()

Series300Consumer332-Dict-008
    Alters consumer to call new interface DataDictionary.entry(int id, DictionaryEntry entryDst)

Module:  Series300Consumer333
-----------------------------

Series300Consumer333-GenM-001
   Alters consumer send genericMsg with connection status on login stream to provider,
   also proccess genericMsg from provider. Need to run with Series300Provider320-GenM-001
   provider tool.

Series300Consumer333-GenM-003
   Alters Series300Consumer333-GenM-001 to clone and decode GenericMsg 

Module:  Series300Consumer340
-----------------------------

Series300Consumer340-ConsFunc-001
   Alters consumer to clone and decode these messages: RefreshMsg, AckMsg, UpdateMsg and StatusMsg 

Series300Consumer360-MultiThreadViews-001
   Alters consumer to request multiple view from the plenty different threads

Module:  Series300Consumer370
---------------------------
Series300Consumer370-BatchView-10Consumers
   Alters the Consumer to bring multiple consumers up and down triggering any memory issues

Module:  Series400Consumer410 
---------------------------
Series400Consumer410-MultiThreaded-001: This test tool implements the following high level steps:

    Provides the following command line options:
    -snapshot: Send item snapshot requests [default = true]
    -numOfItemPerLoop: Send the number of item request per loop [default = 50]
    -userDispatch: Use UserDispatch Operation Model [default = false]
    -userDispatchTimeout: Set dispatch timeout period in microseconds 
     if UserDispatch Operation Model [default = 1000]
    -runtime: Run time for test case in milliseconds [default = 60000]
                
    Consumer does the following things: 
    + Implements ApiThread/ConsumerThread to manage OmmConsumer and control apiDispatch  
       thread/userDispatch thread
      - overrides desired methods
      - provides own methods as needed
    + Implements OmmConsumerClient class in AppClient
    + ConsumerInstance instantiates OmmConsumer object which 
      initializes connection and logins into specified server
    + Instantiates three ConsumerThreads to access the same OmmConsumer instance. 
    + Instantiates AppClient object that receives and processes item messages
    + Opens MarketPrice items (either streaming or snapshot per user input) from 
      each ConsumerThread where $$$ is replace by an index based 
      on the number of items sent in a loop. Consumer sends these requests until 
      runtime expiration.
      - Open "IBM$$$" items for first ConsumerThread
      - Open "TRI$$$" items for second ConsumerThread
      - Open "YAHOO$$$" items for third ConsumerThread
    + Processes data received from in ConsumerInstance
      - all received messages are processed on ApiDispatch thread or UserDispath thread control  
    + Exits

Series400Consumer410-MultiThreadBatchView-001: This test tool implements the following high level steps:

    Provides the following command line options:
    -numOfItemPerLoop: Send the number of item request per loop [default = 50]
    -userDispatch: Use UserDispatch Operation Model [default = false]
    -userDispatchTimeout: Set dispatch timeout period in microseconds 
     if UserDispatch Operation Model [default = 1000]
    -runtime: Run time for test case in milliseconds [default = 60000]
                
    Consumer does the following things: 
    + Implements ApiThread/ConsumerThread to manage OmmConsumer and control apiDispatch  
       thread/userDispatch thread
      - overrides desired methods
      - provides own methods as needed
    + Implements OmmConsumerClient class in AppClient
    + ConsumerInstance instantiates OmmConsumer object which 
      initializes connection and logins into specified server
    + Instantiates three ConsumerThreads to access the same OmmConsumer instance. 
    + Instantiates AppClient object that receives and processes item messages
    + Opens MarketPrice items from each ConsumerThread where $$$ is replace by an index based 
      on the number of items sent in a loop. Consumer sends these requests until 
      runtime expiration.
      - Open "TRI$$$", with streaming view1 for first ConsumerThread
      - Open "TRI$$$", with snapshot view2 for second ConsumerThread
      - Open items named "BATCHITEM1" and "BATCHITEM2" as batch request for third ConsumerThread
      NOTE: This show aggregation and disaggregation of requests from multiple threads 
      as request go out on the network
    + Processes data received from in ConsumerInstance 
      - all received messages are processed on ApiDispatch thread or UserDispath thread control  
    + Exits

Series400Consumer410-Encryption-001
    Alters consumer which remove set host with api call OmmConsumerConfig(),in file ConsumerManager.cpp	

Module:  Series400Consumer421 
---------------------------

Series400Consumer421-DirectWrite-001
    Alters consumer such that Channel_1 added configuration for DirectWrite
	.addUInt( "DirectWrite", 1 ).
    Note: to display the writeFlags parameter in rsslWrite/rsslWriteEx use eta-rsslWrite-001

Series400Consumer421-PConfig-001
    Alters consumer such that Channel_1 added configuration for CompressionThreshold and alter value for InterfaceName 
    to consumerInterface, 
    Also additional changes are
    - replace .addEnum( "DictionaryType", 1 ) to .addEnum( "DictionaryType", 0 ) 
    - replace .addEnum( "LoggerType", 0 ) to .addEnum( "LoggerType", logger1Type ) 
    - replace .addEnum( "LoggerSeverity", 1 ) to .addEnum( "LoggerSeverity", logger1Severity  ) 

Series400Consumer421-PConfig-002
    Alters consumer to specify Channelset parameters using programmatic config for two channels 

Series400Consumer421-PConfig-003
    Alters consumer so it is doing a combination of programmatic and function calls
    In programmatic Channel_1 added configuration for CompressionThreshold and alter value for InterfaceName 
    to consumerInterface, also replace Enum ( "DictionaryType", 0 ) 
    instead of ( "DictionaryType", 1 ). 
    In functional configuration for consumerName as Consumer_3 and  host("localhost:14002")

Series400Consumer421-PTimeout-001
    Alters consumer such that Channel_1 added configuration settings for proxy connections.
    Under Windows:
	.addAscii("ProxyHost", "webproxy.pln.colo.services")
	.addAscii("ProxyPort", "80")
	.addUInt("ProxyConnectionTimeout", 7)
    Under Linux:
	.addAscii("ProxyHost", "google.com")
	.addAscii("ProxyPort", "102")
	.addUInt("ProxyConnectionTimeout", 7)
    Also added configuration option for Reactor initialization timeout:
	.addUInt("InitializationTimeout", 10)
    Note: to display the curl_easy_setopt's parameters in runBlockingLibcurlProxyConnection use eta-PTimeout-001


Module:  Series400Consumer430
-----------------------------

Series400Consumer430-Auth-002
    Alters consumer to send login reissues with Pause and Resume using specified 
    authentication token.

Series400Consumer430-Auth-003
    Alters consumer to send login reissues with Pause, token renewal, and Resume using 
    specified authentication token.

Series400Consumer430-Auth-004
    Alters consumer to support user input of two authentication tokens.
    This consumer sends login reissue with pause flag and first token (Pauses data). 
    Then it sends a login reissue with second token and without pause flag (Renewal). 
    Then it sends another login reissue with second token and without pause flag (Resume). 

Series400Consumer430-Auth-005
    Alters consumer to support user input of two authentication tokens.
    This consumer sends login reissue with pause flag and first token (Pauses data). 
    Then it sends a login reissue with second token and without pause flag (Renewal);
     this results in data not resuming.  

Series400Consumer430-Auth-006
    Alters consumer to support user input of two authentication tokens.
    This consumer sends login reissue with pause flag and first token (Pauses data). 
    Then it sends another login reissue with first token and without pause flag (Resume). 
    Then it sends a login reissue with second token (Renewal). 

Series400Consumer430-Auth-007
    Alters consumer which supports an authentication to do offstream posting.

Series500-Consumer500-ConsFunc-001
    Alter consumer to have file config, retister client for logitn to test preferred host feature.

Series500-Consumer501-ConsFunc-001
    Alter consumer to have programaticaly config, 3 warm standby groups with two channels each, 3 channels in channelset to test preferred host feature.

Series500-Consumer501-ConsFunc-002
    Alter consumer to have programatic config, retister client for logitn to test preferred host feature.

Series500-Consumer501-BatchView-10Consumers
    Alters the Consumer to bring miltiple consumers up and down triggering any memory issues.

Series500-Consumer502-ConsFunc-001
    Alter consumer retister client for logitn to test preferred host feature.
	
Module:  Series400Consumer440
-----------------------------
Series400Consumer440-TS-001
    Alters consumer to accept these options: -m <1-99>
    Explanation of -m: Each value passed as a argument to this option will run a different test.  
    Below is a list of tests:
    -m 1 Sets CosDataIntegrity::ReliableEnum, CosGuarantee::NoneEnum, 
           CosFlowControl::BidirectionalEnum
    -m 2 Sets CosDataIntegrity::ReliableEnum, CosGuarantee::NoneEnum, CosFlowControl::NoneEnum
    -m 3 Sets CosDataIntegrity::BestEffortEnum, CosGuarantee::NoneEnum, 
         CosFlowControl::BidirectionalEnum
    -m 4 Sets CosDataIntegrity::BestEffortEnum, CosGuarantee::NoneEnum, CosFlowControl::NoneEnum
    -m 5 Sets CosDataIntegrity::ReliableEnum, CosGuarantee::NoneEnum, 
         CosFlowControl::BidirectionalEnum ,it set recvWindowSize( 20 )
    -m 6 Sets CosDataIntegrity::ReliableEnum, CosGuarantee::PersistentQueueEnum, 
         CosFlowControl::BidirectionalEnum 

Series400Consumer440-TsFrag-001
    Alters consumer to accept -bufSize and -fillSize as inputs.
    The bufSize is the size of buffer requested by getBuffer().  
    The fillSize is is the number of bytes written.  
    For example, if bufSize is 1000 and fillSize is 900, then the Consumer gets 
    a buffer of 1000 bytes but only writes 900 bytes before sending. The filled 
    buffer is sent to the provider/ads in fragments if larger than max fragment 
    size. This code change fills the buffer with "1, 2, 3....255" and repeats 
    this pattern of data up to the fillSize. The provider will then need to check 
    for the same pattern in the fully assembled message and print "TEST PASSED".


Module:  Series400Consumer450
-----------------------------

Series400Consumer450-RestLogCallback-001
   Alters consumer to test a user callback that receive REST logging messages.
   Added command line options:
   -restLogCallback: Enable REST logging callback.
   -restLogFilename: File name for printing the REST logging messages.
   ./Cons450 -username <> -password <> -clientId <> -restLogCallback -restLogFilename "filename.log"

Module:  Series400Consumer490
-----------------------------
    Alters consumer to test a dictionary loading from object
    Added command line options:
    -loadLocalDictTwoConsSeq: load local dictionary to object then use this object to run 2 consumers sequntially
    -loadChannelDictTwoConsSeq: load channel dictionary to object then use this object to run 2 consumers sequntially
    -loadLocalDictTwoConsConcur: load local dictionary to object then use this object to run 2 consumers concurrently
    -loadChannelDictTwoConsConcur: load channel dictionary to object then use this object to run 2 consumers concurrently
    -shouldCopyIntoAPI: enable deep copy dictionary object into API

Module:  Series100Provider100 
-------------------------------

Series100Provider100-ProvFunc-001
    Alters Interactive Provider to support two incoming consumer connections. 
    This is done by commenting out code to not run processInvalidItemRequest.

Series100Provider100-ProvFunc-002
    Alters Interactive Provider to send one or more of following source directory 
    updates based on user input for TestFlag. See usage in tool for more details:
        -groupStatusDataSuspect
            Provider sends source directory update group stale
        -groupStatusDataOk
            Provider sends source directory update group ok
        -mergedToGroup
            Provider sends source directory update group merge status
        -groupStatusClosedRecover 
            Provider sends source directory update group closed recover status
        -mergedToGroupAndSendGroupStatusClosedRecover
            Provider sends source directory update group merge then send group 
            closed recover for the new group
        -acceptingRequest    
            Provider send source directory update Accepting Request
            set to false.
        -serviceDown: 
            Provider sends source directory update service state down
        -infoAddCapabilities
            Provider sends source directory update for capability change.
        -chagneLinkTypeAndStateAndCode
            Provider sends source directory update link's type, state, and code.
        -openWindowsAndLoadFactor: 
            Provider sends 2 source directory updates to alter openWindow and 
            Service Load.
        -changeDataType
            Provider sends source directory update data type change.
        -serviceDelete
            Provider sends source directory update to delete the service.

Series100Provider100-ProvFunc-003
    Alters Interactive Provider to handle genericMsg on login stream when 
    consumer sends GenericMsg on Login Stream.

Series100Provider100-ProvFunc-004
    Alters Interactive Provider to support the following user inputs: 
        -notAcceptLoginReq: Provider responds to directory, dictionary and item requests
         with a reject status message if consumer sends any of the above before login
        -sendLoginCloseStatus: Provider responds to any login request with login 
         close status message.
        -sendCloseStatus: Provider responds with item close status to an item request with
         non-existing serviceID or duplicate key in a new request.

Series100Provider100-ProvFunc-005
    Alters Interactive Provider item refersh and updates to include a different 
    payload consisting of multiple enum type fields.  

Series100Provider100-ProvFunc-007
    Alters Interactive Provider to publish a symbol list with a configurable number of items in the list.
    Command line argument:  -c <count>
    where <count> represents the number of items to be published in symbolist: A0, A1,... A<count>

Series100Provider100-ProvFunc-008
    Alters Interactive Provider to to repeat multiple iterations of the following: initialize provider, publish item, sleep 1 sec, uninitializing provider

Series100Provider130-ProvFunc-001
    Alters Interactive Provider UserDispatch to accept multiple clients.

Series100IProvider100-ProvFunc-009
	Alters Interactive Provider to submit huge updateMsg and resubmit it again after adjust guaranteedOutputBuffers to 10000 using method modifyCtrlIO()(since v. 1.4.0).
	
Series100IProvider100-ProvFunc-010
	Alters Interactive Provider to register appClientError (since v. 1.4.0) and submit huge updateMsg.

Series100IProvider100-ProvFunc-011
	Alerts Interactive Provider to support the following user inputs:
	-port: Provider port

Module:	 Series100IProvider170
---------------------------
Series100IProvider170-ProvFunc-001
	Alters Interactive Provider to parse IOControl parameters as an input argument to modify their values and get those values to display after modifying i.e.
    -maxOutputBuffers, 	-guranteedOutputBuffers, -compressionThreshold

Module:	 Series200IProvider200
--------------------------
Series200Provider200-ProvFunc-001
	Alters Interactive Provider to provide views for multi threaded consumer. Compatible with Series300Consumer360-MultiThreadViews-001.

Module:	 Series200IProvider260
---------------------------
Series200Provider260-toStringTest-001
	Alters Interactive Provider to encode all Container Types and Msg Types and call function .toString() and .toString(dictionary).

Module:  Series300Provider320
---------------------------
Series300Provider320-GenM-001
    Alters Interactive Provider which can process genericMsg on login stream and directory Stream,
    after receive genericMsg from consumer it send a genericMsg back to consumer.
    Need to run with Series300Consumer331-GenM-001 or Series300Consumer333-GenM-001.

Series300Provider320-GenM-002
     Alters Series300Provider320-GenM-001 to clone and decode these messages: RequestMsg and GenericMsg 

Series300Provider320-ProvFunc-001
    Alters Interactive Provider to do two things after sending some item updates:
    - After 2 seconds, provider sends a dictionary status message with 
      Open/Suspect state on serviceID of 1.
    - After 6 seconds, provider sends a dictionary status message with 
      Closed/Suspect state with serviceName. 

Series300Provider320-ProvFunc-002
    Alters Interactive Provider to send an update message on dictionary domain using serviceName. 
    This provider is created with configuration for API_CONTROL.

Series300Provider320-ProvFunc-003
    Alters Interactive Provider to send a refresh message on dictionary domain 
    specifying a service Id(5) that is not included in the source directory.

Series300Provider320-ProvFunc-004
    Alters Interactive Provider to send the dictionary with serviceId of 1. 
    Also, the provider sends item refreshes using serviceId of 1.

Series300Provider320-ProvFunc-005
    Alters Interactive Provider to send a dictionary refresh message using an invalid service name.

Series300Provider320-ProvFunc-006
    Alters Interactive Provider to send a dictionary in fragments starting at size 9600 
    bytes and incrementing subsequent fragments by 10000 bytes.

Module:  Series300Provider340
----------------------------------
Series300Provider340-ProvFunc-001
    Alter IProv340 to set AckId

Series300Provider340-ProvFunc-002	
    Alter IProv340 to add 3 options for testing set ackMsg with serviceName or serviceId or invalid serviceId.
   -ackWithServiceId Set serviceName on Ack messages.
   -ackWithServiceName Set serviceId on Ack messages.
   -ackWithInvalidSID Set invalid serviceId on Ack messages.

Module:  Series300Provider350
----------------------------------
Series300Provider350-ProvFunc-001:  Alters Series300Provider350 to request dictionary
as a snapshot with specified filter using command line argument: -filter. Values to -filter
can be INFO, NORMAL, MINIMAL or VERBOSE. Additionally, this tool also unregisters dictionary
handles after sending 6 updates.

Series300Provider350-ProvFunc-002:  Alters Series300Provider350 to make 3 additional
requests for both dictionaries

Series300Provider350-ProvFunc-003:  Alters Series300Provider350 send Login Close Status message
after it receive MarketPrice Request from consumer

Series300Provider350-ProvFunc-004:  Alters Series300Provider350 to request dictionaries
using invalid serviceName, also request dictionaries using invalid serviceId.

Series300Provider350-Reissue-001:  Alters Series300Provider350 send reissue on both dictionary
handles with no changes in filter after sending 6 updates, send reissue on both dictionary handles
with change in filter to NORMAL after sending 8 updates, also send update on source directory with
delete service after sending 9 updates.


Module:  Series400Provider421
---------------------------

Series400Provider421-DirectWrite-001
    Alters Provider such that Server_1 added configuration for DirectWrite
	.addUInt( "DirectWrite", 1 ).
    Note: to display the writeFlags parameter in rsslWrite/rsslWriteEx use eta-rsslWrite-001

Series400Provider421-PConfig-001: 
    Alters Provider for following changes
        -Added programmatic config for number of Provider configuration in Provider_1 
         MaxDispatchCountApiThread, MaxDispatchCountUserThread, MergeSourceDirectoryStreams, etc 
        -Added server configuration in Server_1 CompressionThreshold, HighWaterMark, etc,

Series400Provider421-PConfig-002: 
    Alters Provider for following changes
        -Added programmatic config for number of Provider configuration in Provider_1 
         MaxDispatchCountApiThread, MaxDispatchCountUserThread, MergeSourceDirectoryStreams, etc 
        -Added server configuration in Server_1 CompressionThreshold, HighWaterMark, etc,
        -Added fuctional configuration to add providerName to "Provider_2"

Series400Provider421-PConfig-003: 
    Alters Provider for following changes
        -Added programmatic config for only Dictionary_1 where RdmFieldDictionaryItemName is RWFFld3 
        and EnumTypeDefItemName RWFEnum3 
        -Removed code for programmatic configure Provider_1 and Server_1

Series400Provider421-Encryption-001
    Alters Provider for following changes
        -Set ServerType to Encrtyped
        -New ServerCert and ServerPrivateKey configuration in Server_1 
        -Set LoggerSeverity to Verbose

Module:  Series100NiProvider100
---------------------------

Series100NiProvider100-NiFunc-001
    Alters NiProvider to encode a field with micro & nano second precision 
    of type DateTime. This code also encodes the following in other fields: 
    Inf, -Inf, NaN, Time, DateTime as blank & leapSeconds

Series100NiProvider100-NiFunc-002
    Alters NiProvider to add a key with a data type of Ascii in a source 
    directory request message. This should cause an exception.

Series100NiProvider100-NiFunc-003
    Alters NIProvider to send only 2 update for item IBM.N and then sleep for 20 seconds.

Series100NiProvider100-Auth-002
    Alters NiProvider sends a market price item refresh after stream state 
    changes from up to down (recovery).

Series100NiProvider100-ProvFunc-001
    Alters NiProvider to repeat multiple iterations of the following: initialize niprovider, request item, sleep 1 sec, uninitializing niprovider

Module:  Series200NiProvider201
----------------------------------
Series200NiProvider201-ProvFunc-001
    Alter NiProvider to use user dispatch and checking for status to recovery connection
    with attached new argument and configuration for NiProvider HTTPs Encryption.

Series200NiProvider201-PConfig-001
    Alter NiProvider to use Programmatic Congig along with Tunneling Connection arguments.

Module:  Series300NiProvider350
----------------------------------
Series300NiProvider350-ProvFunc-001:  Alters Series300NiProvider350 to request dictionary
as a snapshot with specified filter using command line argument: -filter. Values to -filter
can be INFO, NORMAL, MINIMAL or VERBOSE. Additionally, this tool also unregisters dictionary
handles after sending 6 updates.

Series300NiProvider350-ProvFunc-002:  Alters Series300NiProvider350 to make 3 additional
requests for both dictionaries

Series300NiProvider350-ProvFunc-004:  Alters Series300NiProvider350 to request dictionaries
using invalid serviceName, also request dictionaries using invalid serviceId.

Series300NiProvider350-Reissue-001:  Alters Series300NiProvider350 send reissue on both dictionary
handles with no changes in filter after sending 6 updates, send reissue on both dictionary handles
with change in filter to NORMAL after sending 8 updates, also send update on source directory with
delete service after sending 9 updates.

Module:  Series300NiProvider360
---------------------------

Series300NiProvider360-ProvFunc-001:  Alters Series300NiProvider360 to provide the following optional
command line arguments to the tool:
    -numOfUpdatesForApp <value; default=600>
    Sends a total of specified number of item updates

    -userDispatch < true|false ; default=false >
    If set to true, tool uses UserDispatch Operation Model; otherwise, tool uses API Dispatch Operation Model

    -dirAdminControl < true|false ; default=false >
    If set to true, tool will send a source directory response; otherwise, the EMA layer will send one on behalf of tool.

    -testChannelInfoWithLoginHandle
    If set to true, tool will create NiProvider without register login in OmmProvider constructor, but it will call register client later.

Module:  Series400NiProvider421
---------------------------

Series400NiProvider421-PConfig-001: 
    Alters NiProvider for following changes
        -Added programmatic config for number of Provider configuration in Provider_1 
         MaxDispatchCountApiThread, MaxDispatchCountUserThread, MergeSourceDirectoryStreams, etc 
        -Added channel configuration in Channel_10 CompressionThreshold, HighWaterMark, etc,

Series400NiProvider421-PConfig-002: 
    Alters NiProvider for following changes
        -Added programmatic config for number of Provider configuration in Provider_1 
         MaxDispatchCountApiThread, MaxDispatchCountUserThread, MergeSourceDirectoryStreams, etc 
        -Added channel configuration in Channel_10 CompressionThreshold, HighWaterMark, etc,
        -Added fuctional configuration to add providerName to "Provider_2"

Series400NiProvider421-PConfig-003: 
    Alters NiProvider for following changes
        -Added programmatic config for number of Provider configuration in Provider_1 
        ChannelSet, MaxDispatchCountApiThread, MaxDispatchCountUserThread, MergeSourceDirectoryStreams, etc 
        -Added channel configuration in Channel_10 CompressionThreshold, HighWaterMark, etc,
        -Added new channel configuration for Channel_11, 
        -Added code for connection recovery functionality from NiProvider360

Module:  Series400NiProvider430
---------------------------

Series400NiProvider430-Auth-001
    Alters NiProvider sends a market price item refresh after stream state 
    changes from up to down (recovery).


Module:  Emalibs 
---------------------------

emalibs-Cons-000
    Alters ema library, specifically ItemCallbackClient.cpp, to change 
    CONSUMER_STARTING_STREAM_ID from 4 to 2147483641.  

emalibs-Cons-001
    Alters ema library, specifically ItemCallbackClient.cpp, to change 
    CONSUMER_STARTING_STREAM_ID from 4 to 2147483636.  


Module:  Emacppconsperf-Rto
----------------------------------
emacppconsperf-Rto-001
    Performance tool with ability to connect to RTO. Requests one item by default; this item is the 1st one in the list specified in 350k.xml
    Alters ConsPerfConfig.cpp, ConsPerfConfig.h, ConsumerThread.cpp, EmaCppConsPerf.cpp to connect to RTO, requires CLI credentials.
    Run EmaCppConsPerf. Sample Cmd:

    # encrypted
    EMACppPerfCons -serviceName ELEKTRON_DD \
           -uname <username> -password <password> -clientId <clientId> \
           -tickRate 1000 -steadyStateTime 300 \
           -itemFile 350k.xml -consumerName Perf_Consumer_1

    # encrypted websocket json rssl.json.v2
    EMACppPerfCons -serviceName ELEKTRON_DD \
           -uname <username> -password <password> -clientId <clientId> \
           -tickRate 1000 -steadyStateTime 300 \
           -itemFile 350k.xml -consumerName Perf_Consumer_WSJSON_1

Module: other
----------------------------------
ContainerPerformance
    Performance tool which evaluate encoding performance for all containers.
