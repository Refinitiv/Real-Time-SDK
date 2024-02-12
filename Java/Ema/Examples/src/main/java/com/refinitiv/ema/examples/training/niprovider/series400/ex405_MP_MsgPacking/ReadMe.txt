Summary
=======

ex405_MP_MsgPacking is an example of an OMM NiProvider application 
written to the EMA library.

This application demonstrates the basic usage Packed Messages, encoding multiple
messages into a single PackedMsgs object that is submitted on the OMM NiProvider.

Detailed Description
====================

ex405_MP_MsgPacking implements the following high-level steps:

+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
  - sends down the source directory refresh message with NI_PUB service info
+ Creates streaming items (refresh and updates) and publishes them
  - MarketPrice IBM.N item on the NI_PUB service
  - Multiple updates are packed together in a single PackedMsg to be published together.
+ Publishes 10 packed updates per second for 60 seconds.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
