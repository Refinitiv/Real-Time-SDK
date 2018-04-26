# Elektron SDK - Java Edition
This is the Elektron SDK. This SDK is an all encompassing package of all Elektron APIs. This currently includes the Elektron Message API (EMA) and the Elektron Transport API (ETA).

The **Elektron Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment.  EMA is written on top of the Elektron Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Elektron Transport API (ETA)** is the re-branded Ultra Performance API (UPA). ETA is Thomson Reuters low-level 
Transport and OMM encoder/decoder API.  It is used by the Thomson Reuters Enterprise Platform for Real Time and Elektron for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. 

# Supported Platforms and Compilers

The Elektron-SDK has support for JDK 1.7 and JDK 1.8.  Please see the individual API README.md files for further details.

# Maven Central

For ease of product use, as of the ESDK 1.2 release, Thomson Reuters maintains its ESDK Jar files on Maven Central.

You can download ESDK libraries and dependencies from Maven Central using several different tools, specific
procedural instructions are not included here. Maven uses the following syntax to specify ESDK dependencies:

	<dependency>
		<groupId>com.thomsonreuters.ema</groupId>
		<artifactId>ema</artifactId>
		<version>3.2.0.1</version>
	</dependency>

	<dependency>
		<groupId>com.thomsonreuters.upa</groupId>
		<artifactId>upa</artifactId>
		<version>3.2.0.1</version>
	</dependency>

	<dependency>
		<groupId>com.thomsonreuters.upa.valueadd</groupId>
		<artifactId>upaValueAdd</artifactId>
		<version>3.2.0.1</version>
	</dependency>

	<dependency>
		<groupId>com.thomsonreuters.upa.valueadd.cache</groupId>
		<artifactId>upaValueAddCache</artifactId>
		<version>3.2.0.1</version>
	</dependency>

	<dependency>
		<groupId>com.thomsonreuters.upa.ansi</groupId>
		<artifactId>ansipage</artifactId>
		<version>3.2.0.1</version>
	</dependency>

Gradle uses the following syntax to specify ESDK dependencies:

	compile group: 'com.thomsonreuters.ema', name: 'ema', version: '3.2.0.1'
	compile group: 'com.thomsonreuters.upa', name: 'upa', version: '3.2.0.1'
	compile group: 'com.thomsonreuters.upa.valueadd', name: 'upaValueAdd', version: '3.2.0.1'
	compile group: 'com.thomsonreuters.upa.valueadd.cache', name: 'upaValueAddCache', version: '3.2.0.1'                    
	compile group: 'com.thomsonreuters.upa.ansi', name: 'ansipage', version: '3.2.0.1'  


# Building the APIs

## Common Setup
This section shows the required setup needed before you can build any of the APIs within this package.

Firstly, obtain the source from this repository. It will contain all of the required source to build ESDK as detailed below.
In addition, this repository depends on the `Elektron-SDK-BinaryPack` (http://www.github.com/thomsonreuters/Elektron-SDK-BinaryPack) repository and pulls the ETA libraries from that location.  That repository contains fully functioning libraries for the closed source portions of the product, allowing users to build and link to have a fully functional product. 

## Building ESDK

**Using Gradle**:

Gradle is now used to build ESDK.
Gradle can be downloaded from https://gradle.org

Refer to the ESDK Java Migration Guide for more detailed Gradle build instructions than what is described below.

Navigate to `Elektron-SDK/Java` and issue the appropriate Gradle command as follows:

	Windows: gradlew.bat jar
	Linux: ./gradlew jar
	
	This command builds the jar files.

#### Running examples

To run an example, issue the appropriate command as follows:
	  
	Windows: gradlew.bat runExampleName [-PcommandLineArgs="arguments"]
	Linux: ./gradlew runExampleName [-PcommandLineArgs="arguments"]
	(where runExampleName is the name of the example to run and arguments are the example arguments, only ETA supports the use of arguments)
	 
Issue the following command to get a list of all example names.
	  
	ETA Windows: gradlew.bat Eta:Applications:Examples:tasks --all
	ETA Linux: ./gradlew Eta:Applications:Examples:tasks --all
	EMA Windows: gradlew.bat Ema:Examples:tasks --all
	EMA Linux: ./gradlew Ema:Examples:tasks --all

ETA example, the following command runs the VAConsumer example.
		
	Windows: gradlew.bat runVaConsumer -PcommandLineArgs="-c localhost:14002 DIRECT_FEED mp:TRI"
	Linux: ./gradlew runVaConsumer -PcommandLineArgs="-c localhost:14002 DIRECT_FEED mp:TRI"

EMA example, the following command runs the example270__SymbolList example.
		
	Windows: gradlew.bat runconsumer270
	Linux: ./gradlew runconsumer270

# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](../Elektron API Individual Contributor License Agreement.pdf)
- [Entity Contributor License Agreement](../Elektron API Entity Contributor License Agreement.pdf)

Please email a signed and scanned copy to sdkagreement@thomsonreuters.com.  If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.


# Notes:
- For more details on each API, please see the corresponding readme file in their top level directory.
- This package contains APIs that are subject to proprietary and open source licenses.  Please make sure to read the readme files within each package for clarification.
- Please make sure to review the LICENSE.md file.
- Java unit tests may use Mockito (http://site.mockito.org/) for creation of mock objects. Mockito is distributed under the MIT license.
