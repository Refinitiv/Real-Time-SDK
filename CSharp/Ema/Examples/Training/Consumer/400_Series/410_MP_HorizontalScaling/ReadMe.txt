Summary
=======

The 410_MP_HorizontalScaling application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketPrice data from Refinitiv Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced Distribution Server.

410_MP_HorizontalScaling showcases horizontal scaling capability
of OMM Consumer to take advantage of multiple core processors.


Detailed Description
====================

410_MP_HorizontalScaling implements the following high level steps:

+ Implements ConsumerManager to manage OmmConsumer and control user dispatch thread
  - overrides desired methods
  - provides own methods as needed
+ Implements OmmConsumerClient class in AppClient
+ Instantiates two ConsumerManager to create OmmConsumer and user threads with the 
  following configuration
  - sets user name to "user1" and "user2" for each ConsumerManager respectively
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ ConsumerInstance instantiates OmmConsumer object which
  - initializes connection and logins into specified server
+ Instantiates AppClient object that receives and processes item messages
+ Opens streaming item interests from each ConsumerManager 
  - MarketPrice IBM.N item from DIRECT_FEED service for first ConsumerManager
  - MarketPrice TRI.N item from DIRECT_FEED service for second ConsumerManager
+ Processes data received from user thread in ConsumerInstance for 60 seconds
  - all received messages are processed on user thread of control  
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
