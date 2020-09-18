Summary
=======

110_MBO_Streaming is an example of an OMM NiProvider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketByOrder data to the Advanced Distribution Hub.

110_MBO_Streaming illustrates how to create and publish a single OMM
streaming item. This application uses hardcoded source directory configuration.


Detailed Description
====================

110_MBO_Streaming implements the following high-level steps:

+ Instantiates and modifies an OmmNiProviderConfig object:
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14003"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the specified ADH
  - sends down the source directory refresh message with NI_PUB service info
+ Creates streaming item (refresh and updates) and publishes it
  - MarketByOrder AAO.V item on the NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
