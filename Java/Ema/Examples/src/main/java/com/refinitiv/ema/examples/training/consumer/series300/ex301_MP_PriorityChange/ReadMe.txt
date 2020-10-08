Summary
=======

The ex301_MP_PriorityChange application is provided as an example
of OMM Consumer application written to the EMA library showcasing a change
to the item priority.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketPrice data from Refinitiv Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced Distribution Server.

ex301_MP_PriorityChange showcases reissue for priority feature
of OMM Consumer. This application arbitrarily chooses to modify item's priority
after receiving an open ok status on this item's refresh.


Detailed Description
====================

ex301_MP_PriorityChange implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets operationModel to UserDispatchEnum
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - modifies item's priority after receiving the first open ok item refresh
  - all received messages are processed on main thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file..
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
