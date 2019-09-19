///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015, 2019. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.List;

import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.ema.access.OmmConsumer;

//This class is created as a connect bridge between JUNIT test and EMA external/internal interface/classes.

public class JUnitTestConnect
{
	public static final int ConfigGroupTypeConsumer = 1;
	public static final int ConfigGroupTypeDictionary = 2;
	public static final int ConfigGroupTypeChannel = 3;
	public static final int ConfigGroupTypeProvider = 4;
	public static final int ConfigGroupTypeDirectory = 5;
	public static final int ConfigGroupTypeServer = 6;

	// Common Parameters:
	public static final int ChannelSet  = ConfigManager.ChannelSet; 
	public static final int XmlTraceToStdout  = ConfigManager.XmlTraceToStdout; 
	public static final int ItemCountHint  = ConfigManager.ItemCountHint ; 
	public static final int DispatchTimeoutApiThread  = ConfigManager.DispatchTimeoutApiThread; 
	public static final int MaxDispatchCountApiThread  = ConfigManager.MaxDispatchCountApiThread; 
	public static final int MaxDispatchCountUserThread  = ConfigManager.MaxDispatchCountUserThread; 
	public static final int RequestTimeout  = ConfigManager.RequestTimeout; 
	public static final int ServiceCountHint  = ConfigManager.ServiceCountHint; 
	public static final int ReconnectAttemptLimit  = ConfigManager.ReconnectAttemptLimit; 
	public static final int ReconnectMaxDelay  = ConfigManager.ReconnectMaxDelay; 
	public static final int ReconnectMinDelay  = ConfigManager.ReconnectMinDelay; 
	public static final int LoginRequestTimeOut  = ConfigManager.LoginRequestTimeOut;
	public static final int RestRequestTimeout  = ConfigManager.RestRequestTimeout; 
	public static final int ReissueTokenAttemptLimit  = ConfigManager.ReissueTokenAttemptLimit; 
	public static final int ReissueTokenAttemptInterval  = ConfigManager.ReissueTokenAttemptInterval; 
	public static final int TokenReissueRatio  = ConfigManager.TokenReissueRatio;

	public static final int ConnectionPingTimeout  = ConfigManager.ConnectionPingTimeout; 
	public static final int GuaranteedOutputBuffers  = ConfigManager.GuaranteedOutputBuffers; 
	public static final int InterfaceName  = ConfigManager.InterfaceName; 
	public static final int NumInputBuffers  = ConfigManager.NumInputBuffers; 
	public static final int SysRecvBufSize  = ConfigManager.SysRecvBufSize; 
	public static final int SysSendBufSize  = ConfigManager.SysSendBufSize; 
	public static final int HighWaterMark  = ConfigManager.HighWaterMark; 
	public static final int CompressionThreshold  = ConfigManager.ChannelCompressionThreshold; 
	public static final int CompressionType  = ConfigManager.ChannelCompressionType; 
	public static final int Host  = ConfigManager.ChannelHost; 
	public static final int ObjectName  = ConfigManager.ChannelObjectName; 
	public static final int ProxyHost  = ConfigManager.ChannelProxyHost;
	public static final int ProxyPort  = ConfigManager.ChannelProxyPort;
	public static final int Port  = ConfigManager.ChannelPort; 
	public static final int TcpNodelay  = ConfigManager.ChannelTcpNodelay;
	public static final int DirectWrite  = ConfigManager.ChannelDirectSocketWrite;
	public static final int EnableSessionMgnt = ConfigManager.ChannelEnableSessionMgnt;
	public static final int Location = ConfigManager.ChannelLocation;
	
	// Consumer Parameters:
	public static final int ConsumerDefaultConsumerName  = ConfigManager.DefaultConsumer; 	
	public static final int ConsumerDictionaryRequestTimeOut  = ConfigManager.DictionaryRequestTimeOut; 
	public static final int ConsumerMaxOutstandingPosts  = ConfigManager.MaxOutstandingPosts ; 
	public static final int ConsumerObeyOpenWindow  = ConfigManager.ObeyOpenWindow; 
	public static final int ConsumerPostAckTimeout  = ConfigManager.PostAckTimeout ; 
	public static final int ConsumerMsgKeyInUpdates  = ConfigManager.MsgKeyInUpdates; 
	
	// Dictionary Parameters:
	public static final int DictionaryName  = ConfigManager.DictionaryName;
	public static final int DictionaryType  = ConfigManager.DictionaryType;
	public static final int DictionaryEnumTypeDefFileName  = ConfigManager.DictionaryEnumTypeDefFileName;
	public static final int DictionaryRDMFieldDictFileName  = ConfigManager.DictionaryRDMFieldDictFileName; 


	// Channel Parameters:
	public static final int ChannelName  = ConfigManager.ChannelName;
	public static final int ChannelType  = ConfigManager.ChannelType;
	public static final int ChannelInitTimeout = ConfigManager.ChannelInitTimeout;
	
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
	
	// Provider Parameters:
	public static final int DictionaryRdmFieldDictionaryItemName = ConfigManager.DictionaryRdmFieldDictionaryItemName;
	public static final int DictionaryEnumTypeDefItemName = ConfigManager.DictionaryEnumTypeDefItemName;
	public static final int DictionaryFieldDictFragmentSize = ConfigManager.DictionaryFieldDictFragmentSize;
	public static final int DictionaryEnumTypeFragmentSize = ConfigManager.DictionaryEnumTypeFragmentSize;
	
	// NIProvider
	public static final int NiProviderGroup = ConfigManager.NiProviderGroup;
	public static final int DefaultNiProvider = ConfigManager.DefaultNiProvider;
	public static final int NiProviderList = ConfigManager.NiProviderList;
	public static final int NiProvider = ConfigManager.NiProvider;
	
	public static final int NiProviderName = ConfigManager.NiProviderName;
	public static final int NiProviderChannelName = ConfigManager.NiProviderChannelName;
	public static final int NiProviderDirectoryName = ConfigManager.NiProviderDirectoryName;
	public static final int NiProviderRefreshFirstRequired = ConfigManager.NiProviderRefreshFirstRequired;
	public static final int NiProviderMergeSourceDirectoryStreams = ConfigManager.NiProviderMergeSourceDirectoryStreams;
	public static final int NiProviderRecoverUserSubmitSourceDirectory = ConfigManager.NiProviderRecoverUserSubmitSourceDirectory;
	public static final int NiProviderRemoveItemsOnDisconnect = ConfigManager.NiProviderRemoveItemsOnDisconnect;
	
