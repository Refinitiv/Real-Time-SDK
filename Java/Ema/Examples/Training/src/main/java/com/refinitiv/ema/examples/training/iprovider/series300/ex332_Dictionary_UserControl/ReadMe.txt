Summary
=======

ex332_Dictionary_UserControl is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

ex332_Dictionary_UserControl illustrates how to create and publish a single OMM
streaming item. This application will push dictionary refresh message.

The RDMFieldDictionary and enumtype.def files must exists in the working directory.

Detailed Description
====================

ex332_Dictionary_UserControl implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object.
  - Set the admin control to use user control for dictionary
+ Instantiates an OmmProvider object which:
  - The source directory is defined from the EmaConfig.xml file
  - listens on the port from the EmaConfig.xml file
+ Accepts a login request
+ Processes a dictionary request.
 - Publishes dictionary refresh messages
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for total 60 updates.
+ Rejects subsequent item requests.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
