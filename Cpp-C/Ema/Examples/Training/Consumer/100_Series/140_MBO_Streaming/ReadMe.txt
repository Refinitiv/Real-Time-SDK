Summary
=======

140_MBO_Streaming is an example of an OMM Consumer application
that demonstrates basic usage of the EMA library to access and parse OMM 
MarketByOrder data from the Data Feed Direct, directly from 
an OMM Provider application, or from an Advanced Distribution Server.

140_MBO_Streaming illustrates how to open and process a
MarketByOrder item. While processing received messages, the application 
iterates through the received Map and FieldList and prints the content
of their entries to the screen.


Detailed Description
====================

140_MBO_Streaming implements the following high-level steps:

+ Implements an OmmConsumerClient class in an AppClient:
  - Overrides desired methods.
  - Provides its own methods as needed (e.g.: decode( const FieldList& ))
    + The decode method iterates through the received FieldList, extracts each
	  FieldEntry reference from its current position on the FieldList, and
	  extracts the current FieldEntry's field id, name, and value.
    + The decode method iterates through the received Map, extracts the Summary
	  (if present), extracts the MapEntry and the MapEntry's key and load 
	  values.
+ Instantiates AppClient object that receives and processes item messages.
+ Instantiates and modifies OmmConsumerConfig object:
  - Sets the username to "user".
  - Sets the hostname on the preconfigured connection to "localhost".
  - Sets the port on the preconfigured connection to "14002".
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest for a MarketByOrder AAO.V item from the DIRECT_FEED
  service
+ Processes data received from the API for 60 seconds. All received messages are 
  processed on the API's thread of control.
+ Exits.

Note: If needed, you can modify these and other details to fit your local environment.
      For details on standard configuration, refer to the EMA library ReadMe.txt file
      or to the EMA COnfiguration Guide.
