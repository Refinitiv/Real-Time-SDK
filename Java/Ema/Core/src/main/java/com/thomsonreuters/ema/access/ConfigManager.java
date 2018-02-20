///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.ArrayList;
import java.util.Hashtable;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

import com.thomsonreuters.ema.access.ConfigReader.XMLnode;
import com.thomsonreuters.ema.rdm.EmaRdm;

class ConfigManager
{
	private static ConfigManager _configManager;
	static Hashtable<Integer, String> _nodeAsText;
	
	static TagDictionary ConsumerTagDict;
	static TagDictionary ChannelTagDict;
	static TagDictionary DictionaryTagDict;
	static TagDictionary NiProviderTagDict;
	static TagDictionary DirectoryTagDict;
	static TagDictionary ServiceTagDict;
	static TagDictionary IProviderTagDict;
	static TagDictionary ServerTagDict;
	
	static Branch DEFAULT_CONSUMER;
	static Branch DEFAULT_NIPROVIDER;
	static Branch DEFAULT_IPROVIDER;
	static Branch DEFAULT_DIRECTORY;
	static Branch CONSUMER_GROUP;
	static Branch CONSUMER_LIST;
	static Branch NIPROVIDER_GROUP;
	static Branch NIPROVIDER_LIST;
	static Branch IPROVIDER_GROUP;
	static Branch IPROVIDER_LIST;
	static Branch DICTIONARY_LIST;
	static Branch CHANNEL_LIST;
	static Branch SERVER_LIST;
	static Branch DIRECTORY_LIST;
	
	
	private static StringBuilder _stringMaker;
	
	static String ConfigName = "EmaConfig";
	
	static int ROOT = 1001;
	
	// Consumer
	public static final int ConsumerGroup = 1;
	public static final int DefaultConsumer = 2;
	public static final int ConsumerList = 3;
	public static final int Consumer = 4;
	
	public static final int ConsumerName = 5;
	public static final int ConsumerChannelName = 6;
	public static final int ConsumerLoggerName = 7;
	public static final int ConsumerDictionaryName = 8;
	
	public static final int CatchUnhandledException = 9;
	public static final int ConsumerCatchUnhandledException = 10;
	public static final int ChannelSet = 11;
	public static final int DictionaryRequestTimeOut = 12;
	public static final int DirectoryRequestTimeOut = 13;
	public static final int DispatchTimeoutApiThread = 14;
	public static final int ItemCountHint = 15;
	public static final int LoginRequestTimeOut = 16;
	public static final int MaxDispatchCountApiThread = 17;
	public static final int MaxDispatchCountUserThread = 18;
	public static final int MaxOutstandingPosts = 19;
	public static final int ObeyOpenWindow = 20;

	public static final int PostAckTimeout = 21;
	public static final int RequestTimeout = 22;
	public static final int ServiceCountHint = 23;
	
	public static final int ConnectionPingTimeout = 24;
	public static final int GuaranteedOutputBuffers = 25;
	public static final int InterfaceName = 26;
	public static final int NumInputBuffers = 27;
	public static final int SysRecvBufSize = 28;
	public static final int SysSendBufSize = 29;
	public static final int XmlTraceFileName = 30;
	public static final int XmlTraceHex = 31;
	public static final int XmlTraceMaxFileSize = 32;
	public static final int XmlTracePing = 33;
	public static final int XmlTraceRead = 34;
	public static final int XmlTraceToFile = 35;
	public static final int XmlTraceToMultipleFiles = 36;
	public static final int XmlTraceToStdout = 37;
	public static final int XmlTraceWrite = 38;
	public static final int HighWaterMark = 39;
	public static final int ConnectionMinPingTimeout = 40;
	public static final int MsgKeyInUpdates = 41;
	public static final int ReconnectAttemptLimit = 42;
	public static final int ReconnectMaxDelay = 43;	
	public static final int ReconnectMinDelay = 44;

	// Channel: Global
	public static final int ChannelGroup = 100;
	public static final int ChannelList = 101;
	public static final int Channel = 102;
	public static final int ChannelName = 103;
	public static final int ChannelType = 104;
	
	@Deprecated
	public static final int ChannelMsgKeyInUpdates = 105;
	@Deprecated
	public static final int ChannelReconnectAttemptLimit = 106;
	@Deprecated
	public static final int ChannelReconnectMaxDelay = 107;
	@Deprecated
	public static final int ChannelReconnectMinDelay = 108;

	
	// Channel: Socket, HTTP, Encrypted
	public static final int ChannelCompressionThreshold = 200;
	public static final int ChannelCompressionType = 201;
	public static final int ChannelHost = 202;
	public static final int ChannelObjectName = 203;
	public static final int ChannelPort = 204;
	public static final int ChannelTcpNodelay = 205;
	public static final int ChannelDirectSocketWrite = 206;
	public static final int ChannelProxyHost = 207;
	public static final int ChannelProxyPort = 208;
	
	// Channel: Multicast
	public static final int ChannelDisconnectOnGap = 300;
	public static final int ChannelHsmInterface = 301;
	public static final int ChannelHsmInterval = 302;
	public static final int ChannelHsmMultAddress = 303;
	public static final int ChannelHsmPort = 304;
	public static final int Channelndata = 305;
	public static final int Channelnmissing = 306;
	public static final int Channelnrreq = 307;
	public static final int ChannelPacketTTL = 308;
	public static final int ChannelpktPoolLimitHigh = 309;
	public static final int ChannelpktPoolLimitLow = 310;
	public static final int ChannelRecvAddress = 311;
	public static final int ChannelRecvPort = 312;
	public static final int ChannelSendAddress = 313;
	public static final int ChannelSendPort = 314;
	public static final int Channeltbchold = 315;
	public static final int ChanneltcpControlPort = 316;
	public static final int Channeltdata = 317;
	public static final int Channeltpphold = 318;
	public static final int Channeltrreq = 319;
	public static final int Channeltwait = 320;
	public static final int ChannelUnicastPort = 321;
	public static final int ChanneluserQLimit = 322;
		
	// Dictionary	
	public static final int DictionaryGroup = 400;
	public static final int DefaultDictionary = 401;
	public static final int DictionaryList = 402;
	public static final int Dictionary = 403;
	public static final int DictionaryName = 404;
	public static final int DictionaryType = 405;
	public static final int DictionaryEnumTypeDefFileName = 406;
	public static final int DictionaryRDMFieldDictFileName = 407;
	public static final int DictionaryDictionaryID = 408;
	public static final int DictionaryRdmFieldDictionaryItemName = 409;
	public static final int DictionaryEnumTypeDefItemName = 410;
	public static final int DictionaryFieldDictFragmentSize = 411;
	public static final int DictionaryEnumTypeFragmentSize = 412;
	
