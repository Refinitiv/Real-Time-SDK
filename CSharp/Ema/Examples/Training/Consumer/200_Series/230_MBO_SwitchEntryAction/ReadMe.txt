Summary
=======

The 230_MBO_SwitchEntryAction application is provided as an
example of OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library for accessing
and parsing of OMM MarketByOrder data from Refinitiv Data Feed Direct (RDF-D),
directly from an OMM Provider application, or from the Advanced
Distribution Server.

230_MBO_SwitchEntryAction showcases application varying processing
of Map Entry based on this Map Entry's Action value (see decode( Map ))
for details.


Detailed Description
====================

230_MBO_SwitchEntryAction implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
    - the method decode( FieldList ) iterates through the received FieldList,
	  extracts each FieldEntry reference from the current position on the
	  FieldList, and extracts field id, name and value of the current FieldEntry
	- the method decode( Map ) iterates through the received Map, extracts Summary
	  if it is present, extracts MapEntry from which it extracts key, and load values
	- both methods use native / RWF data format while extracting data
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
  - sets host name on the preconfigured connection to "localhost"
  - sets port on the preconfigured connection to "14002"
  - sets operationModel to UserDisptchEnum
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens streaming item interest
  - MarketByOrder Domain AAO.V item from DIRECT_FEED service
+ Processes data received with user dispatch loop for 60 seconds
  - all received messages are processed on main application thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
	  
Note: please refer to the EMA library ReadMe.txt file for details on
      standard configuration.
