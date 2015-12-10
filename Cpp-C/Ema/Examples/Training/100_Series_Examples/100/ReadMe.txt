Summary
=======

100__NiProvider__X is an example of an OMM NiProvider application written to the EMA library.

This application demonstrates the basic usage of the EMA library connecting to a ADH,
sending a directory message, sending the initial refresh message, and sending a single
update.

Received messages are printed to the screen.

Detailed Description
===============

100__NiProvider__X implements the following high-level steps:

+ Implements the OmmConsumerClient class in the AppClient
  - Overrides desired methods
+ Instantiates an AppClient object to receive and process item messages (in this case, resulting from
login or dictionary messages)
+ Instantiates an OmmNiProvider object which initializes the connection and logs into an ADH at the
default location ( localhost:14002 ) using the default username ( "user" )
 - the OmmNiProvider uses a default configuration.
+ Creates and sends the directory message
  - service name NI_PUB on capability MMT_MARKET_PRICE
+ Creates and sends the initial refresh message for item TRI.N
+ Creates and sends an update message for item TRI.N
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
