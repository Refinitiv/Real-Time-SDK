Summary
=======

The ex370_MP_Batch application is provided as an example of OMM
Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library while opening
multiple item stream via a single request (a.k.a., batch request) and parsing
of OMM MarketPrice data from Data Feed Direct (LDFD), directly from
an OMM Provider application, or from an Advanced Distribution Server.

ex370_MP_Batch showcases usage of batch request feature of OMM
Consumer.

Detailed Description
====================

ex370_MP_Batch implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a batch of streaming item interests
  - MarketPrice Domain batch request from DIRECT_FEED service
  - the batch (a list of item names) definition is added to the request
	using the payload() method
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
