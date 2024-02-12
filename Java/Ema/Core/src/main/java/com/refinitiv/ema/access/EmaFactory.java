///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2022,2024 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.access.OmmProviderConfig.ProviderRole;
import com.refinitiv.ema.domain.login.Login.LoginRefresh;
import com.refinitiv.ema.domain.login.Login.LoginReq;
import com.refinitiv.ema.domain.login.Login.LoginStatus;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.DictionaryEntry;
import com.refinitiv.ema.rdm.DictionaryUtility;

/**
 * EmaFactory is a factory class that creates Omm message objects and Omm container objects.
 * <p>EmaFactory can also create OmmConsumer and OmmConsumerConfig objects.</p>
 */
public class EmaFactory
{
	/**
	 * This class is not instantiated
	 */
	private EmaFactory()
	{
		throw new AssertionError();
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.ReqMsg}.
	 * @return {@link com.refinitiv.ema.access.ReqMsg}
	 */
	public static ReqMsg createReqMsg()
	{
		return new ReqMsgImpl();
	}
	
	/**
	 * Creates a clone of {@link com.refinitiv.ema.access.ReqMsg}.
	 * @param other ReqMsg clone source
	 * @return {@link com.refinitiv.ema.access.ReqMsg}
	 */
	public static ReqMsg createReqMsg(ReqMsg other)
	{
		return new ReqMsgImpl(other);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.RefreshMsg}.
	 * @return {@link com.refinitiv.ema.access.RefreshMsg}
	 */
	public static RefreshMsg createRefreshMsg()
	{
		return new RefreshMsgImpl();
	}
	
	/**
	 * Creates a clone of {@link com.refinitiv.ema.access.RefreshMsg}.
	 * @param other RefreshMsg clone source
	 * @return {@link com.refinitiv.ema.access.RefreshMsg}
	 */
	public static RefreshMsg createRefreshMsg(RefreshMsg other)
	{
		return new RefreshMsgImpl(other);
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.UpdateMsg}.
	 * @return {@link com.refinitiv.ema.access.UpdateMsg}
	 */
	public static UpdateMsg createUpdateMsg()
	{
		return new UpdateMsgImpl();
	}
	
	/**
	 * Creates a clone of {@link com.refinitiv.ema.access.UpdateMsg}.
	 * @param other UpdateMsg clone source
	 * @return {@link com.refinitiv.ema.access.UpdateMsg}
	 */
	public static UpdateMsg createUpdateMsg(UpdateMsg other)
	{
		return new UpdateMsgImpl(other);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.StatusMsg}.
	 * @return {@link com.refinitiv.ema.access.StatusMsg}
	 */
	public static StatusMsg createStatusMsg()
	{
		return new StatusMsgImpl();
	}
	
	/**
	 * Creates a clone of {@link com.refinitiv.ema.access.StatusMsg}.
	 * @param other StatusMsg clone source
	 * @return {@link com.refinitiv.ema.access.StatusMsg}
	 */
	public static StatusMsg createStatusMsg(StatusMsg other)
	{
		return new StatusMsgImpl(other);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.PostMsg}.
	 * @return {@link com.refinitiv.ema.access.PostMsg}
	 */
	public static PostMsg createPostMsg()
	{
		return new PostMsgImpl();
	}
	

	/**
	 * Creates a clone of {@link com.refinitiv.ema.access.PostMsg}.
	 * @param other PostMsg clone source
	 * @return {@link com.refinitiv.ema.access.PostMsg}
	 */
	public static PostMsg createPostMsg(PostMsg other)
	{
		return new PostMsgImpl(other);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.AckMsg}.
	 * @return {@link com.refinitiv.ema.access.AckMsg}
	 */
	public static AckMsg createAckMsg()
	{
		return new AckMsgImpl();
	}
	
	/**
	 * Creates a clone of {@link com.refinitiv.ema.access.AckMsg}.
	 * @param other AckMsg clone source
	 * @return {@link com.refinitiv.ema.access.AckMsg}
	 */
	public static AckMsg createAckMsg(AckMsg other)
	{
		return new AckMsgImpl(other);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.GenericMsg}.
	 * @return {@link com.refinitiv.ema.access.GenericMsg}
	 */
	public static GenericMsg createGenericMsg()
	{
		return new GenericMsgImpl();
	}
	
