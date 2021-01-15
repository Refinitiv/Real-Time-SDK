///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.ProgrammaticConfigure.InstanceEntryFlag;

class OmmIProviderConfigImpl extends EmaConfigServerImpl implements OmmIProviderConfig
{
	
	private int _operationModel;
	private int _adminControlDirectory;
	private int _adminControlDictionary;
	private String _keystoreFile;
	private String _keystorePasswd;
	private String _keystoreType;
	private String _securityProtocol;
	private String _securityProvider;
	private String _keyManagerAlgorithm;
	private String _trustManagerAlgorithm;

	OmmIProviderConfigImpl()
	{
		clear();
	}

	OmmIProviderConfigImpl(String path)
	{
		super(path);
		clear();
	}
	
	@Override
	int operationModel()
	{
		return _operationModel;
	}

	@Override
	public int providerRole()
	{
		return OmmProviderConfig.ProviderRole.INTERACTIVE;
	}

	@Override
	public OmmIProviderConfig clear()
	{
		super.clearInt();
		_operationModel = OperationModel.API_DISPATCH;
		_adminControlDirectory = AdminControl.API_CONTROL;
		_adminControlDictionary = AdminControl.API_CONTROL;
		_keystoreFile = null;
		_keystorePasswd = null;
		_securityProtocol = null;
		_securityProvider = null;
		_keyManagerAlgorithm = null;
		_trustManagerAlgorithm = null;
		return this;
	}

	@Override
	public OmmIProviderConfig port(String port)
	{
		super.portInt(port, OmmIProviderActiveConfig.DEFAULT_SERVICE_NAME);
		return this;
	}

	@Override
	public OmmIProviderConfig operationModel(int operationModel)
	{
		_operationModel = operationModel;
		return this;
	}

	@Override
	public OmmIProviderConfig adminControlDirectory(int adminControlDirectory)
	{
		_adminControlDirectory = adminControlDirectory;
		return this;
	}

	@Override
	public OmmIProviderConfig adminControlDictionary(int adminControlDictionary)
	{
		_adminControlDictionary = adminControlDictionary;
		return this;
	}
	
	int adminControlDirectory()
	{
		return _adminControlDirectory;
	}
	
	int adminControlDictionary()
	{
		return _adminControlDictionary;
	}

	@Override
	public OmmIProviderConfig providerName(String providerName)
	{
		// Keep the session name to check later after all configuration methods is called.
		_configSessionName = providerName;
	
		return this;
	}
	
	void validateSpecifiedSessionName()
	{
		if(_configSessionName == null || _configSessionName.isEmpty())
			return;
		
		if ( _programmaticConfigure != null && _programmaticConfigure.specifyIProviderName( _configSessionName ) )
			return;
		
		String name = (String) xmlConfig().getIProviderAttributeValue(_configSessionName,ConfigManager.IProviderName);

		if ( name == null ) 
		{
			if ( _configSessionName.equals(ActiveConfig.DEFAULT_IPROV_NAME) )
			{
				boolean bFoundChild = xmlConfig().isNiProviderChildAvailable();
				if( bFoundChild == false )
					return;
			}

			configStrBuilder().append( "OmmIProviderConfigImpl::providerName parameter [" )
									.append( _configSessionName )
									.append( "] is an non-existent iprovider name" );
			_configSessionName = null;
			throw ( oommICExcept().message( _configStrBuilder.toString()));
		}
 		else //if ( name != null ) 
		{
 			boolean bSetAttributeValue = xmlConfig().setDefaultIProvider(name);
			if ( bSetAttributeValue == false )
			{
				xmlConfig().appendAttributeValue(ConfigManager.IPROVIDER_GROUP, "DefaultIProvider", ConfigManager.DefaultIProvider,_configSessionName);
				xmlConfig().verifyAndGetDefaultIProvider();
			}
		}
	}
	
	@Override
	String configuredName()
	{
		if (_configSessionName != null && !_configSessionName.isEmpty())
			return _configSessionName;

		String defaultIProviderName = null;

		if ( _programmaticConfigure != null  && (defaultIProviderName = _programmaticConfigure.defaultIProvider()) != null)
			return defaultIProviderName;
		
		defaultIProviderName = xmlConfig().defaultIProviderName();

		// check if default IProvider Name and the IProvider name matched
		if ( defaultIProviderName != null )
		{
			String checkValue = (String) xmlConfig().getIProviderAttributeValue(defaultIProviderName,ConfigManager.IProviderName);

			if ( checkValue != null )
	            return defaultIProviderName;
			else
			{		
				errorTracker().append( "default IProvider name [" )
							.append( defaultIProviderName )
							.append( "] is an non-existent IProvider name; DefaultIProvider specification ignored" )
							.create(Severity.ERROR);
				return "EmaIProvider";
			}
		}
		
		String firstIProviderName = (String) xmlConfig().getFirstIProvider();
		if( firstIProviderName != null )
			return firstIProviderName;
		
	    return ActiveConfig.DEFAULT_IPROV_NAME;	
	}

	@Override
	String directoryName(String instanceName)
	{
		String directoryName = null;

		if ( _programmaticConfigure != null &&
			(directoryName = _programmaticConfigure.activeEntryNames(instanceName, InstanceEntryFlag.DIRECTORY_FLAG)) != null)
				return directoryName;

		directoryName = (String)xmlConfig().getIProviderAttributeValue(instanceName,ConfigManager.IProviderDirectoryName);
		return directoryName;
	}
	
	@Override
	public OmmIProviderConfig config(Data config)
	{
		configInt(config);
		return this;
	}

	@Override
	public OmmIProviderConfig addAdminMsg(RefreshMsg refreshMsg)
	{
		addAdminMsgInt(refreshMsg);
		return this;
	}
	
	@Override
	public OmmIProviderConfig keystoreFile(String keystoreFile)
	{
		_keystoreFile = keystoreFile;
		return this;
	}
	
	public String keystoreFile()
	{
		return _keystoreFile;
	}
	
	@Override
	public OmmIProviderConfig keystorePasswd(String keystorePasswd)
	{
		_keystorePasswd = keystorePasswd;
		return this;
	}
	
	public String keystorePasswd()
	{
		return _keystorePasswd;
	}
	
	@Override
	public OmmIProviderConfig keystoreType(String keystoreType)
	{
		_keystoreType = keystoreType;
		return this;
	}
	
	public String keystoreType()
	{
		return _keystoreType;
	}
	
	
	@Override
	public OmmIProviderConfig securityProtocol(String securityProtocol)
	{
		_securityProtocol = securityProtocol;
		return this;
	}
	
	public String securityProtocol()
	{
		return _securityProtocol;
	}
	
	@Override
	public OmmIProviderConfig securityProvider(String securityProvider)
	{
		_securityProvider = securityProvider;
		return this;
	}
	
	public String securityProvider()
	{
		return _securityProvider;
	}
	
	
	@Override
	public OmmIProviderConfig keyManagerAlgorithm(String keyManagerAlgorithm)
	{
		_keyManagerAlgorithm = keyManagerAlgorithm;
		return this;
	}
	
	public String keyManagerAlgorithm()
	{
		return _keyManagerAlgorithm;
	}
	
	@Override
	public OmmIProviderConfig trustManagerAlgorithm(String trustManagerAlgorithm)
	{
		_trustManagerAlgorithm = trustManagerAlgorithm;
		return this;
	}
	
	public String trustManagerAlgorithm()
	{
		return _trustManagerAlgorithm;
	}
}
