@ echo off

rem ##################################################
rem # if required, please modify this
rem ##################################################

rem java bin location
set JAVA_BIN="%JAVA_HOME%/bin/java"

rem command line options
rem ..........................
rem - uncomment and add to APP_ARGS to specify the command line options
set APP_ARGS=-steadyStateTime 90 -tickRate 1000 -requestRate 5000 -itemCount 100000 -itemFile 350k.xml -threads 1

rem JVM parameters
set JVM_OPTIONS=-server -XX:+ForceTimeHighResolution -Xms2048m -Xmx2048m

rem ##################################################

rem display
echo .................................................
echo . EMAJ OMM Consumer performance application 
echo .................................................
echo.

rem application class
set APPNAME=com.thomsonreuters.ema.perftools.emajconsperf.emajConsPerf
set ETA_SUBPATH=../../../../Elektron-SDK-BinaryPack/Java/Eta/Libs

rem classpath
set APP_DEPENDENTPATH=./xpp3-1.1.3_8.jar;./xpp3_min-1.1.3_8.jar;../../Libs/apache/org.apache.commons.collections.jar;../../Libs/apache/commons-configuration-1.10.jar;../../Libs/apache/commons-lang-2.6.jar;../../Libs/apache/commons-logging-1.2.jar;../../Libs/SLF4J/slf4j-1.7.12/slf4j-api-1.7.12.jar;../../Libs/SLF4J/slf4j-1.7.12/slf4j-jdk14-1.7.12.jar
set APP_CLASSPATH=./java;../../Libs/ema.jar;../../../Eta/Libs/upa.jar;../../../Eta/Libs/upaValueAdd.jar;%ETA_SUBPATH%/upa.jar;%APP_DEPENDENTPATH%

rem command
set RUN_CMD=%JAVA_BIN%  %JVM_OPTIONS% -cp %APP_CLASSPATH% %APPNAME% %APP_ARGS%
echo.
echo %RUN_CMD%
echo.


rem run command
%RUN_CMD%


