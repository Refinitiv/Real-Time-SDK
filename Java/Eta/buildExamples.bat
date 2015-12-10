mkdir bin
set JAVAC="%JAVA_HOME%\bin\javac"
set CLASSPATH=.\;..\Libs\upa.jar;..\Libs\jdacsUpalib.jar;..\ValueAdd\Libs\upaValueAdd.jar;..\Libs\ansipage.jar

%JAVAC% -d bin com\thomsonreuters\upa\examples\codec\*.java 
%JAVAC% -d bin com\thomsonreuters\upa\examples\common\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\consumer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\niprovider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\provider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\genericcons\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\genericprov\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\authlock\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\ansipage\*.java 
%JAVAC% -d bin com\thomsonreuters\upa\examples\rdm\marketprice\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\newsviewer\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\edfexamples\common\*.java
%JAVAC% -d bin com\thomsonreuters\upa\examples\edfexamples\edfconsumer\*.java

