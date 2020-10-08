Summary
=======

The 422_MP_ErrorClient application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA for accessing and parsing
of OMM MarketPrice data from Refinitiv Data Feed Direct, directly from
an OMM Provider application, or from Advanced Distribution Server.

The 422_MP_ErrorClient showcases usage of the OmmConsumerErrorClient
functionality. OMM Consumer provides two ways of notifying application about
encountered user errors. By default OMM Consumer throws a respective exception
when an error is encountered. Users may specify to receive a respective callback
instead of exception.


Detailed Description
====================

The 422_MP_ErrorClient implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
+ Implements OmmConsumerErrorClient class in AppErrorClient
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates AppErrorClient object that receives error messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets operationModel to UserDispatchEnum
+ Instantiates OmmConsumer object which initializes connection and logins into
  specified server
  - handles errors with AppErrorClient
+ Performs the following functionalities (this error scenarios are done on purpose
  to demonstrate the OmmConsumerErrorClient functionality)
  - Invalid handle to send reissue request 
  - Invalid handle to submit PostMsg
  - Invalid handle to submit PostMsg
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Processes data received from main thread for 60 seconds
  - all received messages and errors are processed on user thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
