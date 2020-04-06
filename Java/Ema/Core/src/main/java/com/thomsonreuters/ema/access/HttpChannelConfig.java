package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.transport.ConnectionTypes;

class HttpChannelConfig extends ChannelConfig
{
	String				hostName;
	String				serviceName;
	String				objectName;
	Boolean				tcpNodelay;
	Boolean 			httpProxy;
	String 				httpProxyHostName;
	String 				httpProxyPort;
	
	/* Credential configuration parameters */
	String				httpProxyUserName;
	String				httpproxyPasswd;
	String				httpProxyDomain;
	String 				httpProxyLocalHostName;
	String				httpProxyKRB5ConfigFile;
	
	HttpChannelConfig()
	{
		clear();
	}

	@Override
	void clear() 
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.HTTP;
		hostName = ActiveConfig.DEFAULT_HOST_NAME;
		serviceName = ActiveConfig.defaultServiceName;
		tcpNodelay = ActiveConfig.DEFAULT_TCP_NODELAY;
		objectName = ActiveConfig.DEFAULT_OBJECT_NAME;
		httpProxy = ActiveConfig.DEFAULT_HTTP_PROXY;
	}
}
