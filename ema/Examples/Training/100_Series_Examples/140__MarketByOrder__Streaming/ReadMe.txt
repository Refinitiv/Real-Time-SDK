Summary
=======

The 140__MarketByOrder__Streaming application is an example of
an OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library to access
and parse OMM MarketByOrder data from the Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from the Thomson Reuters Advanced
Distribution Server.

The 140__MarketByOrder__Streaming showcases opening and processing of 
MarketByOrder item. While processing received messages, application walks
or iterates through the received Map and FieldList and prints the content
of their entries to the screen.


Detailed Description
====================

The 140__MarketByOrder__Streaming implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - the decode( const FieldList& ) iterates through the received FieldList,
	  extracts each FieldEntry reference from the current position on the
	  FieldList, and extracts field id, name and value of the current FieldEntry
	- the decode( const Map& ) iterates through the received Map, extracts Summary
	  if it is present, extracts MapEntry from which it extracts key, and load values
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - MarketByOrder AAO.V item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
