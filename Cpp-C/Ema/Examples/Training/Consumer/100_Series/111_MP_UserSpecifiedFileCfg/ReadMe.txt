Summary
=======

111_MP_UserSpecifiedFileCfg illustrates the ability of the user
to specify a configuration file used when creating OmmConsumerConfig.

Detailed Description
====================

111_MP_UserSpecifiedFileCfg implements the following high-level steps:

+ The application accepts an optional argument, which is a configuration file or
+ a directory (which must contain EmaConfig.xml). Without an argument, the application
+ works the same as 110_MP_FileCfg.
+
+ Note that 111_MP_UserSpecifiedFileCfg requires a configuration file
+ to work properly.

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the consumer name to "Consumer_2"
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds
  - All received messages are processed on the API's thread of control
+ Exits

Note: If needed, you can modify these and other details to fit your local
      environment using the EmaConfig.xml file. For details on the standard 
      configuration, refer to the EMA library ReadMe.txt file or the EMA 
      Configuration Guide.
