Summary
=======

130_MP_UserDisp is provided as an example of an OMM Consumer 
application that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Data Feed Direct,
directly from an OMM Provider  application, or from an Advanced Distribution
Server.

130_MP_UserDisp illustrates a "user dispatch" mode in which all 
callbacks execute on the application's thread of control and call the 
dispatch() method.

 
Detailed Description
====================

130_MP_UserDisp implements the following high-level steps:

+ Implements an OmmConsumerClient class in an AppClient
  - Overrides desired methods.
  - Provides its own methods as needed (e.g.: decode( const FieldList& ))
    - The decode( const FieldList& ) iterates through the received FieldList
	  extracting each FieldEntry reference from the current position on the
	  FieldList. The decode method then extracts the fieldentry's field id, 
	  name, and value.
+ Instantiates an AppClient object that receives and processes item messages.
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the username to "user".
  - Sets the hostname on the preconfigured connection to "localhost".
  - Sets the port on the preconfigured connection to "14002".
  - Sets the operationModel to UserDispatchEnum.
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
  - The application calls dispatch() in a while loop.
+ Opens a streaming item interest for the MarketPrice IBM.N item from the DIRECT_FEED
  service
+ Processes data received with a user dispatch loop for 60 seconds
  - All messages received are processed on the application's main thread of control
+ Exits

Note: If needed, you can modify these and other details to fit your local environment.
      For details on standard configuration, refer to the EMA library ReadMe.txt file
      or to the EMA COnfiguration Guide.