	// Directory
	public static final int Directory = ConfigManager.Directory;
	public static final int DirectoryName = ConfigManager.DirectoryName;
	
	// Service
	public static final int Service = ConfigManager.Service;
	public static final int ServiceName = ConfigManager.ServiceName;
	public static final int ServiceInfoFilter = ConfigManager.ServiceInfoFilter;
	public static final int ServiceInfoFilterServiceId = ConfigManager.ServiceInfoFilterServiceId;
	public static final int ServiceInfoFilterVendor = ConfigManager.ServiceInfoFilterVendor;
	public static final int ServiceInfoFilterIsSource = ConfigManager.ServiceInfoFilterIsSource;
	public static final int ServiceInfoFilterCapabilities = ConfigManager.ServiceInfoFilterCapabilities;
	public static final int ServiceInfoFilterCapabilitiesCapabilitiesEntry = ConfigManager.ServiceInfoFilterCapabilitiesCapabilitiesEntry;
	public static final int ServiceInfoFilterDictionariesProvided = ConfigManager.ServiceInfoFilterDictionariesProvided;
	public static final int ServiceInfoFilterDictionariesProvidedDictionariesProvidedEntry = ConfigManager.ServiceInfoFilterDictionariesProvidedDictionariesProvidedEntry;
	public static final int ServiceInfoFilterDictionariesUsed = ConfigManager.ServiceInfoFilterDictionariesUsed;
	public static final int ServiceInfoFilterDictionariesUsedDictionariesUsedEntry = ConfigManager.ServiceInfoFilterDictionariesUsedDictionariesUsedEntry;
	public static final int ServiceInfoFilterQoS = ConfigManager.ServiceInfoFilterQoS;
	public static final int ServiceInfoFilterQoSEntry = ConfigManager.ServiceInfoFilterQoSEntry;
	public static final int ServiceInfoFilterQoSEntryTimeliness = ConfigManager.ServiceInfoFilterQoSEntryTimeliness;
	public static final int ServiceInfoFilterQoSEntryRate = ConfigManager.ServiceInfoFilterQoSEntryRate;
	public static final int ServiceInfoFilterSupportsQoSRange = ConfigManager.ServiceInfoFilterSupportsQoSRange;
	public static final int ServiceInfoFilterItemList = ConfigManager.ServiceInfoFilterItemList;
	public static final int ServiceInfoFilterAcceptingConsumerStatus = ConfigManager.ServiceInfoFilterAcceptingConsumerStatus;
	public static final int ServiceInfoFilterSupportsOutOfBandSnapshots = ConfigManager.ServiceInfoFilterSupportsOutOfBandSnapshots;
	public static final int ServiceStateFilter = ConfigManager.ServiceStateFilter;
	public static final int ServiceStateFilterServiceState = ConfigManager.ServiceStateFilterServiceState;
	public static final int ServiceStateFilterAcceptingRequests = ConfigManager.ServiceStateFilterAcceptingRequests;
	public static final int ServiceStateFilterStatus = ConfigManager.ServiceStateFilterStatus;
	public static final int ServiceStateFilterStatusStreamState = ConfigManager.ServiceStateFilterStatusStreamState;
	public static final int ServiceStateFilterStatusDataState = ConfigManager.ServiceStateFilterStatusDataState;
	public static final int ServiceStateFilterStatusStatusCode = ConfigManager.ServiceStateFilterStatusStatusCode;
	public static final int ServiceStateFilterStatusStatusText = ConfigManager.ServiceStateFilterStatusStatusText;
	
	// IProvider
	public static final int IProviderName = ConfigManager.IProviderName;
	public static final int IProviderServerName = ConfigManager.IProviderServerName;
	public static final int IProviderDirectoryName = ConfigManager.IProviderDirectoryName;
	public static final int IProviderRefreshFirstRequired = ConfigManager.IProviderRefreshFirstRequired;
	public static final int IProviderAcceptMessageWithoutAcceptingRequests = ConfigManager.IProviderAcceptMessageWithoutAcceptingRequests;
	public static final int IProviderAcceptDirMessageWithoutMinFilters = ConfigManager.IProviderAcceptDirMessageWithoutMinFilters;
	public static final int IProviderAcceptMessageWithoutBeingLogin = ConfigManager.IProviderAcceptMessageWithoutBeingLogin;
	public static final int IProviderAcceptMessageSameKeyButDiffStream = ConfigManager.IProviderAcceptMessageSameKeyButDiffStream;
	public static final int IProviderAcceptMessageThatChangesService = ConfigManager.IProviderAcceptMessageThatChangesService;
	public static final int IProviderAcceptMessageWithoutQosInRange = ConfigManager.IProviderAcceptMessageWithoutQosInRange;
	
	// Server: Global
	public static final int Server = ConfigManager.Server;
	public static final int ServerName = ConfigManager.ServerName;
	public static final int ServerType = ConfigManager.ServerType;
	public static final int ServerInitTimeout = ConfigManager.ServerInitTimeout;
	
	// Server: Socket
	public static final int ServerCompressionThreshold = ConfigManager.ServerCompressionThreshold;
	public static final int ServerCompressionType = ConfigManager.ServerCompressionType;
	public static final int ServerPort = ConfigManager.ServerPort;
	public static final int ServerTcpNodelay = ConfigManager.ServerTcpNodelay;
	public static final int ServerDirectSocketWrite = ConfigManager.ServerDirectSocketWrite;
	public static final int ConnectionMinPingTimeout = ConfigManager.ConnectionMinPingTimeout;
	
	public static String _lastErrorText = "";
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
	public static com.thomsonreuters.upa.codec.DataDictionary loadDictionary(String dictPath)
	{
		 com.thomsonreuters.upa.transport.Error error = com.thomsonreuters.upa.transport.TransportFactory.createError();
	     com.thomsonreuters.upa.codec.DataDictionary dictionary = com.thomsonreuters.upa.codec.CodecFactory.createDataDictionary();
	    	
	     if ( CodecReturnCodes.SUCCESS != dictionary.loadFieldDictionary(dictPath+"RDMFieldDictionary", error))
	    	 return null;
	     if ( CodecReturnCodes.SUCCESS != dictionary.loadEnumTypeDictionary(dictPath+"enumtype.def", error))
	    	 return null;
	     
	     return dictionary;
	}
	
