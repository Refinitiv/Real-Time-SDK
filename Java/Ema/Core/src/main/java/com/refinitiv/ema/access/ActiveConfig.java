/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;


import java.util.ArrayList;
import java.util.List;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.CompressionTypes;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode;

abstract class ActiveConfig extends BaseConfig
{
	final static int DEFAULT_COMPRESSION_THRESHOLD				= 30;
	final static int DEFAULT_COMPRESSION_THRESHOLD_LZ4          = 300;
	final static int DEFAULT_COMPRESSION_TYPE					= CompressionTypes.NONE;
	final static int DEFAULT_CONNECTION_TYPE					= ConnectionTypes.SOCKET;
	final static int DEFAULT_ENCRYPTED_PROTOCOL_TYPE			= ConnectionTypes.SOCKET;
	final static int DEFAULT_CONNECTION_PINGTIMEOUT				= 30000;
	final static int DEFAULT_INITIALIZATION_TIMEOUT				= 5;
	final static int DEFAULT_INITIALIZATION_ACCEPT_TIMEOUT		= 60;
	final static int DEFAULT_DICTIONARY_REQUEST_TIMEOUT			= 45000;
	final static int DEFAULT_DIRECTORY_REQUEST_TIMEOUT			= 45000;
	final static boolean DEFAULT_ENABLE_SESSION_MGNT			= false;
	final static int DEFAULT_GUARANTEED_OUTPUT_BUFFERS			= 100;
	final static String DEFAULT_REGION_LOCATION					= "us-east-1";
	final static int DEFAULT_NUM_INPUT_BUFFERS					= 100;
	final static int DEFAULT_SYS_SEND_BUFFER_SIZE				= 65535;
	final static int DEFAULT_SYS_RECEIVE_BUFFER_SIZE			= 65535;
	final static int DEFAULT_HIGH_WATER_MARK					= 0;
	final static boolean DEFAULT_HANDLE_EXCEPTION				= true;
	final static String DEFAULT_HOST_NAME						= "localhost";
	final static String DEFAULT_CHANNEL_SET_NAME				= ""; 
	final static boolean DEFAULT_INCLUDE_DATE_IN_LOGGER_OUTPUT	= false;
	final static String DEFAULT_INTERFACE_NAME					= "" ;
	final static int DEFAULT_LOGIN_REQUEST_TIMEOUT              = 45000;
	final static int DEFAULT_MAX_OUTSTANDING_POSTS				= 100000;
	final static boolean DEFAULT_MSGKEYINUPDATES				= true;
	final static int DEFAULT_OBEY_OPEN_WINDOW					= 1;
	final static int DEFAULT_POST_ACK_TIMEOUT					= 15000;
	final static int DEFAULT_REACTOR_EVENTFD_PORT				= 55000;
	final static int DEFAULT_RECONNECT_ATTEMPT_LIMIT			= -1;
	final static int DEFAULT_RECONNECT_MAX_DELAY				= 5000;
	final static int DEFAULT_RECONNECT_MIN_DELAY				= 1000;
	final static String DEFAULT_OBJECT_NAME						= "";
	final static boolean DEFAULT_TCP_NODELAY					= true;
	final static String DEFAULT_CONS_MCAST_CFGSTRING			= "";
	final static int DEFAULT_PACKET_TTL							= 5;
	final static int DEFAULT_NDATA								= 7;
	final static int DEFAULT_NMISSING							= 128;
	final static int DEFAULT_NREQ								= 3;
	final static int DEFAULT_PKT_POOLLIMIT_HIGH					= 190000;
	final static int DEFAULT_PKT_POOLLIMIT_LOW					= 180000;
	final static int DEFAULT_TDATA								= 1;
	final static int DEFAULT_TRREQ								= 4;
	final static int DEFAULT_TWAIT								= 3;
	final static int DEFAULT_TBCHOLD							= 3;
	final static int DEFAULT_TPPHOLD							= 3;
	final static int DEFAULT_USER_QLIMIT						= 65535;
	final static int DEFAULT_REST_REQUEST_TIMEOUT				= 45000;
	final static int DEFAULT_REISSUE_TOKEN_ATTEMPT_LIMIT		= -1;
	final static int DEFAULT_REISSUE_TOKEN_ATTEMPT_INTERVAL		= 5000;
	final static double DEFAULT_TOKEN_REISSUE_RATIO				= 0.8;
	final static boolean DEFAULT_XML_TRACE_ENABLE				= false;
	final static boolean DEFAULT_XML_TRACE_TO_FILE_ENABLE		= false;
	final static long DEFAULT_XML_TRACE_MAX_FILE_SIZE			= 100000000;
	final static String DEFAULT_XML_TRACE_FILE_NAME				= "EmaTrace";
	final static boolean DEFAULT_XML_TRACE_TO_MULTIPLE_FILES	= false;
	final static boolean DEFAULT_XML_TRACE_WRITE				= true;
	final static boolean DEFAULT_XML_TRACE_READ					= true;
	final static boolean DEFAULT_XML_TRACE_PING					= false;
	final static boolean DEFAULT_DIRECT_SOCKET_WRITE			= false;
	final static boolean DEFAULT_HTTP_PROXY					    = false;
	final static String DEFAULT_CONS_NAME						= "EmaConsumer";
	final static String DEFAULT_IPROV_NAME						= "EmaIProvider";
	final static String DEFAULT_NIPROV_NAME						= "EmaNiProvider";
	final static String DEFAULT_WS_PROTOCOLS					= "tr_json2, rssl.rwf, rssl.json.v2";
	final static int DEFAULT_WS_MAX_MSG_SIZE					= 61440;
	final static int DEFAULT_MAX_FRAGMENT_SIZE 					= 6144;
	final static int DEFAULT_SERVICE_DISCOVERY_RETRY_COUNT		= 3;
	final static boolean DEFAULT_WSB_DOWNLOAD_CONNECTION_CONFIG = true;
	final static int DEFAULT_WSB_MODE							= ReactorWarmStandbyMode.LOGIN_BASED;
	final static boolean DEFAULT_SESSION_ENHANCED_ITEM_RECOVERY = true;
	
