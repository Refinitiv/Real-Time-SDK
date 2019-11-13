Summary
=======

350__Dictionary__Streaming is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates how to implement a provider to download dictionary 
and provide OMM MarketPrice data to Advanced Distribution Hub.

350__Dictionary__Streaming illustrates how to register for the dictionary domain, 
create and publish one OMM streaming item then process received dictionary messages.
This application uses source directory configured in the EmaConfig.xml file.

The 350__Dictionary__Streaming provides an command line argument as follows:

-dumpDictionary 		Dump data dictionary information to the console if specified. 

Detailed Description
====================

350__Dictionary__Streaming implements the following high-level steps:

+ Implements OmmProviderClient class in AppClient
  - overrides desired methods
+ Instantiates AppClient object that receives and processes dictionary messages
+ Instantiates OmmIProviderConfig object.
+ Instantiates an OmmProvider object which:
  - The source directory is defined from the EmaConfig.xml file
  - listens on the port from the EmaConfig.xml file
+ Accepts a login request from ADH
+ Registers for dictionary stream
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for total 60 updates.
+ Rejects subsequent item requests.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
