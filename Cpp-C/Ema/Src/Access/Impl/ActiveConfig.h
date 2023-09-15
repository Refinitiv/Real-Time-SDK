/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2020-2023 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ActiveConfig_h
#define __refinitiv_ema_access_ActiveConfig_h

#include "OmmLoggerClient.h"
#include "DictionaryCallbackClient.h"
#include "EmaConfigImpl.h"
#include "OmmIProviderConfig.h"
#include "OAuth2Credential.h"
#include "OmmOAuth2CredentialImpl.h"
#include "DataDictionary.h"

#include "LoginRdmReqMsgImpl.h"

#include "rtr/rsslTransport.h"
#include "rtr/rwfNet.h"
#include "rtr/rsslReactor.h"

#define DEFAULT_ACCEPT_DIR_MSG_WITHOUT_MIN_FILTERS      false
#define DEFAULT_ACCEPT_MSG_SAMEKEY_BUT_DIFF_STREAM      false
#define DEFAULT_ACCEPT_MSG_THAT_CHANGES_SERVICE         false
#define DEFAULT_ACCEPT_MSG_WITHOUT_ACCEPTING_REQUESTS   false
#define DEFAULT_ACCEPT_MSG_WITHOUT_BEING_LOGIN          false
#define DEFAULT_ACCEPT_MSG_WITHOUT_QOS_IN_RANGE         false
#define DEFAULT_REFRESH_FIRST_REQUIRED					true
// values for DEFAULT_COMPRESSION_THRESHOLD* must match (or exceed) those found in
// rsslSocketTransportImpl.c
#define DEFAULT_COMPRESSION_THRESHOLD					30
#define DEFAULT_COMPRESSION_THRESHOLD_LZ4				300

