JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:../../Libs/ema.jar
export CLASSPATH
EXAMPLE_PATH=./java/com/thomsonreuters/ema/examples/training
export EXAMPLE_PATH

rm -f `find . -name *.class`

$JAVAC -version -target 1.7 -source 1.7 $EXAMPLE_PATH/*/*/*.java 
