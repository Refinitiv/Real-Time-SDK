///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;

class OmmIProviderConfigImpl extends EmaConfigServerImpl implements OmmIProviderConfig
{
	
	private int _operationModel;
	private int _adminControlDirectory;
	private int _adminControlDictionary;

	OmmIProviderConfigImpl()
	{
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
		_operationModel = OperationModel.API_DISPATCH;
		_adminControlDirectory = AdminControl.API_CONTROL;
		_adminControlDictionary = AdminControl.API_CONTROL;
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
		// TODO: Retrieve from programmatic configuration

		String name = (String) xmlConfig().getIProviderAttributeValue(providerName,ConfigManager.IProviderName);

		if ( name == null ) 
		{
			if ( providerName.equals("EmaIProvider") )
			{
				boolean bFoundChild = xmlConfig().isNiProviderChildAvailable();
				if( bFoundChild == false )
					return this;
			}

			configStrBuilder().append( "OmmIProviderConfigImpl::providerName parameter [" )
									.append( providerName )
									.append( "] is an non-existent iprovider name" );
			throw ( oommICExcept().message( _configStrBuilder.toString()));
		}
 		else //if ( name != null ) 
		{
 			boolean bSetAttributeValue = xmlConfig().setDefaultIProvider(name);
			if ( bSetAttributeValue == false )
			{
				xmlConfig().appendAttributeValue(ConfigManager.IPROVIDER_GROUP, "DefaultIProvider", ConfigManager.DefaultIProvider,providerName);
				xmlConfig().verifyAndGetDefaultIProvider();
			}
		}
	
		return this;
	}
	
	@Override
	String configuredName()
	{
	
		String defaultIProviderName = null;

		// TODO: Retrieve from programmatic configuration
		
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
		
	    return "EmaIProvider";	
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

}
