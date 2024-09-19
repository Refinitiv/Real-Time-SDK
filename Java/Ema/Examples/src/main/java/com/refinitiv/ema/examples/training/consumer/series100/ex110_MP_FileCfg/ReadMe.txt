Summary
=======

ex110_MP_FileConfig is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Data Feed Direct (LDFD),
directly from an OMM Provider application, or from an Advanced
Distribution Server.

ex110_MP_FileConfig illustrates an implicit dependence of the 
OMM Consumer on the EmaConfig.xml file located in the application's
working directory. Unlike other training examples, this application depends 
on EmaConfig.xml to determine its connectivity parameters.


Detailed Description
====================

ex110_MP_FileConfig implements the following high-level steps:

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
