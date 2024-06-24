Summary
=======

The 101_MP_QosPriority is an an example of an OMM Consumer application
written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing and
parsing OMM MarketPrice data from Data Feed Direct, directly
from an OMM Provider application, or from the Advanced Distribution
Server.

The 101_MP_QosPriority showcases opening and processing of a single OMM
item with a specified priority and desired Quality of Service. While processing
received messages, this application simply prints them to the screen.


Detailed Description
====================

101_MP_QosPriority implements the following high-level steps:

+ Implements an OmmConsumerClient class in an AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes its connection and logs
  into a specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item) from the DIRECT_FEED service with a priority class of 2,
    priority count of 1, and a Qos of RealTime / TickByTick.
+ Processes data received from the API for 60 seconds.
  - Received messages are processed on the API thread of control
+ Exits

Note: If needed, you can modify these and other details to fit your local
      environment. For details on standard configuration, refer to the 
      EMA library ReadMe.txt file and to the EMA Configuration Guide.
