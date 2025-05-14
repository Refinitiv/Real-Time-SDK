Summary
=======

511_RequestRouting_ProgrammaticCfg is an example of an OMM Consumer application
written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing and
parsing of OMM MarketPrice Domain data from Data Feed Direct,
directly from an OMM Provider application, or from the Advanced
Distribution Server.

511_RequestRouting_ProgrammaticCfg showcases an usage of the request routing
feature to route market data item requests to multiple connections depending
on the availability of each connection. This example gets a session channel
information from OmmConsumerEvent.

This example uses the programmatic configuration to enable the request routing feature.

Detailed Description
====================

511_RequestRouting_ProgrammaticCfg implements the following high-level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates a Map (configMap object) and populates it with configuration info
+ Instantiates and modifies OmmConsumerConfig object
  - sets Omm Consumer configuration with data from the programmatic configuration
+ Instantiates OmmConsumer object which initializes connection and logins into
  the specified server
+ Opens a streaming item interest
  - MarketPrice Domain LSEG.L item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
