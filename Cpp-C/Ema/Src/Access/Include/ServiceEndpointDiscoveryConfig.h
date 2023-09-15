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

private:

	friend class ServiceEndpointDiscoveryImpl;

	const EmaString* tokenServiceURL_V1; // Defaults to empty
	const EmaString* tokenServiceURL_V2; // Defaults to empty
	const EmaString* serviceDiscoveryURL;// Defaults to empty
	FILE* restLogOutputStreamFile;		 // Defaults to NULL to send logs to stdout
	bool restEnableLogValue;			 // Defaults to false
};

}

}

}

#endif // __refinitiv_ema_access_ServiceEndpointDiscoveryConfig_h
