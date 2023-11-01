Summary
=======

111_MP_UserSpecifiedFileConfig is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Refinitiv Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced
Distribution Server.

111_MP_UserSpecifiedFileConfig illustrates the abilitily of the user
to specify a configuration file to be used when creating the OmmConsumerConfig. When
running this application, the user may specify either a configuration file or a directory
(which must contain EmaConfig.xml).

if no argument is supplied, the application works the same as 110_MP_FileConfig.

Detailed Description
====================

111_MP_UserSpecifiedFileConfig implements the following high-level steps:

+ Uses the customer-supplied argument to locate a configuration file; if no argument is supplied,
   locates the configuration file in the run-time directory or uses the default configuration (note: this
   application requires a configuration file).
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
