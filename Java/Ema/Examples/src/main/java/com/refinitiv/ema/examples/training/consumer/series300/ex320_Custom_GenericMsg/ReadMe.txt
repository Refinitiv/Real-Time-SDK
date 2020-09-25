Summary
=======

The ex320__Consumer__GenericMsg application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM Custom data from Reuters Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from an Advanced Distribution Server.

ex320_Custom_GenericMsg showcases usage of GenericMsg in OMM Consumer
application. It demonstrates submission of a GenericMsg as well as receiving
and processing it.


Detailed Description
====================

ex320_Custom_GenericMsg implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods (e.g. onGenericMsg() )
  - provides own methods as needed, e.g. decode( FieldList )
  - each of the methods provided in this example use the ease of use
	data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets operationModel to OperationModel.USER_DISPATCH
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a streaming item interest
  - Custom Domain IBM.XYZ item from DIRECT_FEED service
+ Sends a GenericMsg with ElementList as payload on the item stream	
+ Processes data received on user thread for 60 seconds
  - all received messages are processed on main thread of control
  - application may receive RespMsg as well as GenericMsg
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
