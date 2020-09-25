///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;


import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.ProgrammaticConfigure.InstanceEntryFlag;

class OmmNiProviderConfigImpl extends EmaConfigImpl implements OmmNiProviderConfig
{
	private int 					_operationModel;
	private int                     _adminControlDirectory;
	
	OmmNiProviderConfigImpl()
	{
		super();
		channelOrChannelSet.add(ConfigManager.NiProviderChannelName);
		channelOrChannelSet.add(ConfigManager.ChannelSet);
		clear();
	}

	OmmNiProviderConfigImpl(String path)
	{
		super(path);
		channelOrChannelSet.add(ConfigManager.NiProviderChannelName);
		channelOrChannelSet.add(ConfigManager.ChannelSet);
		clear();
	}

	@Override
	public OmmNiProviderConfig clear() {
		clearInt();
		_operationModel = OperationModel.API_DISPATCH;
		_adminControlDirectory = AdminControl.API_CONTROL;
		return this;
	}

	@Override
	public OmmNiProviderConfig username(String username) {
		usernameInt(username);
		return this;
	}

	@Override
	public OmmNiProviderConfig password(String password) {
		passwordInt(password);
		return this;
	}

	@Override
	public OmmNiProviderConfig position(String position) {
		positionInt(position);
		return this;
	}

	@Override
	public OmmNiProviderConfig applicationId(String applicationId) {
		applicationIdInt(applicationId);
		return this;
	}

	@Override
	public OmmNiProviderConfig instanceId(String instanceId) {
		instanceIdInt(instanceId);
		return this;
	}

	@Override
	public OmmNiProviderConfig host(String host) {
		hostInt(host, OmmNiProviderActiveConfig.DEFAULT_NIPROVIDER_SERVICE_NAME);
		return this;
	}

	@Override
	public OmmNiProviderConfig operationModel(int operationModel) {
		_operationModel = operationModel;
		return this;
	}

	@Override
	public OmmNiProviderConfig adminControlDirectory(int control) {
		_adminControlDirectory = control;
		return this;
	}

	@Override
	public OmmNiProviderConfig providerName(String providerName) {
		
		// Keep the session name to check later after all configuration methods is called.
		_configSessionName = providerName;
		return this;
	}
	
	void validateSpecifiedSessionName()
	{
		if(_configSessionName == null || _configSessionName.isEmpty())
			return;
		
		if ( _programmaticConfigure != null && _programmaticConfigure.specifyNiProviderName( _configSessionName ) )
			return;

		String name = (String) xmlConfig().getNiProviderAttributeValue(_configSessionName,ConfigManager.NiProviderName);

		if ( name == null ) 
		{
			if ( _configSessionName.equals(ActiveConfig.DEFAULT_NIPROV_NAME) )
			{
				boolean bFoundChild = xmlConfig().isNiProviderChildAvailable();
				if( bFoundChild == false )
					return;
			}

			configStrBuilder().append( "OmmNiProviderConfigImpl::providerName parameter [" )
									.append( _configSessionName )
									.append( "] is an non-existent niprovider name" );
			_configSessionName = null;
			throw ( oommICExcept().message( _configStrBuilder.toString()));
		}
 		else //if ( name != null ) 
		{
 			boolean bSetAttributeValue = xmlConfig().setDefaultNiProvider(name);
			if ( bSetAttributeValue == false )
			{
				xmlConfig().appendAttributeValue(ConfigManager.NIPROVIDER_GROUP, "DefaultNiProvider", ConfigManager.DefaultNiProvider,_configSessionName);
				xmlConfig().verifyAndGetDefaultNiProvider();
			}
		}
	}

	@Override
	public OmmNiProviderConfig config(Data config) {
		configInt(config);
		return this;
	}

	@Override
	public OmmNiProviderConfig addAdminMsg(ReqMsg reqMsg) {
		addAdminMsgInt(reqMsg);
		return this;
	}

	@Override
	public OmmNiProviderConfig addAdminMsg(RefreshMsg refreshMsg) {
		addAdminMsgInt(refreshMsg);
		return this;
	}
	
	@Override
	int operationModel()
	{
		return _operationModel;
	}
	
	int adminControlDirectory()
	{
		return _adminControlDirectory;
	}
	
