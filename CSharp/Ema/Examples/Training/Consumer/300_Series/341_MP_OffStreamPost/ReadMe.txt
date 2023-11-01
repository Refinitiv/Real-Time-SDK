Summary
=======

The 341_MP_OffStreamPost application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing,
posting and parsing of MarketPrice data directly directly from and to an OMM
Provider application, or to an Advanced Distribution Server.

341_MP_OffStreamPost showcases the usage of the off stream posting
feature in OMM Consumer. It submits OMM Data for the open item on the login
stream and processes a requested AckMsg when received.


Detailed Description
====================

341_MP_OffStreamPost implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
  - each of the methods provided in this example use the ease of use
	data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens two streaming item interests
  - Login Domain stream on which it will submit off stream post messages
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Sends a PostMsg with FieldList as payload on login stream
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
  - application may receive RespMsg as well as AckMsg
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
