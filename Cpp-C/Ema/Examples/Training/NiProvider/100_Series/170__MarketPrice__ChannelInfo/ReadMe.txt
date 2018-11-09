Summary
=======

170__MarketPrice__ChannelInfo is an example of an OMM NiProvider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Thomson Reuters Advanced Distribution Hub. It
also demonstrates retrieving and displaying information about the channel used
in that connection.

Detailed Description
====================

170__MarketPrice__ChannelInfo implements the following high-level steps:

+ Instantiates and modifies an OmmNiProviderConfig object:
  - Sets the username to "user"
  - Sets the config file to EmaConfig.xml so that a proper ADH can be configured
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the specified ADH
  - sends down the source directory refresh message with NI_PUB service
+ Retrieves and displays information about the channel used in the
  connection both from the OmmProvider object and an OmmProviderEvent.
+ Creates streaming item (refresh and updates) and publishes it
  - MarketPrice IBM.N item on the NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
