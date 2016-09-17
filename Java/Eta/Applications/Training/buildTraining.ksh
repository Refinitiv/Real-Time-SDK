#!/bin/ksh

mkdir -p bin
JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:../Examples:../../Libs/upa.jar:../../Libs/upaValueAdd.jar:../../../../Elektron-SDK-BinaryPack/Java/Eta/Libs/upa.jar
export CLASSPATH
TRAINING_EXAMPLE_PATH=com/thomsonreuters/upa/training
export TRAINING_EXAMPLE_PATH
EXAMPLE_PATH=../Examples/com/thomsonreuters/upa/examples
export EXAMPLE_PATH


rm -f `find . -name *.class`

$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/rdm/marketprice/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $TRAINING_EXAMPLE_PATH/consumer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $TRAINING_EXAMPLE_PATH/niprovider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $TRAINING_EXAMPLE_PATH/provider/*.java

