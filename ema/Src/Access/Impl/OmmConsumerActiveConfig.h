/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerActiveConfig_h
#define __thomsonreuters_ema_access_OmmConsumerActiveConfig_h

#include "EmaString.h"
#include "OmmLoggerClient.h"
#include "DictionaryCallbackClient.h"
#include "OmmConsumerConfig.h"
#include "rtr/rsslTransport.h"

#define DEFAULT_COMPRESSION_THRESHOLD				0
#define DEFAULT_COMPRESSION_TYPE					RSSL_COMP_NONE
#define DEFAULT_CONNECTION_TYPE						RSSL_CONN_TYPE_SOCKET
#define DEFAULT_CONNECTION_PINGTIMEOUT				30000
#define DEFAULT_DICTIONARY_REQUEST_TIMEOUT			45000
#define DEFAULT_DICTIONARY_TYPE						Dictionary::FileDictionaryEnum
#define DEFAULT_DIRECTORY_REQUEST_TIMEOUT			45000
#define DEFAULT_DISPATCH_TIMEOUT_API_THREAD			-1
#define DEFAULT_GUARANTEED_OUTPUT_BUFFERS			100
#define DEFAULT_HANDLE_EXCEPTION					true
#define DEFAULT_HOST_NAME							EmaString( "localhost" )
#define DEFAULT_INCLUDE_DATE_IN_LOGGER_OUTPUT		false
#define DEFAULT_INTERFACE_NAME						EmaString( "" )
#define DEFAULT_ITEM_COUNT_HINT						100000
#define DEFAULT_LOGGER_SEVERITY						OmmLoggerClient::SuccessEnum
#define DEFAULT_LOGGER_TYPE							OmmLoggerClient::StdoutEnum
#define DEFAULT_LOGIN_REQUEST_TIMEOUT               45000
#define DEFAULT_MAX_DISPATCH_COUNT_API_THREAD		100
#define DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD		100
#define DEFAULT_MAX_OUTSTANDING_POSTS				100000
#define DEFAULT_MSGKEYINUPDATES						true
#define DEFAULT_OBEY_OPEN_WINDOW					1
#define DEFAULT_PIPE_PORT							9001
#define DEFAULT_POST_ACK_TIMEOUT					15000
#define DEFAULT_REACTOR_EVENTFD_PORT				55000
#define DEFAULT_RECONNECT_ATTEMPT_LIMIT				-1
#define DEFAULT_RECONNECT_MAX_DELAY					5000
#define DEFAULT_RECONNECT_MIN_DELAY					1000
#define DEFAULT_REQUEST_TIMEOUT						15000
#define DEFAULT_SERVICE_COUNT_HINT					513
#define DEFAULT_SERVICE_NAME						EmaString( "14002" )
#define DEFAULT_TCP_NODELAY							RSSL_TRUE
#define DEFAULT_USER_DISPATCH						OmmConsumerConfig::ApiDispatchEnum
#define DEFAULT_XML_TRACE_FILE_NAME					EmaString( "EmaTrace" )
#define DEFAULT_XML_TRACE_MAX_FILE_SIZE				100000000
#define DEFAULT_XML_TRACE_READ						true
#define DEFAULT_XML_TRACE_TO_FILE					false
#define DEFAULT_XML_TRACE_TO_MULTIPLE_FILE			false
#define DEFAULT_XML_TRACE_TO_STDOUT					false
#define DEFAULT_XML_TRACE_WRITE						true

namespace thomsonreuters {

namespace ema {

namespace access {

class ChannelConfig
{
public :

	enum ChannelType {

		SocketChannelEnum = RSSL_CONN_TYPE_SOCKET,			// indicates config for tcp ip connection
		EncryptedChannelEnum = RSSL_CONN_TYPE_ENCRYPTED,
		HttpChannelEnum = RSSL_CONN_TYPE_HTTP,
		ReliableMcastChannelEnum = RSSL_CONN_TYPE_RELIABLE_MCAST	// indicates config for mcast connection
	};

	ChannelConfig( RsslConnectionTypes );

	virtual ~ChannelConfig();

	virtual void clear();

	void setGuaranteedOutputBuffers(UInt64 value);

	virtual ChannelType getType() const = 0;

