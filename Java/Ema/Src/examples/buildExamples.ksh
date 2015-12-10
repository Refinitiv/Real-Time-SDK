JAVAC="$JAVA_HOME/bin/javac"
export JAVAC
CLASSPATH=./:../../Libs/ema.jar
export CLASSPATH

rm -f `find . -name *.class`

javac $(find . | grep .java)


