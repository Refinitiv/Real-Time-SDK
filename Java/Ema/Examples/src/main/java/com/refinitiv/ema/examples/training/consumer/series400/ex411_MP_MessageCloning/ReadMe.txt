Summary
=======

The ex411_MP_MessageCloning application is provided as an example
of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketPrice data from Data Feed Direct (LDFD),
directly from an OMM Provider application, or from an Advanced Distribution Server.

ex411_MP_MessageCloning demonstrates how to copy messages from EMA's callback methods
and add to a message queue to be processed by another thread later. This would help to
minimize processing time in the callback methods in order to read more messages from 
the channel.


Detailed Description
====================

ex411_MP_MessageCloning implements the following high level steps:

+ Implements the OmmConsumerClient class in the AppClient:
  - Overrides desired methods
+ Additional implementation for cloning messages:
  - Creates message pools for reusing RefreshMsg, UpdateMsg and StatusMsg for
    cloning messages from the callback methods.
  - Creates a message queue to keep cloned messages to be processed by another thread.
  - Creates a thread to decode data from the message queue separately.
+ Instantiates an AppClient object to receive and process item messages by specifying UpdateMsg pool and UpdateMsg's buffer size
+ Instantiates and modifies an OmmConsumerConfig object:
  - Sets the username to "user"
  - Sets the hostname on the preconfigured connection to "localhost"
  - Sets the port on the preconfigured connection to "14002"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - MarketPrice LSEG.L item from the DIRECT_FEED service
+ Processes data received from the API for 60 seconds:
  - Received messages on the API thread of control
  - Processed messages on the application thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
