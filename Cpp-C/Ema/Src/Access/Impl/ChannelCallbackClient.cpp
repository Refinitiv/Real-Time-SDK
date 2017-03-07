/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ChannelCallbackClient.h"
#include "LoginCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "DictionaryCallbackClient.h"
#include "OmmBaseImpl.h"
#include "OmmConsumerErrorClient.h"
#include "StreamId.h"
#include "../EmaVersion.h"
#include "ExceptionTranslator.h"

#include <new>

using namespace thomsonreuters::ema::access;

const EmaString ChannelCallbackClient::_clientName( "ChannelCallbackClient" );

Channel* Channel::create( OmmBaseImpl& ommBaseImpl, const EmaString& name , RsslReactor* pRsslReactor )
{
	Channel* pChannel = 0;

	try
	{
		pChannel = new Channel( name, pRsslReactor );
	}
	catch ( std::bad_alloc ) {}

	if ( !pChannel )
	{
		const char* temp = "Failed to create Channel.";
		if ( OmmLoggerClient::ErrorEnum >= ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommBaseImpl.getOmmLoggerClient().log( "Channel", OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
	}

	return pChannel;
}

void Channel::destroy( Channel*& pChannel )
{
	if ( pChannel )
	{
		delete pChannel;
		pChannel = 0;
	}
}

Channel::Channel( const EmaString& name, RsslReactor* pRsslReactor ) :
	_name( name ),
	_pRsslReactor( pRsslReactor ),
	_toString(),
	_rsslSocket( 0 ),
	_pRsslChannel( 0 ),
	_state( ChannelDownEnum ),
	_pLogin( 0 ),
	_pDictionary( 0 ),
	_directoryList(),
	_toStringSet( false )
{
}

Channel::~Channel()
{
}

Channel& Channel::setRsslChannel( RsslReactorChannel* pRsslChannel )
{
	_pRsslChannel = pRsslChannel;
	return *this;
}

Channel& Channel::setRsslSocket( RsslSocket rsslSocket )
{
	_toStringSet = false;
	_rsslSocket = rsslSocket;
	return *this;
}

Channel& Channel::setChannelState( ChannelState state )
{
	_toStringSet = false;
	_state = state;
	return *this;
}

Channel::ChannelState Channel::getChannelState() const
{
	return _state;
}

RsslSocket Channel::getRsslSocket() const
{
	return _rsslSocket;
}

RsslReactorChannel* Channel::getRsslChannel() const
{
	return _pRsslChannel;
}

RsslReactor* Channel::getRsslReactor() const
{
	return _pRsslReactor;
}

const EmaString& Channel::getName() const
{
	return _name;
}

Channel& Channel::setLogin( Login* pLogin )
{
	_pLogin = pLogin;
	return *this;
}

Channel& Channel::addDirectory( Directory* pDirectory )
{
	_toStringSet = false;
	_directoryList.push_back( pDirectory );
	return *this;
}

Channel& Channel::removeDirectory( Directory* pDirectory )
{
	_toStringSet = false;
	_directoryList.removeValue( pDirectory );
	return *this;
}

Login* Channel::getLogin() const
{
	return _pLogin;
}

Dictionary* Channel::getDictionary() const
{
	return _pDictionary;
}

Channel& Channel::setDictionary( Dictionary* pDictionary )
{
	_pDictionary = pDictionary;
	return *this;
}

const EmaString& Channel::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;
		_toString.set( "\tRsslReactorChannel name " ).append( _name ).append( CR )
		.append( "\tRsslReactor " ).append( ptrToStringAsHex( _pRsslReactor ) ).append( CR )
		.append( "\tRsslReactorChannel " ).append( ptrToStringAsHex( _pRsslChannel ) ).append( CR )
		.append( "\tRsslSocket " ).append( ( UInt64 )_rsslSocket );

		if ( ! _directoryList.empty() )
		{
			_toString.append( CR ).append( "\tDirectory " );
			for ( UInt32 idx = 0; idx < _directoryList.size(); ++idx )
			{
 				_toString.append( _directoryList[idx]->getName() ).append( " " );
			}
			_toString.append( CR );
		}
	}
	return _toString;
}

bool Channel::operator==( const Channel& other )
{
	if ( this == &other ) return true;

	if ( _pRsslChannel != other._pRsslChannel ) return false;

	if ( _rsslSocket != other._rsslSocket ) return false;

	if ( _pRsslReactor != other._pRsslReactor ) return false;

	if ( _name != other._name ) return false;

	return true;
}

