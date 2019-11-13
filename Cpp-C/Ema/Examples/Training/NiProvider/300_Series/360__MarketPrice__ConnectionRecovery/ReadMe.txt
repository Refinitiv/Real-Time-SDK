Summary
=======

360__MarketPrice__ConnectionRecovery is an example of an OMM NiProvider application
written to the EMA library.

This application demonstrates the basic usage of the EMA library in providing
of OMM MarketPrice data to the Advanced Distribution Hub.

360__MarketPrice__ConnectionRecovery illustrates how to create and publish one OMM item,
and register for and process login events. The login events signify connection state.
For example, login status of Open / Suspect / None / "channel down" signifies a disconnect,
login status of Open / Ok / None / "channel up" signifies a successful reconnect.
This application uses source directory configured in the EmaConfig.xml file.


Detailed Description
====================

360__MarketPrice__ConnectionRecovery implements the following high-level steps:

+ Implements OmmProviderClient class in AppClient
  - overrides desired methods
  - the onStatusMsg() and onRefreshMsg() recognize the Open / Suspect and Open / Ok status
    as indication of the underlying connection state
+ Instantiates AppClient object that receives and processes login messages
+ Instantiates and modifies an OmmNiProviderConfig object:
  - which reads in the EmaConfig.xml file with:
    - specified SourceDirectory of TEST_NI_PUB
	- specified Channel's host and port
  - Sets the username to "user"
  - Sets the operationModel to UserDispatchEnum.
+ Instantiates an OmmProvider object which:
  - initializes the connection and logs into the configured ADH
  - sends down the source directory refresh message with TEST_NI_PUB service info
+ Registers for login stream
+ Creates streaming item (refresh and updates) and publishes it
  - MarketPrice TRI.N item on the TEST_NI_PUB service
+ Publishes updates 1 per second
  - items are not published if connection is down
+ Processes data received while calling OmmProvider::dispatch( 1000000 )
  - All messages received are processed on the application's main thread of control
+ Exits after 60 updates is sent out

Note: Please refer to the EMA Configuration Guide for details on usage of the following
      configuration parameters: 'RemoveItemsOnDisconnect' and 'RecoverUserSubmitSourceDirectory'.

Note: If needed, these and other details may be modified to fit your local
      environment. For details on standard configuration, refer to the EMA library
      ReadMe.txt file and EMA Configuration Guide.
