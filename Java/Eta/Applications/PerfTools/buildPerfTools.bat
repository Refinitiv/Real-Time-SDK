mkdir bin
set JAVAC="%JAVA_HOME%\bin\javac"
set CLASSPATH=.\;.\xpp3-1.1.3_8.jar;.\xpp3_min-1.1.3_8.jar;..\..\Libs\upa.jar;..\..\Libs\upaValueAdd.jar

del /S *.class

%JAVAC% -d bin com\thomsonreuters\upa\perftools\common\*.java
%JAVAC% -d bin com\thomsonreuters\upa\perftools\upajconsperf\*.java
%JAVAC% -d bin com\thomsonreuters\upa\perftools\upajniprovperf\*.java
%JAVAC% -d bin com\thomsonreuters\upa\perftools\upajprovperf\*.java
%JAVAC% -d bin com\thomsonreuters\upa\perftools\upajtransportperf\*.java
