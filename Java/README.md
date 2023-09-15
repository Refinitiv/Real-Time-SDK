# Refinitiv Real-Time SDK - Java Edition
This is the Refinitiv Real-Time SDK. This SDK encompasses these Real-Time APIs: Enterprise Message API (EMA) and the Enterprise Transport API (ETA).

The **Enterprise Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment. EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Enterprise Transport API (ETA)** is an open source Refinitiv low-level Transport and OMM encoder/decoder API. It is used by the Refinitiv Real-Time Distribution Systems and Refinitiv Real-Time for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages. Applications may be written to core ETA, to ValueAdd/Reactor layer or to Watchlist layer.

Copyright (C) 2019-2023 Refinitiv. All rights reserved.

# New In This Release

Please refer to the CHANGELOG file in this section to see what is new in this release of Refinitiv Real-Time SDK - Java Edition. Also in CHANGELOG is a list of issues fixed in this release and a history of features and fixes introduced per released version. 

### External Dependencies

External modules used by this version of RTSDK Java:

	Dependency				Version
	----------				-------
	commons-codec				1.11
	commons-configuration2			2.9.0
	commons-collections4			4.4	
	commons-lang3				3.3.2
	commons-logging				1.2
	commons-text		 		1.10.0
	jackson-annotations	 		2.15.2
	jackson-core	 			2.15.2  
	jackson-databind 			2.15.2
	jose4j					0.9.3
	junit					4.12
	json					20210307
	httpclient				4.5.14
	httpclient-cache 			4.5.14
	httpcore				4.4.13
	httpcore-nio	 			4.4.16
	httpmime				4.5.14
	lz4-java				1.8.0
	mockito-all				1.9.0
	slf4j-api				2.0.7
	slf4j-jdk14				2.0.7
	xpp3					1.1.4c


### Supported Platforms, OSs, Compilers

#### Hardware/OS Requirements

- HP Intel PC or AMD Opteron (64-bit)
- CPUs must have high resolution timer frequencies greater than 1GHz.

#### Supported Java Version 
The Refinitiv Real-Time-SDK supports Oracle JDK 1.8, 1.11 & 1.17, OpenJDK 1.8, 1.11 & 1.17, Amazon Corretto 8 & 11.

NOTE: RRT Viewer requires JavaFX which is bundled with open/JDK 1.11, and Amazon Corretto. JavaFX must be explicitly downloaded if using open/JDK 1.8. 

NOTE: Full JWT support requires at least Java 1.8u251.

Refinitiv fully supports the use of the EMA Java Edition developers kit on the core linux and windows platforms listed below.

Refinitiv will extend support to other platforms based on the following criteria:
- EMA Java is used with a J2SE 11 compliant JVM
- All problems must be reproducible on one of the core platforms listed below. Refinitiv support teams will only be able to reproduce problems on the core platforms.

#### Supported Platforms
The Refinitiv Real-Time-SDK provides support for multicast connections using JNI libraries. Also included are closed source libraries for reliable multicast support and value add cache. These libraries are available for the following platform and compiler combinations:

##### Windows

Platforms:

	Microsoft Windows Server 2012 Enterprise Edition or later 64-bit
	Microsoft Windows Server 2016 Enterprise Edition or later 64-bit
	Microsoft Windows Server 2019 Standard Edition or later 64-bit
	Microsoft Windows 10 Professional 64-bit

Compilers (only on OSs supported by Microsoft):

	Microsoft Visual Studio 14.0 (2015) 64-bit (JNI Libraries)
	Microsoft Visual Studio 14.1 (2017) 64-bit (JNI Libraries)
	Microsoft Visual Studio 14.2 (2019) 64-bit (JNI Libraries)
	Microsoft Visual Studio 14.3 (2022) 64-bit (JNI Libraries)

