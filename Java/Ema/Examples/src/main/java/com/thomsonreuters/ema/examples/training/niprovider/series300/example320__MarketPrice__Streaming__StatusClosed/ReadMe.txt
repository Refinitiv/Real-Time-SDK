Summary
=======

320__MarketPrice__Streaming__StatusClosed is an example of an OMM NiProvider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

320_MarketPrice_Streaming_StatusClosed illustrates how to create and publish a single OMM
streaming item and a source directory.


Detailed Description
====================

320__MarketPrice__Streaming__StatusClosed implements the following high-level steps:

+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
	- specified SourceDirectory of NI_PUB
	- specified Channel's host and port
  - Sets the username to "user"
  - sets the UserControl mode for the source directory
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
+ Creates source directory refresh message with TEST_NI_PUB service and publishes it 
+ Creates streaming item (refresh and updates) and publishes it
  - MarketPrice IBM.N item on the NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Publishes item status closed message.
+ Waits for 20 seconds
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
