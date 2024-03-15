/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|        Copyright (C) 2024 Refinitiv. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_EmaConfig_h
#define __refinitiv_ema_access_EmaConfig_h

namespace refinitiv {

namespace ema {

namespace access {

/** This class contains enumerations and other configuration values used in the EMA configuration classes.
*/
class EmaConfig
{
public:
	/** @enum This enumeration defines the connection types available for the Channel ConnectionType configuration elements.
	*/
	enum class ConnectionTypeEnum
	{
		SOCKET = 0,                   /*!< This connection type uses the TCP-based ETA Socket connection protocol. */
		ENCRYPTED = 1,                /*!< This connection type uses a TLS encrypted connection.  Additional protocol configuration is defined in EncryptedProtocolTypeEnum. */
		HTTP = 2,                     /*!< This connection type uses HTTP TCP-based socket connection protocol. */
		WEBSOCKET = 7                 /*!< This connection type uses TCP-based WebSocket connection protocol. */
	};

	/** @enum This enumeration defines the protocol type available for ENCRYPTED ConnectionType.
	*/
	enum class EncryptedProtocolTypeEnum
	{
		SOCKET = 0,                 /*!< This encrypted protocol type uses the TCP-based ETA Socket encrypted connection protocol. */
		HTTP = 2,                   /*!< This encrypted protocol type uses HTTP TCP-based socket encrypted connection protocol. */
		WEBSOCKET = 7               /*!< This encrypted protocol type uses TCP-based WebSocket encrypted connection protocol. */
	};
};

}
}
}

#endif // __refinitiv_ema_access_EmaConfig_h