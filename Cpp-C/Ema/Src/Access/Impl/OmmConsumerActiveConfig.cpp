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
 dictionaryRequestTimeOut( DEFAULT_DICTIONARY_REQUEST_TIMEOUT ),
 userDispatch( DEFAULT_USER_DISPATCH ),
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
	clearChannelSet();
}

void OmmConsumerActiveConfig::clearChannelSet()
{
	if(configChannelSet.size() == 0)
		return;
   for(unsigned int i = 0; i < configChannelSet.size(); ++i)
	{
		if ( configChannelSet[i] != NULL )
		{
			delete configChannelSet[i];
			configChannelSet[i] = NULL;
		}
	}		
	configChannelSet.clear();
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

	clearChannelSet();
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

void OmmConsumerActiveConfig::setDictionaryRequestTimeOut( UInt64 value )
{
	if ( value > 0xFFFFFFFF )
		dictionaryRequestTimeOut = 0xFFFFFFFF;
	else if ( value > 0 )
		dictionaryRequestTimeOut = (UInt32) value;
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
ChannelConfig* OmmConsumerActiveConfig::findChannelConfig(const Channel* pChannel)
{
	ChannelConfig* retChannelCfg = 0;
	for(unsigned int i = 0; i < configChannelSet.size(); ++i)
	{
		if(configChannelSet[i]->pChannel ==  pChannel) {
			retChannelCfg = configChannelSet[i];
			break;
		}	
	}
	return retChannelCfg;
}

bool OmmConsumerActiveConfig::findChannelConfig(EmaVector< ChannelConfig* > &cfgChannelSet, const EmaString& channelName, unsigned int &pos)
{
	bool channelFound = false;
	if(cfgChannelSet.size() > 0)
	{	
		for(pos = 0; pos < cfgChannelSet.size(); ++pos)
		{
			if(cfgChannelSet[pos]->name ==  channelName) {
				channelFound = true;
				break;
			}	
		}
	}
	return channelFound;
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
 numInputBuffers( DEFAULT_NUM_INPUT_BUFFERS ),
 sysSendBufSize( DEFAULT_SYS_SEND_BUFFER_SIZE ),
 sysRecvBufSize( DEFAULT_SYS_RECEIVE_BUFFER_SIZE ),
 reconnectAttemptLimit( DEFAULT_RECONNECT_ATTEMPT_LIMIT ),
 reconnectMinDelay( DEFAULT_RECONNECT_MIN_DELAY ),
 reconnectMaxDelay( DEFAULT_RECONNECT_MAX_DELAY ),
 xmlTraceMaxFileSize( DEFAULT_XML_TRACE_MAX_FILE_SIZE ),
 xmlTraceToFile( DEFAULT_XML_TRACE_TO_FILE ),
 xmlTraceToStdout( DEFAULT_XML_TRACE_TO_STDOUT ),
 xmlTraceToMultipleFiles( DEFAULT_XML_TRACE_TO_MULTIPLE_FILE ),
 xmlTraceWrite( DEFAULT_XML_TRACE_WRITE ),
 xmlTraceRead( DEFAULT_XML_TRACE_READ ),
 msgKeyInUpdates( DEFAULT_MSGKEYINUPDATES ),
 pChannel(0)
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
	numInputBuffers = DEFAULT_NUM_INPUT_BUFFERS;
	sysSendBufSize = DEFAULT_SYS_SEND_BUFFER_SIZE;
	sysRecvBufSize = DEFAULT_SYS_RECEIVE_BUFFER_SIZE;
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
	pChannel = 0;
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

void ChannelConfig::setNumInputBuffers(UInt64 value)
{
	if ( value == 0 ) {} 
	else
	{
		numInputBuffers = value > 0xFFFFFFFF ? 0xFFFFFFFF : (UInt32)value;
	}
}

void ChannelConfig::setReconnectAttemptLimit(Int64 value)
{
	if ( value >= 0 )
	{
		reconnectAttemptLimit = value > 0x7FFFFFFF ? 0x7FFFFFFF : (Int32)value;
	}
}
void ChannelConfig::setReconnectMinDelay(Int64 value)
{
	if( value > 0 )
	{
		reconnectMinDelay = value > 0x7FFFFFFF ? 0x7FFFFFFF : (Int32)value;
	}
}
void ChannelConfig::setReconnectMaxDelay(Int64 value)
{
	if( value > 0 )
	{
		reconnectMaxDelay = value > 0x7FFFFFFF ? 0x7FFFFFFF : (Int32)value;
	}
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
 ChannelConfig( RSSL_CONN_TYPE_RELIABLE_MCAST ),
 recvAddress( DEFAULT_CONS_MCAST_CFGSTRING ),
 recvServiceName( DEFAULT_CONS_MCAST_CFGSTRING ),
 unicastServiceName( DEFAULT_CONS_MCAST_CFGSTRING ),
 sendAddress( DEFAULT_CONS_MCAST_CFGSTRING ),
 sendServiceName( DEFAULT_CONS_MCAST_CFGSTRING ),
 hsmInterface( DEFAULT_CONS_MCAST_CFGSTRING ),
 tcpControlPort( DEFAULT_CONS_MCAST_CFGSTRING ),
 hsmMultAddress( DEFAULT_CONS_MCAST_CFGSTRING ),
 hsmPort( DEFAULT_CONS_MCAST_CFGSTRING ),
 hsmInterval( 0  ),
 packetTTL( DEFAULT_PACKET_TTL ),
 ndata( DEFAULT_NDATA ),
 nmissing( DEFAULT_NMISSING ),
 nrreq( DEFAULT_NREQ ),
 pktPoolLimitHigh( DEFAULT_PKT_POOLLIMIT_HIGH ),
 pktPoolLimitLow( DEFAULT_PKT_POOLLIMIT_LOW ),
 tdata( DEFAULT_TDATA ),
 trreq( DEFAULT_TRREQ ),
 twait( DEFAULT_TWAIT ),
 tbchold( DEFAULT_TBCHOLD ),
 tpphold( DEFAULT_TPPHOLD ),
 userQLimit( DEFAULT_USER_QLIMIT ),
 disconnectOnGap( RSSL_FALSE )
{
}

ReliableMcastChannelConfig::~ReliableMcastChannelConfig()
{
}
void ReliableMcastChannelConfig::setPacketTTL(UInt64 value)
{
	if ( value > 255 )
		packetTTL= 255;
	else
		packetTTL= (RsslUInt8) value;
}

void ReliableMcastChannelConfig::setHsmInterval(UInt64 value)
{
	if( value > 0 )
		hsmInterval = value > 0xFFFF ? 0xFFFF : (UInt16)value;
}

void ReliableMcastChannelConfig::setNdata(UInt64 value)
{
	if ( value > 0xFFFFF )
		ndata = 0xFFFFF;
	else
		ndata = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setNmissing(UInt64 value)
{
	if ( value > 0xFFFF )
		nmissing = 0xFFFF;
	else
		nmissing = (RsslUInt16) value;
}

void ReliableMcastChannelConfig::setNrreq(UInt64 value)
{
	if ( value > 0xFFFFF )
		nrreq = 0xFFFFF;
	else
		nrreq = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setTdata(UInt64 value)
{
	if ( value > 0xFFFFF )
		tdata = 0xFFFFF;
	else
		tdata = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setTrreq(UInt64 value)
{
	if ( value > 0xFFFFF )
		trreq = 0xFFFFF;
	else
		trreq = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setPktPoolLimitHigh(UInt64 value)
{
	if ( value > 0xFFFFF )
		pktPoolLimitHigh = 0xFFFFF;
	else
		pktPoolLimitHigh = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setPktPoolLimitLow(UInt64 value)
{
    if ( value > 0xFFFFF )
        pktPoolLimitLow = 0xFFFFF;
    else
        pktPoolLimitLow = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setTwait(UInt64 value)
{
    if ( value > 0xFFFFF )
        twait = 0xFFFFF;
    else
        twait = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setTbchold(UInt64 value)
{
    if ( value > 0xFFFFF )
        tbchold = 0xFFFFF;
    else
        tbchold = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setTpphold(UInt64 value)
{
    if ( value > 0xFFFFF )
        tpphold = 0xFFFFF;
    else
        tpphold = (RsslUInt32) value;
}

void ReliableMcastChannelConfig::setUserQLimit(UInt64 value)
{
    if ( value > DEFAULT_USER_QLIMIT )
        userQLimit = DEFAULT_USER_QLIMIT;
    else
        userQLimit = (RsslUInt32) value;
}


void ReliableMcastChannelConfig::clear()
{
	ChannelConfig::clear();
	recvAddress = DEFAULT_CONS_MCAST_CFGSTRING;
	recvServiceName = DEFAULT_CONS_MCAST_CFGSTRING;
	unicastServiceName = DEFAULT_CONS_MCAST_CFGSTRING;
	sendAddress = DEFAULT_CONS_MCAST_CFGSTRING;
	sendServiceName = DEFAULT_CONS_MCAST_CFGSTRING;
	hsmInterface = DEFAULT_CONS_MCAST_CFGSTRING;
	tcpControlPort = DEFAULT_CONS_MCAST_CFGSTRING;
	hsmMultAddress = DEFAULT_CONS_MCAST_CFGSTRING;
	hsmPort = DEFAULT_CONS_MCAST_CFGSTRING;
	hsmInterval = 0;
	packetTTL	= DEFAULT_PACKET_TTL;
    ndata = DEFAULT_NDATA;
    nmissing = DEFAULT_NMISSING;
    nrreq = DEFAULT_NREQ;
    pktPoolLimitHigh = DEFAULT_PKT_POOLLIMIT_HIGH;
    pktPoolLimitLow = DEFAULT_PKT_POOLLIMIT_LOW;
    tdata = DEFAULT_TDATA;
    trreq = DEFAULT_TRREQ;
    twait = DEFAULT_TWAIT;
    tbchold = DEFAULT_TBCHOLD;
    tpphold = DEFAULT_TPPHOLD;
    userQLimit = DEFAULT_USER_QLIMIT;
	disconnectOnGap = RSSL_FALSE;
}

ChannelConfig::ChannelType ReliableMcastChannelConfig::getType() const
{
	return ChannelConfig::ReliableMcastChannelEnum;
}

EncryptedChannelConfig::EncryptedChannelConfig() :
 ChannelConfig( RSSL_CONN_TYPE_ENCRYPTED ), 
 objectName(DEFAULT_OBJECT_NAME)
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
	objectName = DEFAULT_OBJECT_NAME;
}

ChannelConfig::ChannelType EncryptedChannelConfig::getType() const
{
	return ChannelConfig::EncryptedChannelEnum;
}

HttpChannelConfig::HttpChannelConfig() :
 ChannelConfig( RSSL_CONN_TYPE_HTTP ),
 objectName(DEFAULT_OBJECT_NAME)
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
	objectName = DEFAULT_OBJECT_NAME;
}

ChannelConfig::ChannelType HttpChannelConfig::getType() const
{
	return ChannelConfig::HttpChannelEnum;
}
