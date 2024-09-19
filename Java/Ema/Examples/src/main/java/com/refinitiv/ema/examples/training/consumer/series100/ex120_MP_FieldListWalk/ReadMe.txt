Summary
=======

ex120_MP_FieldListWalk is an example of an OMM Consumer application 
and demonstrates how to use the EMA library for accessing and parsing OMM 
MarketPrice data from Data Feed Direct (LDFD), directly from an OMM
Provider application, or from an Advanced Distribution Server.

ex120_MP_FieldListWalk illustrates how to extract OMM data from the
received FieldList. The Application "walks" or iterates over all field entries.
While iterating, this application extracts the content of each field entry
and prints it to the screen.


Detailed Description
====================

ex120_MP_FieldListWalk implements the following high-level steps:

+ Implements the OmmConsumerClient class in an AppClient
  - Overrides desired methods
  - Provides its own methods as needed (e.g.: decode( FieldList ) )
  - The method decode( FieldList ) iterates through the received FieldList,
	  extracts each FieldEntry reference from the current position on the
	  FieldList, and then extracts the field id, name, and value of the current
	  FieldEntry
+ Instantiates an AppClient object that receives and processes item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, you can modify these and other details to fit your local
      environment. For details on standard configuration, refer to the EMA library 
      ReadMe.txt file or EMA Configuration Guide.
