#!/bin/ksh

mkdir -p bin
JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:../Libs/upa.jar:../Libs/jdacsUpalib.jar:../ValueAdd/Libs/upaValueAdd.jar:../Libs/ansipage.jar
export CLASSPATH
EXAMPLE_PATH=com/thomsonreuters/upa/examples
export EXAMPLE_PATH

rm -f `find . -name *.class`

$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/codec/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/consumer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/niprovider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/provider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/genericcons/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/genericprov/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/authlock/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/ansipage/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/rdm/marketprice/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/newsviewer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/edfexamples/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/edfexamples/edfconsumer/*.java

