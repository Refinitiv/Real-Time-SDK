Summary
=======

102_MP_Snapshot is an example of an OMM Consumer application
written to the EMA library and demonstrates basic usage of the EMA library 
in accessing and parsing OMM MarketPrice data either from Data Feed Direct (LDFD),
directly from an OMM Provider application, or from the Advanced
Distribution Server.

102_MP_Snapshot illustrates how to open a single snapshot OMM item.
While processing received messages, the application prints them to the screen.


Detailed Description
====================

102_MP_Snapshot implements the following high-level steps:

+ Implements the OmmConsumerClient class in an AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object:
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens snapshot item interest:
  - MarketPrice IBM.N item from the DIRECT_FEED service
  - Uses interestAfterRefresh( false ) to make it a snapshot request
+ Processes data received from the API for 60 seconds
  - The API thread of control processes all received messages.
+ Exits

Note: If needed, you can modify these and other details to fit your local
      environment. For details on standard configuration, refer to the
      EMA library ReadMe.txt file and the EMA Configuration Guide.