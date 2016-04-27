///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


import com.thomsonreuters.ema.access.OmmConsumerConfig.OperationModel;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.CompressionTypes;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;

class OmmConsumerActiveConfig
{
	final static int DEFAULT_COMPRESSION_THRESHOLD				= 30;
	final static int DEFAULT_COMPRESSION_TYPE					= CompressionTypes.NONE;
	final static int DEFAULT_CONNECTION_TYPE					= ConnectionTypes.SOCKET;
	final static int DEFAULT_CONNECTION_PINGTIMEOUT				= 30000;
	final static int DEFAULT_DICTIONARY_REQUEST_TIMEOUT			= 45000;
	final static int DEFAULT_DIRECTORY_REQUEST_TIMEOUT			= 45000;
	final static int DEFAULT_DISPATCH_TIMEOUT_API_THREAD		= 0;
	final static int DEFAULT_GUARANTEED_OUTPUT_BUFFERS			= 100;
	final static int DEFAULT_NUM_INPUT_BUFFERS					= 10;
	final static int DEFAULT_SYS_SEND_BUFFER_SIZE				= 0;
	final static int DEFAULT_SYS_RECEIVE_BUFFER_SIZE			= 0;
	final static boolean DEFAULT_HANDLE_EXCEPTION				= true;
	final static String DEFAULT_HOST_NAME						= "localhost";
	final static String DEFAULT_CHANNEL_SET_NAME				= ""; 
	final static boolean DEFAULT_INCLUDE_DATE_IN_LOGGER_OUTPUT	= false;
	final static String DEFAULT_INTERFACE_NAME					= "" ;
	final static int DEFAULT_ITEM_COUNT_HINT					= 100000;
	final static int DEFAULT_LOGIN_REQUEST_TIMEOUT              = 45000;
	final static int DEFAULT_MAX_DISPATCH_COUNT_API_THREAD		= 100;
	final static int DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD	    = 100;
	final static int DEFAULT_MAX_OUTSTANDING_POSTS				= 100000;
	final static boolean DEFAULT_MSGKEYINUPDATES				= true;
	final static int DEFAULT_OBEY_OPEN_WINDOW					= 1;
	final static int DEFAULT_PIPE_PORT							= 9001;
	final static int DEFAULT_POST_ACK_TIMEOUT					= 15000;
	final static int DEFAULT_REACTOR_EVENTFD_PORT				= 55000;
	final static int DEFAULT_RECONNECT_ATTEMPT_LIMIT			= -1;
	final static int DEFAULT_RECONNECT_MAX_DELAY				= 5000;
	final static int DEFAULT_RECONNECT_MIN_DELAY				= 1000;
	final static int DEFAULT_REQUEST_TIMEOUT					= 15000;
	final static int DEFAULT_SERVICE_COUNT_HINT				    = 513;
	final static String DEFAULT_SERVICE_NAME					= "14002";
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
	final static int DEFAULT_USER_DISPATCH						= OperationModel.API_DISPATCH;
	final static boolean DEFAULT_XML_TRACE_ENABLE				= false;
	
	String		consumerName;
	OmmConsumerImpl consumer;
	String      instanceName;
	int			itemCountHint;
	int		    serviceCountHint;
	int			dispatchTimeoutApiThread;
	int			maxDispatchCountApiThread;
	int			maxDispatchCountUserThread;
	int			obeyOpenWindow;
	int			requestTimeout;
	int			postAckTimeout;
	int			maxOutstandingPosts;
	int			loginRequestTimeOut;
	int			directoryRequestTimeOut;
	int			dictionaryRequestTimeOut;
	int		    userDispatch;
	ChannelConfig			channelConfig;
	DictionaryConfig		dictionaryConfig;
	LoginRequest			rsslRDMLoginRequest;
	DirectoryRequest		rsslDirectoryRequest;
	DictionaryRequest		rsslFldDictRequest;
	DictionaryRequest		rsslEnumDictRequest;
	String 								fldDictReqServiceName;
	String 								enumDictReqServiceName;
	
	OmmConsumerActiveConfig(OmmConsumerImpl consumerImpl)
	{
		 consumer = consumerImpl;
		 itemCountHint = DEFAULT_ITEM_COUNT_HINT;
		 serviceCountHint = DEFAULT_SERVICE_COUNT_HINT;
		 dispatchTimeoutApiThread = DEFAULT_DISPATCH_TIMEOUT_API_THREAD;
		 maxDispatchCountApiThread = DEFAULT_MAX_DISPATCH_COUNT_API_THREAD;
		 maxDispatchCountUserThread = DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD;
		 obeyOpenWindow = DEFAULT_OBEY_OPEN_WINDOW;
		 requestTimeout = DEFAULT_REQUEST_TIMEOUT;
		 postAckTimeout = DEFAULT_POST_ACK_TIMEOUT;
		 maxOutstandingPosts = DEFAULT_MAX_OUTSTANDING_POSTS;
		 loginRequestTimeOut = DEFAULT_LOGIN_REQUEST_TIMEOUT;
		 directoryRequestTimeOut = DEFAULT_DIRECTORY_REQUEST_TIMEOUT;
		 dictionaryRequestTimeOut = DEFAULT_DICTIONARY_REQUEST_TIMEOUT;
		 userDispatch = DEFAULT_USER_DISPATCH;
	}

