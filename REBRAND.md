# Refinitiv Real Time SDK Customer Impact Changes

1.   [Overview](#RefinitivRealTimeSDKCustomerImpactChanges-Overview)
2.   [Namespace changes](#RefinitivRealTimeSDKCustomerImpactChanges-Namespacechanges)
    1.   [Customer application Impact:  ETA Java Edition](#RefinitivRealTimeSDKCustomerImpactChanges-CustomerapplicationImpact:ETAJavaEdition)
    2.   [Customer application Impact:  EMA Java Edition](#RefinitivRealTimeSDKCustomerImpactChanges-CustomerapplicationImpact:EMAJavaEdition)
    3.   [Customer application Impact:  EMA C++ Edition](#RefinitivRealTimeSDKCustomerImpactChanges-CustomerapplicationImpact:EMAC++Edition)
3.   [Library name changes](#RefinitivRealTimeSDKCustomerImpactChanges-Librarynamechanges)
4.   [Maven Central location](#RefinitivRealTimeSDKCustomerImpactChanges-MavenCentrallocation)
5.   [Changes to Build](#RefinitivRealTimeSDKCustomerImpactChanges-ChangestoBuild)
6.   [Other Non-Impactful Changes](#RefinitivRealTimeSDKCustomerImpactChanges-OtherChanges)
7.   [Documentation ONLY Changes: Re-branded Product Names](#RefinitivRealTimeSDKCustomerImpactChanges-DocumentationChanges:Re-brandedProductNames)

<a name="RefinitivRealTimeSDKCustomerImpactChanges-Overview"></a>
## Overview

The purpose of this document is to provide a guide for customers to understand impact to applications written to Refinitiv Real-Time SDK or RTSDK (formerly Elektron SDK or ESDK) due to rebranding to Refinitiv. Starting with release Refinitiv Real-Time SDK version 2.X (ETA & EMA 3.6.X), rebranding impacts customers in the following manner: **namespace** changes, **library name** changes, location of libraries in **Maven Central**, and changes to **building** libraries or examples. These changes impact applications written to EMA C++ (EMACPP), EMA Java (EMAJ) and ETA Java (ETAJ). Customers of impacted applications written to ESDK/RTSDK will be required to alter code, re-compile and redeploy applications in order to adapt to changes.

Please note that **connectivity** to Refinitiv products **will not** be impacted after rebranding and existing applications will continue to work. However, all future fixes will be made only on re-branded releases. Post July 2021, non-rebranded revisions will no longer be available for download for licensing reasons. We will continue to offer support for non-re-branded versions of the API that includes investigating reported issues, analysis to identify and reproduce the problem and recommend workarounds. Please note that a workaround may not always be possible or feasible. Therefore, we recommend moving to re-branded versions of API.

In addition to above mentioned impacts, this document also provides a list of products that have been renamed and referenced within the SDK documentation.

<a name="RefinitivRealTimeSDKCustomerImpactChanges-Namespacechanges"></a>
## Namespace changes

ACTION:  Applications must be changed to new references, recompile & redeploy. 

### Customer application Impact: ETA Java Edition <a name="RefinitivRealTimeSDKCustomerImpactChanges-CustomerapplicationImpact:ETAJavaEdition"></a>

Generally, all applications written to EMA C++ should replace EVERY reference to "com.thomsonreuters.upa" with "com.refinitiv.eta". Here are **sample** code snippets:

|Prior to Re-brand Reference|Re-branded Reference|
|--------------------------------------------------|--------------------------------------------------|
|import com.thomsonreuters.upa.dacs.\*;|import com.refinitiv.eta.dacs.\*;|
|import com.thomsonreuters.upa.codec.CodecFactory;|import com.refinitiv.eta.codec.CodecFactory;|
|com.thomsonreuters.upa.transport.Error error|com.refinitiv.eta.transport.Error error|
|private com.thomsonreuters.upa.codec.Enum|private com.refinitiv.eta.codec.Enum|
|com.thomsonreuters.upa.transport.Error|com.refinitiv.eta.transport.Error|

### Customer application Impact: EMA Java Edition <a name="RefinitivRealTimeSDKCustomerImpactChanges-CustomerapplicationImpact:EMAJavaEdition"></a>

Generally, all applications written to EMA Java should replace EVERY reference to "thomsonreuters" with "refinitiv". Here are **sample** code snippets:

|Prior to Re-brand Reference|Re-branded Reference|
|--------------------------------------------------|--------------------------------------------------|
|import com.thomsonreuters.ema.access.Msg;|import com.refinitiv.ema.access.Msg;|

### Customer application Impact: EMA C++ Edition <a name="RefinitivRealTimeSDKCustomerImpactChanges-CustomerapplicationImpact:EMAC++Edition"></a>

Generally, all applications written to EMA C++ should replace EVERY reference to "thomsonreuters::ema" with "refinitiv::ema". Here are **sample** code snippets:

|Prior to Re-brand Reference|Re-branded Reference|
|--------------------------------------------------|--------------------------------------------------|
|using namespace thomsonreuters::ema::|using namespace refinitiv::ema::|
|const thomsonreuters::ema::access|const refinitiv::ema::access|
|thomsonreuters::ema::access::UInt64|refinitiv::ema::access::UInt64|
|thomsonreuters::ema::rdm::MMT\_MARKET\_PRICE|refinitiv::ema::rdm::MMT\_MARKET\_PRICE|

## Library name changes <a name="RefinitivRealTimeSDKCustomerImpactChanges-Librarynamechanges"></a>

ACTION:  If Applications are using deprecated interfaces, please move to newer interfaces and recompile. Otherwise, these library may be "dropped" in.

|API Flavor|Prior to Re-brand Library|Re-branded Library|
|-------------------------------------|-------------------------------------|-------------------------------------|
|ETAJ|upa.jar|eta.jar|
|ETAJ|upaValueAdd-\<version\>.jar|etaValueAdd-\<version\>.jar|
|ETAJ|upaValueAddCache-\<version\>.jar|etaValueAddCache-\<version\>.jar|
|ETAJ|jdacsUpalib.jar|jdacsEtalib.jar|

## Maven Central location <a name="RefinitivRealTimeSDKCustomerImpactChanges-MavenCentrallocation"></a>

ACTION: Due to namespace changes, the libraries will be available in a new space.

|API Flavor|Prior to Re-brand Location|Re-branded Location|
|-------------------------------------|-------------------------------------|-------------------------------------|
|ETAJ, EMAJ|com.thomsonreuters|com.refinitiv|

## Changes to Build <a name="RefinitivRealTimeSDKCustomerImpactChanges-ChangestoBuild"></a>

Description:  This applies to applications that make use of SDK's CMake

|Prior to Re-brand Reference|Re-branded Reference|Comment|
|-------------------------------------|-------------------------------------|-------------------------------------|
|esdk|rtsdk|solution name and cmake references to project|
|BUILD-ELEKTRON-SDK-BINARYPACK|BUILD\_RTSDK-BINARYPACK|build option|


## Other Non-Impactful Changes <a name="RefinitivRealTimeSDKCustomerImpactChanges-OtherChanges"></a>

|API Flavor|Prior to Re-brand Reference|Re-branded Reference|Comment|
|-------------------------|------------------|------------------|-----------------------------------------------------------|
|ETAC, ETAJ, EMAJ, EMACPP|\_UPA\_ITEM\_LIST|\_ETA\_ITEM\_LIST|Symbol list name used in direct connect scenario|
|EMACPP|DEFAULT\_EDP\_RT\_LOCATION|DEFAULT\_RDP\_RT\_LOCATION||


## Documentation ONLY Changes: Re-branded Product Names <a name="RefinitivRealTimeSDKCustomerImpactChanges-DocumentationChanges:Re-brandedProductNames"></a>

The following list is a sample of Refinitiv product name changes which are referenced in Refinitiv Real-Time SDK documentation. Please note that this is NOT a comprehensive list of all product names. These changes have no impact to application development.

|Prior to Re-branding|After Re-branding|
|--------------------------------------------------|--------------------------------------------------|
|ADS|Refinitiv Real-Time Advanced Distribution Server|
|ADH|Refinitiv Real-Time Advanced Data Hub|
|ATS|Refinitiv Real-Time Advanced Transformation Server|
|ADSPOP|Refinitiv Real-Time Advanced Distribution Broker|
|DACS|Refinitiv Data Access Control System|
|EDP|Refinitiv Data Platform (RDP)|
|Elektron|Refinitiv Real-Time|
|Elektron SDK|Refinitiv Real-Time SDK|
|Elektron Real Time in Cloud|Refinitiv Real-Time - Optimized|
|EMA|Enterprise Message API|
|ESDK|RTSDK|
|ETA|Enterprise Transport API|
|ERT Cloud|Refinitiv Real-Time - Optimized|
|RDF-D|Refinitiv Data Feed Direct (RDF-D)|
|RMDS|Refinitiv Real-Time Distribution System|
|RMTES|Refinitiv Multilingual Text Encoding Standard|
|RSSL|Refinitiv Source Sink Library|
|RWF|Refinitiv Wire Format|
|TREP|Refinitiv Real-Time Distribution System|
|TREP Authentication|UserAuthn Authentication, Authentication, Auth|
|UPA|ETA|
|Websocket API|Websocket API for Pricing Streaming and Real-Time Services|
