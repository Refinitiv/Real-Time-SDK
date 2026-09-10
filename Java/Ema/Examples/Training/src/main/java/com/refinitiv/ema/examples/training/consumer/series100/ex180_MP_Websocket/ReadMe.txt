Summary
=======

180_MP_Websocket is an example of an OMM Consumer application
written to the EMA library.

This application demonstrates websocket connection as it is specified in
a configuration file used when creating OmmConsumerConfig.


Detailed Description
====================

180_MP_Websocket implements the following high-level steps:

+ The application reads EmaConfig.xml from its working folder.
+
+ Note that 180_MP_Websocket requires a configuration file
+ to work properly.

+ Implements the OmmConsumerClient class in the AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object:
  - Sets the consumer name to "Consumer_6"
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection
  and logs into the specified server.
+ Opens streaming item interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds.
  - Received messages are processed on the API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA
      library ReadMe.txt file and EMA COnfiguration Guide.
