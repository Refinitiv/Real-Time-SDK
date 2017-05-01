Summary
=======

161__Login__AcceptDeny__DomainRepresentation is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Thomson Reuters Advanced Distribution Hub.

161__Login__AcceptDeny__DomainRepresentation illustrates how to create and publish a single OMM
streaming item. This application uses hardcoded source directory configuration.

This application is the Domain Representation version of example 160__Login__AcceptDeny. It demonstrates the
ease-of-use Domain Representation functionality.

Detailed Description
====================

161__Login__AcceptDeny__DomainRepresentation implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Sets the listening  port to "14002"
+ Instantiates an OmmProvider object which:
  - listens on the above port
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