#define DEFAULT_COMPRESSION_TYPE						RSSL_COMP_NONE
#define DEFAULT_CONNECTION_TYPE							RSSL_CONN_TYPE_SOCKET
#define DEFAULT_CONNECTION_PINGTIMEOUT					60000
#define DEFAULT_CONNECTION_MINPINGTIMEOUT				20000
#define DEFAULT_DICTIONARY_REQUEST_TIMEOUT				45000
#define DEFAULT_DICTIONARY_TYPE							Dictionary::FileDictionaryEnum
#define DEFAULT_DIRECTORY_REQUEST_TIMEOUT				45000
#define DEFAULT_DIRECT_WRITE							0
#define DEFAULT_DISPATCH_TIMEOUT_API_THREAD				-1
#define DEFAULT_RDP_RT_LOCATION							EmaString( "us-east-1" )
#define DEFAULT_REISSUE_TOKEN_ATTEMP_LIMIT				-1
#define DEFAULT_REISSUE_TOKEN_ATTEMP_INTERVAL			5000
#define DEFAULT_GUARANTEED_OUTPUT_BUFFERS				100
#define DEFAULT_PROVIDER_GUARANTEED_OUTPUT_BUFFERS		5000
#define DEFAULT_NUM_INPUT_BUFFERS					    10
#if defined(_WIN32) || defined(WIN32)
#define DEFAULT_SYS_SEND_BUFFER_SIZE				    65535
#define DEFAULT_SYS_RECEIVE_BUFFER_SIZE				    65535
#else
#define DEFAULT_SYS_SEND_BUFFER_SIZE				    0
#define DEFAULT_SYS_RECEIVE_BUFFER_SIZE				    0
#endif
#define DEFAULT_PROVIDER_SYS_SEND_BUFFER_SIZE	        (RWF_MAX_16)	
#define DEFAULT_PROVIDER_SYS_RECEIVE_BUFFER_SIZE        (RWF_MAX_16)	
#define DEFAULT_HIGH_WATER_MARK						    0
#define DEFAULT_HANDLE_EXCEPTION					    true
#define DEFAULT_HOST_NAME							    EmaString( "localhost" )
#define DEFAULT_CHANNEL_SET_NAME					    EmaString( "" )
#define DEFAULT_INCLUDE_DATE_IN_LOGGER_OUTPUT		    false
#define DEFAULT_INITIALIZATION_TIMEOUT				    5
#define DEFAULT_INITIALIZATION_TIMEOUT_ENCRYPTED_CON	10
#define DEFAULT_INITIALIZATION_ACCEPT_TIMEOUT		    60
#define DEFAULT_INTERFACE_NAME						    EmaString( "" )
#define DEFAULT_ITEM_COUNT_HINT						    100000
#define DEFAULT_LOGGER_SEVERITY						    OmmLoggerClient::SuccessEnum
#define DEFAULT_LOGGER_TYPE							    OmmLoggerClient::StdoutEnum
#define DEFAULT_LOGIN_REQUEST_TIMEOUT                   45000
#define DEFAULT_MAX_DISPATCH_COUNT_API_THREAD		    100
#define DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD		    100
#define DEFAULT_MAX_FILE_SIZE                           0
#define DEFAULT_MAX_FILE_NUMBER                         0
#define DEFAULT_MAX_OUTSTANDING_POSTS				    100000
#define DEFAULT_MSGKEYINUPDATES						    true
#define DEFAULT_OBEY_OPEN_WINDOW					    1
#define DEFAULT_PIPE_PORT							    9001
#define DEFAULT_SERVER_PIPE_PORT					    9009
#define DEFAULT_POST_ACK_TIMEOUT					    15000
#define DEFAULT_PROXY_CONNECTION_TIMEOUT			    40
#define DEFAULT_REACTOR_EVENTFD_PORT				    55000
#define DEFAULT_RECONNECT_ATTEMPT_LIMIT				    -1
#define DEFAULT_RECONNECT_MAX_DELAY					    5000
#define DEFAULT_RECONNECT_MIN_DELAY					    1000
#define DEFAULT_REQUEST_TIMEOUT						   15000
#define DEFAULT_REST_REQUEST_TIMEOUT				   90
#define DEFAULT_SERVICE_COUNT_HINT					   513
#define DEFAULT_OBJECT_NAME							   EmaString( "" )
#define DEFAULT_SSL_CA_STORE						   EmaString( "" )
#define DEFAULT_TCP_NODELAY							   RSSL_TRUE
#define DEFAULT_SERVER_SHAREDSOCKET					   RSSL_FALSE
#define DEFAULT_CONS_MCAST_CFGSTRING				   EmaString( "" )
#define DEFAULT_PACKET_TTL							  5
#define DEFAULT_NDATA								  7
#define DEFAULT_NMISSING							  128
#define DEFAULT_NREQ								  3
#define DEFAULT_PKT_POOLLIMIT_HIGH					  190000
#define DEFAULT_PKT_POOLLIMIT_LOW					  180000
#define DEFAULT_MAX_EVENT_IN_POOL					  -1
#define DEFAULT_TDATA								  1
#define DEFAULT_TOKEN_REISSUE_RATIO					  0.8
#define DEFAULT_TRREQ								  4
#define DEFAULT_TWAIT								  3
#define DEFAULT_TBCHOLD								  3
#define DEFAULT_TPPHOLD								  3
#define DEFAULT_USER_QLIMIT							  (RWF_MAX_16)
#define LOWLIMIT_USER_QLIMIT						  4096
#define DEFAULT_XML_TRACE_DUMP						  false
#define DEFAULT_XML_TRACE_FILE_NAME					  EmaString( "EmaTrace" )
#define DEFAULT_XML_TRACE_HEX						  false
#define DEFAULT_XML_TRACE_MAX_FILE_SIZE				  100000000
#define DEFAULT_XML_TRACE_PING						  false
#define DEFAULT_XML_TRACE_READ						  true
#define DEFAULT_XML_TRACE_TO_FILE					  false
#define DEFAULT_XML_TRACE_TO_MULTIPLE_FILE			  false
#define DEFAULT_XML_TRACE_TO_STDOUT					  false
#define DEFAULT_XML_TRACE_WRITE						  true
#define DEFAULT_SEND_JSON_CONV_ERROR		  		  false
#define DEFAULT_SERVICE_DISCOVERY_RETRY_COUNT		  3
#define DEFAULT_WS_MAXMSGSIZE						  61440
#define DEFAULT_WS_PROTOCLOS						  EmaString( "tr_json2, rssl.rwf, rssl.json.v2" )
#define DEFAULT_MAX_FRAGMENT_SIZE					  6144
#define DEFAULT_CATCH_UNKNOWN_JSON_FIDS				  true
#define DEFAULT_CATCH_UNKNOWN_JSON_KEYS				  false
#define DEFAULT_CLOSE_CHANNEL_FROM_FAILURE			  true
#define DEFAULT_SERVICE_ID_FOR_CONVERTER			  1
#define DEFAULT_JSON_EXPANDED_ENUM_FIELDS			  false
#define DEFAULT_OUTPUT_BUFFER_SIZE					  (RWF_MAX_16)
#define DEFAULT_JSON_TOKEN_INCREMENT_SIZE			  500
#define DEFAULT_ENABLE_RTT							  false
#define DEFAULT_REST_ENABLE_LOG						  false
#define DEFAULT_REST_ENABLE_LOG_VIA_CALLBACK		  false
#define DEFAULT_WSB_DOWNLOAD_CONNECTION_CONFIG		  false;
#define DEFAULT_WSB_MODE							  RSSL_RWSB_MODE_LOGIN_BASED
#define DEFAULT_SHOULD_INIT_CPUID_LIB				  true

