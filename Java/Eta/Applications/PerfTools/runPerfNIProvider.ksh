#!/bin/ksh

######################################################
# if required, please modify this
######################################################

# java bin location
JAVA_BIN="$JAVA_HOME"/bin/java

# command line options
#.............................
# - uncomment APP_ARGS to specify the command line options
# - add to this if necessary
#APP_ARGS="-runTime 60 -latencyUpdateRate 1000 -tickRate 1000 -updateRate 100000"

# JVM parameters
JVM_OPTIONS="-server -XX:+ForceTimeHighResolution -Xms2048m -Xmx2048m"

######################################################

# display
echo .................................................
echo . UPAJ OMM NIProvider performance application 
echo .................................................

# application class
APPNAME=com.thomsonreuters.upa.perftools.upajniprovperf.upajNIProvPerf

# classpath
APP_CLASSPATH=../../Libs/upa.jar:../../Libs/upaValueAdd.jar:./xpp3-1.1.3_8.jar:./xpp3_min-1.1.3_8.jar:bin:../../../../Elektron-SDK-BinaryPack/Java/Eta/Libs/upa.jar

# command
RUN_CMD="$JAVA_BIN  $JVM_OPTIONS -cp $APP_CLASSPATH $APPNAME $APP_ARGS"

echo $RUN_CMD
echo

# run command
$RUN_CMD
