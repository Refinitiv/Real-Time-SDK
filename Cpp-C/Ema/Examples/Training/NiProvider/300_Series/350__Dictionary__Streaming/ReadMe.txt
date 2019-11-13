Summary
=======

350__Dictionary__Streaming is an example of an OMM NiProvider application written
to the EMA library.

This application demonstrates how to implement a provider to download dictionary
and provide OMM MarketPrice data to Advanced Distribution Hub.

350__Dictionary__Streaming illustrates how to register for the dictionary domain, 
create and publish one OMM streaming item then process received dictionary messages.
This application uses source directory configured in the EmaConfig.xml file.

The 350__Dictionary__Streaming provides an command line argument as follows:

-dumpDictionary 		Dump data dictionary information to the console if specified. 

Detailed Description
====================

350__Dictionary__Streaming implements the following high-level steps:

+ Implements OmmProviderClient class in AppClient
  - overrides desired methods
+ Instantiates AppClient object that receives and processes dictionary messages
+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
	- specified SourceDirectory of TEST_NI_PUB
	- specified SourceDirectory of NI_PUB
	- specified Channel's host and port
  - Sets the username to "user"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Registers for dictionary stream
+ Creates streaming item (refresh and updates) and publishes it
  - MarketPrice TRI.N item on the TEST_NI_PUB service
+ Publishes updates 1 per second
+ Processes dictionary messages received from the API
  - all received messages are processed on the API thread of control
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
