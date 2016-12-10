Summary
=======

280__MarketPrice__Performance is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the performance usage of the EMA library in providing
of OMM MarketPrice data a Consumer application.

280__MarketPrice__Performance illustrates how to efficiently create and publish a number
of OMM streaming items. This application uses source directory configured in the EmaConfig.xml
file.


Detailed Description
====================

280__MarketPrice__Performance implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - which reads in the EmaConfig.xml file with:
    - specified SourceDirectory of DIRECT_FEED
+ Instantiates an OmmProvider object which:
  - listens on a port from the EmaConfig.xml file
+ Accepts a login request
  - sends down the source directory refresh message with DIRECT_FEED service info
+ Processes a number of item requests.
 - Creates a number of streaming items (refresh and updates) and publishes them
 - Publishes a number of updates for 300 seconds.
+ Prints out approximate rates of refreshes and updates every second or so
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
