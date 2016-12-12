@ echo off

rem ##################################################
rem # if required, please modify this
rem ##################################################

rem java bin location
set JAVA_BIN="%JAVA_HOME%\bin\java"

rem command line options
rem ..........................
rem - uncomment and add to APP_ARGS to specify the command line options
rem set APP_ARGS=-runTime 60 -latencyUpdateRate 1000 -tickRate 1000 -updateRate 100000

rem JVM parameters
set JVM_OPTIONS=-server -XX:+ForceTimeHighResolution

rem ##################################################

rem display
echo .................................................
echo . UPAJ OMM Provider performance application 
echo .................................................
echo.

rem application class
set APPNAME=com.thomsonreuters.upa.perftools.upajprovperf.upajProvPerf

rem classpath
set APP_CLASSPATH=../Shared;../../Libs/upa.jar;../../Libs/upaValueAdd.jar;./xpp3-1.1.3_8.jar;./xpp3_min-1.1.3_8.jar;bin;..\..\..\..\Elektron-SDK-BinaryPack\Java\Eta\Libs\upa.jar

rem command
set RUN_CMD=%JAVA_BIN%  %JVM_OPTIONS% -cp %APP_CLASSPATH% %APPNAME% %APP_ARGS%
echo.
echo %RUN_CMD%
echo.


rem run command
%RUN_CMD%


