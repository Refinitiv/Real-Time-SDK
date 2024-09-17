Summary
=======

201_MP_Encrypted is an example of an OMM Interactive Provider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to a Consumer application with an Encrypted connection.

201_MP_Encrypted illustrates how to create and publish a single OMM
streaming item. This application uses source directorythe EncryptedProvider configuration 
configured in the EmaConfig.xml file. 


Detailed Description
====================

Prerequisites: This application will need a jks encoded keystore that contains private key
infrastructure(PKI) information. This jks file will need to contain the server's 
certificate and private key.  For more information on how to generate this file, please 
refer to your PKI provider and Oracle's documentation.

201_MP_Encrypted implements the following high-level steps:

+ Instantiates and modifies an OmmIProviderConfig object:
  - Set the operation model to use user dispatch
  - Sets the keystore and key password (both must be the same) from a jks encoded keystore.
+ Instantiates an OmmProvider object which:
  - listens on the port from the EmaConfig.xml file
+ Accepts a login request
+ Processes an item request for MarketPrice domain.
 - Creates streaming item (refresh and updates) and publishes them
 - Publishes updates 1 per second for 60 seconds.
+ Rejects subsequent item requests.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
