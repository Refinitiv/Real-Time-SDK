Summary
=======

ex421_MP_ProgrammaticCfg is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to a Consumer application.

ex421_MP_ProgrammaticCfg illustrates how to create and publish a single OMM
streaming item. The example showcases creation and usage of the 
programmatic configuration feature of EMA. In addition to file configuration,
EMA allows users to programmatically configure Omm Provider instances.


Detailed Description
====================

ex421_MP_ProgrammaticCfg implements the following high-level steps:

+ Instantiates a Map (configMap object) and populates it with configuration info
+ Instantiates and modifies an OmmIProviderConfig object:
  - sets Omm Provider configuration with data from the programmatic configuration
  - Set the operation model to use user dispatch
+ Instantiates an OmmProvider object with programmatic configuration
+ Accepts a login request
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for 60 seconds.
+ Rejects subsequent item requests.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