ChannelList::ChannelList()
{
}

ChannelList::~ChannelList()
{
	Channel* channel = _list.pop_back();
	while ( channel )
	{
		Channel::destroy( channel );
		channel = _list.pop_back();
	}
}

void ChannelList::addChannel( Channel* pChannel )
{
	_list.push_back( pChannel );
}

void ChannelList::removeChannel( Channel* pChannel )
{
	_list.remove( pChannel );
}

UInt32 ChannelList::size() const
{
	return _list.size();
}

RsslReactorChannel* ChannelList::operator[]( UInt32 idx )
{
	UInt32 index = 0;
	Channel* channel = _list.front();

	while ( channel )
	{
		if ( index == idx )
		{
			return channel->getRsslChannel();
		}
		else
		{
			index++;
			channel = channel->next();
		}
	}

	return 0;
}

Channel* ChannelList::getChannel( const EmaString& name ) const
{
	Channel* channel = _list.front();
	while ( channel )
	{
		if ( channel->getName() == name )
		{
			return channel;
		}
		else
			channel = channel->next();
	}

	return 0;
}

Channel* ChannelList::getChannel( const RsslReactorChannel* pRsslChannel ) const
{
	Channel* channel = _list.front();
	while ( channel )
	{
		if ( channel->getRsslChannel() == pRsslChannel )
		{
			return channel;
		}
		else
			channel = channel->next();
	}

	return 0;
}

Channel* ChannelList::getChannel( RsslSocket rsslsocket ) const
{
	Channel* channel = _list.front();
	while ( channel )
	{
		if ( channel->getRsslSocket() == rsslsocket )
		{
			return channel;
		}
		else
			channel = channel->next();
	}

	return 0;
}

Channel* ChannelList::front() const
{
	return _list.front();
}

void ChannelCallbackClient::closeChannels()
{
	UInt32 size = _channelList.size();

	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		_ommBaseImpl.closeChannel( _channelList[idx] );
	}
}

ChannelCallbackClient::ChannelCallbackClient( OmmBaseImpl& ommBaseImpl, RsslReactor* pRsslReactor ) :
	_channelList(),
	_ommBaseImpl( ommBaseImpl ),
	_pRsslReactor( pRsslReactor ),
	_bInitialChannelReadyEventReceived( false )
{
	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created ChannelCallbackClient" );
	}
}

ChannelCallbackClient::~ChannelCallbackClient()
{
	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed ChannelCallbackClient" );
	}
}

