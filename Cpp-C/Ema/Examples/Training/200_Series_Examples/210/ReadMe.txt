Summary
=======

210__NiProvider__X is an example of an OMM NiProvider application written to the EMA library.

This application demonstrates the basic usage of the EMA library connecting to a ADH,
sending a directory message, sending the initial refresh message for multiple items,
and sending 50 updates for multiple items.

Received messages are printed to the screen.

Detailed Description
===============

210__NiProvider__X implements the following high-level steps:

+ Instantiates an OmmNiProvider object which initializes the connection and logs into an ADH at the
default location ( localhost:14002 ) using the default username ( "user" )
 - the OmmNiProvider uses a default configuration.
+ Creates and sends the directory message
  - service name NI_PUB on capability MMT_MARKET_PRICE
+ Creates and sends the initial refresh message for item TRI.N
+ Creates and sends the initial refresh message for item IBM.N
+ Creates and sends 50 update message for item TRI.N and IBM.N
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