	// NIProvider
	public static final int NiProviderGroup = 500;
	public static final int DefaultNiProvider = 501;
	public static final int NiProviderList = 502;
	public static final int NiProvider = 503;
	
	public static final int NiProviderName = 504;
	public static final int NiProviderChannelName = 505;
	public static final int NiProviderDirectoryName = 506;
	public static final int NiProviderRefreshFirstRequired = 507;
	public static final int NiProviderMergeSourceDirectoryStreams = 508;
	public static final int NiProviderRecoverUserSubmitSourceDirectory = 509;
	public static final int NiProviderRemoveItemsOnDisconnect = 510;
	
	// Directory
	public static final int DirectoryGroup = 600;
	public static final int DefaultDirectory = 601;
	public static final int DirectoryList = 602;
	public static final int Directory = 603;
	public static final int DirectoryName = 604;
	
	// Service
	public static final int Service = 605;
	public static final int ServiceName = 606;
	public static final int ServiceInfoFilter = 607;
	public static final int ServiceInfoFilterServiceId = 608;
	public static final int ServiceInfoFilterVendor = 609;
	public static final int ServiceInfoFilterIsSource = 610;
	public static final int ServiceInfoFilterCapabilities = 611;
	public static final int ServiceInfoFilterCapabilitiesCapabilitiesEntry = 612;
	public static final int ServiceInfoFilterDictionariesProvided = 613;
	public static final int ServiceInfoFilterDictionariesProvidedDictionariesProvidedEntry = 614;
	public static final int ServiceInfoFilterDictionariesUsed = 615;
	public static final int ServiceInfoFilterDictionariesUsedDictionariesUsedEntry = 616;
	public static final int ServiceInfoFilterQoS = 617;
	public static final int ServiceInfoFilterQoSEntry = 618;
	public static final int ServiceInfoFilterQoSEntryTimeliness = 619;
	public static final int ServiceInfoFilterQoSEntryRate = 620;
	public static final int ServiceInfoFilterSupportsQoSRange = 621;
	public static final int ServiceInfoFilterItemList = 622;
	public static final int ServiceInfoFilterAcceptingConsumerStatus = 623;
	public static final int ServiceInfoFilterSupportsOutOfBandSnapshots = 624;
	public static final int ServiceStateFilter = 625;
	public static final int ServiceStateFilterServiceState = 626;
	public static final int ServiceStateFilterAcceptingRequests = 627;
	public static final int ServiceStateFilterStatus = 628;
	public static final int ServiceStateFilterStatusStreamState = 629;
	public static final int ServiceStateFilterStatusDataState = 630;
	public static final int ServiceStateFilterStatusStatusCode = 631;
	public static final int ServiceStateFilterStatusStatusText = 632;
	
	// IProvider
	public static final int IProviderGroup = 800;
	public static final int DefaultIProvider = 801;
	public static final int IProviderList = 802;
	public static final int IProvider = 803;
	
	public static final int IProviderName = 804;
	public static final int IProviderServerName = 805;
	public static final int IProviderDirectoryName = 806;
	public static final int IProviderRefreshFirstRequired = 807;
	public static final int IProviderAcceptMessageWithoutAcceptingRequests = 808;
	public static final int IProviderAcceptDirMessageWithoutMinFilters = 809;
	public static final int IProviderAcceptMessageWithoutBeingLogin = 810;
	public static final int IProviderAcceptMessageSameKeyButDiffStream = 811;
	public static final int IProviderAcceptMessageThatChangesService = 812;
	public static final int IProviderAcceptMessageWithoutQosInRange = 813;
	
	// Server: Global
	public static final int ServerGroup = 900;
	public static final int ServerList = 901;
	public static final int Server = 902;
	public static final int ServerName = 903;
	public static final int ServerType = 904;
	
	// Server: Socket
	public static final int ServerCompressionThreshold = 1000;
	public static final int ServerCompressionType = 1001;
	public static final int ServerPort = 1002;
	public static final int ServerTcpNodelay = 1003;
	public static final int ServerDirectSocketWrite = 1004;
	
	public static final int MAX_UINT16 = 0xFFFF;
	
