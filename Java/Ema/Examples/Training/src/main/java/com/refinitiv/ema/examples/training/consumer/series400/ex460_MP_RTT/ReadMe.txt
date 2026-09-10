Summary
=======

ex460_MP_RTT is an example of an OMM Consumer application
written to the EMA library.

This application demonstrates the basic usage of the EMA library in accessing and
parsing OMM MarketPrice data from either Data Feed Direct (LDFD), directly
from an OMM Provider application, or from the Advanced Distribution
Server.

ex460_MP_RTT illustrates how to open and process a single OMM
item. While processing received messages, the application prints them
to the screen.


Detailed Description
====================

ex460_MP_RTT implements the following high-level steps:

+ Implements the OmmConsumerClient class in the AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object:
  - Sets the consumer name to "Consumer_RTT"
  - EnableRtt flag is set to "true" for this consumer in EmaConfig.xml
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens login stream to listen for generic RTT messages
+ Opens streaming item of interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds.
  - Received messages are processed on the API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
