Summary
=======

380_MP_ConnectionLost is an example of an OMM Interactive Provider application
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing of
OMM MarketPrice data to the Advanced Distribution Hub.

380_MP_ConnectionLost illustrates how to create and publish a single OMM
streaming item. This application uses source directory configured in the
EmaConfig.xml file.


Detailed Description
====================

380_MP_ConnectionLost implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object.
+ Instantiates an OmmProvider object which.
  - listens on the port from the EmaConfig.xml file
+ Accepts an login request.
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second.
+ Rejects subsequent item requests.
+ Processes a login close request after the channel is disconnected.
 - reset item handle to 0
 - stop publishing updates after receiving the login close request.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA
      library ReadMe.txt file and EMA Configuration Guide.
