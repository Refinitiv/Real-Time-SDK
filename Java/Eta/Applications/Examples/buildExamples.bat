mkdir bin
set JAVAC="%JAVA_HOME%\bin\javac"
set CLASSPATH=.\;..\..\Libs\upa.jar;..\..\Libs\jdacsUpalib.jar;..\..\Libs\upaValueAdd.jar;..\..\Libs\upaValueAddCache.jar;..\..\Libs\ansipage.jar

%JAVAC% -d bin com\thomsonreuters\upa\examples\codec\*.java 
%JAVAC% -d bin com\thomsonreuters\upa\examples\common\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\consumer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\niprovider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\provider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\genericcons\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\genericprov\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\rdm\marketprice\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\newsviewer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\edfexamples\common\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\edfexamples\edfconsumer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\common\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\consumer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\niprovider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\provider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\queueconsumer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\watchlistconsumer\*.java

@ECHO OFF
if exist ..\..\Libs\ansipage.jar (
	echo Building AnsiPageExample...
	%JAVAC% -d bin com\thomsonreuters\upa\examples\ansipage\*.java 
) else (
	echo Warning: ansipage.jar not found; not building AnsiPageExample.
)

@ECHO OFF
if exist ..\..\Libs\jdacsUpalib.jar (
	echo Building AuthLockExample...
	%JAVAC% -d bin com\thomsonreuters\upa\examples\authlock\*.java
) else (
	echo Warning: jdacsUpalib.jar not found; not building AuthLockExample.
)

