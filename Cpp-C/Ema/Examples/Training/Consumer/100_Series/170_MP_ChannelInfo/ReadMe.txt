Summary
=======

170_MP_ChannelInfo is an example of an OMM Consumer application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in accessing and
parsing OMM MarketPrice data from either Refinitiv Data Feed Direct, directly
from an OMM Provider application, or from the Advanced Distribution
Server.

In addition, this application demonstrates retrieving information about the
connection from either the OmmConsumer or the OmmConsumerEvent.

Detailed Description
====================

170_MP_ChannelInfo implements the following high-level steps:

+ Implements the OmmConsumerClient class in the AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object:
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Displays channel information for the OmmConsumer object
+ Processes data received from the API for 60 seconds.
  - Received messages are processed on the API thread of control
  - Displays channel information for OmmConsumerEvents
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA COnfiguration Guide.
