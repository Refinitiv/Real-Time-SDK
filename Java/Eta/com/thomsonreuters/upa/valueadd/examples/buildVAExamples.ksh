#!/bin/ksh

mkdir -p bin
JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:../../Libs/upa.jar:../../Libs/jdacsUpalib.jar:../Libs/upaValueAdd.jar:../Libs/upaValueAddCache.jar:../../Libs/ansipage.jar
export CLASSPATH
EXAMPLE_PATH=com/thomsonreuters/upa/valueadd/examples
export EXAMPLE_PATH

rm -f `find . -name *.class`

$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/consumer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/niprovider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/provider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/queueconsumer/*.java