	// used only for JUNIT tests
	public static void setRsslData(Msg msg, com.thomsonreuters.upa.codec.Msg rsslMsgEncoded, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		((MsgImpl) msg).decode(rsslMsgEncoded, majVer, minVer, rsslDictionary);
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
	public static String getLastErrorText()
	{
		return _lastErrorText;
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
	
	// used only for JUINT tests
	public static int configVerifyChannelEncrypTypeAttribs(ChannelConfig chanCfg, String position,  OmmConsumerConfig consConfig, String channelName)
	{
		int result = 0;
		_lastErrorText = "";
		EncryptedChannelConfig encCfg = (EncryptedChannelConfig) chanCfg;
		String strValue = configGetChanPort(consConfig, channelName);
		if(strValue.equals(encCfg.serviceName) == false)
		{
			_lastErrorText = "Port mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig port='";
			_lastErrorText += strValue;
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] port='";
			_lastErrorText += encCfg.serviceName;
			_lastErrorText += "' for ";
			return 5;
		}
		strValue = configGetChanHost(consConfig, channelName);
		if(strValue.equals(encCfg.hostName) == false)
		{
			_lastErrorText = "HostName mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig host='";
			_lastErrorText += strValue;
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] host='";
			_lastErrorText += encCfg.hostName;
			_lastErrorText += "' for ";
			return 6;	
		}	
		Boolean boolValue = JUnitTestConnect.configGetBooleanValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay);
		if(boolValue != encCfg.tcpNodelay)
		{
			_lastErrorText = "TcpNodelay mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig tcpNodelay ='";
			_lastErrorText += (boolValue ? "1" : "0");
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] tcpNodelay ='";
			_lastErrorText += (encCfg.tcpNodelay ? "1" : "0");
			_lastErrorText += "' for ";
			return 7;
		}	
		
		strValue = configGetStringValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ObjectName);
		if(strValue.equals(encCfg.objectName) == false)
		{
			_lastErrorText = "ObjectName mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig objectName='";
			_lastErrorText += strValue;
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] objectName='";
			_lastErrorText += encCfg.objectName;
			_lastErrorText += "' for ";
			return 8;	
		}	
	
		return result;			
	}

	// used only for JUINT tests
	public static int configVerifyChannelSocketTypeAttribs(ChannelConfig chanCfg, String position,  OmmConsumerConfig consConfig, String channelName)
	{
		int result = 0;
		_lastErrorText = "";
		SocketChannelConfig socCfg = (SocketChannelConfig) chanCfg;
		String strValue = configGetChanPort(consConfig, channelName);
		if(strValue.equals(socCfg.serviceName) == false)
		{
			_lastErrorText = "Port mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig port='";
			_lastErrorText += strValue;
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] port='";
			_lastErrorText += socCfg.serviceName;
			_lastErrorText += "' for ";
			return 5;
		}
		strValue = configGetChanHost(consConfig, channelName);
		if(strValue.equals(socCfg.hostName) == false)
		{
			_lastErrorText = "HostName mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig host='";
			_lastErrorText += strValue;
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] host='";
			_lastErrorText += socCfg.hostName;
			_lastErrorText += "' for ";
			return 6;	
		}	
		Boolean boolValue = JUnitTestConnect.configGetBooleanValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.TcpNodelay);
		if(boolValue != socCfg.tcpNodelay)
		{
			_lastErrorText = "TcpNodelay mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig tcpNodelay ='";
			_lastErrorText += (boolValue ? "1" : "0");
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] tcpNodelay ='";
			_lastErrorText += (socCfg.tcpNodelay ? "1" : "0");
			_lastErrorText += "' for ";
			return 7;
		}		
		return result;
	}

	// used only for JUINT tests
	public static int configVerifyChannelCommonAttribs(ChannelConfig chanCfg, String position,  OmmConsumerConfig consConfig, String channelName, ChannelConfig lastChanCfg)
	{
		int result = 0;
		int intValue = JUnitTestConnect.configGetIntValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionType);
		if(intValue != chanCfg.compressionType)
		{
			_lastErrorText = "CompressionType mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig CompressionType='";
			_lastErrorText += Integer.toString(intValue);
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] CompressionType='";
			_lastErrorText += Integer.toString(chanCfg.compressionType);
			_lastErrorText += "' for ";
			return 9;
		}
		
		int intLongValue = JUnitTestConnect.configGetIntLongValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.GuaranteedOutputBuffers);
		if(intLongValue != chanCfg.guaranteedOutputBuffers)
		{
			_lastErrorText = "GuaranteedOutputBuffers mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig guaranteedOutputBuffers ='";
			_lastErrorText += Integer.toString(intValue);
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] guaranteedOutputBuffers ='";
			_lastErrorText += Integer.toString(chanCfg.guaranteedOutputBuffers);
			_lastErrorText += "' for ";
			return 10;
		}
		
		intLongValue = JUnitTestConnect.configGetIntLongValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.NumInputBuffers);
		if(intLongValue != chanCfg.numInputBuffers)
		{
			_lastErrorText = "NumInputBuffers mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig numInputBuffers ='";
			_lastErrorText += Integer.toString(intValue);
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] numInputBuffers ='";
			_lastErrorText += Integer.toString(chanCfg.numInputBuffers);
			_lastErrorText += "' for ";
			return 11;
		}

		intLongValue = JUnitTestConnect.configGetIntLongValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysRecvBufSize);
		if(intLongValue != chanCfg.sysRecvBufSize)
		{
			_lastErrorText = "SysRecvBufSize mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig sysRecvBufSize ='";
			_lastErrorText += Integer.toString(intValue);
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] sysRecvBufSize ='";
			_lastErrorText += Integer.toString(chanCfg.sysRecvBufSize);
			_lastErrorText += "' for ";
			return 12;
		}

		intLongValue = JUnitTestConnect.configGetIntLongValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.SysSendBufSize);
		if(intLongValue != chanCfg.sysSendBufSize)
		{
			_lastErrorText = "SysSendBufSize mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig sysSendBufSize ='";
			_lastErrorText += Integer.toString(intValue);
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] sysSendBufSize ='";
			_lastErrorText += Integer.toString(chanCfg.sysSendBufSize);
			_lastErrorText += "' for ";
			return 13;
		}
		
		intLongValue = JUnitTestConnect.configGetIntLongValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.CompressionThreshold);
		if(intLongValue != chanCfg.compressionThreshold)
		{
			_lastErrorText = "CompressionThreshold mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig compressionThreshold ='";
			_lastErrorText += Integer.toString(intValue);
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] compressionThreshold ='";
			_lastErrorText += Integer.toString(chanCfg.compressionThreshold);
			_lastErrorText += "' for ";
			return 14;
		}

		intLongValue = JUnitTestConnect.configGetIntLongValue(consConfig, channelName, JUnitTestConnect.ConfigGroupTypeChannel, JUnitTestConnect.ConnectionPingTimeout);
		if(intLongValue != chanCfg.connectionPingTimeout)
		{
			_lastErrorText = "ConnectionPingTimeout mismatch in '";
			_lastErrorText += channelName;
			_lastErrorText += "' FileConfig connectionPingTimeout ='";
			_lastErrorText += Integer.toString(intValue);
			_lastErrorText += "' Internal Active ChannelSet[";
			_lastErrorText += position;
			_lastErrorText += "] connectionPingTimeout ='";
			_lastErrorText += Integer.toString(chanCfg.connectionPingTimeout);
			_lastErrorText += "' for ";
			return 15;
		}

	
		return result;
	}
	
	// used only for JUNIT tests
	public static int configVerifyConsChannelSetAttribs(OmmConsumer consumer, OmmConsumerConfig consConfig, String consumerName )
	{
		_lastErrorText = "";
		int result = 0;
		OmmConsumerImpl consImpl = ( OmmConsumerImpl ) consumer;
		
		String channelName = configGetChannelName(consConfig, consumerName);
		if(channelName == null)
		{
			_lastErrorText = "Channel is null for ";
			_lastErrorText += consImpl.consumerName();
			result = 1;
			return 1;
		}
		
		String [] channels  = channelName.split(",");
		if(channels.length != consImpl.activeConfig().channelConfigSet.size())
		{
			_lastErrorText = "Channel set size is != number of channels in the file config channelSet for ";
			_lastErrorText += consImpl.consumerName();
			return 2;
		}
		String channName = null;
		String position = null;
		ChannelConfig lastChanCfg = consImpl.activeConfig().channelConfigSet.get( channels.length - 1);
		for (int i = 0; i < channels.length; i++)
		{
			ChannelConfig chanCfg = consImpl.activeConfig().channelConfigSet.get(i);
			channName = channels[i];
			position = Integer.toString(i);
			int channelConnType = configGetChannelType(consConfig, channName);
			if( channName.equals(chanCfg.name) == false )
			{
				_lastErrorText = "ChannelName mismatch: FileConfig name='";
				_lastErrorText += channName;
				_lastErrorText += "' Internal Active ChannelSet[";
				_lastErrorText += position;
				_lastErrorText += "] name='";
				_lastErrorText += chanCfg.name;
				_lastErrorText += "' for ";
				_lastErrorText += consImpl.consumerName();
				return 3;
			}
			if( channelConnType != chanCfg.rsslConnectionType )
			{
				_lastErrorText = "ConnectionType mismatch in '";
				_lastErrorText += channName;
				_lastErrorText += "' FileConfig ConnectionType='";
				_lastErrorText += Integer.toString(channelConnType);
				_lastErrorText += "' Internal Active ChannelSet[";
				_lastErrorText += position;
				_lastErrorText += "] ConnectionType='";
				_lastErrorText += Integer.toString(chanCfg.rsslConnectionType);
				_lastErrorText += "' for ";
				_lastErrorText += consImpl.consumerName();
				return 4;
			}
			switch( channelConnType )
			{
			case com.thomsonreuters.upa.transport.ConnectionTypes.SOCKET:
				{
					result = configVerifyChannelSocketTypeAttribs(chanCfg, position, consConfig, channName);
					break;
				}
			case com.thomsonreuters.upa.transport.ConnectionTypes.ENCRYPTED:
				{
					result = configVerifyChannelEncrypTypeAttribs(chanCfg, position, consConfig, channName);
					break;
				}			
			default:
				break;
			}
			if(result != 0)
			{
				_lastErrorText += consImpl.consumerName();
				break;
			}
			else
			{
				result = configVerifyChannelCommonAttribs(chanCfg, position, consConfig, channName, lastChanCfg);
				if(result != 0)
				{
					_lastErrorText += consImpl.consumerName();
					break;
				}
			}
		}		

		return result;
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
		String host =  configImpl.getUserSpecifiedHostname();
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
		String port =  configImpl.getUserSpecifiedPort();
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
	
	public static double configDoubleIntValue(OmmConsumerConfig consConfig, String name, int type, int configParam)
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
				return (ce.doubleValue());
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
	
	public static OmmConsumer createOmmConsumer(OmmConsumerConfig consConfig)
	{
		return new OmmConsumerImpl(consConfig, true);
	}
	
	public static OmmProvider createOmmIProvider(OmmIProviderConfig provConfig)
	{
		return new OmmIProviderImpl(provConfig);
	}
	
	public static OmmProvider createOmmNiProvider(OmmNiProviderConfig provConfig)
	{
		return new OmmNiProviderImpl(provConfig, true);
	}
	
	public static boolean activeConfigGetBooleanValue(OmmConsumer consumer, int type, int configParam, int channelIndex)
	{
		ChannelConfig chanConfig = null;
		OmmConsumerImpl consImpl = (OmmConsumerImpl) consumer;
		
		if (consImpl == null || consImpl.activeConfig() == null)
		{
			_lastErrorText = "Not initialize OmmConsumerImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		
		ActiveConfig activeConfig = consImpl.activeConfig();
		
		if (type == ConfigGroupTypeConsumer)
		{
			if (configParam == XmlTraceToStdout)
				return activeConfig.xmlTraceEnable;
			else if (configParam == ConsumerMsgKeyInUpdates)
				return activeConfig.msgKeyInUpdates;
		}
		else if (type == ConfigGroupTypeChannel)
		{
			if (channelIndex >= 0)
			{
				if (channelIndex >= activeConfig.channelConfigSet.size())
				{
					_lastErrorText = "ChannelIndex is out of range ";
					throw new IllegalArgumentException(_lastErrorText);   
				}
				
				chanConfig = activeConfig.channelConfigSet.get(channelIndex);
				if (chanConfig == null)
				{
					_lastErrorText = "Unable to find the active channel config object ";
					throw new NullPointerException(_lastErrorText);  
				}
			}
			
			if (configParam == TcpNodelay)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketChannelConfig)chanConfig).tcpNodelay;
				else if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).tcpNodelay;
			}
			else if (configParam == DirectWrite)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketChannelConfig)chanConfig).directWrite;
			}
			else if (configParam == EnableSessionMgnt)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((EncryptedChannelConfig)chanConfig).enableSessionMgnt;
			}
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}	
	
	public static int activeConfigGetIntLongValue(OmmConsumer consumer, int type, int configParam, int channelIndex)
	{
		ChannelConfig chanConfig = null;
		OmmConsumerImpl consImpl = (OmmConsumerImpl) consumer;
		
		if (consImpl == null || consImpl.activeConfig() == null)
		{
			_lastErrorText = "Not initialize OmmConsumerImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);   
		}
		
		ActiveConfig activeConfig = consImpl.activeConfig();
		
		if (type == ConfigGroupTypeConsumer)
		{
			if (configParam == ItemCountHint)
				return activeConfig.itemCountHint;
			else if (configParam == ServiceCountHint)
				return activeConfig.serviceCountHint;
			else if (configParam == ConsumerObeyOpenWindow)
				return activeConfig.obeyOpenWindow;
			else if (configParam == ConsumerPostAckTimeout)
				return activeConfig.postAckTimeout;
			else if (configParam == RequestTimeout)
				return activeConfig.requestTimeout;
			else if (configParam == ConsumerMaxOutstandingPosts)
				return activeConfig.maxOutstandingPosts;
			else if (configParam == DispatchTimeoutApiThread)
				return activeConfig.dispatchTimeoutApiThread;
			else if (configParam == MaxDispatchCountUserThread)
				return activeConfig.maxDispatchCountUserThread;
			else if (configParam == MaxDispatchCountApiThread)
				return activeConfig.maxDispatchCountApiThread;
			else if (configParam == ReconnectAttemptLimit)
				return activeConfig.reconnectAttemptLimit;
			else if (configParam == ReconnectMinDelay)
				return activeConfig.reconnectMinDelay;
			else if (configParam == ReconnectMaxDelay)
				return activeConfig.reconnectMaxDelay;
			else if (configParam == RestRequestTimeout)
				return activeConfig.restRequestTimeout;
			else if (configParam == ReissueTokenAttemptLimit)
				return activeConfig.reissueTokenAttemptLimit;
			else if (configParam == ReissueTokenAttemptInterval)
				return activeConfig.reissueTokenAttemptInterval;
		
			
		}
		else if (type == ConfigGroupTypeChannel)
		{
			if (channelIndex >= 0)
			{
				if (channelIndex >= activeConfig.channelConfigSet.size())
				{
					_lastErrorText = "ChannelIndex is out of range ";
					throw new IllegalArgumentException(_lastErrorText);  
				}
				
				chanConfig = activeConfig.channelConfigSet.get(channelIndex);
				if (chanConfig == null)
				{
					_lastErrorText = "Unable to find the active channel config object ";
					throw new NullPointerException(_lastErrorText);  
				}
			}
			
			if (configParam == ChannelType)
				return chanConfig.rsslConnectionType;
			else if (configParam == CompressionType)
				return chanConfig.compressionType;
			else if (configParam == GuaranteedOutputBuffers)
				return chanConfig.guaranteedOutputBuffers;
			else if (configParam == NumInputBuffers)
				return chanConfig.numInputBuffers;
			else if (configParam == SysRecvBufSize)
				return chanConfig.sysRecvBufSize;
			else if (configParam == SysSendBufSize)
				return chanConfig.sysSendBufSize;
			else if (configParam == HighWaterMark)
				return chanConfig.highWaterMark;
			else if (configParam == CompressionThreshold)
				return chanConfig.compressionThreshold;
			else if (configParam == ConnectionPingTimeout)
				return chanConfig.connectionPingTimeout;
			else if (configParam == ChannelInitTimeout)
				return chanConfig.initializationTimeout;
		}
		else if (type == ConfigGroupTypeDictionary)
		{
			DictionaryConfig dictConfig = activeConfig.dictionaryConfig;
			if (dictConfig == null)
			{
				_lastErrorText = "Unable to find the active dictionary config object ";
				throw new NullPointerException(_lastErrorText);  
			}
			if (configParam == DictionaryType)
				return dictConfig.isLocalDictionary ? 1 : 0;
			
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}
	
	public static double activeConfigGetDoubleValue(OmmConsumer consumer, int type, int configParam, int channelIndex)
	{
		ChannelConfig chanConfig = null;
		OmmConsumerImpl consImpl = (OmmConsumerImpl) consumer;
		
		if (consImpl == null || consImpl.activeConfig() == null)
		{
			_lastErrorText = "Not initialize OmmConsumerImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);   
		}
		
		ActiveConfig activeConfig = consImpl.activeConfig();
		
		if (type == ConfigGroupTypeConsumer)
		{
			if (configParam == TokenReissueRatio )
				return activeConfig.tokenReissueRatio;
		}
		else if (type == ConfigGroupTypeChannel)
		{
			if (channelIndex >= 0)
			{
				if (channelIndex >= activeConfig.channelConfigSet.size())
				{
					_lastErrorText = "ChannelIndex is out of range ";
					throw new IllegalArgumentException(_lastErrorText);  
				}
				
				chanConfig = activeConfig.channelConfigSet.get(channelIndex);
				if (chanConfig == null)
				{
					_lastErrorText = "Unable to find the active channel config object ";
					throw new NullPointerException(_lastErrorText);  
				}
			}
		}
		else if (type == ConfigGroupTypeDictionary)
		{
			DictionaryConfig dictConfig = activeConfig.dictionaryConfig;
			if (dictConfig == null)
			{
				_lastErrorText = "Unable to find the active dictionary config object ";
				throw new NullPointerException(_lastErrorText);  
			}			
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}
	
	public static String activeConfigGetStringValue(OmmConsumer consumer, int type, int configParam, int channelIndex)
	{
		ChannelConfig chanConfig = null;
		OmmConsumerImpl consImpl = (OmmConsumerImpl) consumer;
		
		if (consImpl == null || consImpl.activeConfig() == null)
		{
			_lastErrorText = "Not initialize OmmConsumerImpl object or Active config object yet ";
			return null;
		}
		
		ActiveConfig activeConfig = consImpl.activeConfig();
		
		if (type == ConfigGroupTypeConsumer)
		{
			if (configParam == ConsumerDefaultConsumerName)
				return activeConfig.configuredName;
		}
		else if (type == ConfigGroupTypeChannel)
		{
			if (channelIndex >= 0)
			{
				if (channelIndex >= activeConfig.channelConfigSet.size())
				{
					_lastErrorText = "ChannelIndex is out of range ";
					throw new IllegalArgumentException(_lastErrorText);  
				}
				
				chanConfig = activeConfig.channelConfigSet.get(channelIndex);
				if (chanConfig == null)
				{
					_lastErrorText = "Unable to find the active channel config object ";
					throw new NullPointerException(_lastErrorText);  
				}
			}
			
			if (configParam == InterfaceName)
				return chanConfig.interfaceName;
			else if (configParam == ChannelName)
				return chanConfig.name;
			else if (configParam == Port)
			{
					if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
						return ((SocketChannelConfig)chanConfig).serviceName;
					else if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
						return ((HttpChannelConfig)chanConfig).serviceName;
			}
			else if (configParam == Host)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketChannelConfig)chanConfig).hostName;
				else if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).hostName;
			}
			else if (configParam == ObjectName)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).objectName;
			}
			else if (configParam == ProxyHost)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).httpProxyHostName;
			}
			else if (configParam == ProxyPort)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).httpProxyPort;
			}
			else if (configParam == Location)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((EncryptedChannelConfig)chanConfig).location;
			}
		}
		else if (type == ConfigGroupTypeDictionary)
		{
			DictionaryConfig dictConfig = activeConfig.dictionaryConfig;
			if (dictConfig == null)
			{
				_lastErrorText = "Unable to find the active dictionary config object ";
				throw new NullPointerException(_lastErrorText);  
			}
			if (configParam == DictionaryName)
				return dictConfig.dictionaryName;
			else if (configParam == DictionaryEnumTypeDefFileName)
				return dictConfig.enumtypeDefFileName;
			else if (configParam == DictionaryRDMFieldDictFileName)
				return dictConfig.rdmfieldDictionaryFileName;
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}	
	
	public static boolean activeConfigGetBooleanValue(OmmProvider provider, int type, int configParam, int channelIndex)
	{
		ChannelConfig chanConfig = null;
		OmmNiProviderImpl niprovImpl = (OmmNiProviderImpl) provider;
		
		if (niprovImpl == null || niprovImpl.activeConfig() == null)
		{
			_lastErrorText = "Not initialize OmmNiProviderImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		ActiveConfig activeConfig = niprovImpl.activeConfig();
		
		if (type == ConfigGroupTypeProvider)
		{
			if (configParam == XmlTraceToStdout)
				return activeConfig.xmlTraceEnable;
			else if (configParam == NiProviderMergeSourceDirectoryStreams)
				return ((OmmNiProviderActiveConfig)activeConfig).mergeSourceDirectoryStreams;
			else if (configParam == NiProviderRefreshFirstRequired)
				return ((OmmNiProviderActiveConfig)activeConfig).refreshFirstRequired;
			else if (configParam == NiProviderRecoverUserSubmitSourceDirectory)
				return ((OmmNiProviderActiveConfig)activeConfig).recoverUserSubmitSourceDirectory;
			else if (configParam == NiProviderRemoveItemsOnDisconnect)
				return ((OmmNiProviderActiveConfig)activeConfig).removeItemsOnDisconnect;
		}
		else if (type == ConfigGroupTypeChannel)
		{
			if (channelIndex >= 0)
			{
				if (channelIndex >= activeConfig.channelConfigSet.size())
				{
					_lastErrorText = "ChannelIndex is out of range ";
					throw new IllegalArgumentException(_lastErrorText);    
				}
				
				chanConfig = activeConfig.channelConfigSet.get(channelIndex);
				if (chanConfig == null)
				{
					_lastErrorText = "Unable to find the active channel config object ";
					throw new NullPointerException(_lastErrorText);  
				}
			}
			
			if (configParam == TcpNodelay)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketChannelConfig)chanConfig).tcpNodelay;
				else if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).tcpNodelay;
			}
			else if (configParam == DirectWrite)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketChannelConfig)chanConfig).directWrite;
			}
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}	
	
	public static int activeConfigGetIntLongValue(OmmProvider provider, int type, int configParam, int channelIndex)
	{
		ChannelConfig chanConfig = null;
		OmmNiProviderImpl niprovImpl = (OmmNiProviderImpl) provider;
		if (niprovImpl == null || niprovImpl.activeConfig() == null)
		{
			_lastErrorText = "Not initialize OmmNiProviderImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		ActiveConfig activeConfig = niprovImpl.activeConfig();
		
		if (type == ConfigGroupTypeProvider)
		{
			if (configParam == ItemCountHint)
				return activeConfig.itemCountHint;
			if (configParam == ServiceCountHint)
				return activeConfig.serviceCountHint;
			else if (configParam == RequestTimeout)
				return activeConfig.requestTimeout;
			else if (configParam == DispatchTimeoutApiThread)
				return activeConfig.dispatchTimeoutApiThread;
			else if (configParam == MaxDispatchCountUserThread)
				return activeConfig.maxDispatchCountUserThread;
			else if (configParam == MaxDispatchCountApiThread)
				return activeConfig.maxDispatchCountApiThread;
			else if (configParam == ReconnectAttemptLimit)
				return activeConfig.reconnectAttemptLimit;
			else if (configParam == ReconnectMinDelay)
				return activeConfig.reconnectMinDelay;
			else if (configParam == ReconnectMaxDelay)
				return activeConfig.reconnectMaxDelay;
			else if (configParam == LoginRequestTimeOut)
				return activeConfig.loginRequestTimeOut;
		}
		else if (type == ConfigGroupTypeChannel)
		{
			if (channelIndex >= 0)
			{
				if (channelIndex >= activeConfig.channelConfigSet.size())
				{
					_lastErrorText = "ChannelIndex is out of range ";
					throw new IllegalArgumentException(_lastErrorText);  
				}
				
				chanConfig = activeConfig.channelConfigSet.get(channelIndex);
				if (chanConfig == null)
				{
					_lastErrorText = "Unable to find the active channel config object ";
					throw new NullPointerException(_lastErrorText);  
				}
			}
			
			if (configParam == ChannelType)
				return chanConfig.rsslConnectionType;
			else if (configParam == CompressionType)
				return chanConfig.compressionType;
			else if (configParam == GuaranteedOutputBuffers)
				return chanConfig.guaranteedOutputBuffers;
			else if (configParam == NumInputBuffers)
				return chanConfig.numInputBuffers;
			else if (configParam == SysRecvBufSize)
				return chanConfig.sysRecvBufSize;
			else if (configParam == SysSendBufSize)
				return chanConfig.sysSendBufSize;
			else if (configParam == HighWaterMark)
				return chanConfig.highWaterMark;
			else if (configParam == CompressionThreshold)
				return chanConfig.compressionThreshold;
			else if (configParam == ConnectionPingTimeout)
				return chanConfig.connectionPingTimeout;
			else if (configParam == ChannelInitTimeout)
				return chanConfig.initializationTimeout;
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}
	
	public static String activeConfigGetStringValue(OmmProvider provider, int type, int configParam, int channelIndex) 
	{
		ChannelConfig chanConfig = null;
		OmmNiProviderImpl niprovImpl = (OmmNiProviderImpl) provider;
		if (niprovImpl == null || niprovImpl.activeConfig() == null)
		{
			_lastErrorText = "Not initialize OmmNiProviderImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		ActiveConfig activeConfig = niprovImpl.activeConfig();
	
		if (type == ConfigGroupTypeProvider)
		{
			if (configParam == NiProviderName)
				return activeConfig.configuredName;
			else if (configParam == DirectoryName)
				return ((OmmNiProviderDirectoryStore)niprovImpl.directoryServiceStore()).getApiControlDirectory().directoryName;
		}
		else if (type == ConfigGroupTypeChannel)
		{
			if (channelIndex >= 0)
			{
				if (channelIndex >= activeConfig.channelConfigSet.size())
				{
					_lastErrorText = "ChannelIndex is out of range ";
					throw new IllegalArgumentException(_lastErrorText);    
				}
				
				chanConfig = activeConfig.channelConfigSet.get(channelIndex);
				if (chanConfig == null)
				{
					_lastErrorText = "Unable to find the active channel config object ";
					throw new NullPointerException(_lastErrorText);  
				}
			}
			
			if (configParam == InterfaceName)
				return chanConfig.interfaceName;
			else if (configParam == ChannelName)
				return chanConfig.name;
			else if (configParam == Port)
			{
					if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
						return ((SocketChannelConfig)chanConfig).serviceName;
					else if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
						return ((HttpChannelConfig)chanConfig).serviceName;
			}
			else if (configParam == Host)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketChannelConfig)chanConfig).hostName;
				else if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).hostName;
			}
			else if (configParam == ObjectName)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).objectName;
			}
			else if (configParam == ProxyHost)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).httpProxyHostName;
			}
			else if (configParam == ProxyPort)
			{
				if (chanConfig.rsslConnectionType == ConnectionTypes.HTTP || chanConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
					return ((HttpChannelConfig)chanConfig).httpProxyPort;
			}
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}	
	
	public static boolean activeConfigGetBooleanValue(OmmProvider provider, int type, int configParam)
	{
		OmmIProviderImpl iprovImpl = (OmmIProviderImpl) provider;
		
		if (iprovImpl == null || iprovImpl._activeServerConfig == null)
		{
			_lastErrorText = "Not initialize OmmIProviderImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		ActiveServerConfig activeConfig = iprovImpl._activeServerConfig;
		
		if (type == ConfigGroupTypeProvider)
		{
			if (configParam == XmlTraceToStdout)
				return activeConfig.xmlTraceEnable;
			else if (configParam == IProviderRefreshFirstRequired)
				return ((OmmIProviderActiveConfig)activeConfig).refreshFirstRequired;
			else if (configParam == IProviderAcceptDirMessageWithoutMinFilters)
				return activeConfig.acceptDirMessageWithoutMinFilters;
			else if (configParam == IProviderAcceptMessageSameKeyButDiffStream)
				return activeConfig.acceptMessageSameKeyButDiffStream;
			else if (configParam == IProviderAcceptMessageThatChangesService)
				return activeConfig.acceptMessageThatChangesService;
			else if (configParam == IProviderAcceptMessageWithoutAcceptingRequests)
				return activeConfig.acceptMessageWithoutAcceptingRequests;
			else if (configParam == IProviderAcceptMessageWithoutBeingLogin)
				return activeConfig.acceptMessageWithoutBeingLogin;
			else if (configParam == IProviderAcceptMessageWithoutQosInRange)
				return activeConfig.acceptMessageWithoutQosInRange;
		}
		else if (type == ConfigGroupTypeServer)
		{
			if (configParam == TcpNodelay)
			{
				if (activeConfig.serverConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketServerConfig)activeConfig.serverConfig).tcpNodelay;
			}
			else if (configParam == DirectWrite)
			{
				if (activeConfig.serverConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketServerConfig)activeConfig.serverConfig).directWrite;
			}
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}	
	
	public static int activeConfigGetIntLongValue(OmmProvider provider, int type, int configParam)
	{
		OmmIProviderImpl iprovImpl = (OmmIProviderImpl) provider;
		if (iprovImpl == null || iprovImpl._activeServerConfig == null)
		{
			_lastErrorText = "Not initialize OmmIProviderImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		ActiveServerConfig activeConfig = iprovImpl._activeServerConfig;
		
		if (type == ConfigGroupTypeProvider)
		{
			if (configParam == ItemCountHint)
				return activeConfig.itemCountHint;
			if (configParam == ServiceCountHint)
				return activeConfig.serviceCountHint;
			else if (configParam == RequestTimeout)
				return activeConfig.requestTimeout;
			else if (configParam == DispatchTimeoutApiThread)
				return activeConfig.dispatchTimeoutApiThread;
			else if (configParam == MaxDispatchCountUserThread)
				return activeConfig.maxDispatchCountUserThread;
			else if (configParam == MaxDispatchCountApiThread)
				return activeConfig.maxDispatchCountApiThread;
			else if (configParam == DictionaryEnumTypeFragmentSize)
				return ((OmmIProviderActiveConfig)activeConfig).maxEnumTypeFragmentSize;
			else if (configParam == DictionaryFieldDictFragmentSize)
				return ((OmmIProviderActiveConfig)activeConfig).maxFieldDictFragmentSize;
		}
		else if (type == ConfigGroupTypeServer)
		{
			if (configParam == ServerType)
				return activeConfig.serverConfig.rsslConnectionType;
			else if (configParam == CompressionType)
				return activeConfig.serverConfig.compressionType;
			else if (configParam == GuaranteedOutputBuffers)
				return activeConfig.serverConfig.guaranteedOutputBuffers;
			else if (configParam == NumInputBuffers)
				return activeConfig.serverConfig.numInputBuffers;
			else if (configParam == SysRecvBufSize)
				return activeConfig.serverConfig.sysRecvBufSize;
			else if (configParam == SysSendBufSize)
				return activeConfig.serverConfig.sysSendBufSize;
			else if (configParam == HighWaterMark)
				return activeConfig.serverConfig.highWaterMark;
			else if (configParam == CompressionThreshold)
				return activeConfig.serverConfig.compressionThreshold;
			else if (configParam == ConnectionPingTimeout)
				return activeConfig.serverConfig.connectionPingTimeout;
			else if (configParam == ConnectionMinPingTimeout)
				return activeConfig.serverConfig.connectionMinPingTimeout;
			else if (configParam == ServerInitTimeout)
				return activeConfig.serverConfig.initializationTimeout;
		}
		
		throw new IllegalArgumentException("Invalid Input");   
	}
	
	public static String activeConfigGetStringValue(OmmProvider provider, int type, int configParam)
	{
		OmmIProviderImpl iprovImpl = (OmmIProviderImpl) provider;
		if (iprovImpl == null || iprovImpl._activeServerConfig == null)
		{
			_lastErrorText = "Not initialize OmmIProviderImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		ActiveServerConfig activeConfig = iprovImpl._activeServerConfig;
	
		if (type == ConfigGroupTypeProvider)
		{
			if (configParam == IProviderName)
				return activeConfig.configuredName;
			else if (configParam == ServerName)
				return activeConfig.serverConfig.name;
			else if (configParam == DirectoryName)
				return iprovImpl.directoryServiceStore().getDirectoryCache().directoryName;
		}
		else if (type == ConfigGroupTypeServer)
		{
			if (configParam == InterfaceName)
				return activeConfig.serverConfig.interfaceName;
			else if (configParam == Port)
			{
				if (activeConfig.serverConfig.rsslConnectionType == ConnectionTypes.SOCKET)
					return ((SocketServerConfig)activeConfig.serverConfig).serviceName;
			}
		}
		
		throw new IllegalArgumentException("Invalid Input");  
	}	
	
	public static List<com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service> activeConfigGetService(OmmProvider provider, boolean isIProv)
	{
		if (isIProv)
		{
			OmmIProviderImpl iprovImpl = (OmmIProviderImpl) provider;
			if (iprovImpl == null)
			{
				_lastErrorText = "Not initialize OmmIProviderImpl object yet ";
				throw new NullPointerException(_lastErrorText);  
			}
			return iprovImpl.directoryServiceStore().getDirectoryCache().serviceList();
		}
		else
		{
			OmmNiProviderImpl niprovImpl = (OmmNiProviderImpl) provider;
			if (niprovImpl == null)
			{
				_lastErrorText = "Not initialize OmmNiProviderImpl object yet ";
				throw new NullPointerException(_lastErrorText);  
			}
			return ((OmmNiProviderDirectoryStore)niprovImpl.directoryServiceStore()).getApiControlDirectory().serviceList();
		}
	}
	
	public static String activeConfigGetServiceDict(OmmProvider provider, int configParam, int serviceId, int dictIndex, boolean isDictProvided)
	{
		OmmIProviderImpl iprovImpl = (OmmIProviderImpl) provider;
		if (iprovImpl == null || iprovImpl._activeServerConfig == null)
		{
			_lastErrorText = "Not initialize OmmIProviderImpl object or Active config object yet ";
			throw new NullPointerException(_lastErrorText);  
		}
		
		ServiceDictionaryConfig serviceDictConfig = iprovImpl._activeServerConfig.getServiceDictionaryConfig(serviceId);
		if (serviceDictConfig == null)
		{
			_lastErrorText = "Unable to find the active dictionary config object ";
			throw new NullPointerException(_lastErrorText);  
		}
		
		List<DictionaryConfig> dictConfig = null;
		
		if (isDictProvided)
			dictConfig = serviceDictConfig.dictionaryProvidedList;
		else
			dictConfig = serviceDictConfig.dictionaryUsedList;
		
		if (dictConfig == null)
		{
			_lastErrorText = "Unable to find the active dictionary config object ";
			throw new NullPointerException(_lastErrorText);  
		}
		
		if (configParam == DictionaryName)
			return dictConfig.get(dictIndex).dictionaryName;
		else if (configParam == DictionaryEnumTypeDefFileName)
			return dictConfig.get(dictIndex).enumtypeDefFileName;
		else if (configParam == DictionaryRDMFieldDictFileName)
			return dictConfig.get(dictIndex).rdmfieldDictionaryFileName;
		else if (configParam == DictionaryEnumTypeDefItemName)
			return dictConfig.get(dictIndex).enumTypeDefItemName;
		else if (configParam == DictionaryRdmFieldDictionaryItemName)
			return dictConfig.get(dictIndex).rdmFieldDictionaryItemName;
		else if (configParam == DictionaryType)
			return (dictConfig.get(dictIndex).isLocalDictionary ? new String("FileDictionary") : new String("ChannelDictionary"));
		
		throw new IllegalArgumentException("Invalid Input");  
	}
}