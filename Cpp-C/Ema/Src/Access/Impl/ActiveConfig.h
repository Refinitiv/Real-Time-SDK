/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ActiveConfig_h
#define __thomsonreuters_ema_access_ActiveConfig_h

#include "OmmLoggerClient.h"
#include "DictionaryCallbackClient.h"
#include "EmaConfigImpl.h"
#include "OmmIProviderConfig.h"

#include "rtr/rsslTransport.h"

#define DEFAULT_ACCEPT_DIR_MSG_WITHOUT_MIN_FILTERS      false
#define DEFAULT_ACCEPT_MSG_SAMEKEY_BUT_DIFF_STREAM      false
#define DEFAULT_ACCEPT_MSG_THAT_CHANGES_SERVICE         false
#define DEFAULT_ACCEPT_MSG_WITHOUT_ACCEPTING_REQUESTS   false
#define DEFAULT_ACCEPT_MSG_WITHOUT_BEING_LOGIN          false
#define DEFAULT_ACCEPT_MSG_WITHOUT_QOS_IN_RANGE         false
#define DEFAULT_COMPRESSION_THRESHOLD					30
#define DEFAULT_COMPRESSION_TYPE						RSSL_COMP_NONE
#define DEFAULT_CONNECTION_TYPE							RSSL_CONN_TYPE_SOCKET
#define DEFAULT_CONNECTION_PINGTIMEOUT					60000
#define DEFAULT_CONNECTION_MINPINGTIMEOUT				20000
#define DEFAULT_DICTIONARY_REQUEST_TIMEOUT				45000
#define DEFAULT_DICTIONARY_TYPE							Dictionary::FileDictionaryEnum
#define DEFAULT_DIRECTORY_REQUEST_TIMEOUT				45000
#define DEFAULT_DISPATCH_TIMEOUT_API_THREAD				-1
#define DEFAULT_GUARANTEED_OUTPUT_BUFFERS				100
#define DEFAULT_PROVIDER_GUARANTEED_OUTPUT_BUFFERS		5000
#define DEFAULT_NUM_INPUT_BUFFERS					    10
#define DEFAULT_SYS_SEND_BUFFER_SIZE				    0
#define DEFAULT_SYS_RECEIVE_BUFFER_SIZE				    0
#define DEFAULT_PROVIDER_SYS_SEND_BUFFER_SIZE	        65535	
#define DEFAULT_PROVIDER_SYS_RECEIVE_BUFFER_SIZE        65535	
#define DEFAULT_HIGH_WATER_MARK						    0
#define DEFAULT_HANDLE_EXCEPTION					    true
#define DEFAULT_HOST_NAME							    EmaString( "localhost" )
#define DEFAULT_CHANNEL_SET_NAME					    EmaString( "" )
#define DEFAULT_INCLUDE_DATE_IN_LOGGER_OUTPUT		    false
#define DEFAULT_INTERFACE_NAME						    EmaString( "" )
#define DEFAULT_ITEM_COUNT_HINT						    100000
#define DEFAULT_LOGGER_SEVERITY						    OmmLoggerClient::SuccessEnum
#define DEFAULT_LOGGER_TYPE							    OmmLoggerClient::StdoutEnum
#define DEFAULT_LOGIN_REQUEST_TIMEOUT                   45000
#define DEFAULT_MAX_DISPATCH_COUNT_API_THREAD		    100
#define DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD		    100
#define DEFAULT_MAX_OUTSTANDING_POSTS				    100000
#define DEFAULT_MSGKEYINUPDATES						    true
#define DEFAULT_OBEY_OPEN_WINDOW					    1
#define DEFAULT_PIPE_PORT							    9001
#define DEFAULT_SERVER_PIPE_PORT					    9009
#define DEFAULT_POST_ACK_TIMEOUT					    15000
#define DEFAULT_REACTOR_EVENTFD_PORT				    55000
#define DEFAULT_RECONNECT_ATTEMPT_LIMIT				    -1
#define DEFAULT_RECONNECT_MAX_DELAY					    5000
#define DEFAULT_RECONNECT_MIN_DELAY					    1000
#define DEFAULT_REQUEST_TIMEOUT						   15000
#define DEFAULT_SERVICE_COUNT_HINT					   513
#define DEFAULT_OBJECT_NAME							   EmaString( "" )
#define DEFAULT_TCP_NODELAY							   RSSL_TRUE
#define DEFAULT_CONS_MCAST_CFGSTRING				   EmaString( "" )
#define DEFAULT_PACKET_TTL							  5
#define DEFAULT_NDATA								  7
#define DEFAULT_NMISSING							  128
#define DEFAULT_NREQ								  3
#define DEFAULT_PKT_POOLLIMIT_HIGH					  190000
#define DEFAULT_PKT_POOLLIMIT_LOW					  180000
#define DEFAULT_TDATA								  1
#define DEFAULT_TRREQ								  4
#define DEFAULT_TWAIT								  3
#define DEFAULT_TBCHOLD								  3
#define DEFAULT_TPPHOLD								  3
#define DEFAULT_USER_QLIMIT							  65535
#define DEFAULT_XML_TRACE_FILE_NAME					  EmaString( "EmaTrace" )
#define DEFAULT_XML_TRACE_HEX						  false
#define DEFAULT_XML_TRACE_MAX_FILE_SIZE				  100000000
#define DEFAULT_XML_TRACE_PING						  false
#define DEFAULT_XML_TRACE_READ						  true
#define DEFAULT_XML_TRACE_TO_FILE					  false
#define DEFAULT_XML_TRACE_TO_MULTIPLE_FILE			  false
#define DEFAULT_XML_TRACE_TO_STDOUT					  false
#define DEFAULT_XML_TRACE_WRITE						  true

