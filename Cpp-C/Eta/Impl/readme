RSSL ValueAdd Component Source

--------
Summary:
--------

The implementation of the Value Added components is provided to allow for
users to modify these components to support their own custom solutions.  
The full source code and necessary project files have been provided, 
however no support will be provided for issues resulting from custom-built 
components.

-----------------
Components:
-----------------

rsslVAUtil.a - static library containing utility functionality used by ValueAdd components.

rsslRDM.a - static library containing functionality related to the administrative domain 
representation structures.

rsslReactor.a - static library containing the RsslReactor functionality.

rsslVA.so - shared library containing all value add functionality
 
-----------------
Compiling Source:
-----------------

The included makefile is set up to run from the file
locations as presented through the distribution package.
It is set up for building on the Transport API supported 
Linux platforms using the supported compilers.

The included makefile is set up to build the Value Add
components as both static and shared libraries.

To compile, run the gmake command.

After building, ensure that all custom libraries and headers are moved to
appropriate locations for linking with applications.

Gmake can be obtained at http://www.gnu.org/software/make/

----------------
Example Content:
----------------

Included for the components are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Util/Impl/rsslMemoryBuffer.c - Contains functionality for apportioning space using an RsslBuffer.

RDM/Impl/rsslRDMLoginMsg.c - Contains the Login domain encoder, decoder, and copy functions.

RDM/Impl/rsslRDMDirectoryMsg.c - Contains the Directory domain encoder, decoder, and copy functions.

RDM/Impl/rsslRDMDictionaryMsg.c - Contains the Dictionary domain encoder, decoder, and copy functions.

Reactor/Impl/rsslReactor.c - Contains implmentation of the RsslReactor functions.

Reactor/Impl/rsslReactorWorker.c - Contains implmentation of the RsslReactor's worker thread.
