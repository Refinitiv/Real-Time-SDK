Summary
=======

340__Login__Streaming is an example of an OMM NiProvider application written
to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

340__Login__Streaming illustrates how to create and publish one OMM streaming item
and register for and process received login messages. This application uses source
directory configured in the EmaConfig.xml file.


Detailed Description
====================

340__Login__Streaming implements the following high-level steps:

+ Implements OmmProviderClient class in AppClient
  - overrides desired methods
+ Instantiates AppClient object that receives and processes login messages
+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
    - specified SourceDirectory of TEST_NI_PUB
	- specified Channel's host and port
  - Sets the username to "user"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Registers for login stream
+ Creates streaming item (refresh and updates) and publishes it
  - MarketPrice IBM.N item on the TEST_NI_PUB service
+ Publishes updates 1 per second
+ Processes login messages received from the API for 60 seconds
  - all received messages are processed on the API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
