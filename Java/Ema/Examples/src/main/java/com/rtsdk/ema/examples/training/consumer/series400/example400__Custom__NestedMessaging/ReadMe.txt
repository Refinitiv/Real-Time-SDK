Summary
=======

The example400__Custom__NestedMessaging application is provided as an example of
OMM Consumer application written to the EMA library.

This application demonstrates basic usage of the EMA library while using
nested messaging feature and parsing of custom OMM Data data from an OMM
Provider application, or from an Advanced Distribution Server.

example400__Custom__NestedMessaging opens up a parent stream on which, when 
this stream is open / ok, it opens a sub stream. This is also known as nested
messaging feature.

Note: effective use of the nested messaging feature requires support from an
	  OMM Provider application.


Detailed Description
====================

example400__Custom__NestedMessaging implements the following high level steps:

+ Implements OmmConsumerClient class in AppClient
  - overrides desired methods
  - provides own methods as needed, e.g. decode( FieldList )
  - each of the methods provided in this example use the ease of use
	data extraction methods that are data type specific
+ Instantiates AppClient object that receives and processes item messages
+ Instantiates and modifies OmmConsumerConfig object
  - sets user name to "user"
+ Instantiates an OmmConsumer object which initializes the connection 
  and logs into the specified server.
+ Opens a private streaming item interest
  - Custom Domain (200) IBM.XYZ item from DIRECT_FEED service
  - when this item becomes open / ok this application requests
    a sub stream using a GenericMsg submitted on the handle of the 
	IBM.XYZ stream
+ Processes data received from API for 60 seconds
  - all received messages are processed on API thread of control
+ Exits

Note: if needed, these and other details may be modified to fit local
      environment using EmaConfig.xml file.
