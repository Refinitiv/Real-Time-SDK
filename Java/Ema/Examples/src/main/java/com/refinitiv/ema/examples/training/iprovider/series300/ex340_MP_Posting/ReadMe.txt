Summary
=======

ex340_MP_Posting is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates how to implement a provider that supports post messages.

ex340_MP_Posting illustrates how to indicate that provider supports post messages and how
to reply with AckMsg when post message is received.

Detailed Description
====================

ex340_MP_Posting implements the following high-level steps:

+ Implements OmmProviderClient class in AppClient
  - overrides desired methods
+ Instantiates AppClient object that receives and processes post messages
+ Instantiates OmmIProviderConfig object.
+ Instantiates an OmmProvider object which:
  - listens on the port 14002
+ Accepts a login request from ADH and replies with SupportOMMPost attribute set to 1
+ Processes item requests for MarketPrice domain by replying with refresh message that contains hardcoded values.
+ Processes post messages by replying with ack message
+ Runs for 120 seconds
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
