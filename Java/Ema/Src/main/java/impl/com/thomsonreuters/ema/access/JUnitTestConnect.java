///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.transport.ConnectionTypes;

//This class is created as a connect bridge between JUNIT test and EMA external/internal interface/classes.

public class JUnitTestConnect
{
	public static final int ConfigGroupTypeConsumer = 1;
	public static final int ConfigGroupTypeDictionary = 2;
	public static final int ConfigGroupTypeChannel = 3;
	
	// Consumer Parameters:
	public static final int ConsumerChannelSet  = ConfigManager.ChannelSet; 
	public static final int ConsumerDictionaryRequestTimeOut  = ConfigManager.DictionaryRequestTimeOut; 
	public static final int ConsumerDispatchTimeoutApiThread  = ConfigManager.DispatchTimeoutApiThread; 
	public static final int ConsumerItemCountHint  = ConfigManager.ItemCountHint ; 
	public static final int ConsumerMaxDispatchCountApiThread  = ConfigManager.MaxDispatchCountApiThread; 
	public static final int ConsumerMaxDispatchCountUserThread  = ConfigManager.MaxDispatchCountUserThread; 
	public static final int ConsumerMaxOutstandingPosts  = ConfigManager.MaxOutstandingPosts ; 
	public static final int ConsumerObeyOpenWindow  = ConfigManager.ObeyOpenWindow; 
	public static final int ConsumerPostAckTimeout  = ConfigManager.PostAckTimeout ; 
	public static final int ConsumerRequestTimeout  = ConfigManager.RequestTimeout; 
	public static final int ConsumerServiceCountHint  = ConfigManager.ServiceCountHint; 
	
	// Dictionary Parameters:
	public static final int DictionaryType  = ConfigManager.DictionaryType;
	public static final int DictionaryEnumTypeDefFileName  = ConfigManager.DictionaryEnumTypeDefFileName;
	public static final int DictionaryRDMFieldDictFileName  = ConfigManager.DictionaryRDMFieldDictFileName; 


	// Channel Parameters:
	public static final int ChannelConnectionPingTimeout  = ConfigManager.ChannelConnectionPingTimeout; 
	public static final int ChannelGuaranteedOutputBuffers  = ConfigManager.ChannelGuaranteedOutputBuffers; 
	public static final int ChannelInterfaceName  = ConfigManager.ChannelInterfaceName; 
	public static final int ChannelMsgKeyInUpdates  = ConfigManager.ChannelMsgKeyInUpdates; 
	public static final int ChannelNumInputBuffers  = ConfigManager.ChannelNumInputBuffers; 
	public static final int ChannelReconnectAttemptLimit  = ConfigManager.ChannelReconnectAttemptLimit; 
	public static final int ChannelReconnectMaxDelay  = ConfigManager.ChannelReconnectMaxDelay; 
	public static final int ChannelReconnectMinDelay  = ConfigManager.ChannelReconnectMinDelay; 
	public static final int ChannelSysRecvBufSize  = ConfigManager.ChannelSysRecvBufSize; 
	public static final int ChannelSysSendBufSize  = ConfigManager.ChannelSysSendBufSize; 
	public static final int ChannelHighWaterMark  = ConfigManager.ChannelHighWaterMark; 
	public static final int ChannelXmlTraceFileName  = ConfigManager.ChannelXmlTraceFileName; 
	public static final int ChannelXmlTraceHex  = ConfigManager.ChannelXmlTraceHex; 
	public static final int ChannelXmlTraceMaxFileSize  = ConfigManager.ChannelXmlTraceMaxFileSize; 
	public static final int ChannelXmlTracePing  = ConfigManager.ChannelXmlTracePing; 
	public static final int ChannelXmlTraceRead  = ConfigManager.ChannelXmlTraceRead; 
	public static final int ChannelXmlTraceToFile  = ConfigManager.ChannelXmlTraceToFile; 
	public static final int ChannelXmlTraceToMultipleFiles  = ConfigManager.ChannelXmlTraceToMultipleFiles; 
	public static final int ChannelXmlTraceToStdout  = ConfigManager.ChannelXmlTraceToStdout; 
	public static final int ChannelXmlTraceWrite  = ConfigManager.ChannelXmlTraceWrite; 	
	public static final int ChannelCompressionThreshold  = ConfigManager.ChannelCompressionThreshold; 
	public static final int ChannelCompressionType  = ConfigManager.ChannelCompressionType; 
	public static final int ChannelHost  = ConfigManager.ChannelHost; 
	public static final int ChannelObjectName  = ConfigManager.ChannelObjectName; 
	public static final int ChannelPort  = ConfigManager.ChannelPort; 
	public static final int ChannelTcpNodelay  = ConfigManager.ChannelTcpNodelay;
	public static final int ChannelDirectWrite  = ConfigManager.ChannelDirectSocketWrite; 
	
	// Channel: Multicast
	public static final int ChannelDisconnectOnGap  = ConfigManager.ChannelDisconnectOnGap; 
	public static final int ChannelHsmInterface  = ConfigManager.ChannelHsmInterface; 
	public static final int ChannelHsmInterval  = ConfigManager.ChannelHsmInterval; 
	public static final int ChannelHsmMultAddress  = ConfigManager.ChannelHsmMultAddress; 
	public static final int ChannelHsmPort  = ConfigManager.ChannelHsmPort; 
	public static final int Channelndata  = ConfigManager.Channelndata; 
	public static final int Channelnmissing  = ConfigManager.Channelnmissing; 
	public static final int Channelnrreq  = ConfigManager.Channelnrreq; 
	public static final int ChannelPacketTTL  = ConfigManager.ChannelPacketTTL; 
	public static final int ChannelpktPoolLimitHigh  = ConfigManager.ChannelpktPoolLimitHigh; 
	public static final int ChannelpktPoolLimitLow  = ConfigManager.ChannelpktPoolLimitLow; 
	public static final int ChannelRecvAddress  = ConfigManager.ChannelRecvAddress; 
	public static final int ChannelRecvPort  = ConfigManager.ChannelRecvPort; 
	public static final int ChannelSendAddress  = ConfigManager.ChannelSendAddress;
	public static final int ChannelSendPort  = ConfigManager.ChannelSendPort;
	public static final int Channeltbchold  = ConfigManager.Channeltbchold;
	public static final int ChanneltcpControlPort  = ConfigManager.ChanneltcpControlPort;
	public static final int Channeltdata  = ConfigManager.Channeltdata; 
	public static final int Channeltpphold  = ConfigManager.Channeltpphold; 
	public static final int Channeltrreq  = ConfigManager.Channeltrreq; 
	public static final int Channeltwait  = ConfigManager.Channeltwait; 
	public static final int ChannelUnicastPort  = ConfigManager.ChannelUnicastPort; 
	public static final int ChanneluserQLimit  = ConfigManager.ChanneluserQLimit; 
	
	
	public static EmaObjectManager _objManager = new EmaObjectManager();
	
	static {
		_objManager.initialize();
	}
	
	// used only for JUNIT tests
	public static FieldListImpl createFieldList()
	{
		return new FieldListImpl(_objManager);
	}

	// used only for JUNIT tests
	public static ElementListImpl createElementList()
	{
		return new ElementListImpl(_objManager);
	}

	// used only for JUNIT tests
	public static MapImpl createMap()
	{
		return new MapImpl(_objManager);
	}
	
	// used only for JUNIT tests
	public static VectorImpl createVector()
	{
		return new VectorImpl(_objManager);
	}

	// used only for JUNIT tests
	public static SeriesImpl createSeries()
	{
		return new SeriesImpl(_objManager);
	}

	// used only for JUNIT tests
	public static FilterListImpl createFilterList()
	{
		return new FilterListImpl(_objManager);
	}
	
	// used only for JUNIT tests
	public static OmmArrayImpl createOmmArray()
	{
		return new OmmArrayImpl(_objManager);
	}

