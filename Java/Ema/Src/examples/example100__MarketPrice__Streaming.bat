rem echo java com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer

set ETAJ_PATH=..\..\..\Eta
set CLASSPATH=.\java;..\..\Libs\ema.jar;%ETAJ_PATH%\Libs\upa.jar;%ETAJ_PATH%\Libs\upaValueAdd.jar;..\..\Libs\apache\org.apache.commons.collections.jar;..\..\Libs\apache\commons-configuration-1.10.jar;..\..\Libs\apache\commons-lang-2.6.jar;..\..\Libs\apache\commons-logging-1.2.jar;..\..\Libs\SLF4J\slf4j-1.7.12\slf4j-api-1.7.12.jar;..\..\Libs\SLF4J\slf4j-1.7.12\slf4j-jdk14-1.7.12.jar

rem set LOGGINGCONFIGPATH=..\main\resource\logging.properties

java -cp %CLASSPATH% com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer
