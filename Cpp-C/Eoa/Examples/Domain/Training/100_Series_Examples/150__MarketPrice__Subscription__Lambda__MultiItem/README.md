Summary
=======


The `150__MarketPrice__Subscription__Lambda__MultiItem` is an example of an OMM Consumer
application written to the EOA Domain Layer library. It demonstrates the basic usage
of the EOA Doamin Layer library in accessing and parsing of OMM MarketPrice data
received either from Reuters Data Feed Direct (RDF-D), directly from an OMM Provider
application, or from a Thomson Reuters Advanced Distribution Server.

The `150__MarketPrice__Subscription__Lambda__MultiItem` illustrates how to open multiple
streaming MarketPrice items. Application passes lambda expressions used to process data of the items.

Detailed Description
====================

The `150__MarketPrice__Subscription__Lambda__MultiItem` implements the following high-level
steps:

+ opens up a MarketPrice DIRECT_FEED service
+ connects to a server at localhost:14002 (by default)
+ requests two MarketPrice streaming items from the service and passes respective lambda expressions
+ when a network event occurs, EOA calls the respective lambda expression to process the received information
+ lambda expressions are executed on the API thread of control
+ Exits after 60 seconds

**Note**: If needed, these and other details may be modified to fit your local environment. For details on standard configuration, refer to the EMA library ReadMe.txt file and the EMA Configuration Guide.
