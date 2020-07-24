Summary
=======

201__MarketPrice__TunnelingConnection is an example of an OMM NiProvider application 
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

201__MarketPrice__TunnelingConnection illustrates the ability of the user
to programmatically pass all http/https related configuration on OmmNiProviderConfig
instance when configuring a tunneling connection. When running this application,
the user will need specify valid tunneling configurations through commandline arguments.


Detailed Description
====================

201__MarketPrice__TunnelingConnection implements the following high-level steps:

+ Passes tunneling related configuration through commandline arguments
including:
-ph Proxy host name
-pp Proxy port number
-spTLSv1.2 enable use of cryptographic protocol TLSv1.2 used with linux encrypted connections
-libsslName name of the libssl.so shared library
-libcryptoName name of the libcrypto.so shared library
+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
    - specified SourceDirectory of TEST_NI_PUB
	- specified Channel's host and port
  - Sets the username to "user"
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Creates streaming items (refresh and updates) and publishes them
  - MarketPrice IBM.N item on the TEST_NI_PUB service
  - MarketPrice TRI.N item on the TEST_NI_PUB service
+ Publishes updates 1 per second for 60 seconds.
+ Exits

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
