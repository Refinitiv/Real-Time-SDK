JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:../../Libs/ema.jar
export CLASSPATH
EXAMPLE_PATH=./java/com/thomsonreuters/ema/examples/training
export EXAMPLE_PATH

rm -f `find . -name *.class`

set -x

for file in $(find . -name '*.java')
do
    $JAVAC $file
done
