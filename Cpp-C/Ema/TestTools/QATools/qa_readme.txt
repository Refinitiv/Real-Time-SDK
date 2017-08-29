Definition QATools: The purpose of QATools is to test variations of examples (see Applications/Examples). For each example that is altered to run a test, there will be a directory here to represent that variation. Example: If VAConsumer was altered 2 times to do 2 different tests, there will be directories here, such as, "VAConsumer-*01" and "VAConsumer*02". In each directory are the files that have been altered. These directories contain ONLY the files that were altered for that example.

How to use QATools: For each QATool direcotry, user must copy or overlay the source files from that directory into the original location in Applications/Examples where the entire source for an example exists.  The user must re-build the code to run the altered example. 

Disclaimer:  Please note that this is not a comprehensive list of all test variations used in test.

List of altered code directories:
--------------------------------------------------------------------------------

Module:  Series100Consumer110 
-----------------

Series100Consumer110-LoginReissue-001
	Alters consumer to send a Login reissue with NameType 3

Series100Consumer110-SrcReissue-001
	Alters consumer to send a directory request for specific service with serviceID along
	with an item request. After receiving one item refresh, this code sends 
	a source directory reissue with a specific serviceID. 

Series100Consumer110-Src-001
	Alters consumer to send a directory request for a specific service with serviceName.

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


Module:  Series300Consumer331
---------------------------

Series300Consumer331-SrcReissue-001
	Alters consumer to create a directory handle with an invalid serviceId. 
	Then this code attempts to send a reissue on the invalid handle.


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


Module:  Series400Consumer421 
---------------------------

Series400Consumer421-PConfig-001
	Alters consumer such that programmatic config adds Enum ( "DictionaryType", 2 ) 
	instead of ( "DictionaryType", 1 ). 

Series400Consumer421-PConfig-002
    Alters consumer such that programmatic config to specify ChannelSet with Two Channels. 


Module:  Series400Consumer430
-----------------------------

Series400Consumer430-TrepAuth-002
	Alters consumer to send login reissues with Pause and Resume using specified 
	authentication token.

Series400Consumer430-TrepAuth-003
	Alters consumer to send login reissues with Pause, token renewal, and Resume using 
	specified authentication token.

Series400Consumer430-TrepAuth-004
    Alters consumer to support user input of two authentication tokens.
	This consumer sends login reissue with pause flag and first token (Pauses data). 
	Then it sends a login reissue with second token and without pause flag (Renewal). 
	Then it sends another login reissue with second token and without pause flag (Resume). 

Series400Consumer430-TrepAuth-005
    Alters consumer to support user input of two authentication tokens.
	This consumer sends login reissue with pause flag and first token (Pauses data). 
	Then it sends a login reissue with second token and without pause flag (Renewal);
	 this results in data not resuming.  

Series400Consumer430-TrepAuth-006
    Alters consumer to support user input of two authentication tokens.
	This consumer sends login reissue with pause flag and first token (Pauses data). 
	Then it sends another login reissue with first token and without pause flag (Resume). 
	Then it sends a login reissue with second token (Renewal). 

Series400Consumer430-TrepAuth-007
	Alters consumer which supports trep authentication to do offstream posting.


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



Module:  Series300Provider320
---------------------------

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


Module:  Series100NiProvider100
---------------------------

Series100NiProvider100-NiFunc-001
    Alters NiProvider to encode a field with micro & nano second precision 
	of type DateTime. This code also encodes the following in other fields: 
	Inf, -Inf, NaN, Time, DateTime as blank & leapSeconds

Series100NiProvider100-NiFunc-002
	Alters NiProvider to add a key with a data type of Ascii in a source 
	directory request message. This should cause an exception.

Series100NiProvider100-TrepAuth-002
	Alters NiProvider sends a market price item refresh after stream state 
	changes from up to down (recovery).


Module:  Series400NiProvider430
---------------------------

Series400NiProvider430-TrepAuth-001
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

