Transport API Consumer Training Application Description

--------
Summary:
--------

The Transport API Consumer Training example provides detailed step-by-step code modules showing
how to create a simple and basic OMM consumer application using the Transport API.

The Transport API Consumer Training example application:
- Provides an implementation of an OMM consumer application.
- Consists of several modules that show incremental progress of how to build a basic
  Transport API consuming application.
- Is written with simplicity in mind and demonstrates the basic functionality of the Transport API.

You can use file comparison tools (such as WinMerge, etc.) to compare any two consecutive
modules. This can help you understand new code added to any specific module. You can also
compare each module of the Transport API Consumer Training example code with the same module name
(if it exists) from another training application, such as the Transport API NIProvider Training example
to find commonality between the applications.

This Transport API Consumer training application provides an outline of how to create an
OMM consumer application. An OMM consumer application can establish a connection
to other OMM Interactive Provider applications, including LSEG Real-Time Distribution Systems
Data Feed Direct, and LSEG Real-Time. After connecting successfully, an OMM consumer
can then consume (i.e., send data requests and receive responses) and
publish data (i.e., post data).

The general process can be summarized by the following steps.

- Establish network communication
- Log in
- Obtain Source Directory information
- Load or Download all necessary dictionary information
- Issue Requests and/or Post information
- Log out and shut down

Content is encoded and decoded using the Transport API Message Package and the Transport API
Data Package.

The Modules available for Consumer training example application are listed
here.

- Transport API Consumer Training Module 1a: Establish network communication
- Transport API Consumer Training Module 1b: Ping (heartbeat) Management
- Transport API Consumer Training Module 1c: Reading and Writing Data
- Transport API Consumer Training Module 2:  Log in
- Transport API Consumer Training Module 3:  Obtain Source Directory
- Transport API Consumer Training Module 4:  Obtain Dictionary Information
- Transport API Consumer Training Module 5:  Issue Item Requests

Please go to each individual module folder for more details. Please note that each
module has its own project file and application name tailored for that module.

Please follow the instructions in readmes for each module for information on setup
environment and command line usage to know how to run the executable and its available
set of parameters for each different module.

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

- project file for each module.

- readme for each module.
