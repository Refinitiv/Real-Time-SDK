Summary
=======

The 410__MarketPrice__HorizontalScaling application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketPrice data from Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from Thomson Reuters Advanced
Distribution Server.

The 410__MarketPrice__HorizontalScaling showcases horizontal scaling capability
of Omm Consumer to take advantage of multiple core processors.


Detailed Description
====================

The 410__MarketPrice__HorizontalScaling implements the following high level steps:

+ Implements ConsumerManager to manage OmmConsumer and control user dispatch thread
  - overrides desired methods
  - provides own methods as needed
+ Implements OmmConsumerClient class in AppClient
+ Instantiates two ConsumerManager to create OmmConsumer and user threads with the 
  following configuration
  - sets user name to "user1" and "user2" for each ConsumerManager respectively
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ ConsumerManager instantiates OmmConsumer object which
  - initializes connection and logins into specified server
+ Instantiates AppClient object that receives and processes item messages
+ Opens streaming item interests from each ConsumerManager 
  - MarketPrice IBM.N item from DIRECT_FEED service for first ConsumerManager
  - MarketPrice TRI.N item from DIRECT_FEED service for second ConsumerManager
+ Processes data received from user thread in ConsumerManager for 60 seconds
  - all received messages are processed on user thread of control  
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.


OmmConsumerClient and Callbacks
===============================

The 410__MarketPrice__HorizontalScaling demonstrates how to receive and process
individual item response messages. Additionally this application shows a native
/ RWF decoding of FieldList container.