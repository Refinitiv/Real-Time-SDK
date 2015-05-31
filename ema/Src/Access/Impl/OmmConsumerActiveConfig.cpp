/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerActiveConfig.h"

using namespace thomsonreuters::ema::access;

DictionaryConfig::DictionaryConfig() :
 dictionaryName(),
 rdmfieldDictionaryFileName(),
 enumtypeDefFileName(),
 dictionaryType( DEFAULT_DICTIONARY_TYPE )
{
}

DictionaryConfig::~DictionaryConfig()
{
}

void DictionaryConfig::clear()
{
	dictionaryName.clear();
	rdmfieldDictionaryFileName.clear();
	enumtypeDefFileName.clear();
	dictionaryType = DEFAULT_DICTIONARY_TYPE;
}

LoggerConfig::LoggerConfig() :
 loggerName(),
 loggerFileName(),
 minLoggerSeverity( DEFAULT_LOGGER_SEVERITY ),
 loggerType( OmmLoggerClient::StdoutEnum ),
 includeDateInLoggerOutput( DEFAULT_INCLUDE_DATE_IN_LOGGER_OUTPUT )
{
}

LoggerConfig::~LoggerConfig()
{
}

void LoggerConfig::clear()
{
	loggerName.clear();
	loggerFileName.clear();
	minLoggerSeverity = DEFAULT_LOGGER_SEVERITY;
	loggerType = OmmLoggerClient::StdoutEnum;
}

OmmConsumerActiveConfig::OmmConsumerActiveConfig() :
 consumerName(),
 instanceName(),
 itemCountHint( DEFAULT_ITEM_COUNT_HINT ),
 serviceCountHint( DEFAULT_SERVICE_COUNT_HINT),
 dispatchTimeoutApiThread( DEFAULT_DISPATCH_TIMEOUT_API_THREAD ),
 maxDispatchCountApiThread( DEFAULT_MAX_DISPATCH_COUNT_API_THREAD ),
 maxDispatchCountUserThread( DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD ),
 pipePort( DEFAULT_PIPE_PORT ),
 obeyOpenWindow( DEFAULT_OBEY_OPEN_WINDOW ),
 requestTimeout( DEFAULT_REQUEST_TIMEOUT ),
 postAckTimeout( DEFAULT_POST_ACK_TIMEOUT ),
 maxOutstandingPosts( DEFAULT_MAX_OUTSTANDING_POSTS ),
 loginRequestTimeOut( DEFAULT_LOGIN_REQUEST_TIMEOUT ),
 directoryRequestTimeOut( DEFAULT_DIRECTORY_REQUEST_TIMEOUT ),
 userDispatch( DEFAULT_USER_DISPATCH ),
 channelConfig( 0 ),
 dictionaryConfig(),
 loggerConfig(),
 pRsslRDMLoginReq( 0 ),
 pRsslDirectoryRequestMsg( 0 ),
 pRsslRdmFldRequestMsg( 0 ),
 pRsslEnumDefRequestMsg( 0 ),
 catchUnhandledException( DEFAULT_HANDLE_EXCEPTION )
{
}

OmmConsumerActiveConfig::~OmmConsumerActiveConfig()
{
	if ( channelConfig )
	{
		delete channelConfig;
		channelConfig = 0;
	}
}

void OmmConsumerActiveConfig::clear()
{
	consumerName.clear();
	instanceName.clear();
	itemCountHint = DEFAULT_ITEM_COUNT_HINT;
	serviceCountHint = DEFAULT_SERVICE_COUNT_HINT;
	dispatchTimeoutApiThread = DEFAULT_DISPATCH_TIMEOUT_API_THREAD;
	maxDispatchCountApiThread = DEFAULT_MAX_DISPATCH_COUNT_API_THREAD;
	maxDispatchCountUserThread = DEFAULT_MAX_DISPATCH_COUNT_USER_THREAD;
	pipePort = DEFAULT_PIPE_PORT;
	obeyOpenWindow = DEFAULT_OBEY_OPEN_WINDOW;
	requestTimeout = DEFAULT_REQUEST_TIMEOUT;
	postAckTimeout = DEFAULT_POST_ACK_TIMEOUT;
	maxOutstandingPosts = DEFAULT_MAX_OUTSTANDING_POSTS;
	userDispatch = DEFAULT_USER_DISPATCH;

	if ( channelConfig )
	{
		delete channelConfig;
		channelConfig = 0;
	}

	dictionaryConfig.clear();
	loggerConfig.clear();
	pRsslRDMLoginReq = 0;
	pRsslDirectoryRequestMsg = 0;
	pRsslRdmFldRequestMsg = 0;
	pRsslEnumDefRequestMsg = 0;
}