	void clear()
	{
		consumerName = null;
		instanceName = null;
		itemCountHint = DEFAULT_ITEM_COUNT_HINT;
		serviceCountHint = DEFAULT_SERVICE_COUNT_HINT;
		dispatchTimeoutApiThread = DEFAULT_DISPATCH_TIMEOUT_API_THREAD;
		maxDispatchCountApiThread = DEFAULT_MAX_DISPATCH_COUNT_API_THREAD;
		maxDispatchCountUserThread = DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD;
		obeyOpenWindow = DEFAULT_OBEY_OPEN_WINDOW;
		requestTimeout = DEFAULT_REQUEST_TIMEOUT;
		postAckTimeout = DEFAULT_POST_ACK_TIMEOUT;
		maxOutstandingPosts = DEFAULT_MAX_OUTSTANDING_POSTS;
		userDispatch = DEFAULT_USER_DISPATCH;
		
		dictionaryConfig.clear();

		rsslRDMLoginRequest = null;
		rsslDirectoryRequest = null;
		rsslFldDictRequest = null;
		rsslEnumDictRequest = null;
	}
	
	void intializeLoginReq(String clientName)
	{
        rsslRDMLoginRequest.applyHasRole();
		rsslRDMLoginRequest.role(Login.RoleTypes.CONS);
		rsslRDMLoginRequest.streamId(1);
		
		if (consumer.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = consumer.consumerStrBuilder();
			
			temp.append("RDMLogin request message was populated with this info: ")
										.append(OmmLoggerClient.CR)
										.append(rsslRDMLoginRequest.toString());
										
			consumer.loggerClient().trace(consumer.formatLogMessage(clientName, 
					temp.toString(),Severity.TRACE).toString());
		}
	}
	
