# Elektron Message API - Java Edition (PREVIEW)
#
 

##Overview

This directory contains preliminary library source code and example code to show off the upcoming Elektron Message API (EMA) - Java Edition Consumer. It is a work in progress, but we would like to share with you what we have done so far. 

## What is new

- **Add EMA Encoding Interface 
- **Add EMA Functionality for reading xml configuration file
- **Add EMA Encoding Examples (OnStream posting and OffStream posting)
- **Remove EMA forth() decoding interface

##Contents

- **EMA Library** One Jar file for the EMA Consumer library nesting dependent jar files.
- **EMA Library Source** Source codes for the EMA Consumer library. 
- **Examples** EMA Consumer examples
- **Example Compile Scripts** Help to compile all examples on windows platform or unix platform
- **EMA Doc** Reference manual for the EMA Consumer interfaces.
- **Logging Properties** Logging configuration for Java Logging API if needed   

####This preview is runnable!  

Within this directory contains everything you need to build the examples and run them.  Just navigate to Src/examples directory and run buildExamples.bat on windows or buildExamples.ksh on unix.

## Currently Supported Features
- TCP connection to ADS
- Direct connection 
- Login
- Source Directory
- Download Dictionary from provider
- Register item interests
- Receive Refresh, Update and Status messages
- Supports multiple item interests
- Enable/Disable log tracing to console or file

## EMA Dependencies
Building of EMA library and Running of EMA examples both depend on apache jars listed as below:

apache\org.apache.commons.collections.jar;
apache\commons-configuration-1.10\commons-configuration-1.10.jar
apache\commons-lang-2.6\commons-lang-2.6.jar
apache\commons-logging-1.2\commons-logging-1.2.jar

Here is one download site for apache jar files http://commons.apache.org/downloads/

## Turn on log tracing ( one way to turn on log, SLF4J interface with java.util.logging implementation )
Add java run option to point to the path of java.util.logging configuration file.

For example:

	set LOGGINGCONFIGPATH=..\main\resource\logging.properties

	java -cp %CLASSPATH% -Djava.util.logging.config.file=%LOGGINGCONFIGPATH% com.thomsonreuters.ema.examples.training.series100.example100__MarketPrice__Streaming.Consumer

## Can You Provide Feedback?
Not just yet!  In the coming months we will continue to update you with ema library open source and more examples and prototypes. As the product matures, we will be able to accept your feedback. 

###Check back over the coming months to see where we are at!