void OmmConsumerActiveConfig::setItemCountHint( UInt64 value )
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		itemCountHint = 0xFFFFFFFF;
	else
		itemCountHint = (UInt32)value;
}

void OmmConsumerActiveConfig::setServiceCountHint( UInt64 value )
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		serviceCountHint = 0xFFFFFFFF;
	else
		serviceCountHint = (UInt32)value;
}
void OmmConsumerActiveConfig::setObeyOpenWindow( UInt64 value )
{
	if ( value <= 0 )
		obeyOpenWindow = 0;
	else
		obeyOpenWindow = 1;
}
void OmmConsumerActiveConfig::setPostAckTimeout( UInt64 value )
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		postAckTimeout = 0xFFFFFFFF;
	else
		postAckTimeout = (UInt32)value;
}
void OmmConsumerActiveConfig::setRequestTimeout( UInt64 value )
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		requestTimeout = 0xFFFFFFFF;
	else
		requestTimeout = (UInt32)value;
}

void OmmConsumerActiveConfig::setLoginRequestTimeOut( UInt64 value )
{
	if ( value > 0xFFFFFFFF )
		loginRequestTimeOut = 0xFFFFFFFF;
	else if ( value > 0 )
		loginRequestTimeOut = (UInt32) value;
}

void OmmConsumerActiveConfig::setDirectoryRequestTimeOut( UInt64 value )
{
	if ( value > 0xFFFFFFFF )
		directoryRequestTimeOut = 0xFFFFFFFF;
	else if ( value > 0 )
		directoryRequestTimeOut = (UInt32) value;
}

void OmmConsumerActiveConfig::setMaxOutstandingPosts( UInt64 value )
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		maxOutstandingPosts = 0xFFFFFFFF;
	else
		maxOutstandingPosts = (UInt32)value;
}

void OmmConsumerActiveConfig::setCatchUnhandledException( UInt64 value )
{
	if ( value > 0 )
		catchUnhandledException = true;
	else
		catchUnhandledException = false;
}

void OmmConsumerActiveConfig::setMaxDispatchCountApiThread( UInt64 value )
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		maxDispatchCountApiThread = 0xFFFFFFFF;
	else
		maxDispatchCountApiThread = (UInt32)value;
}
void OmmConsumerActiveConfig::setMaxDispatchCountUserThread( UInt64 value )
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		maxDispatchCountUserThread = 0xFFFFFFFF;
	else
		maxDispatchCountUserThread = (UInt32)value;
}

ChannelConfig::ChannelConfig()
{
}

ChannelConfig::ChannelConfig( RsslConnectionTypes type ) :
 name(),
 interfaceName( DEFAULT_INTERFACE_NAME ),
 xmlTraceFileName( DEFAULT_XML_TRACE_FILE_NAME ),
 compressionType( DEFAULT_COMPRESSION_TYPE ),
 compressionThreshold( DEFAULT_COMPRESSION_THRESHOLD ),
 connectionType( type ),
 connectionPingTimeout( DEFAULT_CONNECTION_PINGTIMEOUT ),
 guaranteedOutputBuffers( DEFAULT_GUARANTEED_OUTPUT_BUFFERS ),
 reconnectAttemptLimit( DEFAULT_RECONNECT_ATTEMPT_LIMIT ),
 reconnectMinDelay( DEFAULT_RECONNECT_MIN_DELAY ),
 reconnectMaxDelay( DEFAULT_RECONNECT_MAX_DELAY ),
 xmlTraceMaxFileSize( DEFAULT_XML_TRACE_MAX_FILE_SIZE ),
 xmlTraceToFile( DEFAULT_XML_TRACE_TO_FILE ),
 xmlTraceToStdout( DEFAULT_XML_TRACE_TO_STDOUT ),
 xmlTraceToMultipleFiles( DEFAULT_XML_TRACE_TO_MULTIPLE_FILE ),
 xmlTraceWrite( DEFAULT_XML_TRACE_WRITE ),
 xmlTraceRead( DEFAULT_XML_TRACE_READ ),
 msgKeyInUpdates( DEFAULT_MSGKEYINUPDATES )
{
}