	// used only for JUNIT tests
	public static RefreshMsgImpl createRefreshMsg()
	{
		return new RefreshMsgImpl(_objManager);
	}
	
	// used only for JUNIT tests
	public static ReqMsgImpl createReqMsg()
	{
		return new ReqMsgImpl(_objManager);
	}

	// used only for JUNIT tests
	public static UpdateMsgImpl createUpdateMsg()
	{
		return new UpdateMsgImpl(_objManager);
	}

	// used only for JUNIT tests
	public static StatusMsgImpl createStatusMsg()
	{
		return new StatusMsgImpl(_objManager);
	}

	// used only for JUNIT tests
	public static PostMsgImpl createPostMsg()
	{
		return new PostMsgImpl(_objManager);
	}

	// used only for JUNIT tests
	public static AckMsgImpl createAckMsg()
	{
		return new AckMsgImpl(_objManager);
	}

	// used only for JUNIT tests
	public static GenericMsgImpl createGenericMsg()
	{
		return new GenericMsgImpl(_objManager);
	}

	// used only for JUNIT tests
	public static void setRsslData(Data data, com.thomsonreuters.upa.codec.Msg rsslMsgEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((CollectionDataImpl) data).decode(rsslMsgEncoded, majVer, minVer, rsslDictionary);
	}

	// used only for JUNIT tests
	public static void setRsslData(Data data, com.thomsonreuters.upa.codec.Buffer rsslBufferEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((CollectionDataImpl) data).decode(rsslBufferEncoded, majVer, minVer, rsslDictionary, localFlSetDefDb);
	}
	
	// used only for JUNIT tests
	public static void setRsslData(Data data, Data dataEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((CollectionDataImpl) data).decode(((DataImpl)dataEncoded).encodedData(), majVer, minVer,  rsslDictionary, localFlSetDefDb);
	}

	public static void setRsslData(Msg msg, com.thomsonreuters.upa.codec.Buffer rsslBufferEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((MsgImpl) msg).decode(rsslBufferEncoded, majVer, minVer, rsslDictionary, localFlSetDefDb);
	}

	public static void setRsslData(Msg msg, Data dataEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((MsgImpl) msg).decode(((DataImpl)dataEncoded).encodedData(), majVer, minVer,  rsslDictionary, localFlSetDefDb);
	}
	
	// used only for JUNIT tests
	public static void setRsslData(Buffer bufEncoded, Data dataEncoded)
	{
		((DataImpl)dataEncoded).encodedData().copy(bufEncoded);
	}
	
	// used only for JUNIT tests
	public static void setRsslData(RmtesBuffer rmtesBuffer, ByteBuffer dataEncoded)
	{
		((RmtesBufferImpl)rmtesBuffer).setRsslData(dataEncoded);
	}
	
	// used only for JUNIT tests
	public static String configGetConsumerName(OmmConsumerConfig consConfig)
	{
		return ((OmmConsumerConfigImpl) consConfig).configuredName();
	}
	
	// used only for JUNIT tests
	public static String configGetChannelName(OmmConsumerConfig consConfig, String consumerName)
	{
		return ((OmmConsumerConfigImpl) consConfig).channelName(consumerName);
	}
	
	// used only for JUNIT tests
	public static String configGetDictionaryName(OmmConsumerConfig consConfig, String consumerName)
	{
		return ((OmmConsumerConfigImpl) consConfig).dictionaryName(consumerName);
	}
	
	// used only for JUNIT tests
	public static int configGetChannelType(OmmConsumerConfig consConfig, String channelName)
	{
		OmmConsumerConfigImpl configImpl = ( (OmmConsumerConfigImpl ) consConfig);
		ConfigAttributes attributes = configImpl.xmlConfig().getChannelAttributes(channelName);
		ConfigElement ce = null;
		int connectionType = ConnectionTypes.SOCKET;
	
		if (configImpl.getUserSpecifiedHostname() != null)
			connectionType = ConnectionTypes.SOCKET;
		else
		{
			if (attributes != null) 
			{
				ce = attributes.getPrimitiveValue(ConfigManager.ChannelType);
				if (ce != null)
					connectionType = ce.intValue();
			}
		}
		return connectionType;
	}	
	