	final static int SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL   = 0x01;  /*!< Indicates that host set though EMA interface function calls for RSSL_SOCKET connection type */
	final static int SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL = 0x02;  /*!< Indicates that server listen port set though EMA interface function call from server client*/
	final static int TUNNELING_PROXY_HOST_CONFIG_BY_FUNCTION_CALL = 0x04;  /*!< Indicates that tunneling proxy host set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
	final static int TUNNELING_PROXY_PORT_CONFIG_BY_FUNCTION_CALL = 0x08;  /*!< Indicates that tunneling proxy host set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
	final static int TUNNELING_OBJNAME_CONFIG_BY_FUNCTION_CALL = 0x10;  /*!< Indicates that tunneling proxy host set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
	final static int PROXY_USERNAME_CONFIG_BY_FUNCTION_CALL 	= 0x20; /*!< Indicates that tunneling proxy user name set though EMA interface function calls */
	final static int PROXY_PASSWD_CONFIG_BY_FUNCTION_CALL 		= 0x40; /*!< Indicates that tunneling proxy password set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
	final static int PROXY_DOMAIN_CONFIG_BY_FUNCTION_CALL 		= 0x80; /*!< Indicates that tunneling proxy domain set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
	
	int						obeyOpenWindow;
	int						postAckTimeout;
	int						maxOutstandingPosts;
	int						loginRequestTimeOut;
	int						directoryRequestTimeOut;
	int						dictionaryRequestTimeOut;
	List<ChannelConfig>		channelConfigSet;
	LoginRequest			rsslRDMLoginRequest;
	DirectoryRequest		rsslDirectoryRequest;
	DirectoryRefresh		rsslDirectoryRefresh;
	DictionaryRequest		rsslFldDictRequest;
	DictionaryRequest		rsslEnumDictRequest;
	int						reissueTokenAttemptLimit;
	int						reissueTokenAttemptInterval;
	int						restRequestTimeout;
	double					tokenReissueRatio;
	String 					fldDictReqServiceName;
	String 					enumDictReqServiceName;
	DictionaryConfig		dictionaryConfig;
	static String		    defaultServiceName;
	int					    reconnectAttemptLimit;
	int						reconnectMinDelay;
	int						reconnectMaxDelay;
	boolean 				msgKeyInUpdates;
	String					wsProtocols;
	int 					wsMaxMsgSize;
	int						maxFragmentSize;
	List<ChannelConfig>		configChannelSetForWSB;
	List<WarmStandbyChannelConfig> configWarmStandbySet;
	List<SessionChannelConfig> configSessionChannelSet;
	String					restProxyHostName;
	String					restProxyPort;
	boolean					sessionEnhancedItemRecovery;
	
	ActiveConfig(String defaultServiceName)
	{
		super();
		
		 obeyOpenWindow = DEFAULT_OBEY_OPEN_WINDOW;
		 postAckTimeout = DEFAULT_POST_ACK_TIMEOUT;
		 maxOutstandingPosts = DEFAULT_MAX_OUTSTANDING_POSTS;
		 loginRequestTimeOut = DEFAULT_LOGIN_REQUEST_TIMEOUT;
		 directoryRequestTimeOut = DEFAULT_DIRECTORY_REQUEST_TIMEOUT;
		 dictionaryRequestTimeOut = DEFAULT_DICTIONARY_REQUEST_TIMEOUT;
		 userDispatch = DEFAULT_USER_DISPATCH;
		 reconnectAttemptLimit = ActiveConfig.DEFAULT_RECONNECT_ATTEMPT_LIMIT;
		 reconnectMinDelay = ActiveConfig.DEFAULT_RECONNECT_MIN_DELAY;
		 reconnectMaxDelay = ActiveConfig.DEFAULT_RECONNECT_MAX_DELAY;
		 msgKeyInUpdates = ActiveConfig.DEFAULT_MSGKEYINUPDATES ;
		 ActiveConfig.defaultServiceName = defaultServiceName;
		 reissueTokenAttemptLimit = DEFAULT_REISSUE_TOKEN_ATTEMPT_LIMIT;
		 reissueTokenAttemptInterval = DEFAULT_REISSUE_TOKEN_ATTEMPT_INTERVAL;
		 restRequestTimeout = DEFAULT_REST_REQUEST_TIMEOUT;
		 tokenReissueRatio = DEFAULT_TOKEN_REISSUE_RATIO;
		 wsProtocols = DEFAULT_WS_PROTOCOLS;
		 wsMaxMsgSize = DEFAULT_WS_MAX_MSG_SIZE;
		 maxFragmentSize = DEFAULT_MAX_FRAGMENT_SIZE;
		 channelConfigSet = new ArrayList<>();
		 configChannelSetForWSB = new ArrayList<>();
		 configWarmStandbySet = new ArrayList<>();
		 configSessionChannelSet = new ArrayList<>();
		 sessionEnhancedItemRecovery = DEFAULT_SESSION_ENHANCED_ITEM_RECOVERY;
	}

	void clear()
	{
		super.clear();
		
		obeyOpenWindow = DEFAULT_OBEY_OPEN_WINDOW;
		postAckTimeout = DEFAULT_POST_ACK_TIMEOUT;
		maxOutstandingPosts = DEFAULT_MAX_OUTSTANDING_POSTS;
		loginRequestTimeOut = DEFAULT_LOGIN_REQUEST_TIMEOUT;
		directoryRequestTimeOut = DEFAULT_DIRECTORY_REQUEST_TIMEOUT;
		dictionaryRequestTimeOut = DEFAULT_DICTIONARY_REQUEST_TIMEOUT;
		userDispatch = DEFAULT_USER_DISPATCH;
		reconnectAttemptLimit = ActiveConfig.DEFAULT_RECONNECT_ATTEMPT_LIMIT;
		reconnectMinDelay = ActiveConfig.DEFAULT_RECONNECT_MIN_DELAY;
		reconnectMaxDelay = ActiveConfig.DEFAULT_RECONNECT_MAX_DELAY;
		msgKeyInUpdates = ActiveConfig.DEFAULT_MSGKEYINUPDATES;
		reissueTokenAttemptLimit = DEFAULT_REISSUE_TOKEN_ATTEMPT_LIMIT;
		reissueTokenAttemptInterval = DEFAULT_REISSUE_TOKEN_ATTEMPT_INTERVAL;
		restRequestTimeout = DEFAULT_REST_REQUEST_TIMEOUT;
		tokenReissueRatio = DEFAULT_TOKEN_REISSUE_RATIO;
		wsProtocols = DEFAULT_WS_PROTOCOLS;
		wsMaxMsgSize = DEFAULT_WS_MAX_MSG_SIZE;
		maxFragmentSize = DEFAULT_MAX_FRAGMENT_SIZE;
		sessionEnhancedItemRecovery = DEFAULT_SESSION_ENHANCED_ITEM_RECOVERY;
		dictionaryConfig.clear();

		rsslRDMLoginRequest = null;
		rsslDirectoryRequest = null;
		rsslFldDictRequest = null;
		rsslEnumDictRequest = null;
		
		restProxyHostName = null;
		restProxyPort = null;
	}
	
	StringBuilder configTrace()
	{
		super.configTrace();
		traceStr.append("\n\t obeyOpenWindow: ").append(obeyOpenWindow) 
		.append("\n\t postAckTimeout: ").append(postAckTimeout) 
		.append("\n\t maxOutstandingPosts: ").append(maxOutstandingPosts) 
		.append("\n\t userDispatch: ").append(userDispatch) 
		.append("\n\t reconnectAttemptLimit: ").append(reconnectAttemptLimit) 
		.append("\n\t reconnectMinDelay: ").append(reconnectMinDelay) 
		.append("\n\t reconnectMaxDelay: ").append(reconnectMaxDelay) 
		.append("\n\t msgKeyInUpdates: ").append(msgKeyInUpdates)
		.append("\n\t directoryRequestTimeOut: ").append(directoryRequestTimeOut)
		.append("\n\t dictionaryRequestTimeOut: ").append(dictionaryRequestTimeOut)
		.append("\n\t reissueTokenAttemptLimit: ").append(reissueTokenAttemptLimit)
		.append("\n\t reissueTokenAttemptInterval: ").append(reissueTokenAttemptInterval)
		.append("\n\t restRequestTimeOut: ").append(restRequestTimeout)
		.append("\n\t tokenReissueRatio: ").append(tokenReissueRatio)
		.append("\n\t loginRequestTimeOut: ").append(loginRequestTimeOut)
		.append("\n\t wsProtocols: ").append(wsProtocols)
		.append("\n\t wsMaxMsgSize: ").append(wsMaxMsgSize)
		.append("\n\t maxFragmentSize: ").append(maxFragmentSize)
		.append("\n\t serviceDiscoveryRetryCount: ")
		.append("\n\t restProxyHostName: ").append(restProxyHostName)
		.append("\n\t restProxyPort: ").append(restProxyPort)
		.append("\n\t sessionEnhancedItemRecovery: ").append(sessionEnhancedItemRecovery)
		.append("\n\t reactorMsgEventPoolLimit: ").append(globalConfig.reactorMsgEventPoolLimit)
		.append("\n\t reactorChannelEventPoolLimit: ").append(globalConfig.reactorChannelEventPoolLimit)
		.append("\n\t workerEventPoolLimit: ").append(globalConfig.workerEventPoolLimit)
		.append("\n\t tunnelStreamMsgEventPoolLimit: ").append(globalConfig.tunnelStreamMsgEventPoolLimit)
		.append("\n\t tunnelStreamStatusEventPoolLimit: ").append(globalConfig.tunnelStreamStatusEventPoolLimit)
		.append("\n\t jsonConverterPoolsSize: ").append(globalConfig.jsonConverterPoolsSize)
		.append("\n\t watchlistObjectsPoolLimit: ").append(globalConfig.watchlistObjectsPoolLimit);

		return traceStr;
	}
	
	void reconnectAttemptLimit(long value) 
	{
		if (value >= 0)
			reconnectAttemptLimit = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
	}
	void reconnectMinDelay(long value)
	{
		if ( value > 0 )
			reconnectMinDelay = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
	}
	void reconnectMaxDelay(long value) 
	{
		if ( value > 0 )
			reconnectMaxDelay = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
	}
	void reissueTokenAttemptLimit(long value) 
	{
		if (value >= 0)
			reissueTokenAttemptLimit = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
		else
			reissueTokenAttemptLimit = DEFAULT_REISSUE_TOKEN_ATTEMPT_LIMIT;
	}
	void reissueTokenAttemptInterval(long value) 
	{
		if (value >= 0)
			reissueTokenAttemptInterval = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
		else
			reissueTokenAttemptInterval = 0;
	}
	
	void restRequestTimeout(long value) 
	{
		if (value >= 0)
			restRequestTimeout = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
		else
			restRequestTimeout = 0;
	}
	
	static boolean findWsbChannelConfig(List<WarmStandbyChannelConfig> cfgWsbChannelSet, String wsbChannelName, int pos)
	{
		return cfgWsbChannelSet.stream()
				.anyMatch(cfgWsbChannel -> cfgWsbChannel.name.equalsIgnoreCase(wsbChannelName));
	}
 
	void clearWSBChannelSet()
	{
		configWarmStandbySet.clear();
	}
 
	void clearChannelSetForWSB()
	{
		configChannelSetForWSB.clear();
	}
	
	void clearSessionChannelSet()
	{
		configSessionChannelSet.clear();
	}

	public static boolean findChannelConfig(List<ChannelConfig> configChannelSetForWSB, String queryName, int pos)
	{
		return configChannelSetForWSB.stream()
				.anyMatch(channelConfig -> channelConfig.name.equalsIgnoreCase(queryName));
	}

	//TODO: need to check
	ChannelConfig findChannelConfig(Channel pChannel)
	{
//		if (pChannel->getParentChannel() == NULL) TODO: need more details
		if(pChannel != null)
		{
			for(ChannelConfig channelCfg : channelConfigSet)
			{
				if(channelCfg.channelInfo.rsslReactorChannel().equals(pChannel))
				{
					return channelCfg;
				}
			}
		}
		else
		{
			for(ChannelConfig channelCfg : configChannelSetForWSB)
			{
				if(channelCfg.channelInfo.rsslReactorChannel().equals(pChannel))
				{
					return channelCfg;
				}
			}
		}
		return null;
	}
}