NOTE: To obtain JNI Libraries for deprecated versions, VS 2013 and VS 2012, please use a BinaryPack from a version [prior to Real-Time-SDK-2.0.3.L1](https://github.com/Refinitiv/Real-Time-SDK/releases/tag/Real-Time-SDK-2.0.2.G3) at your own risk as changes to BinaryPacks will not be availble for deprecated compilers.

##### Linux

Platforms:

	Oracle Linux Server 7.X 64-bit
        Red Hat Enterprise Server 7.X Release 64-bit
        Red Hat Enterprise Server 8.X Release 64-bit
        CentOS 7.X Release 64-bit Qualification
	Ubuntu 20.04 64-bit Qualification

#### Tested Versions

This release has been tested with the following:

	Oracle Java SE 8 (JDK1.8)
	Oracle Java SE 11 (JDK1.11)
	Oracle Open JDK (1.8)
	Oracle Open JDK (1.11)
	Oracle Open JDK (1.17)
	Amazon Corretto 8
	Amazon Corretto 11

#### Proxy Authentication Support

Proxies:

- Microsoft's Forefront
- [Free Proxy](http://www.handcraftedsoftware.org/)

Authentication Schemes:

- Basic
- NTLM
- NEGOTIATE/Kerberos

#### Encryption Support

This release supports encryption using TLS 1.2.  

##### Generating a keystore file
The **keystore** file can contain custom private keys and public key certificates (including your server certificate) that are used in TLS handshake to establish an encrypted connection. By default, the API will automatically use the cacerts file as your keystore file. The **cacerts** file comes with your java installation. If you create your own keystore file, please follow industry [standard instructions](https://docs.oracle.com/cd/E19509-01/820-3503/ggfen/index.html) to do so. This custom keystore file must be specified an input into API to successfully connect to the encrypted server.

### Interoperability

RTSDK Java supports connectivity to the following platforms:

- Refinitiv Real-Time Distribution System (RSSL/RWF connections): ADS/ADH all supported versions 
- Refinitiv Real-Time: Refinitiv Real-Time Deployed
- Refinitiv Real-Time Hosted
- Refinitiv Real-Time - Optimized (RTO)
- Refinitiv Direct Feed

NOTE: Connectivity to RDF-Direct is supported for Level 1 and Level 2 data.

This release has been tested with the following:

- ADS 3.7.1
- ADH 3.7.1
- DACS 7.8

# Documentation
  
Please refer to top level README.md and to Java/Eta/README.md or Java/Ema/README.md files to find more information. In this directory, please find the test plan with test results: RTSDK-Java-Edition\_Test\_Plan.xlsx

# Installation & Build

Please refer to Installation Guides for [ETA](Eta/Docs/RTSDK_J_Installation_Java.pdf) and [EMA](Ema/Docs/RTSDK_J_Installation_Java.pdf) for detailed instructions. In this section are some basic details.

## Install RTSDK 
This section shows the required setup needed before you can build any of the Java APIs.

Obtain the source **from this repository** on GitHub. It will contain all of the required source to build RTSDK as detailed below. In addition, this repository depends on a Binary Pack found in the [release assets](https://github.com/Refinitiv/Real-Time-SDK/releases) section that is auto pulled by a build. The BinaryPack contains libraries for the closed source portions of the product, permitting users to build and link all dependent libraries to have a fully functional product. 

Refinitiv Real-Time SDK package may also be [downloaded from Refinitiv Developer Portal](https://developers.refinitiv.com/en/api-catalog/refinitiv-real-time-opnsrc/rt-sdk-java/download).

Refinitiv Real-Time SDK package is also available on [MyRefinitiv.com](https://my.refinitiv.com/content/mytr/en/downloadcenter.html).

## Building RTSDK

**Using Gradle**:

Gradle is used to build RTSDK and may be downloaded from https://gradle.org. For a minimum version of gradle required by gradle wrapper, please check [build.gradle](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Java/build.gradle#L186).

Refer to the RTSDK Java Installation Guide for more detailed Gradle build instructions than what is described below.

Navigate to `RTSDK/Java` and issue the appropriate Gradle command as follows:

	Windows: gradlew.bat jar
	Linux: ./gradlew jar
	
	This command builds the jar files.

#### Running examples

To run an example, issue the appropriate command as follows:
	  
	Windows: gradlew.bat runExampleName  --args='<arguments>'
	Linux: ./gradlew runExampleName --args='<arguments>'
	 
Issue the following command to get a list of all example names.
	  
	ETA Windows: gradlew.bat Eta:Applications:Examples:tasks --all
	ETA Linux: ./gradlew Eta:Applications:Examples:tasks --all
	EMA Windows: gradlew.bat Ema:Examples:tasks --all
	EMA Linux: ./gradlew Ema:Examples:tasks --all

ETA example, the following command runs the VAConsumer example.
		
	Windows: gradlew.bat runVaConsumer --args='-c localhost:14002 DIRECT_FEED mp:TRI'
	Linux: ./gradlew runVaConsumer --args='-c localhost:14002 DIRECT_FEED mp:TRI'

EMA example, the following command runs the example270__SymbolList example.
		
	Windows: gradlew.bat runconsumer270
	Linux: ./gradlew runconsumer270

#### Running examples with Debug

To debug a encrypted consumer connection, you can add the following JVM argument:

	-Djavax.net.debug=all

This provides TLS details that can be useful if TLS handshake failed


# Obtaining the Refinitiv Field Dictionaries

The Refinitiv `RDMFieldDictionary` and `enumtype.def` files are present in this GitHub repo under `Java/etc`. In addition, the most current version can be downloaded from [MyRefinitiv.com](https://my.refinitiv.com/content/mytr/en/downloadcenter.html). Search for "Service Pack" and choose the latest version of Refinitiv Template Service Pack.

# Maven Central

For ease of product use, as of the RTSDK 1.2 release, Refinitiv maintains its RTSDK Jar files on Maven Central.

You can download RTSDK libraries and dependencies from Maven Central using several different tools, specific procedural instructions are not included here. Maven uses the following syntax to specify RTSDK dependencies (this is *sample* code) :

	<dependency>
		<groupId>com.refinitiv.ema</groupId>
		<artifactId>ema</artifactId>
		<version>3.7.2.0</version>
	</dependency>

	<dependency>
		<groupId>com.refinitiv.eta</groupId>
		<artifactId>eta</artifactId>
		<version>3.7.2.0</version>
	</dependency>

	<dependency>
		<groupId>com.refinitiv.eta.valueadd</groupId>
		<artifactId>etaValueAdd</artifactId>
		<version>3.7.2.0</version>
	</dependency>

	<dependency>
		<groupId>com.refinitiv.eta.valueadd.cache</groupId>
		<artifactId>etaValueAddCache</artifactId>
		<version>3.7.2.0</version>
	</dependency>

	<dependency>
		<groupId>com.refinitiv.eta.ansi</groupId>
		<artifactId>ansipage</artifactId>
		<version>3.7.2.0</version>
	</dependency>

Gradle uses the following syntax to specify RTSDK dependencies:

	compile group: 'com.refinitiv.ema', name: 'ema', version: '3.7.2.0'
	compile group: 'com.refinitiv.eta', name: 'eta', version: '3.7.2.0'
	compile group: 'com.refinitiv.eta.valueadd', name: 'etaValueAdd', version: '3.7.2.0'
	compile group: 'com.refinitiv.eta.valueadd.cache', name: 'etaValueAddCache', version: '3.7.2.0'
        compile group: 'com.refinitiv.eta.ansi', name: 'ansipage', version: '3.7.2.0'

# Developing 

If you discover any issues with this project, please feel free to create an Issue.

If you have coding suggestions that you would like to provide for review, please create a Pull Request.

We will review issues and pull requests to determine any appropriate changes.


# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Individual%20Contributor%20License%20Agreement.pdf)
- [Entity Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Entity%20Contributor%20License%20Agreement.pdf)


Please email a signed and scanned copy to sdkagreement@refinitiv.com.  If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.


# Notes:
- For more details on each API, please see the corresponding readme file in their top level directory.
- This section contains APIs that are subject to proprietary and open source licenses.  Please make sure to read the readme files within each API flavor directory for clarification.
- Please make sure to review the LICENSE.md file.
- Java unit tests may use [Mockito](http://site.mockito.org/) for creation of mock objects. Mockito is distributed under the MIT license.