	static
	{
		_stringMaker = new StringBuilder(512);
		_nodeAsText = new Hashtable<Integer,String>();
		
		ConsumerTagDict = acquire().new TagDictionary();
		ChannelTagDict = acquire().new TagDictionary();
		DictionaryTagDict = acquire().new TagDictionary();
		NiProviderTagDict = acquire().new TagDictionary();
		DirectoryTagDict = acquire().new TagDictionary();
		ServiceTagDict = acquire().new TagDictionary();
		IProviderTagDict = acquire().new TagDictionary();
		ServerTagDict = acquire().new TagDictionary();
		
		ConsumerTagDict.add( "ConsumerGroup",ConsumerGroup );
		ConsumerTagDict.add( "DefaultConsumer",DefaultConsumer );
		ConsumerTagDict.add( "ConsumerList",ConsumerList );
		ConsumerTagDict.add( "Consumer",Consumer );
		
		ConsumerTagDict.add( "Name",ConsumerName );
		ConsumerTagDict.add( "Channel",ConsumerChannelName );
		ConsumerTagDict.add( "ChannelSet",ChannelSet );
		ConsumerTagDict.add( "Logger",ConsumerLoggerName );
		ConsumerTagDict.add( "Dictionary",ConsumerDictionaryName );
		
		ConsumerTagDict.add( "CatchUnhandledException",CatchUnhandledException );
		ConsumerTagDict.add( "DictionaryRequestTimeOut",DictionaryRequestTimeOut );
		ConsumerTagDict.add( "DirectoryRequestTimeOut",DirectoryRequestTimeOut );
		ConsumerTagDict.add( "DispatchTimeoutApiThread",DispatchTimeoutApiThread );
		ConsumerTagDict.add( "ItemCountHint",ItemCountHint );
		ConsumerTagDict.add( "LoginRequestTimeOut",LoginRequestTimeOut );
		ConsumerTagDict.add( "MaxDispatchCountApiThread",MaxDispatchCountApiThread );
		ConsumerTagDict.add( "MaxDispatchCountUserThread",MaxDispatchCountUserThread );
		ConsumerTagDict.add( "MaxOutstandingPosts",MaxOutstandingPosts );
		ConsumerTagDict.add( "ObeyOpenWindow",ObeyOpenWindow );
		ConsumerTagDict.add( "PostAckTimeout",PostAckTimeout );
		ConsumerTagDict.add( "RequestTimeout",RequestTimeout );
		ConsumerTagDict.add( "ServiceCountHint",ServiceCountHint );
		ConsumerTagDict.add( "MsgKeyInUpdates",MsgKeyInUpdates );
		ConsumerTagDict.add( "ReconnectAttemptLimit",ReconnectAttemptLimit );		
		ConsumerTagDict.add( "ReconnectMaxDelay",ReconnectMaxDelay );
		ConsumerTagDict.add( "ReconnectMinDelay",ReconnectMinDelay );
		ConsumerTagDict.add( "XmlTraceFileName",XmlTraceFileName );		
		ConsumerTagDict.add( "XmlTraceHex", XmlTraceHex );
		ConsumerTagDict.add( "XmlTraceMaxFileSize", XmlTraceMaxFileSize );
		ConsumerTagDict.add( "XmlTracePing", XmlTracePing );
		ConsumerTagDict.add( "XmlTraceRead",XmlTraceRead );
		ConsumerTagDict.add( "XmlTraceToFile",XmlTraceToFile );
		ConsumerTagDict.add( "XmlTraceToMultipleFiles",XmlTraceToMultipleFiles );
		ConsumerTagDict.add( "XmlTraceToStdout",XmlTraceToStdout );
		ConsumerTagDict.add( "XmlTraceWrite",XmlTraceWrite );
		
		ChannelTagDict.add( "ChannelGroup",ChannelGroup );
		ChannelTagDict.add( "ChannelList",ChannelList );
		ChannelTagDict.add( "Channel",Channel );
		ChannelTagDict.add( "Name",ChannelName );
		ChannelTagDict.add( "ChannelType",ChannelType );
		ChannelTagDict.add( "ConnectionPingTimeout",ConnectionPingTimeout );
		ChannelTagDict.add( "GuaranteedOutputBuffers",GuaranteedOutputBuffers );
		ChannelTagDict.add( "InterfaceName",InterfaceName );
		ChannelTagDict.add( "MsgKeyInUpdates",ChannelMsgKeyInUpdates );
		ChannelTagDict.add( "NumInputBuffers",NumInputBuffers );		
		ChannelTagDict.add( "ReconnectAttemptLimit",ChannelReconnectAttemptLimit );		
		ChannelTagDict.add( "ReconnectMaxDelay",ChannelReconnectMaxDelay );
		ChannelTagDict.add( "ReconnectMinDelay",ChannelReconnectMinDelay );
		ChannelTagDict.add( "SysRecvBufSize",SysRecvBufSize );
		ChannelTagDict.add( "SysSendBufSize",SysSendBufSize );
		ChannelTagDict.add( "HighWaterMark",HighWaterMark );
		
		/* @deprecated The following code will be removed once all deprecated parameters are removed. */
		ChannelTagDict.add( "XmlTraceFileName",XmlTraceFileName );		
		ChannelTagDict.add( "XmlTraceHex", XmlTraceHex );
		ChannelTagDict.add( "XmlTraceMaxFileSize", XmlTraceMaxFileSize );
		ChannelTagDict.add( "XmlTracePing", XmlTracePing );
		ChannelTagDict.add( "XmlTraceRead",XmlTraceRead );
		ChannelTagDict.add( "XmlTraceToFile",XmlTraceToFile );
		ChannelTagDict.add( "XmlTraceToMultipleFiles",XmlTraceToMultipleFiles );
		ChannelTagDict.add( "XmlTraceToStdout",XmlTraceToStdout );
		ChannelTagDict.add( "XmlTraceWrite",XmlTraceWrite );
		
		// ConnectionTypes.SOCKET, ConnectionTypes.HTTP, ConnectionTypes.ENCRYPTED 
		ChannelTagDict.add( "CompressionThreshold",ChannelCompressionThreshold );
		ChannelTagDict.add( "CompressionType",ChannelCompressionType );
		ChannelTagDict.add( "Host",ChannelHost );
		ChannelTagDict.add( "ObjectName",ChannelObjectName );
		ChannelTagDict.add( "Port",ChannelPort );
		ChannelTagDict.add( "TcpNodelay",ChannelTcpNodelay );
		ChannelTagDict.add( "DirectWrite",ChannelDirectSocketWrite );
		ChannelTagDict.add( "ProxyHost",ChannelProxyHost );
		ChannelTagDict.add( "ProxyPort",ChannelProxyPort );
		
		// ConnectionTypes.MCAST
		ChannelTagDict.add( "DisconnectOnGap",ChannelDisconnectOnGap );
		ChannelTagDict.add( "HsmInterface",ChannelHsmInterface );
		ChannelTagDict.add( "HsmInterval",ChannelHsmInterval );
		ChannelTagDict.add( "HsmMultAddress",ChannelHsmMultAddress );
		ChannelTagDict.add( "HsmPort",ChannelHsmPort );
		ChannelTagDict.add( "ndata",Channelndata );
		ChannelTagDict.add( "nmissing",Channelnmissing );
		ChannelTagDict.add( "nrreq",Channelnrreq );
		ChannelTagDict.add( "PacketTTL",ChannelPacketTTL );
		ChannelTagDict.add( "pktPoolLimitHigh",ChannelpktPoolLimitHigh );
		ChannelTagDict.add( "pktPoolLimitLow",ChannelpktPoolLimitLow );
		ChannelTagDict.add( "RecvAddress",ChannelRecvAddress );
		ChannelTagDict.add( "RecvPort",ChannelRecvPort );
		ChannelTagDict.add( "SendAddress",ChannelSendAddress );
		ChannelTagDict.add( "SendPort",ChannelSendPort );
		ChannelTagDict.add( "tbchold",Channeltbchold );
		ChannelTagDict.add( "tcpControlPort",ChanneltcpControlPort );
		ChannelTagDict.add( "tdata",Channeltdata );
		ChannelTagDict.add( "tpphold",Channeltpphold );
		ChannelTagDict.add( "trreq",Channeltrreq );
		ChannelTagDict.add( "twait",Channeltwait );
		ChannelTagDict.add( "UnicastPort",ChannelUnicastPort );
		ChannelTagDict.add( "userQLimit",ChanneluserQLimit );
				
		DictionaryTagDict.add( "DictionaryGroup",DictionaryGroup );
		DictionaryTagDict.add( "DefaultDictionary",DefaultDictionary);
		DictionaryTagDict.add( "DictionaryList",DictionaryList );
		DictionaryTagDict.add( "Dictionary",Dictionary );
		DictionaryTagDict.add( "Name",DictionaryName );
		DictionaryTagDict.add( "DictionaryType",DictionaryType );
		DictionaryTagDict.add( "RdmFieldDictionaryFileName",DictionaryRDMFieldDictFileName );
		DictionaryTagDict.add( "EnumTypeDefFileName",DictionaryEnumTypeDefFileName );
		DictionaryTagDict.add( "DictionaryID", DictionaryDictionaryID);
		DictionaryTagDict.add( "RdmFieldDictionaryItemName", DictionaryRdmFieldDictionaryItemName);
		DictionaryTagDict.add( "EnumTypeDefItemName", DictionaryEnumTypeDefItemName);
		
		NiProviderTagDict.add( "NiProviderGroup", NiProviderGroup);
		NiProviderTagDict.add( "DefaultNiProvider", DefaultNiProvider);
		NiProviderTagDict.add( "NiProviderList", NiProviderList);
		NiProviderTagDict.add( "NiProvider", NiProvider);
		NiProviderTagDict.add( "Name", NiProviderName);
		NiProviderTagDict.add( "Channel", NiProviderChannelName);
		NiProviderTagDict.add( "ChannelSet", ChannelSet);
		NiProviderTagDict.add( "Directory", NiProviderDirectoryName);
        NiProviderTagDict.add( "DictionaryRequestTimeOut",DictionaryRequestTimeOut );
		NiProviderTagDict.add( "DispatchTimeoutApiThread",DispatchTimeoutApiThread );
		NiProviderTagDict.add( "ItemCountHint",ItemCountHint );
		NiProviderTagDict.add( "LoginRequestTimeOut",LoginRequestTimeOut );
		NiProviderTagDict.add( "MaxDispatchCountApiThread",MaxDispatchCountApiThread );
		NiProviderTagDict.add( "MaxDispatchCountUserThread",MaxDispatchCountUserThread );
		NiProviderTagDict.add( "ServiceCountHint",ServiceCountHint );
		NiProviderTagDict.add( "RefreshFirstRequired", NiProviderRefreshFirstRequired);
		NiProviderTagDict.add( "MergeSourceDirectoryStreams", NiProviderMergeSourceDirectoryStreams);
		NiProviderTagDict.add( "RecoverUserSubmitSourceDirectory", NiProviderRecoverUserSubmitSourceDirectory);
		NiProviderTagDict.add( "RemoveItemsOnDisconnect", NiProviderRemoveItemsOnDisconnect);
		NiProviderTagDict.add( "MsgKeyInUpdates",MsgKeyInUpdates );
		NiProviderTagDict.add( "ReconnectAttemptLimit",ReconnectAttemptLimit );		
		NiProviderTagDict.add( "ReconnectMaxDelay",ReconnectMaxDelay );
		NiProviderTagDict.add( "ReconnectMinDelay",ReconnectMinDelay );
		NiProviderTagDict.add( "RequestTimeout", RequestTimeout);
		NiProviderTagDict.add( "XmlTraceFileName",XmlTraceFileName );		
		NiProviderTagDict.add( "XmlTraceHex", XmlTraceHex );
		NiProviderTagDict.add( "XmlTraceMaxFileSize", XmlTraceMaxFileSize );
		NiProviderTagDict.add( "XmlTracePing", XmlTracePing );
		NiProviderTagDict.add( "XmlTraceRead",XmlTraceRead );
		NiProviderTagDict.add( "XmlTraceToFile",XmlTraceToFile );
		NiProviderTagDict.add( "XmlTraceToMultipleFiles",XmlTraceToMultipleFiles );
		NiProviderTagDict.add( "XmlTraceToStdout",XmlTraceToStdout );
		NiProviderTagDict.add( "XmlTraceWrite",XmlTraceWrite );
		
		
		DirectoryTagDict.add( "DirectoryGroup", DirectoryGroup);
		DirectoryTagDict.add( "DefaultDirectory", DefaultDirectory);
		DirectoryTagDict.add( "DirectoryList", DirectoryList);
		DirectoryTagDict.add( "Directory", Directory);
		DirectoryTagDict.add( "Name", DirectoryName);
		
		ServiceTagDict.add( "Service", Service);
		ServiceTagDict.add( "Name", ServiceName);
		ServiceTagDict.add( "InfoFilter", ServiceInfoFilter);
		ServiceTagDict.add( "ServiceId", ServiceInfoFilterServiceId);
		ServiceTagDict.add( "Vendor", ServiceInfoFilterVendor);
		ServiceTagDict.add( "IsSource", ServiceInfoFilterIsSource);
		ServiceTagDict.add( "Capabilities", ServiceInfoFilterCapabilities);
		ServiceTagDict.add( "CapabilitiesEntry", ServiceInfoFilterCapabilitiesCapabilitiesEntry);
		ServiceTagDict.add( "DictionariesProvided", ServiceInfoFilterDictionariesProvided);
		ServiceTagDict.add( "DictionariesProvidedEntry", ServiceInfoFilterDictionariesProvidedDictionariesProvidedEntry);
		ServiceTagDict.add( "DictionariesUsed", ServiceInfoFilterDictionariesUsed);
		ServiceTagDict.add( "DictionariesUsedEntry", ServiceInfoFilterDictionariesUsedDictionariesUsedEntry);
		ServiceTagDict.add( "QoS", ServiceInfoFilterQoS);
		ServiceTagDict.add( "QoSEntry", ServiceInfoFilterQoSEntry);
		ServiceTagDict.add( "Timeliness", ServiceInfoFilterQoSEntryTimeliness);
		ServiceTagDict.add( "Rate", ServiceInfoFilterQoSEntryRate);
		ServiceTagDict.add( "SupportsQoSRange", ServiceInfoFilterSupportsQoSRange);
		ServiceTagDict.add( "ItemList", ServiceInfoFilterItemList);
		ServiceTagDict.add( "AcceptingConsumerStatus", ServiceInfoFilterAcceptingConsumerStatus);
		ServiceTagDict.add( "SupportsOutOfBandSnapshots", ServiceInfoFilterSupportsOutOfBandSnapshots);
		ServiceTagDict.add( "StateFilter", ServiceStateFilter);
		ServiceTagDict.add( "ServiceState", ServiceStateFilterServiceState);
		ServiceTagDict.add( "AcceptingRequests", ServiceStateFilterAcceptingRequests);
		ServiceTagDict.add( "Status", ServiceStateFilterStatus);
		ServiceTagDict.add( "StreamState", ServiceStateFilterStatusStreamState);
		ServiceTagDict.add( "DataState", ServiceStateFilterStatusDataState);
		ServiceTagDict.add( "StatusCode", ServiceStateFilterStatusStatusCode);
		ServiceTagDict.add( "StatusText", ServiceStateFilterStatusStatusText);
		
		IProviderTagDict.add("IProviderGroup", IProviderGroup);
		IProviderTagDict.add("DefaultIProvider", DefaultIProvider);
		IProviderTagDict.add("IProviderList", IProviderList);
		IProviderTagDict.add("IProvider", IProvider);
		IProviderTagDict.add("Name", IProviderName);
		IProviderTagDict.add("Server", IProviderServerName);
		IProviderTagDict.add("Directory", IProviderDirectoryName );
		IProviderTagDict.add( "CatchUnhandledException",CatchUnhandledException );
		IProviderTagDict.add( "DictionaryRequestTimeOut",DictionaryRequestTimeOut );
		IProviderTagDict.add( "DispatchTimeoutApiThread",DispatchTimeoutApiThread );
		IProviderTagDict.add( "ItemCountHint",ItemCountHint );
		IProviderTagDict.add( "MaxDispatchCountApiThread",MaxDispatchCountApiThread );
		IProviderTagDict.add( "MaxDispatchCountUserThread",MaxDispatchCountUserThread );
		IProviderTagDict.add( "PostAckTimeout",PostAckTimeout );
		IProviderTagDict.add( "ServiceCountHint",ServiceCountHint );
		IProviderTagDict.add( "RefreshFirstRequired", IProviderRefreshFirstRequired);
		IProviderTagDict.add( "RequestTimeout", RequestTimeout);
		IProviderTagDict.add( "AcceptMessageWithoutAcceptingRequests", IProviderAcceptMessageWithoutAcceptingRequests);
		IProviderTagDict.add( "AcceptDirMessageWithoutMinFilters", IProviderAcceptDirMessageWithoutMinFilters);
		IProviderTagDict.add( "AcceptMessageWithoutBeingLogin", IProviderAcceptMessageWithoutBeingLogin);
		IProviderTagDict.add( "AcceptMessageSameKeyButDiffStream", IProviderAcceptMessageSameKeyButDiffStream);
		IProviderTagDict.add( "AcceptMessageThatChangesService", IProviderAcceptMessageThatChangesService);
		IProviderTagDict.add( "AcceptMessageWithoutQosInRange", IProviderAcceptMessageWithoutQosInRange);
		IProviderTagDict.add( "FieldDictionaryFragmentSize", DictionaryFieldDictFragmentSize);
		IProviderTagDict.add( "EnumTypeFragmentSize", DictionaryEnumTypeFragmentSize);
		IProviderTagDict.add( "XmlTraceFileName",XmlTraceFileName );		
		IProviderTagDict.add( "XmlTraceHex", XmlTraceHex );
		IProviderTagDict.add( "XmlTraceMaxFileSize", XmlTraceMaxFileSize );
		IProviderTagDict.add( "XmlTracePing", XmlTracePing );
		IProviderTagDict.add( "XmlTraceRead", XmlTraceRead );
		IProviderTagDict.add( "XmlTraceToFile", XmlTraceToFile );
		IProviderTagDict.add( "XmlTraceToMultipleFiles", XmlTraceToMultipleFiles );
		IProviderTagDict.add( "XmlTraceToStdout", XmlTraceToStdout );
		
		ServerTagDict.add( "ServerGroup", ServerGroup );
		ServerTagDict.add( "ServerList" , ServerList );
		ServerTagDict.add( "Server", Server );
		ServerTagDict.add( "Name", ServerName );
		ServerTagDict.add( "ServerType", ServerType );
		ServerTagDict.add( "CompressionThreshold", ServerCompressionThreshold);
		ServerTagDict.add( "CompressionType", ServerCompressionType);
		ServerTagDict.add( "ConnectionPingTimeout",ConnectionPingTimeout );
		ServerTagDict.add( "GuaranteedOutputBuffers",GuaranteedOutputBuffers );
		ServerTagDict.add( "InterfaceName",InterfaceName );
		ServerTagDict.add( "NumInputBuffers",NumInputBuffers );		
		ServerTagDict.add( "SysRecvBufSize",SysRecvBufSize );
		ServerTagDict.add( "SysSendBufSize",SysSendBufSize );
		ServerTagDict.add( "HighWaterMark",HighWaterMark );
		
		/* @deprecated The following code will be removed once all deprecated parameters are removed. */
		ServerTagDict.add( "XmlTraceFileName",XmlTraceFileName );		
		ServerTagDict.add( "XmlTraceHex", XmlTraceHex );
		ServerTagDict.add( "XmlTraceMaxFileSize", XmlTraceMaxFileSize );
		ServerTagDict.add( "XmlTracePing", XmlTracePing );
		ServerTagDict.add( "XmlTraceRead", XmlTraceRead );
		ServerTagDict.add( "XmlTraceToFile", XmlTraceToFile );
		ServerTagDict.add( "XmlTraceToMultipleFiles", XmlTraceToMultipleFiles );
		ServerTagDict.add( "XmlTraceToStdout", XmlTraceToStdout );
		
		ServerTagDict.add( "CompressionThreshold",ServerCompressionThreshold );
		ServerTagDict.add( "Port", ServerPort );
		ServerTagDict.add( "TcpNodelay",ServerTcpNodelay );
		ServerTagDict.add( "DirectWrite",ServerDirectSocketWrite );
		ServerTagDict.add( "ConnectionMinPingTimeout", ConnectionMinPingTimeout);
		
		CONSUMER_GROUP = ConfigManager.acquire().new Branch();
		CONSUMER_GROUP.add(ConfigManager.ConsumerGroup,ConfigManager.ConsumerTagDict);
		CONSUMER_GROUP.complete();

		DEFAULT_CONSUMER = ConfigManager.acquire().new Branch();
		DEFAULT_CONSUMER.add(ConfigManager.ConsumerGroup,ConfigManager.ConsumerTagDict);
		DEFAULT_CONSUMER.add(ConfigManager.DefaultConsumer,ConfigManager.ConsumerTagDict);
		DEFAULT_CONSUMER.complete();

		CONSUMER_LIST = ConfigManager.acquire().new Branch();
		CONSUMER_LIST.add(ConfigManager.ConsumerGroup,ConfigManager.ConsumerTagDict);
		CONSUMER_LIST.add(ConfigManager.ConsumerList,ConfigManager.ConsumerTagDict);
		CONSUMER_LIST.complete();
		
		DICTIONARY_LIST = ConfigManager.acquire().new Branch();
		DICTIONARY_LIST.add(ConfigManager.DictionaryGroup,ConfigManager.DictionaryTagDict);
		DICTIONARY_LIST.add(ConfigManager.DictionaryList,ConfigManager.DictionaryTagDict);
		DICTIONARY_LIST.complete();
		
		CHANNEL_LIST = ConfigManager.acquire().new Branch();
		CHANNEL_LIST.add(ConfigManager.ChannelGroup,ConfigManager.ChannelTagDict);
		CHANNEL_LIST.add(ConfigManager.ChannelList,ConfigManager.ChannelTagDict);
		CHANNEL_LIST.complete();
		
		NIPROVIDER_GROUP = ConfigManager.acquire().new Branch();
		NIPROVIDER_GROUP.add(ConfigManager.NiProviderGroup,ConfigManager.NiProviderTagDict);
		NIPROVIDER_GROUP.complete();
		
		DEFAULT_NIPROVIDER = ConfigManager.acquire().new Branch();
		DEFAULT_NIPROVIDER.add(ConfigManager.NiProviderGroup,ConfigManager.NiProviderTagDict);
		DEFAULT_NIPROVIDER.add(ConfigManager.DefaultNiProvider,ConfigManager.NiProviderTagDict);
		DEFAULT_NIPROVIDER.complete();
		
		NIPROVIDER_LIST = ConfigManager.acquire().new Branch();
		NIPROVIDER_LIST.add(ConfigManager.NiProviderGroup,ConfigManager.NiProviderTagDict);
		NIPROVIDER_LIST.add(ConfigManager.NiProviderList,ConfigManager.NiProviderTagDict);
		NIPROVIDER_LIST.complete();
		
		DEFAULT_DIRECTORY = ConfigManager.acquire().new Branch();
		DEFAULT_DIRECTORY.add(ConfigManager.DirectoryGroup,ConfigManager.DirectoryTagDict);
		DEFAULT_DIRECTORY.add(ConfigManager.DefaultDirectory,ConfigManager.DirectoryTagDict);
		DEFAULT_DIRECTORY.complete();
		
		DIRECTORY_LIST = ConfigManager.acquire().new Branch();
		DIRECTORY_LIST.add(ConfigManager.DirectoryGroup,ConfigManager.DirectoryTagDict);
		DIRECTORY_LIST.add(ConfigManager.DirectoryList,ConfigManager.DirectoryTagDict);
		DIRECTORY_LIST.complete();
		
		IPROVIDER_GROUP = ConfigManager.acquire().new Branch();
		IPROVIDER_GROUP.add(ConfigManager.IProviderGroup,ConfigManager.IProviderTagDict);
		IPROVIDER_GROUP.complete();
		
		DEFAULT_IPROVIDER = ConfigManager.acquire().new Branch();
		DEFAULT_IPROVIDER.add(ConfigManager.IProviderGroup,ConfigManager.IProviderTagDict);
		DEFAULT_IPROVIDER.add(ConfigManager.DefaultIProvider,ConfigManager.IProviderTagDict);
		DEFAULT_IPROVIDER.complete();
		
		IPROVIDER_LIST = ConfigManager.acquire().new Branch();
		IPROVIDER_LIST.add(ConfigManager.IProviderGroup,ConfigManager.IProviderTagDict);
		IPROVIDER_LIST.add(ConfigManager.IProviderList,ConfigManager.IProviderTagDict);
		IPROVIDER_LIST.complete();
		
		SERVER_LIST = ConfigManager.acquire().new Branch();
		SERVER_LIST.add(ConfigManager.ServerGroup,ConfigManager.ServerTagDict);
		SERVER_LIST.add(ConfigManager.ServerList,ConfigManager.ServerTagDict);
		SERVER_LIST.complete();
	}

	public static String AsciiValues[] = {
			"CapabilitiesEntry",
			"Channel",
			"ChannelSet",
			"DefaultConsumer",
			"DefaultDictionary",
			"DefaultDirectory",
			"DefaultIProvider",
			"DefaultNiProvider",
			"DictionariesProvidedEntry",
			"DictionariesUsedEntry",
			"Dictionary",
			"Directory",
			"EnumTypeDefFileName",
			"EnumTypeDefItemName",
			"FileName",
			"Host",
			"HsmInterface",
			"HsmMultAddress",
			"HsmPort",
			"InterfaceName",
			"ItemList",
			"Logger",
			"Name",
			"ObjectName",
			"Port",
			"Rate",
			"RdmFieldDictionaryFileName",
			"RdmFieldDictionaryItemName",
			"RecvAddress",
			"RecvPort",
			"SendAddress",
			"SendPort",
			"Server",
			"StatusText",
			"tcpControlPort",
			"Timeliness",
			"UnicastPort",
			"Vendor",
			"XmlTraceFileName",
			"ProxyHost",
			"ProxyPort"
	};

	public static String EnumeratedValues[] = {
		"ChannelType",
		"CompressionType",
		"DataState",
		"DictionaryType",
		"ServerType",
		"StatusCode",
		"StreamState"
	};

