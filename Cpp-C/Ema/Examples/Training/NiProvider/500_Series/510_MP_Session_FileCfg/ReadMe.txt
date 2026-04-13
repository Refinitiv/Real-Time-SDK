Summary
=======

ex510_MP_Session_FileCfg is an example of an OMM NiProvider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

ex510_MP_Session_FileCfg illustrates how to create and publish two OMM streaming
items. This application uses source directory configured in the EmaConfig.xml file.


Detailed Description
====================

ex510_MP_Session_FileCfg implements the following high-level steps:

+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
    - specified SourceDirectory of TEST_NI_PUB
	- specified Session configuration with multiple ADH connections
  - Sets the username to "user"
+ Instantiates AppClient object that receives and processes login messages  
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH instances
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Creates streaming items (refresh and updates) and publishes them
  - MarketPrice IBM.N item on the TEST_NI_PUB service
  - MarketPrice TRI.N item on the TEST_NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