#define SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL	0x01  /*!< Indicates that host set though EMA interface function calls for RSSL_SOCKET connection type */
#define SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL	0x02  /*!< Indicates that server listen port set though EMA interface function call from server client*/
#define PROXY_HOST_CONFIG_BY_FUNCTION_CALL 0x04  /*!< Indicates that tunneling proxy host set though EMA interface function calls */
#define PROXY_PORT_CONFIG_BY_FUNCTION_CALL 0x08  /*!< Indicates that tunneling proxy host set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
#define TUNNELING_OBJNAME_CONFIG_BY_FUNCTION_CALL 0x10  /*!< Indicates that tunneling proxy host set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
#define PROXY_USERNAME_CONFIG_BY_FUNCTION_CALL 0x20  /*!< Indicates that tunneling proxy host set though EMA interface function calls */
#define PROXY_PASSWD_CONFIG_BY_FUNCTION_CALL 0x40  /*!< Indicates that tunneling proxy host set though EMA interface function calls for HTTP/ENCRYPTED connection type*/
#define PROXY_DOMAIN_CONFIG_BY_FUNCTION_CALL 0x80  /*!< Indicates that tunneling proxy host set though EMA interface function calls for HTTP/ENCRYPTED connection type*/

namespace refinitiv {

namespace ema {

namespace access {

class Channel;
class WarmStandbyChannelConfig;

class ChannelConfig
{
public :

	enum ChannelType
	{
		SocketChannelEnum = RSSL_CONN_TYPE_SOCKET,
		ReliableMcastChannelEnum = RSSL_CONN_TYPE_RELIABLE_MCAST
	};

	ChannelConfig( RsslConnectionTypes );

	virtual ~ChannelConfig();

	virtual void clear();

	void setGuaranteedOutputBuffers( UInt64 value );
	void setNumInputBuffers( UInt64 value );

	virtual ChannelType getType() const = 0;

	EmaString				name;
	EmaString				interfaceName;
	RsslCompTypes			compressionType;
	UInt32					compressionThreshold;
	bool					compressionThresholdSet;
	RsslConnectionTypes		connectionType;
	UInt32					connectionPingTimeout;
	UInt32					directWrite;
	UInt32					initializationTimeout;
	UInt32					guaranteedOutputBuffers;
	UInt32					numInputBuffers;
	UInt32					sysRecvBufSize;
	UInt32					sysSendBufSize;
	UInt32					highWaterMark;
	Channel*				pChannel;

private :

	ChannelConfig();
};

class ServerConfig
{
public:

	enum ServerType
	{
		SocketChannelEnum = RSSL_CONN_TYPE_SOCKET
	};

	ServerConfig(RsslConnectionTypes);

	virtual ~ServerConfig();

	virtual void clear();

	void setGuaranteedOutputBuffers(UInt64 value);
	void setNumInputBuffers(UInt64 value);

	virtual ServerType getType() const = 0;

	EmaString				name;
	EmaString				interfaceName;
	RsslCompTypes			compressionType;
	bool					compressionThresholdSet;
	UInt32					compressionThreshold;
	RsslConnectionTypes		connectionType;
	UInt32					connectionPingTimeout;
	UInt32					connectionMinPingTimeout;
	UInt32					directWrite;
	UInt32					initializationTimeout;
	UInt32					guaranteedOutputBuffers;
	UInt32					numInputBuffers;
	UInt32					sysRecvBufSize;
	UInt32					sysSendBufSize;
	UInt32					highWaterMark;

private:

	ServerConfig();
};

class DictionaryConfig : public ListLinks<DictionaryConfig>
{
public:

	DictionaryConfig();

	virtual ~DictionaryConfig();

	void clear();

