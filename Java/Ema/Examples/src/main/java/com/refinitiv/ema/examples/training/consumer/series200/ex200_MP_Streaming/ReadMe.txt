Summary
=======

The ex200_MP_Streaming application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketPrice data from Refinitiv Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from the Advanced
Distribution Server.

ex200_MP_Streaming showcases native or RWF decoding of received
FieldList. This application iterates through the received FieldList and calls
respective (based on the data type) accessor methods to extract data in native
format.


Detailed Description
====================

ex200_MP_Streaming implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
    - the decode( FieldList ) iterates through the received FieldList,
	  extracts each FieldEntry reference from the current position on the
	  FieldList, and extracts field id, name and load of the FieldEntry.
	  The load extraction is done using native or RWF data formats
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received with user dispatch loop for 60 seconds
  - all received messages are processed on main application thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
