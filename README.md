# Refinitiv Real-Time SDK

This is the Refinitiv Real-Time SDK (RTSDK). This SDK encompasses these open source Real-Time APIs: Enterprise Message API (EMA) and Enterprise Transport API (ETA).

The **Enterprise Message API (EMA)** is an ease of use, open source, OMM API. EMA is designed to provide clients rapid development of applications, minimizing lines of code and providing a broad range of flexibility. It provides flexible configuration with default values to simplify use and deployment.  EMA is written on top of the Enterprise Transport API (ETA) utilizing the Value Added Reactor and Watchlist. 

The **Enterprise Transport API (ETA)** is an open source Refinitiv low-level Transport and OMM encoder/decoder API. It is used by the Refinitiv Real-Time Distribution Systems and Refinitiv Real-Time for the optimal distribution of OMM/RWF data and allows applications to achieve the highest performance, highest throughput, and lowest latency. ETA fully supports all OMM constructs and messages.

The RTSDK was formerly known as Elektron SDK (ESDK). Starting with version 2.0 (ETA & EMA 3.6), SDK was rebranded with the following impact: namespace changes and jar files names. This applies to applications written to EMA C++, EMA Java and ETA Java. If upgrading from a version prior to RTSDK 2.0, please alter code, re-compile and redeploy applications to pick up these changes. Please note that connectivity to Refinitiv products will not be impacted after rebranding and existing applications will continue to work. For more details, please see the following:

- For specific details on impact of changes to applications with RTSDK 2.0, please see [REBRAND.md](REBRAND.md).
- For general details regarding impact of rebranding, support questions, and timelines with RTSDK and other APIs, please see the [API Product Change Notification (PCN)](https://my.refinitiv.com/content/mytr/en/pcnpage/12072.html). 


# Supported Languages, Platforms and Compilers

The Refinitiv Real-Time-SDK will support multiple languages across different combinations of `Linux` and `Windows` and their corresponding compilers. Navigate to the language and API of your choice to see the individual API README.md files for further details on building and running the API and examples. You can click on the below links to take you to the language of your choice.

- [C++](Cpp-C)
- [Java](Java)
- [C#](CSharp)


# Documentation
Documentation is available in **PDF format** on GitHub and Refinitiv Developer Portal. 

GitHub PDF format docs per API:

- [Enterprise Message API - C++ Edition](Cpp-C/Ema/Docs)
- [Enterprise Transport API - C Edition](Cpp-C/Eta/Docs)
- [Enterprise Message API - Java Edition](Java/Ema/Docs)
- [Enterprise Transport API - Java Edition](Java/Eta/Docs)
- [Enterprise Transport API - CSharp Edition](CSharp/Eta/Docs)

Refinitiv Developer Portal documentation section contains docs in PDF format for both C++ and Java:

- [C++/C](https://developers.refinitiv.com/en/api-catalog/refinitiv-real-time-opnsrc/rt-sdk-cc/documentation)
- [Java](https://developers.refinitiv.com/en/api-catalog/refinitiv-real-time-opnsrc/rt-sdk-java/documentation)
- [CSharp](https://developers.refinitiv.com/en/api-catalog/refinitiv-real-time-opnsrc/refinitiv-real-time-csharp-sdk/documentation)

Documentation is also available in **HTML format** on a package or Refinitiv Developer Portal. 

Packages are available for download in Developer Portal. If viewing docs included with a locally installed package, please consider hosting the HTML docs in an internal portal, using Internet Explorer, or, modifying security settings with Firefox, etc., to do so. HTML documentation is also available on Developer Portal for download in documentation sections.


# Developing 
If you discover any issues with this project, please feel free to create an Issue.
If you have coding suggestions that you would like to provide for review, please create a Pull Request.
We will review issues and pull requests to determine any appropriate changes.

# Contributing
In the event you would like to contribute to this repository, it is required that you read and sign the following:

- [Individual Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Individual%20Contributor%20License%20Agreement.pdf)
- [Entity Contributor License Agreement](https://github.com/Refinitiv/Real-Time-SDK/blob/master/Refinitiv%20Real-Time%20API%20Entity%20Contributor%20License%20Agreement.pdf)

Please email a signed and scanned copy to `sdkagreement@refinitiv.com`.  If you require that a signed agreement has to be physically mailed to us, please email the request for a mailing address and we will get back to you on where you can send the signed documents.

# License Information

#### Open Source License Information

License details can be found in the LICENSE.md file contained in this section. The included code is governed by the Apache License, Version 2.0. This applies only to the software provided in the following locations:

- Cpp-C/Ema/Src
- Cpp-C/Ema/Examples
- Cpp-C/Ema/TestTools
- Cpp-C/Eta/Impl
- Cpp-C/Eta/Applications
- Cpp-C/Eta/Include
- Cpp-C/Eta/TestTools
- Java/Ema/Core
- Java/Ema/Examples
- Java/Ema/PerfTools
- Java/Ema/TestTools
- Java/Eta/Core
- Java/Eta/Applications
- Java/Eta/TestTools
- Java/Eta/ValueAdd
- Java/Eta/ValueAddCache
- CSharp/Eta/Src/Core
- CSharp/Eta/Src/ValueAdd
- CSharp/Eta/Applications
- CSharp/Eta/TestTools

Any source code, header files not specified above (even if included by header files in the locations above), libraries, and underlying dependencies continue to be governed by the licensing and agreements per the [MyRefinitiv](https://my.refinitiv.com/content/mytr/en/signin.html) site and RDC Program.


# Notes:
- This section contains APIs that are subject to proprietary and open source licenses.  Please make sure to read the readme files within each API flavor directory for clarification.
- Please make sure to review the LICENSE.md file.

# Support SLA

Issues raised via GitHub will be addressed in a best-effort manner. For broad questions regarding RTSDK, please refer to documentation (see Documentation section above) and [Q&A forum](https://community.developers.refinitiv.com/index.html) on Developer Community which supported by an active community of API users. Please contact Premium Support for any issues or questions that require prompt responses.
