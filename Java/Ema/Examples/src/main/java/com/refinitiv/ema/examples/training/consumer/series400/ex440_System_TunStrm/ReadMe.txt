Summary
=======

ex440_System_TunStrm is an example of an OMM Consumer application written to the EMA
library.

This application demonstrates the usage of the EMA library in accessing and parsing of
OMM Data received through the Tunnel Stream connectivity. Such connectivity and OMM Data
can be obtained from the Auxiliary Services Gateway. Referential data and Time Series are
examples of OMM Data for which one requires the Tunnel Stream connectivity.

ex440_System_TunStrm illustrates how to open and process a tunnel stream and a sub-stream.
While processing received messages, the application prints them to the screen.


Detailed Description
====================

ex440_System_TunStrm implements the following high-level steps:

+ Implements an OmmConsumerClient class in an AppClient:
  - Overrides desired methods.
  + Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the user name to "user"
  - Reads in EmaConfig.xml file
+ Instantiates an OmmConsumer object which initializes the connection
  and logs into the specified server
+ Opens a tunnel stream
  - TUNNEL on MMT_SYSTEM domain from the service of DIRECT_FEED
+ Opens streaming sub stream item on the tunnel stream
  - MarketPrice TUNNEL_ITEM from the service with id of 1
+ Processes data received from the API for 60 seconds
  - Received messages are processed on the API thread of control
+ Exits

Note: If needed, you can modify these and other details to fit your local environment.
      For details on standard configuration, refer to the EMA library ReadMe.txt file
      or to the EMA Configuration Guide.