	// used only for JUNIT tests
	public static String configGetChanHost(OmmConsumerConfig consConfig, String channelName)
	{
		OmmConsumerConfigImpl configImpl = ( (OmmConsumerConfigImpl ) consConfig);
		ConfigAttributes attributes = configImpl.xmlConfig().getChannelAttributes(channelName);
		ConfigElement ce = null;
		String host =  configImpl.getUserSpecifiedHostname();;
		if (host == null)
		{
			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
				host = ce.asciiValue();
		}
	
		return host;
	}	
	
	// used only for JUNIT tests
	public static String configGetChanPort(OmmConsumerConfig consConfig, String channelName)
	{
		OmmConsumerConfigImpl configImpl = ( (OmmConsumerConfigImpl ) consConfig);
		ConfigAttributes attributes = configImpl.xmlConfig().getChannelAttributes(channelName);
		ConfigElement ce = null;
		String port =  configImpl.getUserSpecifiedPort();;
		if (port == null)
		{
			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
				port = ce.asciiValue();
		}

		return port;
	}	
	
	// used only for JUNIT tests
	public static int configGetIntLongValue(OmmConsumerConfig consConfig, String name, int type, int configParam)
	{
		ConfigAttributes attributes = null;
		if(type == ConfigGroupTypeConsumer)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getConsumerAttributes(name);
		else if (type == ConfigGroupTypeChannel)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getChannelAttributes(name);
		else if (type == ConfigGroupTypeDictionary)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getDictionaryAttributes(name);
	
		ConfigElement ce = null;
		int maxInt = Integer.MAX_VALUE;
		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(configParam)) != null)
				return (ce.intLongValue() > maxInt ? maxInt : ce.intLongValue());
		}
	
		return 0;
	}	
	
	// used only for JUNIT tests
	public static int configGetIntValue(OmmConsumerConfig consConfig, String name, int type, int configParam)
	{
		ConfigAttributes attributes = null;
		if(type == ConfigGroupTypeConsumer)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getConsumerAttributes(name);
		else if (type == ConfigGroupTypeChannel)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getChannelAttributes(name);
		else if (type == ConfigGroupTypeDictionary)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getDictionaryAttributes(name);
	
		ConfigElement ce = null;
		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(configParam)) != null)
				return (ce.intValue());
		}
	
		return 0;
	}	

	// used only for JUNIT tests
	public static String configGetStringValue(OmmConsumerConfig consConfig, String name, int type, int configParam)
	{
		ConfigAttributes attributes = null;
		if(type == ConfigGroupTypeConsumer)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getConsumerAttributes(name);
		else if (type == ConfigGroupTypeChannel)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getChannelAttributes(name);
		else if (type == ConfigGroupTypeDictionary)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getDictionaryAttributes(name);
	
		ConfigElement ce = null;
		String configParamValue =  null;
		if (attributes != null )
		{
			ce = attributes.getPrimitiveValue(configParam);
			if(ce != null)
				configParamValue = ce.asciiValue();
		}
		return configParamValue;
	}	

	// used only for JUNIT tests
	public static Boolean configGetBooleanValue(OmmConsumerConfig consConfig, String name, int type, int configParam)
	{
		ConfigAttributes attributes = null;
		if(type == ConfigGroupTypeConsumer)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getConsumerAttributes(name);
		else if (type == ConfigGroupTypeChannel)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getChannelAttributes(name);
		else if (type == ConfigGroupTypeDictionary)
			attributes = ((OmmConsumerConfigImpl) consConfig).xmlConfig().getDictionaryAttributes(name);
	
		ConfigElement ce = null;
		Boolean configParamValue = false;
		if (attributes != null) 
		{
			ce = attributes.getPrimitiveValue(configParam);

			if (ce != null)
				configParamValue = ce.booleanValue();
		}
		return configParamValue;
	}	
}