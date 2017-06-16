Definition of QATools: The purpose of QATools is to test variations of examples (see Applications/Examples). For each example that is altered to run a test, there will be a directory here to represent that variation. Example: If Series400Consumer410 was altered 2 times to do 2 different tests, there will be directories here, such as, "Series400Consumer410*01" and "Series400Consumer410*02". In each directory are the files that have been altered. These directories contain ONLY the files that were altered for that example.

How to use QATools: For each QATool directory, user must copy or overlay the source files from that directory into the original location where the entire source for an example exists. The user must re-build the code to run the altered example.

Disclaimer:  Please note that this is not a comprehensive list of all test variations used in test.

List of altered code directories:
-----------------------------------------------------------------------------------------

Module:  Series400Consumer410 
----------------------------------

Series400Consumer410-MultiThreaded-001:  Alters multi-threaded consumer to do the following
	+ Provides commandline argument parsing for the test tool.
	+ "  -?\tShows this usage\n\n"
	+ "  -snapshot \tSend item snapshot requests [default = true]\n"
	+ "  -numOfItemPerLoop \tSend the number of item request per loop [default = 50]\n"
	+ "  -userDispatch \tUse UserDispatch Operation Model [default = false]\n"
	+ "  -userDispatchTimeout \tSet dispatch timeout period in microseconds if UserDispatch Operation Model [default = 1000]\n"
	+ "  -runtime \tRun time for test case in milliseconds [default = 60000]\n" 
	+ Implements ApiThread/ConsumerThread to manage OmmConsumer and control apiDispatch  
	   thread/userDispatch thread
	  - overrides desired methods
	  - provides own methods as needed
	+ Implements OmmConsumerClient class in AppClient
	+ ConsumerInstance instantiates OmmConsumer object which
	  - initializes connection and logins into specified server
	+ Instantiates three ConsumerThreads to access the same OmmConsumer instance. 
	+ Instantiates AppClient object that receives and processes item messages
	+ Opens streaming item interests from each ConsumerThread 
	  - Open MarketPrice "IBM$$$" items from DIRECT_FEED service for first ConsumerThread
	  - Open MarketPrice "TRI$$$" items from DIRECT_FEED service for second ConsumerThread
	  - Open MarketPrice "YAHOO$$$" items from DIRECT_FEED service for third ConsumerThread
	+ Processes data received from in ConsumerInstance for 60 seconds
	  - all received messages are processed on ApiDispatch thread or UserDispath thread control  
	+ Exits 
