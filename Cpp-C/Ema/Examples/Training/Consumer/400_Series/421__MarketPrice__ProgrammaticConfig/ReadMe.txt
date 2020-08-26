Summary
=======

The 421__MarketPrice__ProgrammaticConfig application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing and
parsing of OMM MarketPrice Domain data from Refinitiv Data Feed Direct,
directly from an OMM Provider application, or from the Advanced
Distribution Server.

The 421__MarketPrice__ProgrammaticConfig showcases creation and usage of the 
programmatic configuration feature of EMA. In addition to file configuration,
EMA allows users to grammatically configure Omm Consumer instances.


Detailed Description
====================

The 421__MarketPrice__ProgrammaticConfig implements the following high level steps:

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
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
