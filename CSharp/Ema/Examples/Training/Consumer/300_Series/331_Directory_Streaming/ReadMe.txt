Summary
=======

The 331_Directory_Streaming application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM Directory and MarketPrice data from Refinitiv Data Feed 
Direct (RDF-D), directly from an OMM Provider application, or from an 
Advanced Distribution Server.

331_Directory_Streaming showcases usage of directory stream in OMM
Consumer. It demonstrates opening of directory stream as well as its processing.
Having a directory stream open is useful for consumer applications willing to
know the state of services and item groups it consumes.


Detailed Description
====================

331_Directory_Streaming implements the following high level steps:

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
  - Directory Domain stream
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
	  
