#!/bin/ksh

mkdir -p bin
JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:../../Libs/upa.jar:../../Libs/jdacsUpalib.jar:../../Libs/upaValueAdd.jar:../../Libs/upaValueAddCache.jar:../../Libs/ansipage.jar
export CLASSPATH
EXAMPLE_PATH=com/thomsonreuters/upa/examples
export EXAMPLE_PATH
VAEXAMPLE_PATH=com/thomsonreuters/upa/valueadd/examples
export VAEXAMPLE_PATH

rm -f `find . -name *.class`

$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/codec/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/consumer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/niprovider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/provider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/genericcons/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/genericprov/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/rdm/marketprice/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/newsviewer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/edfexamples/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/edfexamples/edfconsumer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $VAEXAMPLE_PATH/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $VAEXAMPLE_PATH/consumer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $VAEXAMPLE_PATH/niprovider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $VAEXAMPLE_PATH/provider/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $VAEXAMPLE_PATH/queueconsumer/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin $VAEXAMPLE_PATH/watchlistconsumer/*.java

if [ -f ../../Libs/ansipage.jar ]; then
	$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/ansipage/*.java
else
	echo "Warning: ansipage.jar not found; not building AnsiPageExample."
fi

if [ -f ../../Libs/jdacsUpalib.jar ]; then
	$JAVAC -version -target 1.7 -source 1.7 -d bin $EXAMPLE_PATH/authlock/*.java
else
	echo "Warning: jdacsUpalib.jar not found; not building AuthLockExample."
fi

