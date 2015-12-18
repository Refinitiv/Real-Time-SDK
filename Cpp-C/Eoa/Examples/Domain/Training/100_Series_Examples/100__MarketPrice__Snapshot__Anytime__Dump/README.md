Summary
=======


The `100__MarketPrice__Snapshot__Anytime__Dump` is an example of an OMM Consumer
application written to the EOA Domain Layer library. It demonstrates the basic usage
of the EOA Doamin Layer library in accessing and parsing of OMM MarketPrice data
received either from Reuters Data Feed Direct (RDF-D), directly from an OMM Provider
application, or from a Thomson Reuters Advanced Distribution Server.

The `100__MarketPrice__Snapshot__Anytime__Dump` illustrates how to open a single
snapshot MarketPrice item. After the refresh complete is received, the application
prints the cached content of the entire item to the screen.

Detailed Description
====================

The `100__MarketPrice__Snapshot__Anytime__Dump` implements the following high-level
steps:

+ opens up a MarketPrice DIRECT_FEED service
+ connects to a server at localhost:14002 (by default)
+ requests a MarketPrice item snapshot from the service
+ when refresh complete is received application prints out content of the entire item from its cache
+ Exits

**Note**: If needed, these and other details may be modified to fit your local environment. For details on standard configuration, refer to the EMA library ReadMe.txt file and the EMA Configuration Guide.
