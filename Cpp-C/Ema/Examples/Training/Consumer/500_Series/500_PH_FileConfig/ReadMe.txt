Summary
=======

The ex500_MP_PreferredHost_FileCfg application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing and
parsing of OMM MarketPrice Domain data from LSEG Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced Distribution Server.

The ex500_MP_PreferredHost_FileCfg illustrates a fallback to preferred host feature,
which allows to switch to preferred host/endpoint if the feature is enabled when 
the connection is lost.The fallback performs full connection recovery to establish 
a new connection, handle admin domains (login, source directory, dictionary), 
re-request market data items when the watchlist is enabled.

This application depends on EmaConfig.xml located in the application's working 
directory to determine its connectivity parameters and additional configuration 
parameters to enable and configure a fallback to preferred host feature.

Detailed Description
====================

The ex500_MP_PreferredHost_FileCfg implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies an OmmConsumerConfig object
  - sets the consumer name to "Consumer_9". It contains channel set with two 
    channels and WSB channel set with two WSB channels 
  - loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates OmmConsumer object which initializes connection and logins into
  the specified server
+ Opens a streaming item interest
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Switches over to a preferred host or WSB group according to a default 
  configuration for a selected consumer
  - preferred host is set to 'Channel_1'
  - WSB group is set to 'WarmStandbyChannel_1'
+ Processes data received from API for 600 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.

Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
