////////////////////////////////////////////////
//
//		EncDecExample application
//
////////////////////////////////////////////////

----------------
Example Name
----------------
EncDecExample


----------------
Example Summary
----------------

This example is intended to serve as a high-level, basic demonstration of
encoding and decoding using the RSSL API. The user can set break points and
step through the code to help get a look and feel for how to use the API in 
several ways.  

The application contains the following message encoding/decoding examples.

Encoding and decoding a simple message that contains a Field List payload
with various primitive types as payload.

Encoding and decoding a more complicated message that contains a Map container
with Field Lists nested inside each Map Entry.

Encoding and decoding a simple RsslRefreshMsg that contains an RSSL_DT_MAP.
For the payload of the message, it just calls the map encoding function.
Same thing for decoding.

Encoding and decoding a simple message that contains a Element List payload
with various primitive types as payload.

Encoding and decoding a more complicated message that contains a Series
container with Element Lists nested inside each Series Entry.

Encoding and decoding a more complicated message that contains a Vector
container with Element Lists nested inside each Vector Entry.

Encoding and decoding a more complicated message that contains a Filter List
container with Element Lists nested inside each Filter List Entry.

Encoding and decoding a simple RsslRefreshMsg that contains an RSSL_DT_SERIES.
For the payload of the message, it just calls the series encoding function.
Same thing for decoding.

The application is written as a basic example.  It is not intended to demonstrate
performance. In addition, because both encode and decode sides know what was
encoded, some presence checking is not occurring. For safety, message and container
masks should be checked before accessing members. The application is heavily
commented and can be used as an additional reference.  

-------------------
Compiling
-------------------

The included makefile is set up to run from the file 
locations as presented through the distribution package.
It is set up for building on the ETA supported Linux
platforms using the ETA supported

The LINKTYPE value in the makefile is used to control
whether the application is built using ETA static or
shared libraries. The default build uses ETA static
libraries. To use ETA shared libraries,
set LINKTYPE=Shared.

To compile run gmake command.  

Gmake can be obtained from http://www.gnu.org/software/make


For windows platform, using Visual Studio, open one of the included vcxproj project
files and build.

