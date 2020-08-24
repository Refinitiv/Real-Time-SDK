Summary
=======

100__MarketPrice__Streaming is an example of an OMM Consumer application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in accessing and
parsing OMM MarketPrice data from either Refinitiv Data Feed Direct, directly
from an OMM Provider application, or from the Advanced Distribution
Server.

100__MarketPrice__Streaming illustrates how to open and process a single OMM
item. While processing received messages, the application prints them
to the screen.


Detailed Description
====================

100__MarketPrice__Streaming implements the following high-level steps:

+ Implements the OmmConsumerClient class in the AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object:
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds.
  - Received messages are processed on the API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA COnfiguration Guide.
