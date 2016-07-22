export JAVA_BIN=$JAVA_HOME/bin/java
export ETAJ_PATH=../../../Eta
export CLASSPATH=./java:../../Libs/ema.jar:$ETAJ_PATH/Libs/upa.jar:$ETAJ_PATH/Libs/upaValueAdd.jar:../../Libs/apache/org.apache.commons.collections.jar:../../Libs/apache/commons-configuration-1.10.jar:../../Libs/apache/commons-lang-2.6.jar:../../Libs/apache/commons-logging-1.2.jar:../../Libs/SLF4J/slf4j-1.7.12/slf4j-api-1.7.12.jar:../../Libs/SLF4J/slf4j-1.7.12/slf4j-jdk14-1.7.12.jar

#export LOGGINGCONFIGPATH=-Djava.util.logging.config.file=../main/resource/logging.properties
#$JAVA_BIN $LOGGINGCONFIGPATH com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming.NiProvider

$JAVA_BIN  com.thomsonreuters.ema.examples.training.niprovider.series100.example100__MarketPrice__Streaming.NiProvider