	public static String Int64Values[] = {
		"DictionaryID",
		"DispatchTimeoutApiThread",
		"ReconnectAttemptLimit",
		"ReconnectMaxDelay",
		"ReconnectMinDelay",
		"XmlTraceMaxFileSize"
	};

	public static String UInt64Values[] = {
	    "AcceptDirMessageWithoutMinFilters",
		"AcceptingConsumerStatus",
		"AcceptingRequests",
		"AcceptMessageSameKeyButDiffStream",
		"AcceptMessageThatChangesService",
		"AcceptMessageWithoutAcceptingRequests",
		"AcceptMessageWithoutBeingLogin",
		"AcceptMessageWithoutQosInRange",
		"CompressionThreshold",
		"ConnectionPingTimeout",
		"ConnectionMinPingTimeout",
		"DictionaryRequestTimeOut",
		"DirectoryRequestTimeOut",
		"DisconnectOnGap",
		"EnumTypeFragmentSize",
		"FieldDictionaryFragmentSize",
		"GuaranteedOutputBuffers",
		"HsmInterval",
		"IncludeDateInLoggerOutput",
		"ItemCountHint",
		"IsSource",
		"LoginRequestTimeOut",
		"MaxDispatchCountApiThread",
		"MaxDispatchCountUserThread",
		"MaxOutstandingPosts",
		"MergeSourceDirectoryStreams",
		"MsgKeyInUpdates",
		"ndata",
		"nmissing",
		"nrreq",
		"NumInputBuffers",
		"ObeyOpenWindow",
		"PacketTTL",
		"pktPoolLimitHigh",
		"pktPoolLimitLow",
		"PostAckTimeout",
		"RecoverUserSubmitSourceDirectory",
		"RefreshFirstRequired",
		"RemoveItemsOnDisconnect",
		"RequestTimeout",
		"ServiceCountHint",
		"ServiceId",
		"ServiceState",
		"SupportsOutOfBandSnapshots",
		"SupportsQoSRange",
		"SysRecvBufSize",
		"SysSendBufSize",
		"HighWaterMark",
		"tbchold",
		"TcpNodelay",
		"tdata",
		"tpphold",
		"trreq",
		"twait",
		"userQLimit",
		"DirectWrite",
		"XmlTraceHex",
		"XmlTraceMaxFileSize",
		"XmlTracePing",
		"XmlTraceRead",
		"XmlTraceToFile",		
		"XmlTraceToMultipleFiles",
		"XmlTraceToStdout",
		"XmlTraceWrite"
	};
	public static String NodesThatRequireName[] = {
		"Channel",
		"Consumer",
		"Dictionary",
		"Directory",
		"IProvider",
		"NiProvider",
		"Service",
		"Server"
	};
	