	EmaString				name;
	EmaString				interfaceName;
	EmaString				xmlTraceFileName;
	RsslCompTypes			compressionType;
	Int64					compressionThreshold; // <TODO> post EAP.
	RsslConnectionTypes		connectionType;
	UInt32					connectionPingTimeout;
	UInt32					guaranteedOutputBuffers;
	Int64					reconnectAttemptLimit;
	Int64					reconnectMinDelay;
	Int64					reconnectMaxDelay;
	Int64					xmlTraceMaxFileSize;
	bool					xmlTraceToFile;
	bool					xmlTraceToStdout;
	bool					xmlTraceToMultipleFiles;
	bool					xmlTraceWrite;
	bool					xmlTraceRead;
	bool					msgKeyInUpdates;

private : 

	ChannelConfig();
};

class SocketChannelConfig : public ChannelConfig
{
public :

	SocketChannelConfig();

	virtual ~SocketChannelConfig();

	void clear();

	ChannelType getType() const;

	EmaString				hostName;
	EmaString				serviceName;
	
	RsslBool				tcpNodelay;

};

class ReliableMcastChannelConfig : public ChannelConfig
{
public :

	ReliableMcastChannelConfig();

	virtual ~ReliableMcastChannelConfig();

	void clear();

	ChannelType getType() const;

};

class EncryptedChannelConfig : public ChannelConfig
{
public :

	EncryptedChannelConfig();

	virtual ~EncryptedChannelConfig();

	void clear();

	ChannelType getType() const;

	EmaString				hostName;
	EmaString				serviceName;
	
	RsslBool				tcpNodelay;

};

class HttpChannelConfig : public ChannelConfig
{
public :

	HttpChannelConfig();

	virtual ~HttpChannelConfig();

	void clear();

	ChannelType getType() const;

	EmaString				hostName;
	EmaString				serviceName;
	
	RsslBool				tcpNodelay;

};

class DictionaryConfig
{
public :

	DictionaryConfig();
	virtual ~DictionaryConfig();

	void clear();

	EmaString						dictionaryName;
	EmaString						rdmfieldDictionaryFileName;
	EmaString						enumtypeDefFileName;
	Dictionary::DictionaryType		dictionaryType;
};

class LoggerConfig
{
public :

	LoggerConfig();
	virtual ~LoggerConfig();

	void clear();

	EmaString						loggerName;
	EmaString						loggerFileName;
	OmmLoggerClient::Severity		minLoggerSeverity;
	OmmLoggerClient::LoggerType		loggerType;
	bool							includeDateInLoggerOutput;
};

class OmmConsumerActiveConfig
{
public :

	OmmConsumerActiveConfig();

	virtual ~OmmConsumerActiveConfig();

	void clear();
	
	void setItemCountHint(UInt64 value);
	void setServiceCountHint(UInt64 value);
	void setObeyOpenWindow(UInt64 value);
	void setPostAckTimeout(UInt64 value);
	void setRequestTimeout(UInt64 value);
	void setMaxOutstandingPosts(UInt64 value);
	void setCatchUnhandledException(UInt64 value);
	void setMaxDispatchCountApiThread(UInt64 value);
	void setMaxDispatchCountUserThread(UInt64 value);
	void setLoginRequestTimeOut( UInt64 );
	void setDirectoryRequestTimeOut( UInt64 );
	void setDictionaryRequestTimeOut( UInt64 );

	EmaString						consumerName;
	EmaString                       instanceName;
	UInt32							itemCountHint;
	UInt32							serviceCountHint;
	Int64							dispatchTimeoutApiThread;
	UInt32							maxDispatchCountApiThread;
	UInt32							maxDispatchCountUserThread;
	Int64							pipePort;
	UInt32							obeyOpenWindow;
	UInt32							requestTimeout;
	UInt32							postAckTimeout;
	UInt32							maxOutstandingPosts;
	UInt32							loginRequestTimeOut;
	UInt32							directoryRequestTimeOut;
	UInt32							dictionaryRequestTimeOut;
	bool                            catchUnhandledException;

	OmmConsumerConfig::OperationModel		userDispatch;
	
	ChannelConfig*					channelConfig;
	DictionaryConfig				dictionaryConfig;
	LoggerConfig					loggerConfig;
	RsslRDMLoginRequest*			pRsslRDMLoginReq;
	RsslRequestMsg*					pRsslDirectoryRequestMsg;
	RsslRequestMsg*					pRsslRdmFldRequestMsg;
	RsslRequestMsg*					pRsslEnumDefRequestMsg;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerActiveConfig_h
