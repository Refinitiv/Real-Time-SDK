Summary
=======

The 333_Login_Streaming_DomainRep application is provided as an example of OMM Consumer
application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM Login and MarketPrice data from Data Feed Direct
(LDFD), directly from an OMM Provider application, or from an
Advanced Distribution Server.

333_Login_Streaming_DomainRep showcases usage of login stream in OMM Consumer.
It demonstrates opening of login stream as well as its processing. Having
a login stream open is useful for consumer applications willing to do
Off Stream Posting and or knowing the state of its connectivity to server.

This application is the Domain Representation version of ex330_Login_Streaming. It demonstrates the
ease-of-use Domain Representation functionality.

Detailed Description
====================

333_Login_Streaming_DomainRep implements the following high level steps:

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
  - Login Domain stream
  - MarketPrice Domain IBM.N item DIRECT_FEED 
+ Processes data received from user thread for 60 seconds
  - all received messages are processed on main thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
