Summary
=======

The 420_MP_AdmDomainCfg application is provided as
an example of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library while specifying
Administrative domain messages to override default EMA behaviour, for accessing
and parsing of OMM MarketPrice data from Refinitiv Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced Distribution Server.

420_MP_AdmDomainCfg showcases customization of the default EMA
behaviour with respect to the administrative domains. EMA defaults login request
as well as directory and dictionary request. Using the addAdminMsg() method, users
are able to modify the EMA to suite their needs and preferences.


Detailed Description
====================

420_MP_AdmDomainCfg implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets request message for Login domain to override default
  - sets request message for Directory domain to override default
  - sets request message for Dictionary domain to override default
  - sets operationModel to OperationModel.USER_DISPATCH
+ Instantiates OmmConsumer object which initializes connection and logins into
  the specified server
+ Opens a streaming item interest
  - MarketPrice Domain IBM.N item from DIRECT_FEED service
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