void ChannelConfig::clear()
{
	name.clear();
	interfaceName = DEFAULT_INTERFACE_NAME;
	xmlTraceFileName = DEFAULT_XML_TRACE_FILE_NAME;
	compressionType = DEFAULT_COMPRESSION_TYPE;
	compressionThreshold = DEFAULT_COMPRESSION_THRESHOLD;
	connectionPingTimeout = DEFAULT_CONNECTION_PINGTIMEOUT;
	guaranteedOutputBuffers = DEFAULT_GUARANTEED_OUTPUT_BUFFERS;
	reconnectAttemptLimit = DEFAULT_RECONNECT_ATTEMPT_LIMIT;
	reconnectMinDelay = DEFAULT_RECONNECT_MIN_DELAY;
	reconnectMaxDelay = DEFAULT_RECONNECT_MAX_DELAY;
	xmlTraceMaxFileSize = DEFAULT_XML_TRACE_MAX_FILE_SIZE;
	xmlTraceToFile = DEFAULT_XML_TRACE_TO_FILE;
	xmlTraceToStdout = DEFAULT_XML_TRACE_TO_STDOUT;
	xmlTraceToMultipleFiles = DEFAULT_XML_TRACE_TO_MULTIPLE_FILE;
	xmlTraceWrite = DEFAULT_XML_TRACE_WRITE;
	xmlTraceRead = DEFAULT_XML_TRACE_READ;
	msgKeyInUpdates = DEFAULT_MSGKEYINUPDATES;
}

ChannelConfig::~ChannelConfig()
{
}

void ChannelConfig::setGuaranteedOutputBuffers(UInt64 value)
{
	if ( value <= 0 ) {}
	else if ( value > 0xFFFFFFFF )
		guaranteedOutputBuffers = 0xFFFFFFFF;
	else
		guaranteedOutputBuffers = (UInt32)value;
}

SocketChannelConfig::SocketChannelConfig() :
 ChannelConfig( RSSL_CONN_TYPE_SOCKET ),
 hostName( DEFAULT_HOST_NAME ),
 serviceName( DEFAULT_SERVICE_NAME ),
 tcpNodelay( DEFAULT_TCP_NODELAY )
{
}

SocketChannelConfig::~SocketChannelConfig()
{
}

void SocketChannelConfig::clear()
{
	ChannelConfig::clear();

	hostName = DEFAULT_HOST_NAME;
	serviceName = DEFAULT_SERVICE_NAME;
	tcpNodelay = DEFAULT_TCP_NODELAY;
}

ChannelConfig::ChannelType SocketChannelConfig::getType() const
{
	return ChannelConfig::SocketChannelEnum;
}

ReliableMcastChannelConfig::ReliableMcastChannelConfig() :
 ChannelConfig( RSSL_CONN_TYPE_RELIABLE_MCAST )
{
}

ReliableMcastChannelConfig::~ReliableMcastChannelConfig()
{
}

void ReliableMcastChannelConfig::clear()
{
	ChannelConfig::clear();
}

ChannelConfig::ChannelType ReliableMcastChannelConfig::getType() const
{
	return ChannelConfig::ReliableMcastChannelEnum;
}

EncryptedChannelConfig::EncryptedChannelConfig() :
 ChannelConfig( RSSL_CONN_TYPE_ENCRYPTED )
{
}

EncryptedChannelConfig::~EncryptedChannelConfig()
{
}

void EncryptedChannelConfig::clear()
{
	ChannelConfig::clear();
	
	hostName = DEFAULT_HOST_NAME;
	serviceName = DEFAULT_SERVICE_NAME;
	tcpNodelay = DEFAULT_TCP_NODELAY;
}

ChannelConfig::ChannelType EncryptedChannelConfig::getType() const
{
	return ChannelConfig::EncryptedChannelEnum;
}

HttpChannelConfig::HttpChannelConfig() :
 ChannelConfig( RSSL_CONN_TYPE_HTTP )
{
}

HttpChannelConfig::~HttpChannelConfig()
{
}

void HttpChannelConfig::clear()
{
	ChannelConfig::clear();

	hostName = DEFAULT_HOST_NAME;
	serviceName = DEFAULT_SERVICE_NAME;
	tcpNodelay = DEFAULT_TCP_NODELAY;
}

ChannelConfig::ChannelType HttpChannelConfig::getType() const
{
	return ChannelConfig::HttpChannelEnum;
}
