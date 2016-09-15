mkdir bin
set JAVAC="%JAVA_HOME%\bin\javac"
set CLASSPATH=.\;..\Examples;..\..\Libs\upa.jar;..\..\Libs\upaValueAdd.jar;

%JAVAC% -d bin ..\Examples\com\thomsonreuters\upa\examples\common\*.java 
%JAVAC% -d bin ..\Examples\com\thomsonreuters\upa\examples\rdm\marketprice\*.java
%JAVAC% -d bin com\thomsonreuters\upa\training\consumer\*.java 
%JAVAC% -d bin com\thomsonreuters\upa\training\niprovider\*.java
%JAVAC% -d bin com\thomsonreuters\upa\training\provider\*.java

