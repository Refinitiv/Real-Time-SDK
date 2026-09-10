Summary
=======

The ex502_PreferredHost_Ioctl application is provided as an example of OMM Consumer 
application written to the EMA library.

Note that "PH", used in ReadMe or example code refers to the "Preferred Host" feature.

This application demonstrates basic usage of the EMA library for accessing and
parsing of OMM MarketPrice Domain data from an LSEG Data Feed, directly from 
an OMM Provider application, or from an Advanced Distribution Server.

The ex502_PreferredHost_Ioctl example illustrates aspects of the preferred host feature,
which, when enabled, allows to switch to preferred host/endpoint upon connection loss,
preferred host feature specific time based triggers or when a function call is made
to perform a fallback to a preferred host. EMA performs fallback and 
full connection recovery by establishing a new connection, handling admin domains 
(login, source directory, dictionary), and re-requesting market data items using 
the underlying ETA layer's watchlist capabilities.

This application depends on EmaConfig.xml located in the application's working 
directory to determine its connectivity parameters and additional configuration 
parameters to enable and configure a fallback to preferred host feature.

Specifically, the ConsumerName used in this example is associated with 
Consumer/Channel/etc sections defined in the default EmaConfig.xml file to
specify fallback triggers and a channel preferences. When used in conjunction
with servers represented in the config, this application may be used to 
visualize the connection recovery to the specified preferred host.

IO Control (IOCtl) call permit changing certain configuration parameters dyanmically
at runtime. Most aspects of the preferred host configuration may be altered
using IOCtl including enabling and disable the feature from the application code.

This application demonstrates how various configuration parameters may be altered
at runtime. If configured to do so via commandline, this application also 
demonstrates the adhoc function call to attempt a fallback to a specified preferred 
host.

Detailed Description
====================

The ex502_PreferredHost_Ioctl example implements the following high level steps:
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
