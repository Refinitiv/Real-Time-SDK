Summary
=======

121_MP_FieldListSearchByName is an example of an OMM Consumer 
application and demonstrates basic usage of the EMA library in accessing and parsing
OMM MarketPrice data from Refinitiv Data Feed Direct, directly from an OMM 
Provider application, or from an Advanced Distribution Server.

121_MP_FieldListSearchByName illustrates how to selectively extract data 
from a field entry whose name is specified on the call to forth().


Detailed Description
====================

121_MP_FieldListSearchByName implements the following high-level steps:

+ Implements an OmmConsumerClient class in an AppClient
  - Overrides desired methods
  - Provides own methods as needed, e.g. decode( const FieldList& )
    + The decode( const FieldList& ) iterates through the received FieldList and
	  searches for a FieldEntry that matches the passed-in name. If found, the 
	  decode extracts the FieldEntry reference from the current position on the 
	  FieldList and extracts its field id, name, and value.
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object:
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection and logs into
  the specified serve.
+ Opens a streaming item interest for MarketPrice IBM.N item from the DIRECT_FEED 
  service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
  - Decodes a FieldList container by searching for a field named "BID"
+ Exits

Note: If needed, you can modify these and other details to fit your local
      environment. for details on standard configuration, refer to the EMA library 
      ReadMe.txt file or the EMA Configuration Guide.
