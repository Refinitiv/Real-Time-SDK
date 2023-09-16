Summary
=======

The 350_MP_PauseAll application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for opening
login and item streams and pausing item response messages from Refinitiv Data
Feed Direct (RDF-D), directly from an OMM Provider application, or from
an Advanced Distribution Server with login stream.

350_MP_PauseAll showcases the pause all feature of OMM Consumer.
It sends a single pause request on login stream to pause all open items.


Detailed Description
====================

350_MP_PauseAll implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens three streaming item interests
  - Login Domain stream
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
  - MarketPrice Domain MSFT.N item from DIRECT_FEED service
+ Sends a PauseAll request message on login stream
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
