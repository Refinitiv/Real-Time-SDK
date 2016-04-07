rem echo java com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer

set CLASSPATH=.\java;..\..\Libs\ema.jar;..\..\Libs\apache\org.apache.commons.collections.jar;..\..\Libs\apache\commons-configuration-1.10.jar;..\..\Libs\apache\commons-lang-2.6.jar;..\..\Libs\apache\commons-logging-1.2.jar

rem set LOGGINGCONFIGPATH=..\main\resource\logging.properties

java -cp %CLASSPATH% com.thomsonreuters.ema.examples.training.consumer.series100.example100__MarketPrice__Streaming.Consumer
