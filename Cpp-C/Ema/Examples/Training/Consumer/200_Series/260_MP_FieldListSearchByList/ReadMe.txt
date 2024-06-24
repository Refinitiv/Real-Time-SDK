Summary
=======

The 260_MP_FieldListSearchByList application is provided as an
example of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and searching with a list of field IDs of OMM MarketPrice data from LSEG
Data Feed Direct, directly from an OMM Provider application, or from
Advanced Distribution Server.


Detailed Description
====================

The 260_MP_FieldListSearchByList implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - the decode( const FieldList& ) specifies a list of field ids it is interested
	  in and passes it to the forth() method. When a matching field is found in the
	  given FieldList, FieldEntry reference is extracted and its content printed out
	  to screen
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
