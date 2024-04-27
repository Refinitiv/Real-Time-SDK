Summary
=======

170_MP_ChannelInfo is an example of an OMM NiProvider application 
written to the EMA library.

170_MP_ChannelInfo illustrates how to  access and parse
channel information on the channel used to connect to the destination
for the data published by the application


Detailed Description
====================

170_MP_ChannelInfo implements the following high-level steps:

+ Instantiates and modifies an OmmNiProviderConfig object:
  - Uses the EmaConfig.xml file for configuration
  - Sets the username to "user"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the specified ADH
  - retrieves and prints the channel information associated with
    the channel used to connect to the ADH
+ Creates streaming item (refresh and updates) and publishes it
  - MarketPrice IBM.N item on the NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Calls registerClient with domain MMT_LOGIN on the provider
   object to create a refresh event. When the event is received
   the channel information associated with the event is printed
+ Adds status message event processing which prints the
   channel information associated with the status event. Stopping
   and restarting the ADH illustrates the differences in the
   channel information
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
