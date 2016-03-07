rem echo java com.thomsonreuters.ema.examples.training.series100.example100__MarketPrice__Streaming.Consumer

set CLASSPATH=.\java;..\..\Libs\ema.jar;..\..\dependencies\apache\org.apache.commons.collections.jar;..\..\dependencies\apache\commons-configuration-1.10\commons-configuration-1.10.jar;..\..\dependencies\apache\commons-lang-2.6\commons-lang-2.6.jar;..\..\dependencies\apache\commons-logging-1.2\commons-logging-1.2.jar

java -cp %CLASSPATH% com.thomsonreuters.ema.examples.training.series100.example100__MarketPrice__Streaming.Consumer