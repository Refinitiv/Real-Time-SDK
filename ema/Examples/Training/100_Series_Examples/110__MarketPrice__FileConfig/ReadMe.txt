Summary
=======

The 110__MarketPrice__FileConfig application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library to access
and parse OMM MarketPrice data from Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from Thomson Reuters Advanced
Distribution Server.

The 110__MarketPrice__FileConfig showcases implicit dependence of the EMA
OmmConsumer on the EmaConfig.xml file locates in the application working
directory. Specific connectivity of this application is specified in the config
file unlike all the other training examples.


Detailed Description
====================

The 110__MarketPrice__FileConfig implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
+ Instantiates AppClient object to receive and processe item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets consumer name to "Consumer_2"
  - loads configuration information for the specified consumer name
    from the EmaConfig.xml file present in its working folder
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - MarketPrice IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
