Summary
=======

160__Login__AcceptDeny is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

160__Login__AcceptDeny illustrates how to create and publish a single OMM
streaming item. This application uses hardcoded source directory configuration.


Detailed Description
====================

160__Login__AcceptDeny implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Sets the listening  port to "14002"
+ Instantiates an OmmProvider object which:
  - listens on the above port
+ Checks if login request nameType is USER_NAME.
  - If yes, it accepts request using OMM constructs
  - Otherwise, rejects request
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
