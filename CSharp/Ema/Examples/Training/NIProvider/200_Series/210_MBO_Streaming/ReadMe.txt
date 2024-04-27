Summary
=======

210_MBO_Streaming is an example of an OMM NiProvider application
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketByOrder data to the Advanced Distribution Hub.

210_MBO_Streaming illustrates how to create and publish two OMM streaming
items. This application uses source directory configured in the EmaConfig.xml file.


Detailed Description
====================

210_MBO_Streaming implements the following high-level steps:

+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
    - specified SourceDirectory of TEST_NI_PUB
	- specified Channel's host and port
  - Sets the username to "user"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the specified ADH
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Creates streaming items (refresh and updates) and publishes them
  - MarketByOrder AAO.V item on the TEST_NI_PUB service
  - MarketByOrder AGG.V item on the TEST_NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
