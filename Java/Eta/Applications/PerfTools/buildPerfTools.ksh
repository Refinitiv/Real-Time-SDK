#!/bin/ksh

mkdir -p bin
JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:./xpp3-1.1.3_8.jar:./xpp3_min-1.1.3_8.jar:../../Libs/upa.jar:../../Libs/upaValueAdd.jar
export CLASSPATH

rm -f `find . -name *.class`

$JAVAC -version -target 1.7 -source 1.7 -d bin com/thomsonreuters/upa/perftools/common/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin com/thomsonreuters/upa/perftools/upajconsperf/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin com/thomsonreuters/upa/perftools/upajniprovperf/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin com/thomsonreuters/upa/perftools/upajprovperf/*.java
$JAVAC -version -target 1.7 -source 1.7 -d bin com/thomsonreuters/upa/perftools/upajtransportperf/*.java