	static ConfigManager acquire() 
	{
		if(_configManager==null)
			_configManager = new ConfigManager();
		
		return _configManager;
	}
	
	 ConfigAttributes createConfigAttributes()
	{
		return( new ConfigAttributes());
	}

	static StringBuilder stringMaker() 
	{
		return _stringMaker;
	}

	static String nodeName(int nodeId) 
	{
		return _nodeAsText.get( nodeId );
	}

	static void addNodeText(int tagId, String tagName) 
	{
		_nodeAsText.put( tagId, tagName );
	}
	
	static Long convertQosRate(String rate)
	{
		switch(rate)
		{
		case"Rate::JustInTimeConflated":
			return new Long(OmmQos.Rate.JUST_IN_TIME_CONFLATED);
		case "Rate::TickByTick":
			return new Long(OmmQos.Rate.TICK_BY_TICK);
		default:
			try
			{
				return Long.parseLong(rate);
			}
			catch(NumberFormatException excp)
			{
				return null;
			}
		}
	}
	
	static Long convertQosTimeliness(String timeliness)
	{
		switch(timeliness)
		{
		case"Timeliness::RealTime":
			return new Long(OmmQos.Timeliness.REALTIME);
		case "Timeliness::InexactDelayed":
			return new Long(OmmQos.Timeliness.INEXACT_DELAYED);
		default:	
			try
			{
				return Long.parseLong(timeliness);
			}
			catch(NumberFormatException excp)
			{
				return null;
			}
		}
	}
	
