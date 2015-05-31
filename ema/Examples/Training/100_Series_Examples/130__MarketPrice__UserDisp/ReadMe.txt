Summary
=======

The 130__MarketPrice__UserDisp application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketPrice data from Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider  application, or from Thomson Reuters Advanced
Distribution Server.

The 130__MarketPrice__UserDisp showcases the so called "user dispatch" mode
in which all callbacks are executed on the application thread of control.
They are executed on the thread calling the dispatch() method.

 
Detailed Description
====================

The 130__MarketPrice__UserDisp implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - the decode( const FieldList& ) iterates through the received FieldList,
	  extracts each FieldEntry reference from the current position on the
	  FieldList, and extracts field id, name and value of the current FieldEntry
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
  - sets operationModel to UserDispatchEnum
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
  - application calls dispatch() in a while loop
+ Opens a streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received with user dispatch loop for 60 seconds
  - all received messages are processed on the main application thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
