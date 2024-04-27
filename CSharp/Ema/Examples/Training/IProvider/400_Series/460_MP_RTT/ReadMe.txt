Summary
=======

460_MP_RTT is an example of an OMM Interactive Provider application written to
the EMA library.

This application demonstrates the basic usage of the EMA library in providing of
OMM MarketPrice data to the Advanced Distribution Hub.

460_MP_RTT illustrates how to send Round Trip Latency requests to consumers
supporting the feature and process the responses. This application uses
hardcoded source directory configuration.


Detailed Description
====================

460_MP_RTT implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Sets the listening  port to "14002"
+ Instantiates an OmmProvider object which:
  - listens on the above port
+ Accepts a login request
+ Processes item requests for MarketPrice domain.
 - Creates streaming items (refresh and updates) and publishes them
 - Publishes updates to all subscribed consumers 1 time per second up to 10
   minutes starting from the first item request
+ Gathers RTT statistics from consumers supporting the feature
 - determines whether the consumer supports RTT stats on the basis of the
   contents of the consumer's initial login request
 - sends RTT request that carries current Ticks and previous Round Trip Latency
   (if available) 1 per second
 - processes Consumer's responses on RTT requests

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA
      library ReadMe.txt file and EMA Configuration Guide.
