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
