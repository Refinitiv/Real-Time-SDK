///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.transport.ConnectionTypes;

class EncryptedChannelConfig extends HttpChannelConfig
{
	/* Additional configuration parameters for ENCRYPTED connection type*/ 
	String				KeyStoreType;
	String				KeyStoreFile;
	String				KeyStorePasswd;
	String				SecurityProvider;
	String 				SecurityProtocol;		
	String				KeyManagerAlgorithm;
	String				TrustManagerAlgorithm;
	String				location;
	boolean				enableSessionMgnt;
	
	EncryptedChannelConfig()
	{
		clear();
	}

	@Override
	void clear() 
	{
		super.clear();
		
		// Override the default value for hostname and port as
		// the Reactor can query them from EDP-RT service discovery if not specified.
		hostName = "";
		serviceName = "";
		rsslConnectionType = ConnectionTypes.ENCRYPTED;
		location = ActiveConfig.DEFAULT_REGION_LOCATION;
		enableSessionMgnt = ActiveConfig.DEFAULT_ENABLE_SESSION_MGNT;
	}
}
