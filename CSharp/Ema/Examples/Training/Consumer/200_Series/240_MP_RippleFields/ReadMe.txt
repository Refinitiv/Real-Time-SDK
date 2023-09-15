Summary
=======

The 240_MP_RippleFields application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and handling ripple fields of OMM MarketPrice data from Refinitiv Data Feed
Direct (RDF-D), directly from an OMM Provider application, or from 
an Advanced Distribution Server.


Detailed Description
====================

240_MP_RippleFields implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
    - the method decode( FieldList ) iterates through the received FieldList,
	  extracts each FieldEntry reference from the current position on the
	  FieldList, and extracts field id, name and value of the current FieldEntry.
	  In addition to that, it checks the ripple to fields and performs the rippling
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received with user dispatch loop for 60 seconds
  - all received messages are processed on main application thread
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
