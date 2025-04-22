Summary
=======

ex510_RequestRouting_FileCfg is an example of an OMM Consumer application
written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing and
parsing of OMM MarketPrice Domain data from Data Feed Direct,
directly from an OMM Provider application, or from the Advanced
Distribution Server.

ex510_RequestRouting_FileCfg showcases an usage of the request routing
feature to route market data item requests to multiple connections depending
on the availability of each connection. This example gets a session information 
from OmmConsumerEvent to display all session information.

This example uses the file configuration to enable the request routing feature.

Detailed Description
====================

ex510_RequestRouting_FileCfg implements the following high-level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed
+ Instantiates AppClient object that receives and processes item messages
+ Create a service list named "SVG1" which contains the "DIRECT_FEED" and "DIRECT_FEED_2"
+ Instantiates and modifies OmmConsumerConfig object
  - Sets the consumer name to "Consumer_10"
  - Adds the service list into this object.
  - Loads configuration information for the specified consumer name
    from the EmaConfig.xml file in the application's working folder
+ Instantiates OmmConsumer object which initializes connection and logins into
  the specified server
+ Opens a streaming item interest
  - MarketPrice Domain LSEG.L item from SVG1 service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
