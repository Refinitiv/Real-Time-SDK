#!/bin/ksh

######################################################
# if required, please modify this
######################################################

# java bin location
JAVA_BIN="$JAVA_HOME"/bin/java

# command line options
#.............................
# - uncomment and add to APP_ARGS to specify the command line options
#APP_ARGS="-msgRate 1000 -runTime 60"

# JVM parameters
JVM_OPTIONS="-server -XX:+ForceTimeHighResolution -Xms2048m -Xmx2048m"

######################################################

# display
echo .................................................
echo . UPAJ Transport performance application 
echo .................................................

# application class
APPNAME=com.thomsonreuters.upa.perftools.upajtransportperf.upajTransportPerf

# classpath
APP_CLASSPATH=../Shared:../../Libs/upa.jar:../../Libs/upaValueAdd.jar:./xpp3-1.1.3_8.jar:./xpp3_min-1.1.3_8.jar:bin:../../../../Elektron-SDK-BinaryPack/Java/Eta/Libs/upa.jar

# command
RUN_CMD="$JAVA_BIN  $JVM_OPTIONS -cp $APP_CLASSPATH $APPNAME $APP_ARGS"

echo $RUN_CMD
echo

# run command
$RUN_CMD
