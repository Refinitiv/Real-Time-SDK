Summary
=======

The 102__MarketPrice__Snapshot is an example of an OMM Consumer application
written to the EMA library.

This application demonstrates basic usage of the EMA library to access and
parse OMM MarketPrice data either from Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from a Thomson Reuters Advanced
Distribution Server.

The 102__MarketPrice__Snapshot showcases opening of a single snapshot OMM item.
While processing received messages this application simply prints them out to the
screen.


Detailed Description
====================

The 102__MarketPrice__Snapshot implements the following high level steps:

+ Implements the OmmConsumerClient class in an AppClient
  - overrides desired methods
+ Instantiates an AppClient object to receive and process item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens snapshot item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
  - uses interestAfterRefresh( false ) to make it a snapshot request
+ Processes data received from API for 60 seconds
  - all received messages are processed on the API thread of control
+ exits

Note: if needed, these and other details may be modified to fit local
      environment.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