	EmaString						dictionaryName;
	EmaString						rdmfieldDictionaryFileName;
	EmaString						enumtypeDefFileName;
	EmaString						rdmFieldDictionaryItemName;
	EmaString						enumTypeDefItemName;
	Dictionary::DictionaryType		dictionaryType;
	DataDictionary*					dataDictionary;
	bool							shouldCopyIntoAPI;
};

class ServiceDictionaryConfig : public ListLinks<ServiceDictionaryConfig>
{
public:

	ServiceDictionaryConfig();

	virtual ~ServiceDictionaryConfig();

	void clear();

	UInt16							serviceId;

	void addDictionaryUsed(DictionaryConfig*);

	void addDictionaryProvided(DictionaryConfig*);

	DictionaryConfig* findDictionary(const EmaString& dictionaryName, bool isDictProvided);

	EmaList<DictionaryConfig*>& getDictionaryUsedList();

	EmaList<DictionaryConfig*>& getDictionaryProvidedList();

private:
	EmaList<DictionaryConfig*>	dictionaryUsedList;
	EmaList<DictionaryConfig*>	dictionaryProvidedList;
};

class SocketChannelConfig : public ChannelConfig
{
public :

	SocketChannelConfig( const EmaString& defaultHostName, const EmaString& defaultServiceName, RsslConnectionTypes connType);

	virtual ~SocketChannelConfig();

	void setProxyConnectionTimeout(UInt64 value);
	void setServiceDiscoveryRetryCount(UInt64 value);
	void setWsMaxMsgSize(UInt64 value);
	void clear();

	ChannelType getType() const;
	RsslConnectionTypes		encryptedConnectionType;
	EmaString		hostName;
	EmaString		serviceName;
	RsslBool		tcpNodelay;
	EmaString				objectName;
	EmaString				proxyHostName;
	EmaString				proxyPort;
	EmaString				proxyUserName;
	EmaString				proxyPasswd;
	EmaString				proxyDomain;
	UInt32					proxyConnectionTimeout;
	EmaString				sslCAStore;
	int						securityProtocol;
	EmaString				location;
	RsslBool				enableSessionMgnt;
	UInt32					serviceDiscoveryRetryCount;
	UInt64			wsMaxMsgSize;
	EmaString		wsProtocols;

private :

	EmaString		defaultServiceName;
	EmaString		defaultHostName;
};

class SocketServerConfig : public ServerConfig
{
public:

	SocketServerConfig(const EmaString&);

	virtual ~SocketServerConfig();

	void clear();

	ServerType getType() const;

	EmaString		serviceName;
	RsslBool		tcpNodelay;
	RsslBool		serverSharedSocket;

	EmaString				libSslName;
	EmaString				libCryptoName;
	EmaString				libCurlName;

	EmaString				serverCert;
	EmaString				serverPrivateKey;
	EmaString				cipherSuite;
	EmaString				dhParams;

	UInt64			maxFragmentSize;
	EmaString		wsProtocols;

private:

	EmaString		defaultServiceName;
};

class ReliableMcastChannelConfig : public ChannelConfig
{
public :

	ReliableMcastChannelConfig();

	virtual ~ReliableMcastChannelConfig();

	void setPacketTTL( UInt64 value );

	void setHsmInterval( UInt64 value );

	void setNdata( UInt64 value );

	void setNmissing( UInt64 value );

	void setNrreq( UInt64 value );

	void setTdata( UInt64 value );

	void setTrreq( UInt64 value );

	void setPktPoolLimitHigh( UInt64 value );

	void setPktPoolLimitLow( UInt64 value );

	void setTwait( UInt64 value );

	void setTbchold( UInt64 value );

	void setTpphold( UInt64 value );

	void setUserQLimit( UInt64 value );

	void clear();

	ChannelType getType() const;

	EmaString				recvAddress;
	EmaString				recvServiceName;
	EmaString				unicastServiceName;
	EmaString				sendAddress;
	EmaString				sendServiceName;
	EmaString				tcpControlPort;
	EmaString				hsmInterface;
	EmaString				hsmMultAddress;
	EmaString				hsmPort;
	UInt16					hsmInterval;
	bool					disconnectOnGap;
	UInt8					packetTTL;
	UInt32					ndata;
	UInt16					nmissing;
	UInt32					nrreq;
	UInt32					tdata;
	UInt32					trreq;
	UInt32					twait;
	UInt32					tbchold;
	UInt32					tpphold;
	UInt32					pktPoolLimitHigh;
	UInt32					pktPoolLimitLow;
	UInt16					userQLimit;
};

struct LoggerConfig
{
	LoggerConfig();

