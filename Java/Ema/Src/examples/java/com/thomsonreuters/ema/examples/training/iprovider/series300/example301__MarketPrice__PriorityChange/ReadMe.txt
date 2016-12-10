Summary
=======

301__MarketPrice__PriorityChange is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Thomson Reuters Advanced Distribution Hub.

301__MarketPrice__PriorityChange illustrates how to create and publish a single OMM
streaming item. This application uses source directory configured in the EmaConfig.xml
file.


Detailed Description
====================

301__MarketPrice__PriorityChange implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object.
 - Set the operation model to use user dispatch
+ Instantiates an OmmProvider object which:
  - listens on the port from the EmaConfig.xml file
+ Accepts a login request
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for total 60 updates
 - Updates are published with a user dispatch loop
+ Rejects subsequent item requests.
+ Receive the item reissue request with priority change.
 - store and print priority information to console.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
