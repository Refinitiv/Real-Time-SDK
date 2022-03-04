///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.ProgrammaticConfigure.InstanceEntryFlag;

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
	public OmmConsumerConfig clientId(String clientId)
	{
		clientIdInt(clientId);
		return this;
	}
	
	@Override
	public OmmConsumerConfig clientSecret(String clientSecret) 
	{
		clientSecretInt(clientSecret);
		return this;
	}

	@Override
	public OmmConsumerConfig tokenScope(String tokenScope)
	{
		tokenScopeInt(tokenScope);
		return this;
	}

	@Override
	public OmmConsumerConfig tokenServiceUrl(String tokenServiceUrl)
	{
		tokenServiceUrlInt(tokenServiceUrl);
		return this;
	}
	
	@Override
	public OmmConsumerConfig tokenServiceUrlV1(String tokenServiceUrlV1)
	{
		tokenServiceUrlV1Int(tokenServiceUrlV1);
		return this;
	}
	
	@Override
	public OmmConsumerConfig tokenServiceUrlV2(String tokenServiceUrlV2)
	{
		tokenServiceUrlV2Int(tokenServiceUrlV2);
		return this;
	}

	@Override
	public OmmConsumerConfig serviceDiscoveryUrl(String serviceDiscoveryUrl)
	{
		serviceDiscoveryUrlInt(serviceDiscoveryUrl);
		return this;
	}
	
	@Override
	public OmmConsumerConfig takeExclusiveSignOnControl(boolean takeExclusiveSignOnControl) 
	{
		takeExclusiveSignOnControlInt(takeExclusiveSignOnControl);
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
		// Keep the session name to check later after all configuration methods is called.
		_configSessionName = consumerName;
		return this;
	}
	
	void validateSpecifiedSessionName()
	{
		if(_configSessionName == null || _configSessionName.isEmpty())
			return;
		
		if ( _programmaticConfigure != null && _programmaticConfigure.specifyConsumerName( _configSessionName ) )
			return;

		String name = (String) xmlConfig().getConsumerAttributeValue(_configSessionName, ConfigManager.ConsumerName);

		if ( name == null ) 
		{
			if ( _configSessionName.equals(ActiveConfig.DEFAULT_CONS_NAME) )
			{
				boolean bFoundChild = xmlConfig().isConsumerChildAvailable();
				if( bFoundChild == false )
					return;
			}

			configStrBuilder().append( "OmmConsumerConfigImpl::consumerName parameter [" )
									.append( _configSessionName )
									.append( "] is an non-existent consumer name" );
			_configSessionName = null;
			throw ( oommICExcept().message( _configStrBuilder.toString()));
		}
 		else //if ( name != null ) 
		{
 			boolean bSetAttributeValue = xmlConfig().setDefaultConsumer(name);
			if ( bSetAttributeValue == false )
			{
				xmlConfig().appendAttributeValue(ConfigManager.CONSUMER_GROUP, "DefaultConsumer", ConfigManager.DefaultConsumer,_configSessionName);
				xmlConfig().verifyAndGetDefaultConsumer();
			}
		}
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
		if (_configSessionName != null && !_configSessionName.isEmpty())
			return _configSessionName;
		
		String defaultConsumerName = null;

		if ( _programmaticConfigure != null  && (defaultConsumerName = _programmaticConfigure.defaultConsumer()) != null)
			return defaultConsumerName;
		
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
		
	    return ActiveConfig.DEFAULT_CONS_NAME;	
	}
	
	@Override
	String channelName(String instanceName)
	{
		String channelName = null;

		if ( _programmaticConfigure != null )
		{
			channelName = _programmaticConfigure.activeEntryNames(instanceName, InstanceEntryFlag.CHANNEL_FLAG);
			if (channelName != null)
				return channelName;
			channelName = _programmaticConfigure.activeEntryNames(instanceName, InstanceEntryFlag.CHANNELSET_FLAG);
			if (channelName != null)
				return channelName;
		}
	
		channelName = (String) xmlConfig().getMutualExclusiveAttribute(ConfigManager.CONSUMER_LIST, ConfigManager.ConsumerName, instanceName, channelOrChannelSet);
		return channelName;
	}
	
	String dictionaryName(String instanceName)
	{
		String dictionaryName = null;

		if ( _programmaticConfigure != null )
		{
			dictionaryName = _programmaticConfigure.activeEntryNames(instanceName, InstanceEntryFlag.DICTIONARY_FLAG); 
			if (dictionaryName != null)
				return dictionaryName;
		}

		return (String) xmlConfig().getConsumerAttributeValue(instanceName,ConfigManager.ConsumerDictionaryName);
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
		encryptionCfg().KeyStoreType = keyStoreType;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingKeyStoreFile(String keyStoreFile)
	{
		encryptionCfg().KeyStoreFile = keyStoreFile;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingKeyStorePasswd(String keyStorePasswd)
	{
		encryptionCfg().KeyStorePasswd = keyStorePasswd;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingSecurityProtocol(String securityProtocol)
	{
		encryptionCfg().SecurityProtocol = securityProtocol;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingSecurityProvider(String securityProvider)
	{
		encryptionCfg().SecurityProvider = securityProvider;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingKeyManagerAlgorithm(String KeyManagerAlgorithm)
	{
		encryptionCfg().KeyManagerAlgorithm = KeyManagerAlgorithm;
		return this;
	}

	@Override
	public OmmConsumerConfig tunnelingTrustManagerAlgorithm(String trustManagerAlgorithm)
	{
		encryptionCfg().TrustManagerAlgorithm = trustManagerAlgorithm;
		return this;
	}
}