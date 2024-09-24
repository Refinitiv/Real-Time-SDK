Transport API Interactive Provider Training Application Description

Module 1c: Reading and Writing Data

--------
Summary:
--------

In this module, when a client or server Channel.State is
ChannelState.ACTIVE, it is possible for an application to receive
data from the connection. Similarly, when a client or server
Channel.State is ChannelState.ACTIVE, it is possible for an
application to write data to the connection. Writing involves a several
step process.

Detailed Descriptions:

When a client or server Channel.State is ChannelState.ACTIVE, it is
possible for an application to receive data from the connection. The
arrival of this information is often announced by the I/O notification
mechanism that the IChannel.Socket is registered with. The Transport API
reads information from the network as a byte stream, after
which it determines Buffer boundaries and returns each buffer one by
one.

When a client or server Channel.State is ChannelState.ACTIVE, it is
possible for an application to write data to the connection. Writing
involves a several step process. Because the Transport API provides
efficient buffer management, the user is required to obtain a buffer
from the Transport API buffer pool. This can be the guaranteed output
buffer pool associated with a Channel. After a buffer is acquired,
the user can populate the Buffer.Data and set the Buffer.Length
to the number of bytes referred to by data. If queued information cannot
be passed to the network, a function is provided to allow the application
to continue attempts to flush data to the connection. An I/O notification
mechanism can be used to help with determining when the network is able
to accept additional bytes for writing. The Transport API can continue to
queue data, even if the network is unable to write.

-----------------
Application Name:
-----------------

ProvMod1c

------------------
Setup Environment:
------------------

No additional files are necessary to run this application.

-------------------
Command line usage:
-------------------

dotnet run

Runs with a default set of parameters (-p 14002)

or

dotnet run -- [-p <SrvrPortNo>]

To view all command-line options, run:

dotnet run -- -?

Pressing the Ctrl+C buttons terminates the program.

-----------------
Compiling Source:
-----------------

The included project file is set up to run from the file locations as presented
through the distribution package.

To compile, run the `dotnet build` command with desired parameters
(configuration, architecture, etc.)

For windows platform, using Visual Studio, open the main ETA.NET.sln solution
file and build the ProvMod1c project.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Module_1c_ReadWrite.cs - The main file for the Transport API Interactive Provider Training application.