	void intializeDirReq(String clientName)
	{
		if (rsslDirectoryRequest == null)
		{
			rsslDirectoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
			rsslDirectoryRequest.clear();
			rsslDirectoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
			rsslDirectoryRequest.streamId(2);
			rsslDirectoryRequest.applyStreaming();
			rsslDirectoryRequest.filter(Directory.ServiceFilterFlags.INFO |
								Directory.ServiceFilterFlags.STATE |
								Directory.ServiceFilterFlags.GROUP |
								Directory.ServiceFilterFlags.LOAD |
								Directory.ServiceFilterFlags.DATA |
								Directory.ServiceFilterFlags.LINK);
		}
		else
		{
			rsslDirectoryRequest.streamId(2);
			long filter = rsslDirectoryRequest.filter();
			if (filter == 0)
			{			
				if (consumer.loggerClient().isWarnEnabled())
				{
					consumer.loggerClient().warn(consumer.formatLogMessage(clientName,
													"Configured source directory request message contains no filter. Will request all filters",
													Severity.WARNING).toString());
				}
				
				rsslDirectoryRequest.filter(Directory.ServiceFilterFlags.INFO |
						Directory.ServiceFilterFlags.STATE |
						Directory.ServiceFilterFlags.GROUP |
						Directory.ServiceFilterFlags.LOAD |
						Directory.ServiceFilterFlags.DATA |
						Directory.ServiceFilterFlags.LINK);
			}

			if (!rsslDirectoryRequest.checkStreaming())
			{
				consumer.loggerClient().warn(consumer.formatLogMessage(clientName, 
						                  		"Configured source directory request message contains no streaming flag. Will request streaming",
												Severity.WARNING).toString());
				
				rsslDirectoryRequest.applyStreaming();
			}
		}

		if (consumer.loggerClient().isTraceEnabled())
		{
			StringBuilder temp = consumer.consumerStrBuilder();
			
			
			temp.append("RDMDirectoryRequest message was populated with Filter(s)");
			int filter = (int)rsslDirectoryRequest.filter();
			
			if ((filter & Directory.ServiceFilterFlags.INFO) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_INFO_FILTER");
			if ((filter & Directory.ServiceFilterFlags.STATE) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_STATE_FILTER");
			if ((filter & Directory.ServiceFilterFlags.GROUP) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_GROUP_FILTER");
			if ((filter & Directory.ServiceFilterFlags.LOAD) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_LOAD_FILTER");
			if ((filter & Directory.ServiceFilterFlags.DATA) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_DATA_FILTER"); 
			if ((filter & Directory.ServiceFilterFlags.LINK) != 0)
				temp.append(OmmLoggerClient.CR).append("RDM_DIRECTORY_SERVICE_LINK_FILTER");

			if (rsslDirectoryRequest.checkHasServiceId())
				temp.append(OmmLoggerClient.CR).append("requesting serviceId ").append(rsslDirectoryRequest.serviceId());
			else
				temp.append(OmmLoggerClient.CR).append("requesting all services");

			consumer.loggerClient().trace(consumer.formatLogMessage(clientName, temp.toString(),
											Severity.TRACE));
		}
	}
	
	void intializeDictReq(String clientName)
	{
		if (rsslFldDictRequest != null && rsslEnumDictRequest != null)
			dictionaryConfig.isLocalDictionary = false;
		else if (rsslFldDictRequest != null && rsslEnumDictRequest == null)
		{
			StringBuilder temp = consumer.consumerStrBuilder();
			
			temp.append("Invalid dictionary configuration was specified through the OmmConsumerConfig.addAdminMsg()")
				.append(OmmLoggerClient.CR)
				.append("Enumeration type definition request message was not populated.");

			if (consumer.loggerClient().isErrorEnabled())
				consumer.loggerClient().error(consumer.formatLogMessage(clientName, temp.toString(),
												Severity.ERROR).toString());

			throw (consumer.ommIUExcept().message(temp.toString()));
		}
		else if (rsslFldDictRequest == null && rsslEnumDictRequest != null)
		{
			StringBuilder temp = consumer.consumerStrBuilder();
			
			temp.append("Invalid dictionary configuration was specified through the OmmConsumerConfig.addAdminMsg()")
				.append(OmmLoggerClient.CR)
				.append("RDM Field Dictionary request message was not populated.");
			
			if (consumer.loggerClient().isErrorEnabled())
				consumer.loggerClient().error(consumer.formatLogMessage(clientName, temp.toString(),
												Severity.ERROR).toString());

			throw (consumer.ommIUExcept().message(temp.toString()));
		}
	}
}

class ChannelConfig
{
	String				name;
	String				interfaceName;
	boolean				xmlTraceEnable;
	int					compressionType;
	int					compressionThreshold;
	int					rsslConnectionType;
	int					connectionPingTimeout;
	int					guaranteedOutputBuffers;
	int					numInputBuffers;
	int					sysRecvBufSize;
	int					sysSendBufSize;
	int					reconnectAttemptLimit;
	int					reconnectMinDelay;
	int					reconnectMaxDelay;
	boolean				msgKeyInUpdates;

	ChannelConfig() 
	{
		clear();	 
	}
	
	void clear() 
	{
		interfaceName =  OmmConsumerActiveConfig.DEFAULT_INTERFACE_NAME;
		compressionType = OmmConsumerActiveConfig.DEFAULT_COMPRESSION_TYPE;
		compressionThreshold = OmmConsumerActiveConfig.DEFAULT_COMPRESSION_THRESHOLD;
		connectionPingTimeout = OmmConsumerActiveConfig.DEFAULT_CONNECTION_PINGTIMEOUT;
		guaranteedOutputBuffers = OmmConsumerActiveConfig.DEFAULT_GUARANTEED_OUTPUT_BUFFERS;
		numInputBuffers = OmmConsumerActiveConfig.DEFAULT_NUM_INPUT_BUFFERS;
		sysSendBufSize = OmmConsumerActiveConfig.DEFAULT_SYS_SEND_BUFFER_SIZE;
		sysRecvBufSize = OmmConsumerActiveConfig.DEFAULT_SYS_RECEIVE_BUFFER_SIZE;
		reconnectAttemptLimit = OmmConsumerActiveConfig.DEFAULT_RECONNECT_ATTEMPT_LIMIT;
		reconnectMinDelay = OmmConsumerActiveConfig.DEFAULT_RECONNECT_MIN_DELAY;
		reconnectMaxDelay = OmmConsumerActiveConfig.DEFAULT_RECONNECT_MAX_DELAY;
		xmlTraceEnable = OmmConsumerActiveConfig.DEFAULT_XML_TRACE_ENABLE;
		msgKeyInUpdates = OmmConsumerActiveConfig.DEFAULT_MSGKEYINUPDATES ;
		rsslConnectionType = OmmConsumerActiveConfig.DEFAULT_CONNECTION_TYPE;	
	}

	void guaranteedOutputBuffers(long value) { }
	void numInputBuffers(long value) { }
	void reconnectAttemptLimit(long value) { }
	void reconnectMinDelay(long value) { }
	void reconnectMaxDelay(long value) { }
}

class SocketChannelConfig extends ChannelConfig
{
	String				hostName;
	String				serviceName;
	boolean				tcpNodelay;
	
	SocketChannelConfig() 
	{
		 clear();
	}

	@Override
	void clear() 
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.SOCKET;	
		hostName = OmmConsumerActiveConfig.DEFAULT_HOST_NAME;
		serviceName = OmmConsumerActiveConfig.DEFAULT_SERVICE_NAME;
		tcpNodelay = OmmConsumerActiveConfig.DEFAULT_TCP_NODELAY;
	}
}

class ReliableMcastChannelConfig extends ChannelConfig
{
	String				recvAddress;
	String				recvServiceName;
	String				unicastServiceName;
	String				sendAddress;
	String				sendServiceName;
	String				tcpControlPort;
	String				hsmInterface;
	String				hsmMultAddress;
	String				hsmPort;
	int					hsmInterval;
	boolean				disconnectOnGap;
	int					packetTTL;
	int					ndata;
	int					nmissing;
	int					nrreq;
	int					tdata;
	int					trreq;
	int					twait;
	int					tbchold;
	int					tpphold;
	int					pktPoolLimitHigh;
	int					pktPoolLimitLow;
	int					userQLimit;

	ReliableMcastChannelConfig()
	{
		clear();
	}

	@Override
	void clear()
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.RELIABLE_MCAST;
		recvAddress = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		recvServiceName = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		unicastServiceName = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		sendAddress = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		sendServiceName = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmInterface = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		tcpControlPort = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmMultAddress = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmPort = OmmConsumerActiveConfig.DEFAULT_CONS_MCAST_CFGSTRING;
		hsmInterval = 0;
		packetTTL	= OmmConsumerActiveConfig.DEFAULT_PACKET_TTL;
	    ndata = OmmConsumerActiveConfig.DEFAULT_NDATA;
	    nmissing = OmmConsumerActiveConfig.DEFAULT_NMISSING;
	    nrreq = OmmConsumerActiveConfig.DEFAULT_NREQ;
	    pktPoolLimitHigh = OmmConsumerActiveConfig.DEFAULT_PKT_POOLLIMIT_HIGH;
	    pktPoolLimitLow = OmmConsumerActiveConfig.DEFAULT_PKT_POOLLIMIT_LOW;
	    tdata = OmmConsumerActiveConfig.DEFAULT_TDATA;
	    trreq = OmmConsumerActiveConfig.DEFAULT_TRREQ;
	    twait = OmmConsumerActiveConfig.DEFAULT_TWAIT;
	    tbchold = OmmConsumerActiveConfig.DEFAULT_TBCHOLD;
	    tpphold = OmmConsumerActiveConfig.DEFAULT_TPPHOLD;
	    userQLimit = OmmConsumerActiveConfig.DEFAULT_USER_QLIMIT;
		disconnectOnGap = false;
	}
}

class EncryptedChannelConfig extends ChannelConfig
{
	String				hostName;
	String				serviceName;
	String				objectName;
	Boolean				tcpNodelay;

	EncryptedChannelConfig()
	{
		clear();
	}

	@Override
	void clear() 
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.ENCRYPTED;
		hostName = OmmConsumerActiveConfig.DEFAULT_HOST_NAME;
		serviceName = OmmConsumerActiveConfig.DEFAULT_SERVICE_NAME;
		tcpNodelay = OmmConsumerActiveConfig.DEFAULT_TCP_NODELAY;
		objectName = OmmConsumerActiveConfig.DEFAULT_OBJECT_NAME;
	}
}

class HttpChannelConfig extends ChannelConfig
{
	String			hostName;
	String			serviceName;
	String			objectName;
	Boolean			tcpNodelay;

	HttpChannelConfig() 
	{
		clear();
	}

	@Override
	void clear()
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.HTTP;
		hostName = OmmConsumerActiveConfig.DEFAULT_HOST_NAME;
		serviceName = OmmConsumerActiveConfig.DEFAULT_SERVICE_NAME;
		tcpNodelay = OmmConsumerActiveConfig.DEFAULT_TCP_NODELAY;
		objectName = OmmConsumerActiveConfig.DEFAULT_OBJECT_NAME;
	}
}

class DictionaryConfig
{
	String		dictionaryName;
	String		rdmfieldDictionaryFileName;
	String		enumtypeDefFileName;
	boolean     isLocalDictionary;

	DictionaryConfig()
	{
		isLocalDictionary = false;
	}

	void clear()
	{
		dictionaryName = null;
		rdmfieldDictionaryFileName = null;
		enumtypeDefFileName = null;
		isLocalDictionary = false;
	}
}
