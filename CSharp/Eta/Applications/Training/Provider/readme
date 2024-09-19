Transport API Interactive Provider Training Application Description

--------
Summary:
--------

An Interactive provider accepts incoming connections from LSEG Real-time
Distribution System(s) and/or Consumers.

An Interactive Provider receives connection requests and responds to requests
for information as to what services, domains, and capabilities it can provide
or for which it can receive requests. It may also receive and respond to
requests for information about its data dictionary, which describes the
format of expected data types. After this is completed, its behavior is
interactive. Interactive Providers act like Servers in a client-server
relationship. The Transport API Interactive Provider can accept connections
from multiple LSEG Real-time Distribution System components and allow the
Interactive Provider Application to manage those connections.

An Interactive provider can also directly connect to consumers.

In this Provider application, we show how to create an OMM Interactive Provider
application. An OMM Interactive Provider application opens a listening socket on a well-known
port allowing OMM consumer applications to connect. Once connected, consumers
can request data from the Interactive Provider.

The following steps summarize this process:

- Establish network communication and listen for incoming connections
- Accept incoming connections
- Perform Login process
- Provide Source Directory information
- Provide necessary Dictionaries
- Handle Requests and publish content
- Disconnect consumers and shut down

The Provider training example application, included in Training folder,
provides one implementation of an OMM Interactive Provider application written with
simplicity in mind and demonstrates the use of the Transport API. It consists of several modules that
show incremental progress of how to build the app from the ground up.

For Provider training example application, you can use any File Comparison
tools to compare between any two consecutive modules so that you will know the
specific incremental code for a particular module, which is the difference
between 2 consecutive modules.

You can also compare each module of the Interactive Provider code with the same module name (if
it exists) of another training app, such as the Non-Interactive Provider (NIP) to find commonality
between Provider and NIProvider training applications, if you wish.

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

The Modules available for Provider training example application are listed
here.

- Transport API Interactive Provider Training Module 1a: Establish network communication
- Transport API Interactive Provider Training Module 1b: Ping (heartbeat) Management
- Transport API Interactive Provider Training Module 1c: Reading and Writing Data
- Transport API Interactive Provider Training Module 2: Perform/Handle Login Process
- Transport API Interactive Provider Training Module 3: Provide Source Directory Information
- Transport API Interactive Provider Training Module 4: Provide Necessary Dictionaries
- Transport API Interactive Provider Training Module 5: Handle Item Requests

Please go to each individual module folder for more details. Please note that each
module has its own project file and application name tailored for that module.

Please follow the instructions in readmes for each module for information on setup
environment and ommand line usage to know how to run the executable and its available
set of parameters for each different modules.

-----------------
Compiling Source:
-----------------

The included project file is set up to run from the file locations as presented
through the distribution package.

To compile, run the `dotnet build` command with desired parameters
(configuration, architecture, etc.)

For windows platform, using Visual Studio, open the main ETA.NET.sln solution
file and build the corresponding project.

----------------
Example Content:
----------------

Included for each module of this application are:

- Source files for each module.

- project files for each module.

- readme for each module.
