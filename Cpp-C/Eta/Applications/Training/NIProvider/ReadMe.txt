Transport API Non-Interactive Provider (NIP) Training Application Description

--------
Summary:
--------

A Non-Interactive Provider (NIP) writes a provider application that 
connects to LSEG Real-Time Distribution and sends a specific set
(non-interactive) of information (services, domains, and capabilities).
NIPs act like clients in a client-server relationship. Multiple NIPs can
connect to the same LSEG Real-Time Distribution and publish the same
items and content. The NIP application sends a login request like a consumer
to the ADH and processes source directory and item requests like a provider.
A source directory refresh message and market price and/or market by order
refresh/update messages are published to the ADH without any request for them.

This Transport API Non-Interactive Provider (NIP) Training application provides 
an outline of how to create an OMM NIP application which can establish
a connection to an ADH server. Once connected, an OMM NIP can publish 
information into the ADH cache without needing to handle requests for 
the information. The ADH can cache the information and along with other 
LSEG Real-Time Distribution System components, provide the information 
to any NIProvider applications that indicate interest.

The general process can be summarized by the following steps.

- Establish network communication
- Perform Login process
- Provide Source Directory information
- Provide content
- Log out and shut down

The NIProvider training example application, included in Training folder, 
provides one implementation of an OMM NIP application written with simplicity 
in mind and demonstrates the use of the Transport API. It consists of several modules that
show incremental progress of how to build the app from the ground up.

For NIProvider training example application, you can use any File Comparison
tools to compare between any two consecutive modules so that you will know the 
specific incremental code for a particular module, which is the difference 
between 2 consecutive modules.

You can also compare each module of the NIP code with the same module name (if
it exists) of another training app, such as the Consumer to find commonality 
between NIProvider and Consumer training applications, if you wish.

Content is encoded and decoded using the Transport API Message Package and the Transport API 
Data Package.  

The Modules available for NIProvider training example application are listed
here. 

- Transport API NIProvider Training Module 1a: Establish network communication
- Transport API NIProvider Training Module 1b: Ping (heartbeat) Management
- Transport API NIProvider Training Module 1c: Reading and Writing Data
- Transport API NIProvider Training Module 2: Log in
- Transport API NIProvider Training Module 3: Provide Source Directory Information
- Transport API NIProvider Training Module 4: Load Dictionary Information
- Transport API NIProvider Training Module 5: Provide Content

Please go to each individual module folder for more details. Please note that each 
module has its own makefiles and application name tailored for that module. 
Please follow the instructions in readmes for each module for information on setup 
environment and ommand line usage to know how to run the executable and its available 
set of parameters for each different modules.

-----------------
Compiling Source:
-----------------

The makefile is included for each module. For each module 
folder, the included makefile is set up to run from the file
locations as presented through the distribution package.
It is set up for building on the Transport API supported 
Linux platforms using the supported compilers.

The LINKTYPE value in the makefile is used to control
whether the application is built using Transport API static or 
shared libraries. The default build uses Transport API static 
libraries. To use Transport API shared libraries, 
set LINKTYPE=Shared.

To compile, run the gmake command.

Gmake can be obtained at http://www.gnu.org/software/make/

For windows platform, using Visual Studio, open one of the included vcxproj project
files and build.

----------------
Example Content:
----------------

Included for each module of this application are:

- Source files for each module.

- makefile for each module.

- readme for each module.

--------------------
Detailed Description
--------------------

NIProvider_Training.c - The main file for each module for the Transport API Non-Interactive 
							Provider (NIP) Training application.