	static Integer convertDomainType(String domainType)
	{
		switch (domainType)
		{
		case "MMT_LOGIN":
			return EmaRdm.MMT_LOGIN;
		case "MMT_DIRECTORY":
			return EmaRdm.MMT_DIRECTORY;
		case "MMT_DICTIONARY":
			return EmaRdm.MMT_DICTIONARY;
		case "MMT_MARKET_PRICE":
			return EmaRdm.MMT_MARKET_PRICE;
		case "MMT_MARKET_BY_ORDER":
			return EmaRdm.MMT_MARKET_BY_ORDER;
		case "MMT_MARKET_BY_PRICE":
			return EmaRdm.MMT_MARKET_BY_PRICE;
		case "MMT_MARKET_MAKER":
			return EmaRdm.MMT_MARKET_MAKER;
		case "MMT_SYMBOL_LIST":
			return EmaRdm.MMT_SYMBOL_LIST;
		case "MMT_SERVICE_PROVIDER_STATUS":
			return EmaRdm.MMT_SERVICE_PROVIDER_STATUS;
		case "MMT_HISTORY":
			return EmaRdm.MMT_HISTORY;
		case "MMT_HEADLINE":
			return EmaRdm.MMT_HEADLINE;
		case "MMT_STORY":
			return EmaRdm.MMT_STORY;
		case "MMT_REPLAYHEADLINE":
			return EmaRdm.MMT_REPLAYHEADLINE;
		case "MMT_REPLAYSTORY":
			return EmaRdm.MMT_REPLAYSTORY;
		case "MMT_TRANSACTION":
			return EmaRdm.MMT_TRANSACTION;
		case "MMT_YIELD_CURVE":
			return EmaRdm.MMT_YIELD_CURVE;
		case "MMT_CONTRIBUTION":
			return EmaRdm.MMT_CONTRIBUTION;
		case "MMT_PROVIDER_ADMIN":
			return EmaRdm.MMT_PROVIDER_ADMIN;
		case "MMT_ANALYTICS":
			return EmaRdm.MMT_ANALYTICS;
		case "MMT_REFERENCE":
			return EmaRdm.MMT_REFERENCE;
		case "MMT_NEWS_TEXT_ANALYTICS":
			return EmaRdm.MMT_NEWS_TEXT_ANALYTICS;
		case "MMT_SYSTEM":
			return EmaRdm.MMT_SYSTEM;
		default:
			try
			{
				return Integer.parseInt(domainType);
			}
			catch(NumberFormatException excp)
			{
				return null;
			}
		}
	}

	class TagDictionary 
	{
		Hashtable<String, Integer> dict = new Hashtable<String, Integer>();

		void add(String tagName, int tagId)
		{
			dict.put( tagName, tagId );
			ConfigManager.addNodeText( tagId, tagName );
		}

		 Integer get(String name) 
		{
			Integer tagId = dict.get(name);
			return tagId;
		}

		 boolean isEmpty() 
		{
			return(dict.isEmpty());
		}

		/* String nodeName(int nodeId) 
		{
			String nodeName = dictText.get(nodeId);
			if( nodeName == null )
				nodeName = "Unknown";
			
			return nodeName;
		}*/
	}

	class Branch 
	{
		ArrayList<Integer> branchAsNumeric = new ArrayList<Integer>();
		ArrayList<String> branchAsString = new ArrayList<String>();