	virtual ~LoggerConfig();

	void clear();

	EmaString						loggerName;
	EmaString						loggerFileName;
	OmmLoggerClient::Severity		minLoggerSeverity;
	OmmLoggerClient::LoggerType		loggerType;
	bool							includeDateInLoggerOutput;

	UInt32	maxFileSize;
	UInt32	maxFileNumber;
};

class BaseConfig
{
public:

	BaseConfig();

	virtual ~BaseConfig();

	void clear();

	void setItemCountHint(UInt64 value);
	void setServiceCountHint(UInt64 value);
	void setCatchUnhandledException(UInt64 value);
	void setMaxDispatchCountApiThread(UInt64 value);
	void setMaxDispatchCountUserThread(UInt64 value);
	void setMaxEventsInPool(Int64 value);
	void setRequestTimeout(UInt64 value);
	virtual EmaString configTrace();

	EmaString				configuredName;
	EmaString				instanceName;
	EmaString				xmlTraceFileName;
	UInt32					itemCountHint;
	UInt32					serviceCountHint;
	Int64					dispatchTimeoutApiThread;
	UInt32					maxDispatchCountApiThread;
	UInt32					maxDispatchCountUserThread;
	Int32					maxEventsInPool;
	Int64					xmlTraceMaxFileSize;
	bool					xmlTraceToFile;
	bool					xmlTraceToStdout;
	bool					xmlTraceToMultipleFiles;
	bool					xmlTraceWrite;
	bool					xmlTraceRead;
	bool					xmlTracePing;
	bool					xmlTraceHex;
	bool					xmlTraceDump;
	bool					enableRtt;
	bool					restEnableLog;
	bool					restEnableLogViaCallback;
	bool					sendJsonConvError;
	/*ReconnectAttemptLimit,ReconnectMinDelay,ReconnectMaxDelay,MsgKeyInUpdates,XmlTrace... is per Consumer, or per NIProvider
	 *or per IProvider instance now. The per channel configuration on these parameters has been deprecated. This variable is 
	 *used for handling deprecation cases.
	 */
	UInt8					parameterConfigGroup;
	bool					catchUnhandledException;
	LoggerConfig			loggerConfig;
	EmaString				libSslName;
	EmaString				libCryptoName;
	EmaString				libcurlName;
	UInt32					requestTimeout;
	EmaString				traceStr;
	Double					tokenReissueRatio;
	EmaString				restLogFileName;
	bool					shouldInitializeCPUIDlib;

	/* Configure the  RsslReactorJsonConverterOptions */
	UInt16					defaultServiceIDForConverter;
	bool					jsonExpandedEnumFields;
	bool					catchUnknownJsonKeys;
	bool					catchUnknownJsonFids;
	bool					closeChannelFromFailure;
	UInt32					outputBufferSize;
	UInt32					jsonTokenIncrementSize;
};

class ActiveConfig : public BaseConfig
{
public:

	ActiveConfig( const EmaString& );

	virtual ~ActiveConfig();

	void clear();

	void setObeyOpenWindow( UInt64 value );
	void setPostAckTimeout( UInt64 value );
	void setMaxOutstandingPosts( UInt64 value );
	void setLoginRequestTimeOut( UInt64 );
	void setDirectoryRequestTimeOut( UInt64 );
	void setDictionaryRequestTimeOut( UInt64 );
	void setReconnectAttemptLimit(Int64 value);
	void setReconnectMinDelay(Int64 value);
	void setReconnectMaxDelay(Int64 value);
	void setRestRequestTimeOut(UInt64 value);

	ChannelConfig* findChannelConfig( const Channel* pChannel );
	static bool findChannelConfig( EmaVector< ChannelConfig* >&, const EmaString&, unsigned int& );
	static bool findWsbChannelConfig(EmaVector< WarmStandbyChannelConfig* >& cfgWsbChannelSet, const EmaString& wsbChannelName, unsigned int& pos);
	void clearChannelSet();
	void clearWSBChannelSet();
	void clearChannelSetForWSB();
	const EmaString& defaultServiceName() { return _defaultServiceName; }
	EmaString configTrace();

