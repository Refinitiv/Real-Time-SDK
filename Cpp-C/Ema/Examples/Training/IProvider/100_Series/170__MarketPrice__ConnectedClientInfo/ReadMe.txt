Summary
=======

170__MarketPrice__ConnectedClientInfo is an example of an OMM Interactive Provider application 
written to the EMA library.

170__MarketPrice__ConnectedClientInfo illustrates printing channel information about connected clients,
and printing channel information on client events.

Detailed Description
====================

170__MarketPrice__ConnectClientInfo implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Sets the listening  port to "14002"
+ Instantiates an OmmProvider object which:
  - listens on the above port
+ Calls an invalid method (getChannelInfo) but catches exception
+ Accepts one or more login requests and prints event information on those requests
+ Processes one or more item requests for MarketPrice domain and prints event information on
  those requests
  - Creates streaming item (refresh and updates) and publishes them
  - Publishes updates 1 per second for 60 seconds.
+ As the number of clients changes, prints information on connected clients
+ Correctly removes disconnected clients
+ Prints connected client information

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