ChannelCallbackClient* ChannelCallbackClient::create( OmmBaseImpl& ommBaseImpl, RsslReactor* pRsslReactor )
{
	ChannelCallbackClient* pClient = 0;

	try
	{
		pClient = new ChannelCallbackClient( ommBaseImpl, pRsslReactor );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
	{
		const char* temp = "Failed to create ChannelCallbackClient";
		if ( OmmLoggerClient::ErrorEnum >= ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
	}

	return pClient;
}

void ChannelCallbackClient::destroy( ChannelCallbackClient*& pClient )
{
	if ( pClient )
	{
		delete pClient;
		pClient = 0;
	}
}

void ChannelCallbackClient::channelParametersToString(ActiveConfig& activeConfig, ChannelConfig* pChannelCfg, EmaString& strChannelParams )
{
	bool bValidChType = true;
	EmaString strConnectionType;
	EmaString cfgParameters;
	EmaString compType;
	switch ( pChannelCfg->compressionType )
	{
	case RSSL_COMP_ZLIB :
		compType.set( "ZLib" );
		break;
	case RSSL_COMP_LZ4 :
		compType.set( "LZ4" );
		break;
	case RSSL_COMP_NONE :
		compType.set( "None" );
		break;
	default :
		compType.set( "Unknown Compression Type" );
		break;
	}
	switch ( pChannelCfg->connectionType )
	{
	case RSSL_CONN_TYPE_SOCKET:
	{
		SocketChannelConfig* pTempChannelCfg = static_cast<SocketChannelConfig*>( pChannelCfg );
		strConnectionType = "RSSL_CONN_TYPE_SOCKET";
		cfgParameters.append( "hostName " ).append( pTempChannelCfg->hostName ).append( CR )
		.append( "port " ).append( pTempChannelCfg->serviceName ).append( CR )
		.append( "CompressionType " ).append( compType ).append( CR )
		.append( "tcpNodelay " ).append( ( pTempChannelCfg->tcpNodelay ? "true" : "false" ) ).append( CR );
		break;
	}
	case RSSL_CONN_TYPE_HTTP:
	{
		HttpChannelConfig* pTempChannelCfg = static_cast<HttpChannelConfig*>( pChannelCfg );
		strConnectionType = "RSSL_CONN_TYPE_HTTP";
		cfgParameters.append( "hostName " ).append( pTempChannelCfg->hostName ).append( CR )
		.append( "port " ).append( pTempChannelCfg->serviceName ).append( CR )
		.append( "CompressionType " ).append( compType ).append( CR )
		.append( "tcpNodelay " ).append( ( pTempChannelCfg->tcpNodelay ? "true" : "false" ) ).append( CR )
		.append( "ObjectName " ).append( pTempChannelCfg->objectName ).append( CR );
		break;
	}
	case RSSL_CONN_TYPE_ENCRYPTED:
	{
		EncryptedChannelConfig* pTempChannelCfg = static_cast<EncryptedChannelConfig*>( pChannelCfg );
		strConnectionType = "RSSL_CONN_TYPE_ENCRYPTED";
		cfgParameters.append( "hostName " ).append( pTempChannelCfg->hostName ).append( CR )
		.append( "port " ).append( pTempChannelCfg->serviceName ).append( CR )
		.append( "CompressionType " ).append( compType ).append( CR )
		.append( "tcpNodelay " ).append( ( pTempChannelCfg->tcpNodelay ? "true" : "false" ) ).append( CR )
		.append( "ObjectName " ).append( pTempChannelCfg->objectName ).append( CR );
		break;
	}
	case RSSL_CONN_TYPE_RELIABLE_MCAST:
	{
		ReliableMcastChannelConfig* pTempChannelCfg = static_cast<ReliableMcastChannelConfig*>( pChannelCfg );
		strConnectionType = "RSSL_CONN_TYPE_MCAST";
		cfgParameters.append( "RecvAddress " ).append( pTempChannelCfg->recvAddress ).append( CR )
		.append( "RecvPort " ).append( pTempChannelCfg->recvServiceName ).append( CR )
		.append( "SendAddress " ).append( pTempChannelCfg->sendAddress ).append( CR )
		.append( "SendPort " ).append( pTempChannelCfg->sendServiceName ).append( CR )
		.append( "UnicastPort " ).append( pTempChannelCfg->unicastServiceName ).append( CR )
		.append( "HsmInterface " ).append( pTempChannelCfg->hsmInterface ).append( CR )
		.append( "HsmMultAddress " ).append( pTempChannelCfg->hsmMultAddress ).append( CR )
		.append( "HsmPort " ).append( pTempChannelCfg->hsmPort ).append( CR )
		.append( "HsmInterval " ).append( pTempChannelCfg->hsmInterval ).append( CR )
		.append( "tcpControlPort " ).append( pTempChannelCfg->tcpControlPort ).append( CR )
		.append( "DisconnectOnGap " ).append( ( pTempChannelCfg->disconnectOnGap ? "true" : "false" ) ).append( CR )
		.append( "PacketTTL " ).append( pTempChannelCfg->packetTTL ).append( CR )
		.append( "ndata " ).append( pTempChannelCfg->ndata ).append( CR )
		.append( "nmissing " ).append( pTempChannelCfg->nmissing ).append( CR )
		.append( "nrreq " ).append( pTempChannelCfg->nrreq ).append( CR )
		.append( "tdata " ).append( pTempChannelCfg->tdata ).append( CR )
		.append( "trreq " ).append( pTempChannelCfg->trreq ).append( CR )
		.append( "twait " ).append( pTempChannelCfg->twait ).append( CR )
		.append( "tbchold " ).append( pTempChannelCfg->tbchold ).append( CR )
		.append( "tpphold " ).append( pTempChannelCfg->tpphold ).append( CR )
		.append( "pktPoolLimitHigh " ).append( pTempChannelCfg->pktPoolLimitHigh ).append( CR )
		.append( "pktPoolLimitLow " ).append( pTempChannelCfg->pktPoolLimitLow ).append( CR )
		.append( "userQLimit " ).append( pTempChannelCfg->userQLimit ).append( CR );

		break;
	}
	default:
	{
		strConnectionType = "Invalid ChannelType: ";
		strConnectionType.append( pChannelCfg->connectionType )
		.append( " " );
		bValidChType = false;
		break;
	}
	}

	strChannelParams.append( strConnectionType ).append( CR )
	.append( "Channel name " ).append( pChannelCfg->name ).append( CR )
	.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR );

	if ( bValidChType )
	{
		strChannelParams.append( "RsslReactor " ).append( ptrToStringAsHex( _pRsslReactor ) ).append( CR )
		.append( "InterfaceName " ).append( pChannelCfg->interfaceName ).append( CR )
		.append( cfgParameters )
		.append( "reconnectAttemptLimit " ).append( activeConfig.reconnectAttemptLimit ).append( CR )
		.append( "reconnectMinDelay " ).append( activeConfig.reconnectMinDelay ).append( " msec" ).append( CR )
		.append( "reconnectMaxDelay " ).append( activeConfig.reconnectMaxDelay ).append( " msec" ).append( CR )
		.append( "connectionPingTimeout " ).append( pChannelCfg->connectionPingTimeout ).append( " msec" ).append( CR );
	}
}

void ChannelCallbackClient::initialize( RsslRDMLoginRequest* loginRequest, RsslRDMDirectoryRequest* dirRequest )
{
	RsslReactorChannelRole role;
	_ommBaseImpl.setRsslReactorChannelRole( role );

	EmaString componentVersionInfo( COMPONENT_NAME );
	componentVersionInfo.append( NEWVERSTRING );
	componentVersionInfo.append( EMA_COMPONENT_VER_PLATFORM );
	componentVersionInfo.append( COMPILE_BITS_STR );
	componentVersionInfo.append( emaComponentLinkType );
	componentVersionInfo.append( "(" );
	componentVersionInfo.append( emaComponentBldtype );
	componentVersionInfo.append( ")" );

	ActiveConfig& activeConfig = _ommBaseImpl.getActiveConfig();
	EmaVector< ChannelConfig* >& activeConfigChannelSet = activeConfig.configChannelSet;
	UInt32 channelCfgSetLastIndex = activeConfigChannelSet.size() - 1;

	RsslReactorConnectInfo* reactorConnectInfo = 0;

	try
	{
		reactorConnectInfo = new RsslReactorConnectInfo[activeConfigChannelSet.size()];
	}
	catch ( std::bad_alloc ) {}

	if ( !reactorConnectInfo )
	{
		const char* temp = "Failed to allocate memory in ChannelCallbackClient::initialize()";
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
		return;
	}

	RsslReactorConnectOptions connectOpt;
	rsslClearReactorConnectOptions( &connectOpt );
	connectOpt.connectionCount = activeConfigChannelSet.size();
	connectOpt.reactorConnectionList = reactorConnectInfo;
	connectOpt.reconnectAttemptLimit = activeConfig.reconnectAttemptLimit;
	connectOpt.reconnectMinDelay = activeConfig.reconnectMinDelay;
	connectOpt.reconnectMaxDelay = activeConfig.reconnectMaxDelay;
	connectOpt.initializationTimeout = 5;

	EmaString channelParams;
	EmaString temp( "Attempt to connect using " );
	if ( connectOpt.connectionCount > 1 )
		temp.set( "Attempt to connect using the following list" );
	UInt32 supportedConnectionTypeChannelCount = 0;
	EmaString errorStrUnsupportedConnectionType( "Unknown connection type. Passed in type is" );

	EmaString channelNames;

	for ( UInt32 i = 0; i < connectOpt.connectionCount; ++i )
	{
		rsslClearReactorConnectInfo( &reactorConnectInfo[i] );

		if ( activeConfigChannelSet[i]->connectionType == RSSL_CONN_TYPE_SOCKET   ||
		     activeConfigChannelSet[i]->connectionType == RSSL_CONN_TYPE_HTTP ||
		     activeConfigChannelSet[i]->connectionType == RSSL_CONN_TYPE_ENCRYPTED ||
		     activeConfigChannelSet[i]->connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST )
		{
			Channel* pChannel = Channel::create( _ommBaseImpl, activeConfigChannelSet[i]->name, _pRsslReactor );

			reactorConnectInfo[i].rsslConnectOptions.userSpecPtr = ( void* )pChannel;
			activeConfigChannelSet[i]->pChannel = pChannel;

			reactorConnectInfo[i].rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
			reactorConnectInfo[i].rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
			reactorConnectInfo[i].rsslConnectOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;
			reactorConnectInfo[i].rsslConnectOptions.connectionType = activeConfigChannelSet[i]->connectionType;
			reactorConnectInfo[i].rsslConnectOptions.pingTimeout = activeConfigChannelSet[i]->connectionPingTimeout;
			reactorConnectInfo[i].rsslConnectOptions.guaranteedOutputBuffers = activeConfigChannelSet[i]->guaranteedOutputBuffers;
			reactorConnectInfo[i].rsslConnectOptions.sysRecvBufSize = activeConfigChannelSet[i]->sysRecvBufSize;
			reactorConnectInfo[i].rsslConnectOptions.sysSendBufSize = activeConfigChannelSet[i]->sysSendBufSize;
			reactorConnectInfo[i].rsslConnectOptions.numInputBuffers = activeConfigChannelSet[i]->numInputBuffers;
			reactorConnectInfo[i].rsslConnectOptions.componentVersion = ( char* ) componentVersionInfo.c_str();

			switch ( reactorConnectInfo[i].rsslConnectOptions.connectionType )
			{
			case RSSL_CONN_TYPE_SOCKET:
			{
				reactorConnectInfo[i].rsslConnectOptions.compressionType = activeConfigChannelSet[i]->compressionType;
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.address = ( char* )static_cast<SocketChannelConfig*>( activeConfigChannelSet[i] )->hostName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.serviceName = ( char* )static_cast<SocketChannelConfig*>( activeConfigChannelSet[i] )->serviceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.tcpOpts.tcp_nodelay = static_cast<SocketChannelConfig*>( activeConfigChannelSet[i] )->tcpNodelay;
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.interfaceName = ( char* )activeConfigChannelSet[i]->interfaceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.unicastServiceName = ( char* ) "";
				break;
			}
			case RSSL_CONN_TYPE_ENCRYPTED:
			{
				reactorConnectInfo[i].rsslConnectOptions.compressionType = activeConfigChannelSet[i]->compressionType;
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.address = ( char* )static_cast<EncryptedChannelConfig*>( activeConfigChannelSet[i] )->hostName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.serviceName = ( char* )static_cast<EncryptedChannelConfig*>( activeConfigChannelSet[i] )->serviceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.tcpOpts.tcp_nodelay = static_cast<EncryptedChannelConfig*>( activeConfigChannelSet[i] )->tcpNodelay;
				reactorConnectInfo[i].rsslConnectOptions.objectName = ( char* ) static_cast<EncryptedChannelConfig*>( activeConfigChannelSet[i] )->objectName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.interfaceName = ( char* )activeConfigChannelSet[i]->interfaceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.unicastServiceName = ( char* ) "";
				break;
			}
			case RSSL_CONN_TYPE_HTTP:
			{
				reactorConnectInfo[i].rsslConnectOptions.compressionType = activeConfigChannelSet[i]->compressionType;
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.address = ( char* )static_cast<HttpChannelConfig*>( activeConfigChannelSet[i] )->hostName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.serviceName = ( char* )static_cast<HttpChannelConfig*>( activeConfigChannelSet[i] )->serviceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.tcpOpts.tcp_nodelay = static_cast<HttpChannelConfig*>( activeConfigChannelSet[i] )->tcpNodelay;
				reactorConnectInfo[i].rsslConnectOptions.objectName = ( char* ) static_cast<HttpChannelConfig*>( activeConfigChannelSet[i] )->objectName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.interfaceName = ( char* )activeConfigChannelSet[i]->interfaceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.unified.unicastServiceName = ( char* ) "";

				break;
			}
			case RSSL_CONN_TYPE_RELIABLE_MCAST:
			{
				ReliableMcastChannelConfig* relMcastCfg = static_cast<ReliableMcastChannelConfig*>( activeConfigChannelSet[i] );
				if ( activeConfigChannelSet[i]->interfaceName.empty() )
					reactorConnectInfo[i].rsslConnectOptions.connectionInfo.segmented.interfaceName = 0;
				else
					reactorConnectInfo[i].rsslConnectOptions.connectionInfo.segmented.interfaceName = ( char* )activeConfigChannelSet[i]->interfaceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.segmented.recvAddress = ( char* ) relMcastCfg->recvAddress.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.segmented.recvServiceName = ( char* ) relMcastCfg->recvServiceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.segmented.unicastServiceName = ( char* ) relMcastCfg->unicastServiceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.segmented.sendAddress = ( char* ) relMcastCfg->sendAddress.c_str();
				reactorConnectInfo[i].rsslConnectOptions.connectionInfo.segmented.sendServiceName = ( char* ) relMcastCfg->sendServiceName.c_str();
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.disconnectOnGaps = relMcastCfg->disconnectOnGap;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.hsmInterface = ( char* ) relMcastCfg->hsmInterface.c_str();
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.hsmMultAddress = ( char* ) relMcastCfg->hsmMultAddress.c_str();
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.hsmPort = ( char* ) relMcastCfg->hsmPort.c_str();
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.hsmInterval =  relMcastCfg->hsmInterval;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.packetTTL =  relMcastCfg->packetTTL;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.tcpControlPort = ( char* ) relMcastCfg->tcpControlPort.c_str();
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.ndata =  relMcastCfg->ndata;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.nmissing =  relMcastCfg->nmissing;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.nrreq =  relMcastCfg->nrreq;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.tdata =  relMcastCfg->tdata;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.trreq =  relMcastCfg->trreq;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.twait =  relMcastCfg->twait;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.tpphold =  relMcastCfg->tpphold;
				reactorConnectInfo[i].rsslConnectOptions.multicastOpts.tbchold =  relMcastCfg->tbchold;
				break;
			}
			default :
				break;
			}

			pChannel->setChannelState( Channel::ChannelDownEnum );
			_channelList.addChannel( pChannel );
			supportedConnectionTypeChannelCount++;

			channelNames += pChannel->getName();

			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				channelParams.clear();
				channelParametersToString(activeConfig, activeConfigChannelSet[i], channelParams );
				temp.append( CR ).append( i + 1 ).append( "] " ).append( channelParams );
				if ( i == ( connectOpt.connectionCount - 1 ) )
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
		}
		else
		{
			reactorConnectInfo[i].rsslConnectOptions.userSpecPtr = 0;
			errorStrUnsupportedConnectionType.append( activeConfigChannelSet[i]->connectionType )
			.append( " for " )
			.append( activeConfigChannelSet[i]->name );
			if ( i < channelCfgSetLastIndex )
				errorStrUnsupportedConnectionType.append( ", " );
		}
	}

	if ( supportedConnectionTypeChannelCount > 0 )
	{
		connectOpt.rsslConnectOptions.userSpecPtr = ( void* ) activeConfigChannelSet[0]->pChannel;

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );

		if ( RSSL_RET_SUCCESS != rsslReactorConnect( _pRsslReactor, &connectOpt, ( RsslReactorChannelRole* )&role, &rsslErrorInfo ) )
		{

			EmaString temp( "Failed to add RsslChannel(s) to RsslReactor. Channel name(s) " );
			temp.append( channelNames ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( _pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ( UInt64 )rsslErrorInfo.rsslError.channel ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );

			for ( UInt32 i = 0; i < connectOpt.connectionCount; ++i )
			{
				Channel* pChannel = ( Channel* )reactorConnectInfo[i].rsslConnectOptions.userSpecPtr;
				if ( pChannel )
				{
					_channelList.removeChannel( pChannel );
					Channel::destroy( pChannel );
				}
			}

			delete [] reactorConnectInfo;
			throwIueException( temp );

			return;
		}

		_ommBaseImpl.setState( OmmBaseImpl::RsslChannelDownEnum );

		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully created a Reactor and Channel(s)" );
			temp.append( CR )
			.append( "Channel name(s) " ).append( channelNames ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( supportedConnectionTypeChannelCount < connectOpt.connectionCount )
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, errorStrUnsupportedConnectionType );
	}
	else
	{
		delete [] reactorConnectInfo;
		throwIueException( errorStrUnsupportedConnectionType );
	}
	delete [] reactorConnectInfo;
}

void ChannelCallbackClient::removeChannel( RsslReactorChannel* pRsslReactorChannel )
{
	if ( pRsslReactorChannel )
		_channelList.removeChannel( ( Channel* )pRsslReactorChannel->userSpecPtr );
}

RsslReactorCallbackRet ChannelCallbackClient::processCallback( RsslReactor* pRsslReactor,
    RsslReactorChannel* pRsslReactorChannel,
    RsslReactorChannelEvent* pEvent )
{
	Channel* pChannel = ( Channel* )( pEvent->pReactorChannel->userSpecPtr );
	ChannelConfig* pChannelConfig = _ommBaseImpl.getActiveConfig().findChannelConfig( pChannel );

	if ( !pChannelConfig )
	{
		EmaString temp( "Failed to find channel config for channel " );
		temp.append( pChannel->getName() )
		.append( " that received event type: " )
		.append( pEvent->channelEventType ).append( CR )
		.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
		.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR );

		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		_ommBaseImpl.closeChannel( pRsslReactorChannel );

		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pEvent->channelEventType )
	{
	case RSSL_RC_CET_CHANNEL_OPENED :
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelOpened on channel " );
			temp.append( pChannel->getName() ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}
		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_CHANNEL_UP:
	{
		RsslReactorChannelInfo channelInfo;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		RsslRet retChanInfo;
		retChanInfo = rsslReactorGetChannelInfo( pRsslReactorChannel, &channelInfo, &rsslErrorInfo );
		EmaString componentInfo( "Connected component version: " );
		for ( unsigned int i = 0; i < channelInfo.rsslChannelInfo.componentInfoCount; ++i )
		{
			componentInfo.append( channelInfo.rsslChannelInfo.componentInfo[i]->componentVersion.data );
			if ( i < ( channelInfo.rsslChannelInfo.componentInfoCount - 1 ) )
				componentInfo.append( ", " );
		}
#ifdef WIN32
		int sendBfrSize = 65535;

		if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_SYSTEM_WRITE_BUFFERS, &sendBfrSize, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Failed to set send buffer size on channel " );
				temp.append( pChannel->getName() ).append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error text " ).append( rsslErrorInfo.rsslError.text );

				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			_ommBaseImpl.closeChannel( pRsslReactorChannel );

			return RSSL_RC_CRET_SUCCESS;
		}

		int rcvBfrSize = 65535;
		if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_SYSTEM_READ_BUFFERS, &rcvBfrSize, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Failed to set recv buffer size on channel " );
				temp.append( pChannel->getName() ).append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error text " ).append( rsslErrorInfo.rsslError.text );

				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			_ommBaseImpl.closeChannel( pRsslReactorChannel );

			return RSSL_RC_CRET_SUCCESS;
		}
#endif
		if ( pChannelConfig->connectionType != RSSL_CONN_TYPE_RELIABLE_MCAST && rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_COMPRESSION_THRESHOLD, &pChannelConfig->compressionThreshold, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Failed to set compression threshold on channel " );
				temp.append( pChannel->getName() ).append( CR )
				.append( "Consumer Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error text " ).append( rsslErrorInfo.rsslError.text );

				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			_ommBaseImpl.closeChannel( pRsslReactorChannel );

			return RSSL_RC_CRET_SUCCESS;
		}

		pChannel->setRsslChannel( pRsslReactorChannel )
		.setRsslSocket( pRsslReactorChannel->socketId )
		.setChannelState( Channel::ChannelUpEnum );
		if ( OmmLoggerClient::SuccessEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelUp event on channel " );
			temp.append( pChannel->getName() ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
			if ( channelInfo.rsslChannelInfo.componentInfoCount > 0 )
				temp.append( CR ).append( componentInfo );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::SuccessEnum, temp );
		}

		// set the high water mark if configured
		if ( pChannelConfig->highWaterMark > 0 )
		{
			if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_HIGH_WATER_MARK, &pChannelConfig->highWaterMark, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Failed to set the high water mark on channel " );
					temp.append( pChannel->getName() ).append( CR )
						.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
						.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
						.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
						.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
						.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
						.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
						.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
				}
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "high water mark set on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
		}

		ActiveConfig& activeConfig = _ommBaseImpl.getActiveConfig();
		if ( activeConfig.xmlTraceToFile || activeConfig.xmlTraceToStdout )
		{
			EmaString fileName( activeConfig.xmlTraceFileName );
			fileName.append( "_" );

			RsslTraceOptions traceOptions;
			rsslClearTraceOptions( &traceOptions );

			traceOptions.traceMsgFileName = ( char* )fileName.c_str();

			if ( activeConfig.xmlTraceToFile )
				traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE;

			if ( activeConfig.xmlTraceToStdout )
				traceOptions.traceFlags |= RSSL_TRACE_TO_STDOUT;

			if ( activeConfig.xmlTraceToMultipleFiles )
				traceOptions.traceFlags |= RSSL_TRACE_TO_MULTIPLE_FILES;

			if ( activeConfig.xmlTraceWrite )
				traceOptions.traceFlags |= RSSL_TRACE_WRITE;

			if ( activeConfig.xmlTraceRead )
				traceOptions.traceFlags |= RSSL_TRACE_READ;

			if ( activeConfig.xmlTracePing )
				traceOptions.traceFlags |= RSSL_TRACE_PING;

			if ( activeConfig.xmlTraceHex )
				traceOptions.traceFlags |= RSSL_TRACE_HEX;

			traceOptions.traceMsgMaxFileSize = activeConfig.xmlTraceMaxFileSize;

			if ( RSSL_RET_SUCCESS != rsslReactorChannelIoctl( pRsslReactorChannel, ( RsslIoctlCodes )RSSL_TRACE, ( void* )&traceOptions, &rsslErrorInfo ) )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Failed to enable Xml Tracing on channel " );
					temp.append( pChannel->getName() ).append( CR )
					.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
					.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
					.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
					.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
				}
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Xml Tracing enabled on channel " );
				temp.append( pChannel->getName() ).append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
		}

		_ommBaseImpl.addSocket( pRsslReactorChannel->socketId );
		_ommBaseImpl.setState( OmmBaseImpl::RsslChannelUpEnum );

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_CHANNEL_READY:
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelReady event on channel " );
			temp.append( pChannel->getName() ).append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		pChannel->setChannelState( Channel::ChannelReadyEnum );

		if ( _bInitialChannelReadyEventReceived )
		{
			_ommBaseImpl.processChannelEvent( pEvent );
			_ommBaseImpl.getLoginCallbackClient().processChannelEvent( pEvent );
		}
		else
			_bInitialChannelReadyEventReceived = true;
	
		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_FD_CHANGE:
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received FD Change event on channel " );
			temp.append( pChannel->getName() ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		pChannel->setRsslChannel( pRsslReactorChannel )
		.setRsslSocket( pRsslReactorChannel->socketId );

		_ommBaseImpl.removeSocket( pRsslReactorChannel->oldSocketId );
		_ommBaseImpl.addSocket( pRsslReactorChannel->socketId );
		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_CHANNEL_DOWN:
	{
		pChannel->setChannelState( Channel::ChannelDownEnum );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelDown event on channel " );
			temp.append( pChannel->getName() ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pEvent->pError->rsslError.rsslErrorId ? pEvent->pError->rsslError.text : "" );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		_ommBaseImpl.setState( OmmBaseImpl::RsslChannelDownEnum );

		_ommBaseImpl.processChannelEvent( pEvent );

		_ommBaseImpl.getLoginCallbackClient().processChannelEvent( pEvent );

		_ommBaseImpl.closeChannel( pRsslReactorChannel );

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
	{
		if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelDownReconnecting event on channel " );
			temp.append( pChannel->getName() ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pEvent->pError->rsslError.rsslErrorId ? pEvent->pError->rsslError.text : "" );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace() );
		}

		if ( pRsslReactorChannel->socketId != REACTOR_INVALID_SOCKET )
			_ommBaseImpl.removeSocket( pRsslReactorChannel->socketId );

		pChannel->setRsslSocket( 0 )
		.setChannelState( Channel::ChannelDownReconnectingEnum );

		_ommBaseImpl.processChannelEvent( pEvent );

		_ommBaseImpl.getLoginCallbackClient().processChannelEvent( pEvent );

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_WARNING:
	{
		if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received Channel warning event on channel " );
			temp.append( pChannel->getName() ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pEvent->pError->rsslError.rsslErrorId ? pEvent->pError->rsslError.text : "" );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace() );
		}
		return RSSL_RC_CRET_SUCCESS;
	}
	default:
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received unknown channel event type " );
			temp.append( pEvent->channelEventType ).append( CR )
			.append( "channel " ).append( pChannel->getName() ).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pEvent->pError->rsslError.rsslErrorId ? pEvent->pError->rsslError.text : "" );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}
		return RSSL_RC_CRET_FAILURE;
	}
	}
}

const ChannelList& ChannelCallbackClient::getChannelList()
{
	return _channelList;
}
