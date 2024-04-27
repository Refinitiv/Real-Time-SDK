Summary
=======

421_MP_ProgrammaticCfg is an example of an OMM NiProvider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

421_MP_ProgrammaticCfg illustrates how to create and publish two OMM streaming
items. The example showcases creation and usage of the 
programmatic configuration feature of EMA. In addition to file configuration,
EMA allows users to programmatically configure Omm NiProvider instances.


Detailed Description
====================

421_MP_ProgrammaticCfg implements the following high-level steps:

+ Instantiates a Map (configMap object) and populates it with configuration info
+ Instantiates and modifies an OmmNiProviderConfig object:
  - sets Omm NiProvider configuration with data from the programmatic configuration
  - sets the username to "user"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Creates streaming items (refresh and updates) and publishes them
  - MarketPrice IBM.N item on the TEST_NI_PUB service
  - MarketPrice TRI.N item on the TEST_NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
