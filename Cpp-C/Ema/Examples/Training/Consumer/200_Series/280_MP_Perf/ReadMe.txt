Summary
=======

The 280_MP_Perf application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketPrice data from Data Feed Direct,
directly from an OMM Provider application, or from the Advanced
Distribution Server.

The 280_MP_Perf is a simple performance focused application
opening and processing a number of MarketPrice items.

Note: to effectively open up items in this example, please connect it up to 
	  custom build provider performance application, e.g. ProvPerf.

Detailed Description
====================

The 280_MP_Perf implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( const FieldList& )
    - the decode( const FieldList& ) is a performance focused method
	  extracting the minimum needed for an application and not performing any
	  time and cpu consuming application specific functionalities (e.g., printing)
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a number of streaming item interests
  - MarketPrice RTR####.N items from DIRECT_FEED service (#### - a sequence number)
+ Processes data received from API for 300 seconds
  - all received messages are processed on API thread of control
  - prints out
    - total number of refreshes
    - total number of status messages
    - number of updates received in a second

+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
