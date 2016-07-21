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
			throw ( oommICExcept().message( configStrBuilder().toString()));
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

		channelName = (String) xmlConfig().getConsumerAttributeValue(instanceName,ConfigManager.ConsumerChannelName);
		return channelName;
	}
	
	String dictionaryName(String instanceName)
	{
		String dictionaryName = null;

		// TODO: Retrieve from programmatic configuration

		dictionaryName = (String) xmlConfig().getConsumerAttributeValue(instanceName,ConfigManager.ConsumerDictionaryName);
		return dictionaryName;
	}
	
}