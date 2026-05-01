/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.transport.ConnectionTypes;

class EncryptedChannelConfig extends HttpChannelConfig
{
	EncryptedChannelConfig(int rsslConnectionType)
	{
		clear();
		this.rsslConnectionType = rsslConnectionType;

		if (rsslConnectionType == ConnectionTypes.SOCKET)
		{
			clearSocketChannelConfig();
		}
		else if (rsslConnectionType == ConnectionTypes.WEBSOCKET)
		{
			clearSocketChannelConfig();
			this.rsslConnectionType = rsslConnectionType;
		}
		else if (rsslConnectionType == ConnectionTypes.HTTP)
		{
			clearHttpChannelConfig();
		}
	}

	@Override
	void clear()
	{
		super.clear();
		if(encryptionConfig != null)
		{
			encryptionConfig.clear();
		}
		// Override the default value for hostname and port as
		// the Reactor can query them from LDP service discovery if not specified.
		hostName = "";
		serviceName = "";
		rsslConnectionType = ConnectionTypes.ENCRYPTED;
	}
}
