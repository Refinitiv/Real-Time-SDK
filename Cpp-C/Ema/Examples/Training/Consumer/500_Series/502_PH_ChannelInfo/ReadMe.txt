Summary
=======

The ex502_PreferredHost_Ioctl application is provided as an example of OMM Consumer 
application written to the EMA library.

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

The ex502_PreferredHost_Ioctl implements the following high level steps:
+ Passes preferred host options through command line arguments
  including:
  - ioctlInterval Specifies time interval in seconds before call IOCtl is 
    invoked (optional).
  - fallbackInterval Specifies time interval in seconds before call Ad Hoc 
    Fallback Function is invoked (optional).
  - ioctlEnablePH Enable preferred host feature (optional).
  - ioctlDetectionTimeSchedule Specifies Cron time format for detection time 
    schedule (optional).
  - ioctlDetectionTimeInterval Detection time interval in seconds. 0 indicates that 
    the detection time interval is disabled (optional).
  - ioctlChannelName Specifies a channel name in the Channel or ChannelSet 
    element (optional).
  - ioctlWarmStandbyGroup Specifies a WSB channel name in the WarmStandbyChannelSet 
    element (optional).
  - ioctlFallBackWithinWSBGroup Specifies whether to fallback within a WSB group 
    instead of moving into a preferred WSB group (optional).

Example command to run the example from the command line from Java folder:
On Unix:
./gradlew runConsumer502 -PcommandLineArgs='-ioctlInterval <ioctlInterval> -fallbackInterval <fallbackInterval> -ioctlEnablePH <true/false> -ioctlDetectionTimeSchedule <cron_expression> -ioctlDetectionTimeInterval <ioctlDetectionTimeInterval> -ioctlChannelName <ioctlChannelName> -ioctlWarmStandbyGroup <ioctlWarmStandbyGroup> -ioctlFallBackWithinWSBGroup <true/false>'

On Windows:
gradlew.bat runConsumer502 -PcommandLineArgs='-ioctlInterval <ioctlInterval> -fallbackInterval <fallbackInterval> -ioctlEnablePH <true/false> -ioctlDetectionTimeSchedule <cron_expression> -ioctlDetectionTimeInterval <ioctlDetectionTimeInterval> -ioctlChannelName <ioctlChannelName> -ioctlWarmStandbyGroup <ioctlWarmStandbyGroup> -ioctlFallBackWithinWSBGroup <true/false>'

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
+ After ioctlInterval seconds calls modifyIOCtl to change preferred host options 
  - default preferred host options can be overridden by command line arguments
+ After fallbackInterval seconds calls fallbackPreferredHost to explicitly 
  switch to preferred host
+ Processes data received from API for 600 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.

Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
