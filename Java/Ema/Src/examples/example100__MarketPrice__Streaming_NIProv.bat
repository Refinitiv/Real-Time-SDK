rem echo java com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming.NiProvider

set JAVA_BIN="%JAVA_HOME%\bin\java"
set ETAJ_PATH=..\..\..\Eta
set CLASSPATH=.\java;..\..\Libs\ema.jar;%ETAJ_PATH%\Libs\upa.jar;%ETAJ_PATH%\Libs\upaValueAdd.jar;..\..\Libs\apache\org.apache.commons.collections.jar;..\..\Libs\apache\commons-configuration-1.10.jar;..\..\Libs\apache\commons-lang-2.6.jar;..\..\Libs\apache\commons-logging-1.2.jar;..\..\Libs\SLF4J\slf4j-1.7.12\slf4j-api-1.7.12.jar;..\..\Libs\SLF4J\slf4j-1.7.12\slf4j-jdk14-1.7.12.jar

rem echo turn on logger with following setting
rem set LOGGINGCONFIGPATH=-Djava.util.logging.config.file=..\main\resource\logging.properties
rem %JAVA_BIN% -cp %CLASSPATH% %LOGGINGCONFIGPATH% com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming.NiProvider
rem java -cp %CLASSPATH% %LOGGINGCONFIGPATH% com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming.NiProvider

%JAVA_BIN% -cp %CLASSPATH% com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming.NiProvider
