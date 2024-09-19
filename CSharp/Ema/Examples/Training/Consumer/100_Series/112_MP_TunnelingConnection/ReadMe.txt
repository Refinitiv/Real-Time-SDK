Summary
=======

112_MP_TunnelingConnection is an OMM Consumer application example
that demonstrates basic usage of the EMA library in accessing
and parsing OMM MarketPrice data from Data Feed Direct (LDFD),
directly from an OMM Provider application, or from an Advanced
Distribution Server.

112_MP_TunnelingConnection illustrates the abilitily of the user
to programmatically pass all encrypted on OmmConsumerConfig
instance when configuring a tunneling connection. When running this application,
the user will need specify valid tunneling configurations through command line arguments.

Detailed Description
====================

112_MP_TunnelingConnection implements the following high-level steps:
+ Passes tunneling related configuration through command line arguments
including:
if the application will attempt to make an encrypted
       connection, ChannelType must be set to
	   ChannelType::RSSL_ENCRYPTED in EMA configuration file
-ph Proxy host name
-pp Proxy port number
-plogin User name on proxy server
-ppasswd Password on proxy server

+ Implements OmmConsumerClient class in AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - Sets the tunneling configurations
  - Sets the consumer name to "Consumer_3"
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
