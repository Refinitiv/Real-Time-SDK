///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


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
	 * Creates a {@link com.thomsonreuters.ema.access.ReqMsg}.
	 * @return {@link com.thomsonreuters.ema.access.ReqMsg}
	 */
	public static ReqMsg createReqMsg()
	{
		return new ReqMsgImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.RefreshMsg}.
	 * @return {@link com.thomsonreuters.ema.access.RefreshMsg}
	 */
	public static RefreshMsg createRefreshMsg()
	{
		return new RefreshMsgImpl();
	}

	/**
	 * Creates a {@link com.thomsonreuters.ema.access.UpdateMsg}.
	 * @return {@link com.thomsonreuters.ema.access.UpdateMsg}
	 */
	public static UpdateMsg createUpdateMsg()
	{
		return new UpdateMsgImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.StatusMsg}.
	 * @return {@link com.thomsonreuters.ema.access.StatusMsg}
	 */
	public static StatusMsg createStatusMsg()
	{
		return new StatusMsgImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.PostMsg}.
	 * @return {@link com.thomsonreuters.ema.access.PostMsg}
	 */
	public static PostMsg createPostMsg()
	{
		return new PostMsgImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.AckMsg}.
	 * @return {@link com.thomsonreuters.ema.access.AckMsg}
	 */
	public static AckMsg createAckMsg()
	{
		return new AckMsgImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.GenericMsg}.
	 * @return {@link com.thomsonreuters.ema.access.GenericMsg}
	 */
	public static GenericMsg createGenericMsg()
	{
		return new GenericMsgImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmArray}.
	 * @return {@link com.thomsonreuters.ema.access.OmmArray}
	 */
	public static OmmArray createOmmArray()
	{
		return new OmmArrayImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.ElementList}.
	 * @return {@link com.thomsonreuters.ema.access.ElementList}
	 */
	public static ElementList createElementList()
	{
		return new ElementListImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.FieldList}.
	 * @return {@link com.thomsonreuters.ema.access.FieldList}
	 */
	public static FieldList createFieldList()
	{
		return new FieldListImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.Map}.
	 * @return {@link com.thomsonreuters.ema.access.Map}
	 */
	public static Map createMap()
	{
		return new MapImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.Vector}.
	 * @return {@link com.thomsonreuters.ema.access.Vector}
	 */
	public static Vector createVector()
	{
		return new VectorImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.Series}.
	 * @return {@link com.thomsonreuters.ema.access.Series}
	 */
	public static Series createSeries()
	{
		return new SeriesImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.FilterList}.
	 * @return {@link com.thomsonreuters.ema.access.FilterList}
	 */
	public static FilterList createFilterList()
	{
		return new FilterListImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmConsumer}.
	 * @param config OmmConsumerConfig providing configuration
	 * @return {@link com.thomsonreuters.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config)
	{
		return new OmmConsumerImpl(config);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmConsumer}.
	 * 
	 * @param config OmmConsumerConfig providing configuration
	 * @param client OmmConsumerClient that provides callback interfaces to be used for item processing
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmConsumer}
	 */
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig config, OmmConsumerErrorClient client)
	{
		return new OmmConsumerImpl(config, client);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmConsumerConfig}.
	 * @return {@link com.thomsonreuters.ema.access.OmmConsumerConfig}
	 */
	public static OmmConsumerConfig createOmmConsumerConfig()
	{
		return new OmmConsumerConfigImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmProvider} for Non-Interactive provider role.<br>
	 * Enables exception throwing as means of error reporting.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @return {@link com.thomsonreuters.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config)
	{
		return new OmmNiProviderImpl(config);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmProvider} for Interactive provider role.<br>
	 * Enables exception throwing as means of error reporting.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @return {@link com.thomsonreuters.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client)
	{
		return new OmmIProviderImpl(config, client, null);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmProvider} for Interactive provider role.<br>
	 * Enables exception throwing as means of error reporting.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @param closure specifies application defined identification 
	 * @return {@link com.thomsonreuters.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client, Object closure)
	{
		return new OmmIProviderImpl(config, client, closure);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmProvider} for Non-Interactive provider role..
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param errorClient OmmProviderErrorClient that provides callback interfaces to be used for error reporting
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderErrorClient errorClient)
	{
		return new OmmNiProviderImpl(config, errorClient);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmProvider}.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @param errorClient OmmProviderErrorClient that provides callback interfaces to be used for error reporting
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client, OmmProviderErrorClient errorClient)
	{
		return new OmmIProviderImpl(config, client, errorClient, null);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmProvider}.
	 * 
	 * @param config OmmProviderConfig providing configuration
	 * @param client OmmProviderClient providing provider client
	 * @param errorClient OmmProviderErrorClient that provides callback interfaces to be used for error reporting
	 * @param closure specifies application defined identification
	 * 
	 * @return {@link com.thomsonreuters.ema.access.OmmProvider}
	 */
	public static OmmProvider createOmmProvider(OmmProviderConfig config, OmmProviderClient client, OmmProviderErrorClient errorClient, Object closure)
	{
		return new OmmIProviderImpl(config, client, errorClient, closure);
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmNiProviderConfig}.
	 * @return {@link com.thomsonreuters.ema.access.OmmNiProviderConfig}
	 */
	public static OmmNiProviderConfig createOmmNiProviderConfig()
	{
		return new OmmNiProviderConfigImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmIProviderConfig}.
	 * @return {@link com.thomsonreuters.ema.access.OmmIProviderConfig}
	 */
	public static OmmIProviderConfig createOmmIProviderConfig()
	{
		return new OmmIProviderConfigImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.RmtesBuffer}.
	 * @return {@link com.thomsonreuters.ema.access.RmtesBuffer}
	 */
	public static RmtesBuffer createRmtesBuffer()
	{
		return new RmtesBufferImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.FieldEntry}.
	 * @return {@link com.thomsonreuters.ema.access.FieldEntry}
	 */
	public static FieldEntry createFieldEntry()
	{
		return new FieldEntryImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.ElementEntry}.
	 * @return {@link com.thomsonreuters.ema.access.ElementEntry}
	 */
	public static ElementEntry createElementEntry()
	{
		return new ElementEntryImpl();
	}

	/**
	 * Creates a {@link com.thomsonreuters.ema.access.FilterEntry}.
	 * @return {@link com.thomsonreuters.ema.access.FilterEntry}
	 */
	public static FilterEntry createFilterEntry()
	{
		return new FilterEntryImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmArrayEntry}.
	 * @return {@link com.thomsonreuters.ema.access.OmmArrayEntry}
	 */
	public static OmmArrayEntry createOmmArrayEntry()
	{
		return new OmmArrayEntryImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.VectorEntry}.
	 * @return {@link com.thomsonreuters.ema.access.VectorEntry}
	 */
	public static VectorEntry createVectorEntry()
	{
		return new VectorEntryImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.SeriesEntry}.
	 * @return {@link com.thomsonreuters.ema.access.SeriesEntry}
	 */
	public static SeriesEntry createSeriesEntry()
	{
		return new SeriesEntryImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.MapEntry}.
	 * @return {@link com.thomsonreuters.ema.access.MapEntry}
	 */
	public static MapEntry createMapEntry()
	{
		return new MapEntryImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmAnsiPage}.
	 * @return {@link com.thomsonreuters.ema.access.OmmAnsiPage}
	 */
	public static OmmAnsiPage createOmmAnsiPage()
	{
		return new OmmAnsiPageImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmOpaque}.
	 * @return {@link com.thomsonreuters.ema.access.OmmOpaque}
	 */
	public static OmmOpaque createOmmOpaque()
	{
		return new OmmOpaqueImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.OmmXml}.
	 * @return {@link com.thomsonreuters.ema.access.OmmXml}
	 */
	public static OmmXml createOmmXml()
	{
		return new OmmXmlImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.ClassOfService}.
	 * @return {@link com.thomsonreuters.ema.access.ClassOfService}
	 */
	public static ClassOfService createClassOfService()
	{
		return new ClassOfServiceImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.CosAuthentication}.
	 * @return {@link com.thomsonreuters.ema.access.CosAuthentication}
	 */
	public static CosAuthentication createCosAuthentication()
	{
		return new CosAuthenticationImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.CosCommon}.
	 * @return {@link com.thomsonreuters.ema.access.CosCommon}
	 */
	public static CosCommon createCosCommon()
	{
		return new CosCommonImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.CosDataIntegrity}.
	 * @return {@link com.thomsonreuters.ema.access.CosDataIntegrity}
	 */
	public static CosDataIntegrity createCosDataIntegrity()
	{
		return new CosDataIntegrityImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.CosFlowControl}.
	 * @return {@link com.thomsonreuters.ema.access.CosFlowControl}
	 */
	public static CosFlowControl createCosFlowControl()
	{
		return new CosFlowControlImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.CosGuarantee}.
	 * @return {@link com.thomsonreuters.ema.access.CosGuarantee}
	 */
	public static CosGuarantee createCosGuarantee()
	{
		return new CosGuaranteeImpl();
	}
	
	/**
	 * Creates a {@link com.thomsonreuters.ema.access.TunnelStreamRequest}.
	 * @return {@link com.thomsonreuters.ema.access.TunnelStreamRequest}
	 */
	public static TunnelStreamRequest createTunnelStreamRequest()
	{
		return new TunnelStreamRequestImpl();
	}	
}
