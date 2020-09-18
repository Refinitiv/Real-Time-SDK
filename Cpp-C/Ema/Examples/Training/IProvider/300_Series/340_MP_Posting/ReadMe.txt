Summary
=======

340__MP__Posting is an example of an OMM Interactive Provider
application written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM Posting data from a Consumer application.

340__MP__Posting illustrates how to handle onPost callback in
Interactive Provider. This application uses source directory configured in the
EmaConfig.xml file.


Detailed Description
====================

340__MP__Posting implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Set the operation model to use user dispatch
+ Instantiates an OmmProvider object which:
  - listens on the port from the EmaConfig.xml file 
+ Accepts a login request
  - Set the "SupportOMMPost" parameter to indicate Provider supports posting
+ Processes an item request for MarketByPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for 60 seconds.
+ Processes a Post Messages
 - Creates ACK message and sends it back to Consumer to confirm Post Message
   receiving
 - Prints an "off-stream" message if the post is on the login stream,
   else prints an "on-stream" message
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA
      library ReadMe.txt file and EMA Configuration Guide.