	Int64			pipePort;
	UInt32			obeyOpenWindow;
	UInt32			postAckTimeout;
	UInt32			maxOutstandingPosts;
	UInt32			loginRequestTimeOut;
	UInt32			directoryRequestTimeOut;
	UInt32			dictionaryRequestTimeOut;
	Int32			reconnectAttemptLimit;
	Int32			reconnectMinDelay;
	Int32			reconnectMaxDelay;
	bool			msgKeyInUpdates;
	Int64			reissueTokenAttemptLimit;
	Int64			reissueTokenAttemptInterval;
	UInt32			restRequestTimeOut; // in seconds

	DictionaryConfig		dictionaryConfig;

	EmaVector< ChannelConfig* >		configChannelSet;
	EmaVector< WarmStandbyChannelConfig* >  configWarmStandbySet;
	EmaVector< ChannelConfig* >		configChannelSetForWSB;

	LoginRdmReqMsgImpl*		pRsslRDMLoginReq;
	RsslRequestMsg*			pRsslDirectoryRequestMsg;
	AdminReqMsg*			pRsslRdmFldRequestMsg;
	AdminReqMsg*			pRsslEnumDefRequestMsg;
	AdminRefreshMsg*		pDirectoryRefreshMsg;

protected:

	EmaString				_defaultServiceName;
};

class ActiveServerConfig : public BaseConfig
{
public:
	ActiveServerConfig(const EmaString&);
	virtual ~ActiveServerConfig();

	void clear();

	const EmaString& defaultServiceName() { return _defaultServiceName; }

	virtual OmmIProviderConfig::AdminControl getDictionaryAdminControl() = 0;

	virtual OmmIProviderConfig::AdminControl getDirectoryAdminControl() = 0;

	Int64						pipePort;
	AdminRefreshMsg*			pDirectoryRefreshMsg;

	ServerConfig*				pServerConfig;

	bool                        acceptMessageWithoutBeingLogin;

	bool                        acceptMessageWithoutAcceptingRequests;

	bool                        acceptDirMessageWithoutMinFilters;

	bool                        acceptMessageWithoutQosInRange;

	bool                        acceptMessageSameKeyButDiffStream;

	bool                        acceptMessageThatChangesService;

	ServiceDictionaryConfig*	getServiceDictionaryConfig(UInt16 serviceId);

	void						addServiceDictionaryConfig(ServiceDictionaryConfig*);

	void						removeServiceDictionaryConfig(ServiceDictionaryConfig*);

	const EmaList<ServiceDictionaryConfig*>& getServiceDictionaryConfigList();

	void setServiceDictionaryConfigList(EmaList<ServiceDictionaryConfig*>&);

	EmaString configTrace();

protected:

	EmaString					_defaultServiceName;

	class UInt16rHasher
	{
	public:
		size_t operator()(const UInt16&) const;
	};

	class UInt16Equal_To
	{
	public:
		bool operator()(const UInt16&, const UInt16&) const;
	};

	typedef HashTable< UInt16, ServiceDictionaryConfig*, UInt16rHasher, UInt16Equal_To > ServiceDictionaryConfigHash;

	ServiceDictionaryConfigHash			_serviceDictionaryConfigHash;
	EmaList<ServiceDictionaryConfig*>	_serviceDictionaryConfigList;
};

class WarmStandbyServerInfoConfig
{
public:

	EmaString						name;
	ChannelConfig*					channelConfig;
	EmaVector< EmaString* >		perServiceNameSet;

	WarmStandbyServerInfoConfig(const EmaString& name);
	virtual ~WarmStandbyServerInfoConfig();

	void clear();

private:

	WarmStandbyServerInfoConfig();
};

class WarmStandbyChannelConfig
{
public:

	enum WarmStandbyMode
	{
		LoginBasedEnum = RSSL_RWSB_MODE_LOGIN_BASED,
		ServiceBasedEnum = RSSL_RWSB_MODE_SERVICE_BASED
	};

	WarmStandbyChannelConfig(const EmaString& name);
	virtual ~WarmStandbyChannelConfig();

	void clear();

	EmaString										name;
	WarmStandbyServerInfoConfig*					startingActiveServer;
	EmaVector<WarmStandbyServerInfoConfig*>		standbyServerSet;
	bool									downloadConnectionConfig;
	WarmStandbyMode							warmStandbyMode;
private:
	WarmStandbyChannelConfig();
};

}

}

}

#endif // __refinitiv_ema_access_ActiveConfig_h
