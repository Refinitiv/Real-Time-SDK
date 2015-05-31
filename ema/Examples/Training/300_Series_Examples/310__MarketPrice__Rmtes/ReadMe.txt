Summary
=======

The 310__MarketPrice__Rmtes application is provided as an example of OMM Consumer
application written to the EMA library showcasing RWF RMTES_STRING processing.

This application demonstrates basic usage of the EMA library for accessing and
parsing of OMM MarketPrice data from Reuters Data Feed Direct (RDF-D), directly
from an OMM Provider application, or from Thomson Reuters Advanced DistributionServer.

The 310__MarketPrice__Rmtes showcases processing of RWF RMTES_STRING data
type.

 
Detailed Description
====================

The 310__MarketPrice__Rmtes implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - each of the method provided in this example use the ease of use
	  data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice N2_UBMS item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - closes the item stream after a number of received update messages
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file..
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
