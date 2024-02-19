/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2023 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

#include "cstdio"
#include "Access/Include/EmaString.h"

#ifndef __refinitiv_ema_access_ServiceEndpointDiscoveryConfig_h
#define __refinitiv_ema_access_ServiceEndpointDiscoveryConfig_h

namespace refinitiv {

namespace ema {

namespace access {

class EMA_ACCESS_API ServiceEndpointDiscoveryConfig
{
public:

	///@name Constructor
	//@{
	/** Create an ServiceEndpointDiscoveryConfig that enables customization of default configurations.
	*/
	ServiceEndpointDiscoveryConfig();

	///@name Operations
	//@{
	/** Clears the ServiceEndpointDiscoveryConfig and sets all the defaults.
	\remark Invoking clear() method clears all the values and resets all the defaults
	@return reference to this object
	*/
	void clear();

	/** Specifies an URL to override the default for token service V1 Password Credentials to perform authentication to get access and refresh tokens.
	@param[in] tokenServiceUrl specifies an URL for token service.
	@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& tokenServiceUrlV1(const EmaString& tokenServiceUrl = "");

	/** Specifies an URL to override the default for token service V2 Client Credentials to perform authentication to get access and refresh tokens.
		@param[in] tokenServiceUrl specifies an URL for token service.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& tokenServiceUrlV2(const EmaString& tokenServiceUrl = "");

	/** Specifies an URL to override the default for the RDP service discovery to get global endpoints
		@param[in] serviceDiscoveryUrl specifies an URL for RDP service discovery.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& serviceDiscoveryUrl(const EmaString& serviceDiscoveryUrl = "");

	/** Specifies file output location for REST Logs. If NULL, sends logs to stdout.
	@param[in] restLogOutputStream specifies the file output location for REST Logs. Defaults to NULL.
	@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& restLogOutputStream(FILE* restLogOutputStream = NULL);

	/** Specifies whether to enable or disable REST logging. If set to true, REST logging is enabled. Defaults to false.
	@param[in] restEnableLog if true, enables REST logging.
	@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& restEnableLog(bool restEnableLog = false);

	/** Specifies whether to enable or disable verbose REST logging. If set to true, verbose REST logging is enabled. Defaults to false.
	@param[in] restVerboseMode if true, enables verbose REST logging.
	@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& restVerboseMode(bool restVerboseMode = false);

	/** Specifies a name to override the default for the openSSL libssl shared library. The RSSL API will attempt to dynamically load this library for encrypted connections.
		@param[in] libsslName specifies a name of the openSSL libssl shared library.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& libSslName(const EmaString& libsslName = "");

	/** Specifies a name to override the default for the openSSL libcrypto shared library. The RSSL API will attempt to dynamically load this library for encrypted connections.
		@param[in] libcryptoName specifies a name of the openSSL libcrypto shared library.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& libCryptoName(const EmaString& libcryptoName = "");

	/** Specifies a name to override the default for the curl shared library. The RSSL API will attempt to dynamically load this library for proxy connections.
		@param[in] libcurlName specifies a name of the curl shared library.
		@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& libCurlName(const EmaString& libcurlName = "");

	/** Specifies whether to enable or disable CPUID library initialization. If set to true, CPUID library is enabled. Defaults to true.
	@param[in] shouldInitializeCPUIDlib if true, enables CPUID library.
	@return reference to this object
	*/
	ServiceEndpointDiscoveryConfig& shouldInitializeCPUIDlib(bool shouldInitializeCPUIDlib = true);

private:

	friend class ServiceEndpointDiscoveryImpl;

	const EmaString* _tokenServiceURL_V1; // Defaults to empty
	const EmaString* _tokenServiceURL_V2; // Defaults to empty
	const EmaString* _serviceDiscoveryURL;// Defaults to empty
	FILE* _restLogOutputStreamFile;		 // Defaults to NULL to send logs to stdout
	bool _restEnableLog;			 // Defaults to false
	bool _restVerboseMode;			 // Defaults to false

	const EmaString* _libsslName;		// Defaults to empty
	const EmaString* _libcryptoName;		// Defaults to empty
	const EmaString* _libcurlName;		// Defaults to empty
	bool _shouldInitializeCPUIDlib;		// Defaults to true
};

}

}

}

#endif // __refinitiv_ema_access_ServiceEndpointDiscoveryConfig_h