	@Override
	String configuredName()
	{
		if (_configSessionName != null && !_configSessionName.isEmpty())
			return _configSessionName;
		
		String defaultNiProviderName = null;

		if ( _programmaticConfigure != null  && (defaultNiProviderName = _programmaticConfigure.defaultNiProvider()) != null)
			return defaultNiProviderName;
		
		defaultNiProviderName = xmlConfig().defaultNiProviderName();

		// check if default NiProvider Name and the niprovider name matched
		if ( defaultNiProviderName != null )
		{
			String checkValue = (String) xmlConfig().getNiProviderAttributeValue(defaultNiProviderName,ConfigManager.NiProviderName);

			if ( checkValue != null )
	            return defaultNiProviderName;
			else
			{		
				errorTracker().append( "default NiProvider name [" )
							.append( defaultNiProviderName )
							.append( "] is an non-existent NiProvider name; DefaultNiProvider specification ignored" )
							.create(Severity.ERROR);
				return "EmaNiProvider";
			}
		}
		
		String firstNiProviderName = (String) xmlConfig().getFirstNiProvider();
		if( firstNiProviderName != null )
			return firstNiProviderName;
		
	    return ActiveConfig.DEFAULT_NIPROV_NAME;	
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

		return (String) xmlConfig().getMutualExclusiveAttribute(ConfigManager.NIPROVIDER_LIST, ConfigManager.NiProviderName, instanceName, channelOrChannelSet);
	}
	
	@Override
	String directoryName(String instanceName)
	{
		String directoryName = null;

		if ( _programmaticConfigure != null &&
			(directoryName = _programmaticConfigure.activeEntryNames(instanceName, InstanceEntryFlag.DIRECTORY_FLAG)) != null)
				return directoryName;

		directoryName = (String)xmlConfig().getNiProviderAttributeValue(instanceName,ConfigManager.NiProviderDirectoryName);
		return directoryName;
	}

	@Override
	public int providerRole()
	{
		return OmmProviderConfig.ProviderRole.NON_INTERACTIVE;
	}
	
	@Override
	public OmmNiProviderConfig tunnelingProxyHostName(String proxyHostName)
	{
		tunnelingChannelCfg().httpProxyHostName = proxyHostName;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingProxyPort(String proxyPort)
	{
		tunnelingChannelCfg().httpProxyPort = proxyPort;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingObjectName(String objectName)
	{
		tunnelingChannelCfg().objectName = objectName;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingCredentialUserName(String userName)
	{
		tunnelingChannelCfg().httpProxyUserName = userName;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingCredentialPasswd(String passwd)
	{
		tunnelingChannelCfg().httpproxyPasswd = passwd;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingCredentialDomain(String domain)
	{
		tunnelingChannelCfg().httpProxyDomain = domain;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingCredentialKRB5ConfigFile(String krb5ConfigFile)
	{
		tunnelingChannelCfg().httpProxyKRB5ConfigFile = krb5ConfigFile;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingCredentialLocalHostName(String localHostName)
	{
		tunnelingChannelCfg().httpProxyLocalHostName = localHostName;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingKeyStoreType(String keyStoreType)
	{
		encryptionCfg().KeyStoreType = keyStoreType;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingKeyStoreFile(String keyStoreFile)
	{
		encryptionCfg().KeyStoreFile = keyStoreFile;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingKeyStorePasswd(String keyStorePasswd)
	{
		encryptionCfg().KeyStorePasswd = keyStorePasswd;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingSecurityProtocol(String securityProtocol)
	{
		encryptionCfg().SecurityProtocol = securityProtocol;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingSecurityProvider(String securityProvider)
	{
		encryptionCfg().SecurityProvider = securityProvider;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingKeyManagerAlgorithm(String keyManagerAlgorithm)
	{
		encryptionCfg().KeyManagerAlgorithm = keyManagerAlgorithm;
		return this;
	}

	@Override
	public OmmNiProviderConfig tunnelingTrustManagerAlgorithm(String trustManagerAlgorithm)
	{
		encryptionCfg().TrustManagerAlgorithm = trustManagerAlgorithm;
		return this;
	}
}
