Summary
=======


The `160__MarketMaker__Subscription__Callback__FieldString` is an example of an OMM Consumer
application written to the EOA Domain Layer library. It demonstrates the basic usage
of the EOA Doamin Layer library in accessing and parsing of OMM MarketMaker data
received either from Reuters Data Feed Direct (RDF-D), directly from an OMM Provider
application, or from a Thomson Reuters Advanced Distribution Server.

The `160__MarketMaker__Subscription__Callback__FieldString` illustrates how to open a single
streaming MarketMaker item. Application passes a callback client used to process data of the item.

Detailed Description
====================

The `160__MarketMaker__Subscription__Callback__FieldString` implements the following high-level
steps:

+ opens up a MarketMaker DIRECT_FEED service
+ connects to a server at localhost:14002 (by default)
+ requests a MarketMaker streaming item from the service and passes a callback client
+ when a network event occurs, EOA calls the respective methods of the callback client to process the received information
+ methods of the callback client are executed on the API thread of control
+ Exits after 60 seconds

**Note**: If needed, these and other details may be modified to fit your local environment. For details on standard configuration, refer to the EMA library ReadMe.txt file and the EMA Configuration Guide.
