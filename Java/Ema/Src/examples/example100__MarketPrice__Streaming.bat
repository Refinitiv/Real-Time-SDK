@ echo off
rem echo java com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer

set JAVA_BIN="%JAVA_HOME%\bin\java"
set ETAJ_PATH=..\..\..\Eta
set ETAJ_SUBPATH=..\..\..\..\Elektron-SDK-BinaryPack\Java\Eta\Libs
set CLASSPATH=.\java;..\..\Libs\ema.jar;%ETAJ_PATH%\Libs\upa.jar;%ETAJ_PATH%\Libs\upaValueAdd.jar;%ETAJ_SUBPATH%\upa.jar;..\..\Libs\apache\org.apache.commons.collections.jar;..\..\Libs\apache\commons-configuration-1.10.jar;..\..\Libs\apache\commons-lang-2.6.jar;..\..\Libs\apache\commons-logging-1.2.jar;..\..\Libs\SLF4J\slf4j-1.7.12\slf4j-api-1.7.12.jar;..\..\Libs\SLF4J\slf4j-1.7.12\slf4j-jdk14-1.7.12.jar

rem echo turn on logger with following setting
rem set LOGGINGCONFIGPATH=-Djava.util.logging.config.file=..\main\resource\logging.properties
rem %JAVA_BIN% -cp %CLASSPATH% %LOGGINGCONFIGPATH% com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer
rem java -cp %CLASSPATH% %LOGGINGCONFIGPATH% com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer

%JAVA_BIN% -cp %CLASSPATH% com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer
