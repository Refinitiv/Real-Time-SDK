rsslRmtesNewsDisplay Application Description

--------
Summary:
--------

The purpose of this application is to provide a visual, managed C++ display of content 
from the N2_UBMS RIC. It is a multi-threaded client application. The
general application flow is that it first initializes the RSSL transport and
connects the client. After that, it sends a login request, a source directory
request, a dictionary request, and one streaming item request for the N2_UBMS RIC.

If the dictionary is found in the directory of execution, then it is loaded
directly from the file. However, the default configuration for this application
is to request the dictionary from the provider. Hence, no link to the dictionary
is made in the execution directory by the build script. The user can change this
behavior by manually creating a link to the dictionary in the execution directory.

This application supports consuming content from N2_UBMS headlines and news stories.

This application is intended as a basic usage example. Some of the design choices
were made to favor simplicity and readability over performance. This application 
is not intended to be used for measuring performance.
 
-----------------
Application Name:
-----------------

RmtesNewsDisplay.exe

------------------
Setup Environment:
------------------

The RDMFieldDictionary and enumtype.def files located in the etc directory
can be located in the directory of execution. If the dictionary files
cannot be found, they are requested from the provider.

-------------------
Program usage:
-------------------

After starting the application, click on the "Connect" button.  This will bring
up a dialog box containing text boxes for the Host Name, the Port Number, Service
Name, and Item name.  Enter the appropriate connection data, then click on "connect".

The application will connect and automatically begin to consume data from the provided RIC.

When headline updates are received, they will be displayed in the top listbox.  Double clicking
a headline will request the story from the connected host.  When this has been retrieved,
the full story will be displayed in the textbox below the list.

To disconnect from the current host, the user should click on the "Disconnect button on 
the lower right.  This will disconnect the application and clear the headline list.

To close the application, the user should click on the "X" button on the top.

-----------------
Compiling Source:
-----------------

Development Tool: 

using Visual Studio, open one of the included vcxproj project
files and build.

----------------
Example Content:
----------------

Included for this application are:

- Source files.

- This document.

--------------------
Detailed Description
--------------------

Form1.h/Form1.cpp - The main windows form for the application.
ConnectForm.h/ConnectForm.cpp - The connection dialog.
NewsConsumer.h/NewsConsumer.cpp - This is the primary Transport API Consumer code.  This handles 
								  all the OMM decoding and RMTES to UCS2 decoding.
NewsWrapper.h/NewsConsumer.cpp - This provides an interface for the display of the decoded 
								 data provided by the Transport API Consumer.
NewsDisplay.cpp - Main entry function for the application.

