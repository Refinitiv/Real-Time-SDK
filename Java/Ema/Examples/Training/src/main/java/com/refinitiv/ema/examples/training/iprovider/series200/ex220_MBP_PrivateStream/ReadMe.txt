Summary
=======

220_MBP_PrivateStream is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketByPrice data to a Consumer application.

220_MBP_PrivateStream illustrates how to create and publish multiple OMM
streaming items with Private Stream capabilities as requested per item. 
This application uses hardcoded source directory configuration.


Detailed Description
====================

220_MBP_PrivateStream implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Sets the listening port to "14002"
+ Instantiates an OmmProvider object which:
  - listens on the above port
+ Accepts a login request
+ Processes item requests for MarketByPrice domain.
 - Refresh for item includes state of private stream.
 - Creates streaming item (refresh and updates) and publishes them.
 - Publishes updates 1 per second for 60 seconds.
+ Subsequent item requests are also responded to.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
