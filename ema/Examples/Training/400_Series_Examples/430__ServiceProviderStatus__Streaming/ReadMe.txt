Summary
=======

The 430__ServiceProviderStatus__Streaming application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for opening
a Service Provider Status (SPS) item for monitoring the health of the Elektron
distribution path.


Detailed Description
====================

This application implements the following high level steps:
- implements OmmConsumerClient class in AppClient
- instantiates AppClient object that receives and processes item messages
- instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost" (user will
    have to change to provider address that supplies SPS domain)
  - sets port on the preconfigured connection to "14002"
- instantiates OmmConsumer object which
  - initializes connection and logs into specified server
- opens item interest
  - Top level SPS, .[SPSAMER (user will have to modify if not in AMER region)
  - Several provider level SPS
  - The subprovider SPS under each provider level
- processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
- exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.

