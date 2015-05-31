Summary
=======

The 121__MarketPrice__FieldListSearchByName application is provided as an
example of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library to access and
parse OMM MarketPrice data from Reuters Data Feed Direct (RDF-D), directly
from an OMM Provider application, or from Thomson Reuters Advanced Distribution
Server.

The 121__MarketPrice__FieldListSearchByName showcases selective extraction of
data from a field entry whose name is specified on the call to forth().


Detailed Description
====================

The 121__MarketPrice__FieldListSearchByName implements the following high
level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - the decode( const FieldList& ) iterates through the received FieldList
	  searching for a FieldEntry matching passed in name, if found extracts this 
	  FieldEntry reference from the current position on the FieldList,
	  and extracts field id, name and value of this FieldEntry
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server
+ Opens streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
  - decodes FieldList container by searching for a field named "BID"
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
