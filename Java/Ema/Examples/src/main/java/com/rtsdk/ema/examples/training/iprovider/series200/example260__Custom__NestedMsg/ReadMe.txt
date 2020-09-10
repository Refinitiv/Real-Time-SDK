Summary
=======

260__Custom__NestedMsg is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM nested message with custom domain type(200) defined by user

260__Custom__NestedMsg illustrates how to create and publish a single OMM
streaming item. This application uses source directory configured in the EmaConfig.xml
file.


Detailed Description
====================

260__Custom__NestedMsg implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Set the operation model to use user dispatch
+ Instantiates an OmmProvider object which:
  - listens on the port from the EmaConfig.xml file
+ Accepts a login request
+ Processes an item request for Custom domain.
 - Publishes a generic message with a refresh message to the requesting stream.
 - Publishes generic messages with update messages 1 per second for 60 seconds.
+ Rejects subsequent item requests.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
