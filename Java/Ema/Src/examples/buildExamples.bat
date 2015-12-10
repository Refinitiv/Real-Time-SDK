set JAVAC="%JAVA_HOME%\bin\javac"
set CLASSPATH=.\;..\..\Libs\ema.jar

del /S *.class

for /r %%a in (*.java) do ( %JAVAC% "%%a")


