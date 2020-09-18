Summary
=======

The 220_MBP_PrivateStream application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketByPrice data from Refinitiv Data Feed Direct,
directly from an OMM Provider application, or from the Advanced
Distribution Server.

The 220_MBP_PrivateStream showcases opening and processing of
private stream MarketByPrice item. The item is requested as private by
setting parivateStream( true ) on the ReqMsg.


Detailed Description
====================

The 220_MBP_PrivateStream implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - the decode( const FieldList& ) iterates through the received FieldList,
	  extracts each FieldEntry reference from the current position on the
	  FieldList, and extracts field id, name and value of the current FieldEntry
	- the decode( const Map& ) iterates through the received Map, extracts Summary
	  if it is present, extracts MapEntry from which it extracts key, and load values
	- both methods use native / RWF data format while extracting data
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
  - sets operationModel to UserDispatchEnum
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens private streaming item interest
  - MarketByPrice BBH.ITC item from DIRECT_FEED service
  - uses privateStream() interface to mark this item as private
+ Processes data received with user dispatch loop for 60 seconds
  - all received messages are processed on main application thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