/* 
 * The following definitions will be removed in the future after the parameterConfigGroup variable has been removed.
 */
#define PARAMETER_NOT_SET							0x00  /*!< Indicates no parameters set in any config group */
#define PARAMETER_SET_IN_CONSUMER_PROVIDER			0x01  /*!< Indicates that there are parameters set in Consumer, NIProvider or IProvider group inside EmaConfig.xml file */
#define PARAMETER_SET_BY_PROGRAMMATIC				0x02  /*!< Indicates that there are parameters set through the programmatical way */

#define SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL	0x01  /*!< Indicates that host set though EMA interface function calls for RSSL_SOCKET connection type */
#define TUNNELING_CONN_CONFIG_BY_FUNCTION_CALL		0x02  /*!< Indicates that tunneling configuration set though EMA interface function calls
															 for RSSL_HTTP or RSSL_ENCRYPTED connection type */


namespace thomsonreuters {

namespace ema {

namespace access {

class Channel;

class ChannelConfig
{
public :

	enum ChannelType
	{
		SocketChannelEnum = RSSL_CONN_TYPE_SOCKET,
		EncryptedChannelEnum = RSSL_CONN_TYPE_ENCRYPTED,
		HttpChannelEnum = RSSL_CONN_TYPE_HTTP,
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
	RsslConnectionTypes		connectionType;
	UInt32					connectionPingTimeout;
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
	EmaString				xmlTraceFileName;
	RsslCompTypes			compressionType;
	UInt32					compressionThreshold;
	RsslConnectionTypes		connectionType;
	UInt32					connectionPingTimeout;
	UInt32					connectionMinPingTimeout;
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

	const EmaList<DictionaryConfig*>& getDictionaryUsedList();

	const EmaList<DictionaryConfig*>& getDictionaryProvidedList();

private:
	EmaList<DictionaryConfig*>	dictionaryUsedList;
	EmaList<DictionaryConfig*>	dictionaryProvidedList;
};

class SocketChannelConfig : public ChannelConfig
{
public :

	SocketChannelConfig( const EmaString& );

	virtual ~SocketChannelConfig();

	void clear();

	ChannelType getType() const;

	EmaString		hostName;
	EmaString		serviceName;
	RsslBool		tcpNodelay;

private :

	EmaString		defaultServiceName;
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

class HttpChannelConfig : public ChannelConfig
{
public :

	HttpChannelConfig();
	HttpChannelConfig(RsslConnectionTypes);

