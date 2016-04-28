General Overview of Message Object API Examples
===============================================

This folder contains a set of example applications written to the Thomson Reuters
Elektron Message API (EMA). These applications present simple usage of the EMA
library exposing its various features and showcasing various levels of application
simplicity and functionality. These applications and their content are intended
to provide a self learning environment. The examples are structured such a way that
the simplest to write and understand examples reside in the 100_Series_Examples
followed by the 200_Series_Examples and higher numbered folders that present more
advanced concepts of the EMA and OMM. Each application is associated with a number.
While the general intent is to uniquely identify each and every application, within
a folder the greater number may be associated with growing advancement and complexity.


Note: All presented example applications may be easily modified and configured
      to fit local needs and environment. The configuration is specified using
      EmaConfig.xml file.      
      

Example Naming Convention
==========================

The EMA training examples follow a general naming convention specified as follows:
<#>__<domain>__<main functionality>

- <#>       represents example number
- <domain>  represents message domain type
- <main showcased functionality>

In general the 100 Series examples present message output converted to string.
The 200 Series and above examples present message output in RWF or native format.
Different individual EMA and OMM functionalities are depicted in respectively named
examples.


Building Examples
=================
Each example folder contains its own Makefile/MSBuild Project Files.

Makefile:
	Use gmake to build with the provided makefile.
	To build using the EMA Static library, set USE_STATIC = 1.

MSBuild Project file:
	Open MSBuild Project file and build the project. Project files exist
        for supported Visual Studio versions for shared and static libraries.

The Makefile/MSBuild Project file will link with EMA library in a directory named

<package directory>/EMA/Libs/<platform_bits_compiler>

The static library will be one level deeper in /Static.

Building Examples With Custom Build
===================================
If libema.lib is built, the build environment for the examples must
be changed to use the custom-built libema.lib.

Makefile:
	The command 'make help' provides details.

MSBuild Project file:
	Open the property manager for the example and open the
        properties for the appropriate build environment (e.g., Debug MDd).
	There should be an entry titled 'customBuild'. Open that entry,
	select 'User Macros', and change USE_CUSTOM_BUILD to 'true'.
	
	Note that the USE_CUSTOM_BUILD setting is used for all environments.

	To revert to the libema.lib shipped with the load, follow the
	procedure above and set USE_CUSTOM_BUILD to false.

Running Examples
=================
To run any example, any OMM provider application/component is needed to source
the data from. Any OMM based provider (UPA/RFA) or (ADS/EED) may be used.
