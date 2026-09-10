Summary
=======

ex405_MP_MsgPacking is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage Packed Messages, encoding multiple
messages into a single PackedMsgs object that is submitted on the OMM Provider.

Detailed Description
====================

ex405_MP_MsgPacking implements the following high-level steps:

+ Instantiates an OmmProvider object
+ Accepts a login request
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh) and publishes it.
 - Creates PackedMsg containing multiple update messages and publishes it.
 - Publishes 10 packed updates per second for 60 seconds.
+ Rejects subsequent item requests.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
