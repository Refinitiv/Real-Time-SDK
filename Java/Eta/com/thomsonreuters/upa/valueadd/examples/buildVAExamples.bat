mkdir bin
set JAVAC="%JAVA_HOME%\bin\javac"
set CLASSPATH=.\;..\..\Libs\upa.jar;..\..\Libs\jdacsUpalib.jar;..\Libs\upaValueAdd.jar;..\Libs\upaValueAddCache.jar;..\..\Libs\ansipage.jar

%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\common\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\consumer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\niprovider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\provider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\valueadd\examples\queueconsumer\*.java

