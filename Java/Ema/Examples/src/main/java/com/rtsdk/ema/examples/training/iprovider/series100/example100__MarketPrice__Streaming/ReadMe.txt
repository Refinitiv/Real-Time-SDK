Summary
=======

100__MarketPrice__Streaming is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

100__MarketPrice__Streaming illustrates how to create and publish a single OMM
streaming item. This application uses hardcoded source directory configuration.


Detailed Description
====================

100__MarketPrice__Streaming implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Sets the listening  port to "14002"
+ Instantiates an OmmProvider object which:
  - listens on the above port
+ Accepts a login request
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for 60 seconds.
+ Rejects subsequent item requests.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
