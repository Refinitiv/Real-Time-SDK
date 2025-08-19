Summary
=======

The ex501_PreferredHost_ProgCfg application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing and
parsing of OMM MarketPrice Domain data from Refinitiv Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced Distribution Server.

The ex501_PreferredHost_ProgCfg illustrates a fallback to preferred host feature,
which allows to switch to preferred host/endpoint if the feature is enabled when 
the connection is lost.The fallback performs full connection recovery to establish 
a new connection, handle admin domains (login, source directory, dictionary), 
re-request market data items when the watchlist is enabled.

This application showcases creation and usage of the programmatic configuration 
feature of EMA. In addition to file configuration, EMA allows users to 
programmatically configure Omm Consumer instances.


Detailed Description
====================

The ex501_PreferredHost_ProgCfg implements the following high level steps:
+ Passes preferred host options through command line arguments
  including:
  - enablePH Enable preferred host feature (optional).
  - detectionTimeSchedule Specifies Cron time format for detection time 
    schedule (optional).
  - detectionTimeInterval Detection time interval in seconds. 0 indicates that 
    the detection time interval is disabled (optional).
  - channelNamePreferred Specifies a channel name in the Channel or ChannelSet 
    element (optional).

Example command to run the example from the command line from Java folder:
On Unix:
./gradlew runConsumer501 -PcommandLineArgs='-enablePH <true/false> -detectionTimeSchedule <cron_expression> -detectionTimeInterval <detectionTimeInterval> -channelNamePreferred <channelNamePreferred>'

On Windows:
gradlew.bat runConsumer501 -PcommandLineArgs='-enablePH <true/false> -detectionTimeSchedule <cron_expression> -detectionTimeInterval <detectionTimeInterval> -channelNamePreferred <channelNamePreferred>'

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates a Map (configMap object) and populates it with configuration info
  - sets the default consumer name to "Consumer_A". It contains channel set with two 
    channels
+ Instantiates and modifies OmmConsumerConfig object
  - sets Omm Consumer configuration with data from the programmatic configuration
+ Instantiates OmmConsumer object which initializes connection and logins into
  the specified server
+ Opens a streaming item interest
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Switches over to a preferred host according to preferred host configuration for 
  a consumer
  - default preferred host is set to 'Channel_A'. This value can be overridden by
    command line argument
+ Processes data received from API for 600 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit your local
      environment.

Note: please refer to the EMA library ReadMe.txt file or to the EMA Configuration 
      Guide for details on standard configuration.
