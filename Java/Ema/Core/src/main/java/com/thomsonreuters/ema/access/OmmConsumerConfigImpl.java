///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;

class OmmConsumerConfigImpl extends EmaConfigImpl implements OmmConsumerConfig
{
	private int 				_operationModel;
	
	OmmConsumerConfigImpl()
	{
		super();
		channelOrChannelSet.add(ConfigManager.ConsumerChannelName);
		channelOrChannelSet.add(ConfigManager.ChannelSet);
		clear();
	}
	
	OmmConsumerConfigImpl(String path)
	{
		super(path);
		channelOrChannelSet.add(ConfigManager.ConsumerChannelName);
		channelOrChannelSet.add(ConfigManager.ChannelSet);
		clear();
	}

	@Override
	public OmmConsumerConfig clear()
	{
		clearInt();
		_operationModel = OperationModel.API_DISPATCH;
		return this;
	}

	@Override
	public OmmConsumerConfig username(String username)
	{
		usernameInt(username);
		return this;
	}

	@Override
	public OmmConsumerConfig password(String password)
	{
		passwordInt(password);
		return this;
	}

	@Override
	public OmmConsumerConfig position(String position)
	{
		positionInt(position);
		return this;
	}

	@Override
	public OmmConsumerConfig applicationId(String applicationId)
	{
		applicationIdInt(applicationId);
		return this;
	}
	
	OmmConsumerConfig applicationName(String applicationName)
	{
		applicationNameInt(applicationName);
		return this;
	}

	@Override
	public OmmConsumerConfig host(String host)
	{
		hostInt(host, OmmConsumerActiveConfig.DEFAULT_CONSUMER_SERVICE_NAME);
		return this;
	}

	@Override
	public OmmConsumerConfig operationModel(int operationModel)
	{
		_operationModel = operationModel;
		return this;
	}

	@Override
	public OmmConsumerConfig consumerName(String consumerName)
	{
        // TODO: Retrieve from programmatic configuration

		String name = (String) xmlConfig().getConsumerAttributeValue(consumerName,ConfigManager.ConsumerName);

		if ( name == null ) 
		{
			if ( consumerName.equals("EmaConsumer") )
			{
				boolean bFoundChild = xmlConfig().isConsumerChildAvailable();
				if( bFoundChild == false )
					return this;
			}

			configStrBuilder().append( "OmmConsumerConfigImpl::consumerName parameter [" )
									.append( consumerName )
									.append( "] is an non-existent consumer name" );
			throw ( oommICExcept().message( _configStrBuilder.toString()));
		}
 		else //if ( name != null ) 
		{
 			boolean bSetAttributeValue = xmlConfig().setDefaultConsumer(name);
			if ( bSetAttributeValue == false )
			{
				xmlConfig().appendAttributeValue(ConfigManager.CONSUMER_GROUP, "DefaultConsumer", ConfigManager.DefaultConsumer,consumerName);
				xmlConfig().verifyAndGetDefaultConsumer();
			}
		}
	
		return this;
	}
	
	@Override
	public OmmConsumerConfig config(Data config)
	{
		configInt(config);
		return this;	
	}

	@Override
	public OmmConsumerConfig addAdminMsg(ReqMsg reqMsg)
	{
		addAdminMsgInt(reqMsg);
		return this;
	}
	
	@Override
	int operationModel()
	{
		return _operationModel;
	}
	
	@Override
	String configuredName()
	{
		String defaultConsumerName = null;

		// TODO: Retrieve from programmatic configuration
				
		defaultConsumerName = xmlConfig().defaultConsumerName();

		// check if default consumer Name and the consumer name matched
		if ( defaultConsumerName != null )
		{
			String checkValue = (String) xmlConfig().getConsumerAttributeValue(defaultConsumerName,ConfigManager.ConsumerName);

			if ( checkValue != null )
	            return defaultConsumerName;
			else
			{		
				errorTracker().append( "default consumer name [" )
							.append( defaultConsumerName )
							.append( "] is an non-existent consumer name; DefaultConsumer specification ignored" )
							.create(Severity.ERROR);
				return null;
			}
		}
		
		String firstConsumerName = (String) xmlConfig().getFirstConsumer();
		if( firstConsumerName != null )
			return firstConsumerName;
		
	    return "EmaConsumer";	
	}
	
	@Override
	String channelName(String instanceName)
	{
		String channelName = null;

		// TODO: Retrieve from programmatic configuration
	
		channelName = (String) xmlConfig().getMutualExclusiveAttribute(ConfigManager.CONSUMER_LIST, ConfigManager.ConsumerName, instanceName, channelOrChannelSet);
		return channelName;
	}
	
	String dictionaryName(String instanceName)
	{
		String dictionaryName = null;

		// TODO: Retrieve from programmatic configuration

		dictionaryName = (String) xmlConfig().getConsumerAttributeValue(instanceName,ConfigManager.ConsumerDictionaryName);
		return dictionaryName;
	}

	@Override
	public OmmConsumerConfig tunnelingProxyHostName(String proxyHostName)
	{
		tunnelingChannelCfg().httpProxyHostName = proxyHostName;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingProxyPort(String proxyPort)
	{
		tunnelingChannelCfg().httpProxyPort = proxyPort;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingObjectName(String objectName)
	{
		tunnelingChannelCfg().objectName = objectName;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingCredentialUserName(String userName)
	{
		tunnelingChannelCfg().httpProxyUserName = userName;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingCredentialPasswd(String passwd)
	{
		tunnelingChannelCfg().httpproxyPasswd = passwd;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingCredentialDomain(String domain)
	{
		tunnelingChannelCfg().httpProxyDomain = domain;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingCredentialKRB5ConfigFile(String krb5ConfigFile)
	{
		tunnelingChannelCfg().httpProxyKRB5ConfigFile = krb5ConfigFile;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingCredentialLocalHostName(String localHostName)
	{
		tunnelingChannelCfg().httpProxyLocalHostName = localHostName;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingKeyStoreType(String keyStoreType)
	{
		((EncryptedChannelConfig)tunnelingChannelCfg()).KeyStoreType = keyStoreType;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingKeyStoreFile(String keyStoreFile)
	{
		((EncryptedChannelConfig)tunnelingChannelCfg()).KeyStoreFile = keyStoreFile;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingKeyStorePasswd(String keyStorePasswd)
	{
		((EncryptedChannelConfig)tunnelingChannelCfg()).KeyStorePasswd = keyStorePasswd;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingSecurityProtocol(String securityProtocol)
	{
		((EncryptedChannelConfig)tunnelingChannelCfg()).SecurityProtocol = securityProtocol;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingSecurityProvider(String securityProvider)
	{
		((EncryptedChannelConfig)tunnelingChannelCfg()).SecurityProvider = securityProvider;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingKeyManagerAlgorithm(String KeyManagerAlgorithm)
	{
		((EncryptedChannelConfig)tunnelingChannelCfg()).KeyManagerAlgorithm = KeyManagerAlgorithm;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingTrustManagerAlgorithm(String trustManagerAlgorithm)
	{
		((EncryptedChannelConfig)tunnelingChannelCfg()).TrustManagerAlgorithm = trustManagerAlgorithm;
		return this;
	}
	
}