		String branchName;
		
		 void add(int nodeId,TagDictionary dictionary) 
		{
			branchAsNumeric.add(nodeId);
			branchAsString.add(ConfigManager.nodeName(nodeId));
		}

		void complete()
		{
			ConfigManager.stringMaker().append(branchAsString.get(0));

			for(int i = 1; i < branchAsNumeric.size(); i++)
			{
				_stringMaker.append("|");
				_stringMaker.append(branchAsString.get(i));
			}
			branchName = _stringMaker.toString();
			_stringMaker.setLength(0);
		}
		
		 int size() 
		{
			return branchAsNumeric.size();
		}
		
		@Override
		public String toString()
		{
			return branchName;
		}

		 int get(int i) 
		{
			return branchAsNumeric.get(i);
		}

		 String nodeAsString(int findNodeId) 
		{
			for( int i = 0; i < branchAsNumeric.size(); i++)
			{
				int nodeId = branchAsNumeric.get(i);
				if(nodeId == findNodeId)
				{
					return(branchAsString.get(i));
				}
			}
			return "Unknown";
		}
	}
	
	class ConfigAttributes 
	{
		Map<Integer, List<ConfigElement>> _list;
		
		 ConfigAttributes()
		{
			_list = new LinkedHashMap<Integer, List<ConfigElement>>();
			
		}
		 
		 Map<Integer, List<ConfigElement>> getList()
		 {			 
			return _list;
		 }
		 
		List<ConfigElement> getConfigElementList(int attributeId)
		{
			return _list.get(attributeId);
		}
		
		 void put(int id, ConfigElement element) 
		{
			if ( !_list.containsKey(id) )
			{
				List<ConfigElement> arrayList = new ArrayList<>();
				arrayList.add(element);
				
				_list.put(id, arrayList);
			}
			else
			{
				_list.get(id).add(element);
			}
		}

		 void dump(String space) 
		{
			if( _list.size() > 0 )
			{
				String aspace = space+" ";

				Set<Integer> keys = _list.keySet();
				for(Integer k:keys)
				{
					List<ConfigElement> ceList = _list.get(k);
					
					for(int index = 0 ; index < ceList.size(); index++ )
					{
						ConfigElement ce = ceList.get(index);
						System.out.format("%s___ %s (Key:%d) - %s\n",aspace,ce._name,k,ce._valueStr);
					}
				}
			}
		}

		Object getValue(int attributeId)
		{
			List<ConfigElement> list = _list.get(attributeId);
			
			if ( list != null )
			{
				ConfigElement ce = list.get(list.size()-1);
				if( ce != null )
					return ce.value();
			}
		
			return null;
		}
		
		Object getElement(int attributeId)
		{
			List<ConfigElement> list = _list.get(attributeId);
			
			if ( list != null )
			{
				ConfigElement ce = list.get(list.size()-1);
				if( ce != null )
					return ce;
			}
		
			return null;
		}

		boolean setValue( int attributeId, String newValue, int dataType )
		{
			Set<Integer> keys = _list.keySet();
			for(Integer k:keys)
			{
				if(k == attributeId)
				{
					List<ConfigElement> list = _list.get(k);
					ConfigElement ce = list.get(list.size() - 1);
					if( dataType == ConfigElement.Type.Ascii )
						ce.setAsciiValue(newValue);
					
					return true;
				}
			}
			
			return false;
		}

		 ConfigElement getPrimitiveValue(int id) 
		{
			 List<ConfigElement> list = _list.get(id);
				
			if ( list != null )
			{
				ConfigElement ce = list.get(list.size()-1);
				if( ce != null )
					return ce;
			}
		
			return null;
		}
	}
	
	 abstract class ConfigElement 
	{
		int _type;
		String _name;
		XMLnode _parent;
		String _valueStr;
		int _tagId;

		 final class Type
		{
			 final static int Int64 = 0;
			 final static int UInt64 = 1;
			 final static int Ascii = 2;
			 final static int Enum = 3;
			 final static int Boolean = 4;
		};

		 ConfigElement( XMLnode  parent )
		{
			this._parent = parent;
		}

		abstract  Object value();
		
		int type()
		{
			return _type;
		}

		void setAsciiValue(String newValue) {}

		 int intValue() 
		{
			try 
			{
				throw new Exception("intValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return 0;
		}

		 int intLongValue() 
		{
			try 
			{
				throw new Exception("intLongValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return 0;
		}

		 String asciiValue() 
		{
			try 
			{
				throw new Exception("asciiValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return null;
		}

		 boolean booleanValue() {
			try 
			{
				throw new Exception("booleanValue() not implemented");
			} catch (Exception e) {
				e.printStackTrace();
			}
			
			return false;
		}
	}

	class IntConfigElement extends ConfigElement
	{
		int intValue;
		Integer intObject;
		
		 IntConfigElement(XMLnode parent, int type, int value) 
		{
			super( parent);
			this._type = type;
			intValue = value;
		}

		@Override
		 Object value() 
		{
			if( intObject == null )
				intObject = new Integer(intValue);
			
			return intObject;
		}
		
		 boolean booleanValue() 
		{
			Integer intValue = (Integer) value();
			if( intValue.intValue() == 1 )
				return true;
			else
				return false;
		}
		
		 int intValue() 
		{
			return( ((Integer)value()).intValue() );
		}
	}

	class LongConfigElement extends ConfigElement
	{
		long longValue;
		Long longObject;

		 LongConfigElement(XMLnode parent, int type, long value) 
		{
			super( parent);
			this._type = type;
			longValue = value;
		}

		 long longValue()
		{
			return longValue;
		}

		 int intLongValue() 
		{
			return (int) longValue;
		}

		@Override
		 Object value() 
		{
			if( longObject == null )
				longObject = new Long(longValue);
			
			return longObject;
		}
		
		 boolean booleanValue() 
		{
			Long longValue = (Long) value();
			if( longValue.longValue() == 1 )
				return true;
			else
				return false;
		}
	}

	class AsciiConfigElement extends ConfigElement
	{
		String asciiValue;
		
		 AsciiConfigElement(XMLnode parent, String value) 
		{
			super( parent);
			asciiValue = value;
		}
		
		 String asciiValue()
		{
			return asciiValue; 
		}

		@Override
		 Object value() 
		{
			return asciiValue;
		}
		
		 void setAsciiValue(String value) 
		{
			asciiValue = value;
		}
	}
}