	virtual ~HttpChannelConfig();

	void clear();

	ChannelType getType() const;

	EmaString				hostName;
	EmaString				serviceName;
	EmaString				objectName;
	RsslBool				tcpNodelay;
	EmaString				proxyHostName;
	EmaString				proxyPort;
};

class EncryptedChannelConfig : public HttpChannelConfig
{
public:

	EncryptedChannelConfig();

	virtual ~EncryptedChannelConfig();

	void clear();

	ChannelType getType() const;

	int		securityProtocol;
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

	EmaString				configuredName;
	EmaString				instanceName;
	EmaString				xmlTraceFileName;
	UInt32					itemCountHint;
	UInt32					serviceCountHint;
	Int64					dispatchTimeoutApiThread;
	UInt32					maxDispatchCountApiThread;
	UInt32					maxDispatchCountUserThread;
	Int64					xmlTraceMaxFileSize;
	bool					xmlTraceToFile;
	bool					xmlTraceToStdout;
	bool					xmlTraceToMultipleFiles;
	bool					xmlTraceWrite;
	bool					xmlTraceRead;
	bool					xmlTracePing;
	bool					xmlTraceHex;
	/*ReconnectAttemptLimit,ReconnectMinDelay,ReconnectMaxDelay,MsgKeyInUpdates,XmlTrace... is per Consumer, or per NIProvider
	 *or per IProvider instance now. The per channel configuration on these parameters has been deprecated. This variable is 
	 *used for handling deprecation cases.
	 */
	UInt8					parameterConfigGroup;
	bool					catchUnhandledException;
	LoggerConfig			loggerConfig;
	EmaString				libSslName;
	EmaString				libCryptoName;
};

class ActiveConfig : public BaseConfig
{
public:

	ActiveConfig( const EmaString& );

	virtual ~ActiveConfig();

	void clear();

	void setObeyOpenWindow( UInt64 value );
	void setPostAckTimeout( UInt64 value );
	void setRequestTimeout( UInt64 value );
	void setMaxOutstandingPosts( UInt64 value );
	void setLoginRequestTimeOut( UInt64 );
	void setDirectoryRequestTimeOut( UInt64 );
	void setDictionaryRequestTimeOut( UInt64 );
	void setReconnectAttemptLimit(Int64 value);
	void setReconnectMinDelay(Int64 value);
	void setReconnectMaxDelay(Int64 value);

	ChannelConfig* findChannelConfig( const Channel* pChannel );
	static bool findChannelConfig( EmaVector< ChannelConfig* >&, const EmaString&, unsigned int& );
	void clearChannelSet();
	const EmaString& defaultServiceName() { return _defaultServiceName; }

	Int64			pipePort;
	UInt32			obeyOpenWindow;
	UInt32			requestTimeout;
	UInt32			postAckTimeout;
	UInt32			maxOutstandingPosts;
	UInt32			loginRequestTimeOut;
	UInt32			directoryRequestTimeOut;
	UInt32			dictionaryRequestTimeOut;
	Int32			reconnectAttemptLimit;
	Int32			reconnectMinDelay;
	Int32			reconnectMaxDelay;
	bool			msgKeyInUpdates;
	bool			catchUnhandledException;

	DictionaryConfig		dictionaryConfig;

	EmaVector< ChannelConfig* >		configChannelSet;

	RsslRDMLoginRequest*	pRsslRDMLoginReq;
	RsslRequestMsg*			pRsslDirectoryRequestMsg;
	AdminReqMsg*			pRsslRdmFldRequestMsg;
	AdminReqMsg*			pRsslEnumDefRequestMsg;
	AdminRefreshMsg*		pDirectoryRefreshMsg;

	EncryptedChannelConfig* getTunnelingChannelCfg()
	{
		if (_tunnelingChannelCfg == NULL)
			_tunnelingChannelCfg = new EncryptedChannelConfig();
		return _tunnelingChannelCfg;
	}


protected:

	EmaString				_defaultServiceName;
	EncryptedChannelConfig*		_tunnelingChannelCfg;
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

}

}

}

#endif // __thomsonreuters_ema_access_ActiveConfig_h
