#!/bin/ksh

######################################################
# if required, please modify this
######################################################

# java bin location
export JAVA_BIN="$JAVA_HOME"/bin/java

# command line options
#.............................
# - uncomment and add to APP_ARGS to specify the command line options
export APP_ARGS="-steadyStateTime 90 -tickRate 1000 -requestRate 5000 -itemCount 100000 -itemFile 350k.xml -threads 1"

# JVM parameters
export JVM_OPTIONS="-server -XX:+ForceTimeHighResolution -Xms2048m -Xmx2048m"

######################################################

# display
echo .................................................
echo . EMAJ OMM Consumer performance application 
echo .................................................

# application class
export APPNAME=com.thomsonreuters.ema.perftools.emajconsperf.emajConsPerf

# classpath
export APP_DEPENDENTPATH="./xpp3-1.1.3_8.jar:./xpp3_min-1.1.3_8.jar:../../Libs/apache/org.apache.commons.collections.jar:../../Libs/apache/commons-configuration-1.10.jar:../../Libs/apache/commons-lang-2.6.jar:../../Libs/apache/commons-logging-1.2.jar:../../Libs/SLF4J/slf4j-1.7.12/slf4j-api-1.7.12.jar:../../Libs/SLF4J/slf4j-1.7.12/slf4j-jdk14-1.7.12.jar"
export APP_CLASSPATH="./java:../../Libs/ema.jar:../../../Eta/Libs/upa.jar:../../../Eta/Libs/upaValueAdd.jar:$APP_DEPENDENTPATH"

# command
export RUN_CMD="$JAVA_BIN  $JVM_OPTIONS -cp $APP_CLASSPATH $APPNAME $APP_ARGS"

echo $RUN_CMD
echo

# run command
$RUN_CMD