	/**
	 * Creates a clone of {@link com.refinitiv.ema.access.GenericMsg}.
	 * @param other GenericMsg clone source
	 * @return {@link com.refinitiv.ema.access.GenericMsg}
	 */
	public static GenericMsg createGenericMsg(GenericMsg other)
	{
		return new GenericMsgImpl(other);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmArray}.
	 * @return {@link com.refinitiv.ema.access.OmmArray}
	 */
	public static OmmArray createOmmArray()
	{
		return new OmmArrayImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.ElementList}.
	 * @return {@link com.refinitiv.ema.access.ElementList}
	 */
	public static ElementList createElementList()
	{
		return new ElementListImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.FieldList}.
	 * @return {@link com.refinitiv.ema.access.FieldList}
	 */
	public static FieldList createFieldList()
	{
		return new FieldListImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.Map}.
	 * @return {@link com.refinitiv.ema.access.Map}
	 */
	public static Map createMap()
	{
		return new MapImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.Vector}.
	 * @return {@link com.refinitiv.ema.access.Vector}
	 */
	public static Vector createVector()
	{
		return new VectorImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.Series}.
	 * @return {@link com.refinitiv.ema.access.Series}
	 */
	public static Series createSeries()
	{
		return new SeriesImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.FilterList}.
	 * @return {@link com.refinitiv.ema.access.FilterList}
	 */
	public static FilterList createFilterList()
	{
		return new FilterListImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * @param config OmmConsumerConfig providing configuration
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmOAuth2ConsumerClient OAuthClient)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, OAuthClient);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @param closure specifies application defined identification 
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmOAuth2ConsumerClient OAuthClient, Object closure)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, OAuthClient, closure);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * @param config OmmConsumerConfig providing configuration
	 * @param client OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient client)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, client);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param adminClient OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmOAuth2ConsumerClient OAuthClient)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, adminClient, OAuthClient);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param adminClient OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @param closure specifies application defined identification 
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmOAuth2ConsumerClient OAuthClient, Object closure)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, adminClient, OAuthClient, closure);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * @param config OmmConsumerConfig providing configuration
	 * @param client OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @param closure Object specifies application defined identification
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient client, Object closure)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, client, closure);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param client OmmConsumerClient that provides callback interfaces to be used for item processing
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerErrorClient client)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, client);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param errorClient OmmConsumerErrorClient that provides callback interfaces to be used for error reporting
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerErrorClient errorClient, OmmOAuth2ConsumerClient OAuthClient)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, errorClient, OAuthClient);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param errorClient OmmConsumerErrorClient that provides callback interfaces to be used for error reporting
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @param closure specifies application defined identification 
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerErrorClient errorClient, OmmOAuth2ConsumerClient OAuthClient, Object closure)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, errorClient, OAuthClient, closure);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param adminClient OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @param errorClient OmmConsumerErrorClient that provides callback interfaces to be used for error reporting
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient errorClient)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, adminClient, errorClient);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param adminClient OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @param errorClient OmmConsumerErrorClient that provides callback interfaces to be used for error reporting
	 * @param closure specifies application defined identification 
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient errorClient, Object closure)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, adminClient, errorClient, closure);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param adminClient OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @param errorClient OmmConsumerErrorClient that provides callback interfaces to be used for error reporting
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient errorClient, OmmOAuth2ConsumerClient OAuthClient)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, adminClient, errorClient, OAuthClient);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param adminClient OmmConsumerClient that provides callback interfaces to be used for item processing
	 * @param errorClient OmmConsumerErrorClient that provides callback interfaces to be used for error reporting
	 * @param OAuthClient OmmOAuth2ConsumerClient that provides callback interfaces for oauth credentials
	 * @param closure specifies application defined identification 
	 * @return {@link com.refinitiv.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient errorClient, OmmOAuth2ConsumerClient OAuthClient, Object closure)
	{
		((OmmConsumerConfigImpl)config).validateSpecifiedSessionName();
		return new OmmConsumerImpl(config, adminClient, errorClient, OAuthClient, closure);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumerConfig}.
	 * @return {@link com.refinitiv.ema.access.OmmConsumerConfig}
	 */
	public static OmmConsumerConfig createOmmConsumerConfig()
	{
		return new OmmConsumerConfigImpl();
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumerConfig}.
	* @param path either a file name (name of the file containing the configuration) or
	*        a directory name (name of a directory containing a configuration file named EmaConfig.xml).
	*        If path is null or empty, application will use EmaConfig.xml (if any) found in the current working directory
	 * @return {@link com.refinitiv.ema.access.OmmConsumerConfig}
	 */
	public static OmmConsumerConfig createOmmConsumerConfig( String path )
	{
		return new OmmConsumerConfigImpl(path);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmProvider} for Non-Interactive provider role.<br>
	 * Enables exception throwing as means of error reporting.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @return {@link com.refinitiv.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config)
	{
		if(config.providerRole() == ProviderRole.NON_INTERACTIVE)
		{
			((OmmNiProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmNiProviderImpl(config);
		}
		else
		{
			throw new OmmInvalidUsageExceptionImpl().message("The createOmmProvider(OmmProviderConfig) method supports Non-Interactive provider role only.",
					OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		}
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmProvider} for Interactive or Non-Interactive provider role.<br>
	 * Enables exception throwing as means of error reporting.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @return {@link com.refinitiv.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client)
	{
		if(config.providerRole() == ProviderRole.INTERACTIVE)
		{
			((OmmIProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmIProviderImpl(config, client, null);
		}
		else
		{
			((OmmNiProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmNiProviderImpl(config, client, null);
		}
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmProvider} for Interactive or Non-Interactive provider role.<br>
	 * Enables exception throwing as means of error reporting.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @param closure specifies application defined identification 
	 * @return {@link com.refinitiv.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client, Object closure)
	{
		if(config.providerRole() == ProviderRole.INTERACTIVE)
		{
			((OmmIProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmIProviderImpl(config, client, closure);
		}
		else
		{
			((OmmNiProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmNiProviderImpl(config, client, closure);
		}
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmProvider} for Non-Interactive provider role.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param errorClient OmmProviderErrorClient that provides callback interfaces to be used for error reporting
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderErrorClient errorClient)
	{
		if(config.providerRole() == ProviderRole.NON_INTERACTIVE)
		{
			((OmmNiProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmNiProviderImpl(config, errorClient);
		}
		else
		{
			if(errorClient != null)
			{
				String errorText = "The createOmmProvider(OmmProviderConfig, OmmProviderErrorClient)) method supports Non-Interactive provider role only.";
				errorClient.onInvalidUsage(errorText);
				errorClient.onInvalidUsage(errorText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}
			
			return null;
		}
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmProvider} for Interactive or Non-Interactive provider role.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @param errorClient OmmProviderErrorClient that provides callback interfaces to be used for error reporting
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client, OmmProviderErrorClient errorClient)
	{
		if(config.providerRole() == ProviderRole.INTERACTIVE)
		{
			((OmmIProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmIProviderImpl(config, client, errorClient, null);
		}
		else
		{
			((OmmNiProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmNiProviderImpl(config, client, errorClient, null);
		}
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmProvider} for Interactive or Non-Interactive provider role.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @param errorClient OmmProviderErrorClient that provides callback interfaces to be used for error reporting
	 * @param closure specifies application defined identification
	 * 
	 * @return {@link com.refinitiv.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client, OmmProviderErrorClient errorClient, Object closure)
	{
		if(config.providerRole() == ProviderRole.INTERACTIVE)
		{
			((OmmIProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmIProviderImpl(config, client, errorClient, closure);
		}
		else
		{
			((OmmNiProviderConfigImpl)config).validateSpecifiedSessionName();
			return new OmmNiProviderImpl(config, client, errorClient, closure);
		}
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmNiProviderConfig}.
	 * @return {@link com.refinitiv.ema.access.OmmNiProviderConfig}
	 */
	public static OmmNiProviderConfig createOmmNiProviderConfig()
	{
		return new OmmNiProviderConfigImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmNiProviderConfig}.
	* @param path either a file name (name of the file containing the configuration) or
	*        a directory name (name of a directory containing a configuration file named EmaConfig.xml).
	*        If path is null or empty, application will use EmaConfig.xml (if any) found in the current working directory
	 * @return {@link com.refinitiv.ema.access.OmmNiProviderConfig}
	 */
	public static OmmNiProviderConfig createOmmNiProviderConfig(String path)
	{
		return new OmmNiProviderConfigImpl(path);
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmIProviderConfig}.
	 * @return {@link com.refinitiv.ema.access.OmmIProviderConfig}
	 */
	public static OmmIProviderConfig createOmmIProviderConfig()
	{
		return new OmmIProviderConfigImpl();
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmIProviderConfig}.
	* @param path either a file name (name of the file containing the configuration) or
	*        a directory name (name of a directory containing a configuration file named EmaConfig.xml).
	*        If path is null or empty, application will use EmaConfig.xml (if any) found in the current working directory
	 * @return {@link com.refinitiv.ema.access.OmmIProviderConfig}
	 */
	public static OmmIProviderConfig createOmmIProviderConfig(String path) {
		return new OmmIProviderConfigImpl(path);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmConsumerConfig}.
	 * @return {@link com.refinitiv.ema.access.OmmConsumerConfig}
	 */
	public static OAuth2CredentialRenewal createOAuth2CredentialRenewal()
	{
		return new OAuth2CredentialRenewalImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.RmtesBuffer}.
	 * @return {@link com.refinitiv.ema.access.RmtesBuffer}
	 */
	public static RmtesBuffer createRmtesBuffer()
	{
		return new RmtesBufferImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.FieldEntry}.
	 * @return {@link com.refinitiv.ema.access.FieldEntry}
	 */
	public static FieldEntry createFieldEntry()
	{
		return new FieldEntryImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.ElementEntry}.
	 * @return {@link com.refinitiv.ema.access.ElementEntry}
	 */
	public static ElementEntry createElementEntry()
	{
		return new ElementEntryImpl();
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.FilterEntry}.
	 * @return {@link com.refinitiv.ema.access.FilterEntry}
	 */
	public static FilterEntry createFilterEntry()
	{
		return new FilterEntryImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmArrayEntry}.
	 * @return {@link com.refinitiv.ema.access.OmmArrayEntry}
	 */
	public static OmmArrayEntry createOmmArrayEntry()
	{
		return new OmmArrayEntryImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.VectorEntry}.
	 * @return {@link com.refinitiv.ema.access.VectorEntry}
	 */
	public static VectorEntry createVectorEntry()
	{
		return new VectorEntryImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.SeriesEntry}.
	 * @return {@link com.refinitiv.ema.access.SeriesEntry}
	 */
	public static SeriesEntry createSeriesEntry()
	{
		return new SeriesEntryImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.MapEntry}.
	 * @return {@link com.refinitiv.ema.access.MapEntry}
	 */
	public static MapEntry createMapEntry()
	{
		return new MapEntryImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmAnsiPage}.
	 * @return {@link com.refinitiv.ema.access.OmmAnsiPage}
	 */
	public static OmmAnsiPage createOmmAnsiPage()
	{
		return new OmmAnsiPageImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmOpaque}.
	 * @return {@link com.refinitiv.ema.access.OmmOpaque}
	 */
	public static OmmOpaque createOmmOpaque()
	{
		return new OmmOpaqueImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.OmmXml}.
	 * @return {@link com.refinitiv.ema.access.OmmXml}
	 */
	public static OmmXml createOmmXml()
	{
		return new OmmXmlImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.ClassOfService}.
	 * @return {@link com.refinitiv.ema.access.ClassOfService}
	 */
	public static ClassOfService createClassOfService()
	{
		return new ClassOfServiceImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.CosAuthentication}.
	 * @return {@link com.refinitiv.ema.access.CosAuthentication}
	 */
	public static CosAuthentication createCosAuthentication()
	{
		return new CosAuthenticationImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.CosCommon}.
	 * @return {@link com.refinitiv.ema.access.CosCommon}
	 */
	public static CosCommon createCosCommon()
	{
		return new CosCommonImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.CosDataIntegrity}.
	 * @return {@link com.refinitiv.ema.access.CosDataIntegrity}
	 */
	public static CosDataIntegrity createCosDataIntegrity()
	{
		return new CosDataIntegrityImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.CosFlowControl}.
	 * @return {@link com.refinitiv.ema.access.CosFlowControl}
	 */
	public static CosFlowControl createCosFlowControl()
	{
		return new CosFlowControlImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.CosGuarantee}.
	 * @return {@link com.refinitiv.ema.access.CosGuarantee}
	 */
	public static CosGuarantee createCosGuarantee()
	{
		return new CosGuaranteeImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.TunnelStreamRequest}.
	 * @return {@link com.refinitiv.ema.access.TunnelStreamRequest}
	 */
	public static TunnelStreamRequest createTunnelStreamRequest()
	{
		return new TunnelStreamRequestImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.ServiceEndpointDiscovery} using the default URLs.
	 * 
	 * <p>The token service URL defaults to https://api.refinitiv.com/auth/oauth2/v1/token<br>
	 * The RDP service discovery URL defaults to https://api.refinitiv.com/streaming/pricing/v1/</p>
	 * 
	 * @return {@link com.refinitiv.ema.access.ServiceEndpointDiscovery}
	 */
	public static ServiceEndpointDiscovery createServiceEndpointDiscovery()
	{
		return new ServiceEndpointDiscoveryImpl();
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.ServiceEndpointDiscovery} with overriding the default URLs.
	 * 
	 * @param tokenServiceUrlV1 specifies the token service V1 URL for password grant types to override the default value.
	 * @param serviceDiscoveryUrl specifies the service discovery URL to override the default value.
	 * @return {@link com.refinitiv.ema.access.ServiceEndpointDiscovery}
	 */
	public static ServiceEndpointDiscovery createServiceEndpointDiscovery(String tokenServiceUrlV1, String serviceDiscoveryUrl)
	{
		return new ServiceEndpointDiscoveryImpl(tokenServiceUrlV1, serviceDiscoveryUrl);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.ServiceEndpointDiscovery} with overriding the default URLs.
	 * 
	 * @param tokenServiceUrlV1 specifies the token service V1 URL for password grant types to override the default value.
	 * @param tokenServiceUrlV2 specifies the token service V2 URL for client credential grant types to override the default value.
	 * @param serviceDiscoveryUrl specifies the service discovery URL to override the default value.
	 * @return {@link com.refinitiv.ema.access.ServiceEndpointDiscovery}
	 */
	public static ServiceEndpointDiscovery createServiceEndpointDiscovery(String tokenServiceUrlV1, String tokenServiceUrlV2, String serviceDiscoveryUrl)
	{
		return new ServiceEndpointDiscoveryImpl(tokenServiceUrlV1, tokenServiceUrlV2, serviceDiscoveryUrl);
	}
	
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.ServiceEndpointDiscoveryOption} to specify query options to get endpoints.
	 * 
	 * @return {@link com.refinitiv.ema.access.ServiceEndpointDiscoveryOption}
	 */
	public static ServiceEndpointDiscoveryOption createServiceEndpointDiscoveryOption()
	{
		return new ServiceEndpointDiscoveryOptionImpl();
	}
	
	/**
	 * Domain is a nested class of EmaFactory that creates Admin Domain-specific message objects.
	 */
	public static class Domain
	{
	    /**
	     * This class is not instantiated
	     */
	    private Domain()
	    {
	        throw new AssertionError();
	    }
	    
	    /**
	     * Creates a {@link com.refinitiv.ema.domain.login.Login.LoginReq}.
	     * @return {@link com.refinitiv.ema.domain.login.Login.LoginReq}
	     */
	    public static LoginReq createLoginReq()
	    {
	        return new LoginReqImpl();
	    }

	    /**
	     * Creates a {@link com.refinitiv.ema.domain.login.Login.LoginReq}.
	     * @param reqMsg specifies ReqMsg to copy infomation from 
	     * @return {@link com.refinitiv.ema.domain.login.Login.LoginReq}
	     */
	    public static LoginReq createLoginReq( ReqMsg reqMsg )
	    {
	        return new LoginReqImpl(reqMsg);
	    }

	    /**
	     * Creates a {@link com.refinitiv.ema.domain.login.Login.LoginRefresh}.
	     * @return {@link com.refinitiv.ema.domain.login.Login.LoginRefresh}
	     */
	    public static LoginRefresh createLoginRefresh()
	    {
	        return new LoginRefreshImpl();
	    }

	    /**
	     * Creates a {@link com.refinitiv.ema.domain.login.Login.LoginRefresh}.
	     * @param refreshMsg specifies RefreshMsg to copy infomation from 
	     * @return {@link com.refinitiv.ema.domain.login.Login.LoginRefresh}
	     */
	    public static LoginRefresh createLoginRefresh( RefreshMsg refreshMsg )
	    {
	        return new LoginRefreshImpl(refreshMsg);
	    }

	    /**
	     * Creates a {@link com.refinitiv.ema.domain.login.Login.LoginStatus}.
	     * @return {@link com.refinitiv.ema.domain.login.Login.LoginStatus}
	     */
	    public static LoginStatus createLoginStatus()
	    {
	        return new LoginStatusImpl();
	    }

	    /**
	     * Creates a {@link com.refinitiv.ema.domain.login.Login.LoginStatus}.
	     * @param statusMsg specifies StatusMsg to copy information from
	     * @return {@link com.refinitiv.ema.domain.login.Login.LoginStatus}
	     */
	    public static LoginStatus createLoginStatus( StatusMsg statusMsg )
	    {
	        return new LoginStatusImpl(statusMsg);
	    }
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.rdm.DataDictionary}.
	 * @return {@link com.refinitiv.ema.rdm.DataDictionary}
	 */
	public static DataDictionary createDataDictionary()
	{
		return new DataDictionaryImpl(true);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.rdm.DataDictionary}.
	 * This method creates a new instance by copying dictionary information from
	 * the passed in parameter
	 * 
	 * @param dataDictionary specifies DataDictionary to copy data dictionary information
	 * 
	 * @return {@link com.refinitiv.ema.rdm.DataDictionary}
	 */
	public static DataDictionary createDataDictionary(DataDictionary dataDictionary)
	{
		return new DataDictionaryImpl((DataDictionaryImpl)dataDictionary);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.rdm.DictionaryUtility}.
	 * @return {@link com.refinitiv.ema.rdm.DictionaryUtility}
	 */
	public static DictionaryUtility createDictionaryUtility()
	{
		return new DictionaryUtilityImpl();
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.DateTimeStringFormat}.
	 * @return {@link com.refinitiv.ema.access.DateTimeStringFormat}
	 */

	public static DateTimeStringFormat createDateTimeStringFormat()
	{
		return new DateTimeStringFormatImpl();
	}

	/**
	 * Creates a {@link com.refinitiv.ema.access.ChannelInformation}.
	 * @return {@link com.refinitiv.ema.access.ChannelInformation}
	 */

	public static ChannelInformation createChannelInformation()
	{
		return new ChannelInformationImpl();
	}

	/**
	 * Creates a {@link com.refinitiv.ema.rdm.DictionaryEntry}.
	 * @return {@link com.refinitiv.ema.rdm.DictionaryEntry}
	 */
	public static DictionaryEntry createDictionaryEntry() {
		return new DictionaryEntryImpl(true);
	}
	
	/**
	 * Creates a {@link com.refinitiv.ema.access.PackedMsg}.
	 * @return {@link com.refinitiv.ema.access.PackedMsg}
	 */
	public static PackedMsg createPackedMsg(OmmProvider provider) {
		return new PackedMsgImpl(provider);
	}
}
