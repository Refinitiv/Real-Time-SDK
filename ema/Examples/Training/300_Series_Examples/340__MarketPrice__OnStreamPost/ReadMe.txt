Summary
=======

The 340__MarketPrice__OnStreamPost application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing,
posting and parsing of MarketPrice data directly directly from and to an OMM
Provider application, or to Thomson Reuters Advanced Distribution Server.

The 340__MarketPrice__OnStreamPost showcases the usage of on stream posting
feature in Omm Consumer. It submits Omm Data for the open item and processes
a requested AckMsg when received.


Detailed Description
====================

The 340__MarketPrice__OnStreamPost implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - each of the method provided in this example use the ease of use
	  data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interests
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Sends a PostMsg with FieldList as payload on this stream
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
  - application may receive RespMsg as well as AckMsg
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.


OmmConsumerClient and Callbacks
===============================

The 340__MarketPrice__OnStreamPost demonstrates how to receive and process individual
item response messages for MarketPrice domain and Ack messages. Additionally this
application shows a native / RWF decoding of FieldList and ElementList containers.