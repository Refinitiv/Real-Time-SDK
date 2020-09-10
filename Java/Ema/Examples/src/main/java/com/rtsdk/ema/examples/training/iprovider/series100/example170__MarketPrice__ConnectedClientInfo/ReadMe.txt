Summary
=======

170__MarketPrice__ConnectedClientInfo is an example of an OMM Interactive Provider application
written to the EMA library.

170__MarketPrice__ConnectedClientInfo demonstrates the basic usage of the EMA library in providing
information on the channels used by application clients to connect to the application.

Detailed Description
====================

170__MarketPrice__ConnectedClientInfo implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Sets the listening  port to "14002"
  - Sets operational model to "USER_DISPATCH"
+ Instantiates an OmmProvider object which:
  - listens on the above port
+ Accepts login request
+ Processes item requests for MarketPrice domain from clients
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for 60 seconds.
+ as the number of connected clients changes, published the channel
   information for each connected client
+ publishes channel information of the channel associated with close, login,
   and market price events
+ at the end of the 60 seconds publishing period, prints channel information
   for all clients still connected to the applications
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
