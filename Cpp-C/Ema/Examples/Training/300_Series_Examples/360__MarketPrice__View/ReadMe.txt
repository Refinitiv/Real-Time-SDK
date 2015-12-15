Summary
=======

The 360__MarketPrice__View application is provided as an example of OMM Consumer
application written to the EMA library.

This application demonstrates basic usage of the EMA library for opening item
stream with a list of field IDs called "View" and parsing of OMM MarketPrice
data from Reuters Data Feed Direct (RDF-D), directly from an OMM Provider
application, or from Thomson Reuters Advanced Distribution Server.

The 360__MarketPrice__View showcases usage of view request feature supported
by OMM Consumer and server.


Detailed Description
====================

The 360__MarketPrice__View implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - each of the method provided in this example use the ease of use
	  data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens one streaming item interest
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
    - the view definition is added to the request with the payload() method
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
