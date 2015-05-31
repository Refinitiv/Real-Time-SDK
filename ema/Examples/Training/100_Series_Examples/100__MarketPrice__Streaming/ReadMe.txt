Summary
=======

The 100__MarketPrice__Streaming is an example of an OMM Consumer application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library to access and
parse OMM MarketPrice data from Reuters Data Feed Direct (RDF-D), directly from
an OMM Provider application, or from a Thomson Reuters Advanced Distribution Server.

The 100__MarketPrice__Streaming showcases opening and processing of a single OMM
item. While processing received messages, this application simply prints them
out to the screen.


Detailed Description
====================

The 100__MarketPrice__Streaming implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies OmmConsumerConfig object:
  - Sets the user name to "user"
  - Sets the host name on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds.
  - all received messages are processed on the API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file .
