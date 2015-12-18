Summary
=======


The `110__MarketByOrder__Subscription__Anytime__Refresh` is an example of an OMM Consumer
application written to the EOA Domain Layer library. It demonstrates the basic usage
of the EOA Doamin Layer library in accessing and parsing of OMM MarketByOrder data
received either from Reuters Data Feed Direct (RDF-D), directly from an OMM Provider
application, or from a Thomson Reuters Advanced Distribution Server.

The `110__MarketByOrder__Subscription__Anytime__Refresh` illustrates how to open a single
streaming MarketByOrder item, refresh content of this item's cache and print it out to a screen.

Detailed Description
====================

The `110__MarketByOrder__Subscription__Anytime__Refresh` implements the following high-level
steps:

+ opens up a MarketByOrder DIRECT_FEED service
+ connects to a server at localhost:14002 (by default)
+ requests a MarketByOrder streaming item from the service
+ for the next 60 seconds, application refreshes content of the item's cache and prints it out
+ Exits

**Note**: If needed, these and other details may be modified to fit your local  environment. For details on standard configuration, refer to the EMA library ReadMe.txt file and the EMA Configuration Guide.
