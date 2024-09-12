Summary
=======

250_MP_Perf is an example of an OMM NiProvider application written
to the EMA library.

This application demonstrates the performance usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

250_MP_Perf illustrates how to efficiently create and publish a number
of OMM streaming items. This application uses source directory configured in the EmaConfig.xml
file.


Detailed Description
====================

250_MP_Perf implements the following high-level steps:

+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
    - specified SourceDirectory of TEST_NI_PUB
	- specified Channel's host and port
  - Sets the username to "user"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Creates a number of streaming items (refresh and updates) and publishes them
  - refresh messages use ServiceName.
+ Publishes a number of updates for 300 seconds.
+ Prints out approximate rates of refreshes and updates every second or so
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
