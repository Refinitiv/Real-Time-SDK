/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2019-2022,2024 LSEG. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ChannelCallbackClient.h"
#include "LoginCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "DictionaryCallbackClient.h"
#include "OmmBaseImpl.h"
#include "OmmConsumerErrorClient.h"
#include "StreamId.h"
#include "EmaVersion.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

#include <new>

#define EMA_INIT_NUMBER_OF_SOCKET 5

using namespace refinitiv::ema::access;

const EmaString ChannelCallbackClient::_clientName( "ChannelCallbackClient" );

Channel* Channel::create( OmmBaseImpl& ommBaseImpl, const EmaString& name , RsslReactor* pRsslReactor, ReactorChannelType reactorChannelType)
{
	try
	{
		return new Channel( name, pRsslReactor, reactorChannelType);
	}
	catch ( std::bad_alloc& )
	{
		const char* temp = "Failed to create Channel.";
		if (OmmLoggerClient::ErrorEnum >= ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			ommBaseImpl.getOmmLoggerClient().log("Channel", OmmLoggerClient::ErrorEnum, temp);

		throwMeeException(temp);
	}

	return NULL;
}

void Channel::destroy( Channel*& pChannel )
{
	if ( pChannel )
	{
		delete pChannel;
		pChannel = 0;
	}
}

Channel::Channel( const EmaString& name, RsslReactor* pRsslReactor, ReactorChannelType reactorChannelType) :
	_name( name ),
	_pRsslReactor( pRsslReactor ),
	_toString(),
	_pRsslChannel( 0 ),
	_state( ChannelDownEnum ),
	_pLogin( 0 ),
	_pDictionary( 0 ),
	_directoryList(),
	_toStringSet( false ),
	_reactorChannelType( reactorChannelType ),
	_pParentChannel( NULL ),
	_inOAuthCallback( false ),
	_addedToDeleteList( false )
{
	_pRsslSocketList = new EmaVector< RsslSocket >(EMA_INIT_NUMBER_OF_SOCKET);
}

Channel::~Channel()
{
	if (_pRsslSocketList)
	{
		delete _pRsslSocketList;
	}
}

Channel& Channel::setRsslChannel( RsslReactorChannel* pRsslChannel )
{
	_pRsslChannel = pRsslChannel;
	return *this;
}

Channel& Channel::clearRsslSocket()
{
	_pRsslSocketList->clear();
	return *this;
}

Channel& Channel::addRsslSocket( RsslSocket rsslSocket )
{
	_toStringSet = false;
	_pRsslSocketList->push_back(rsslSocket);
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

EmaVector<RsslSocket>& Channel::getRsslSocket() const
{
	return *_pRsslSocketList;
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

Channel::ReactorChannelType Channel::getReactorChannelType() const
{
	return _reactorChannelType;
}

void Channel::setParentChannel(Channel* channel)
{
	_pParentChannel = channel;
}

Channel* Channel::getParentChannel() const
{
	return _pParentChannel;
}

void Channel::setAddedToDeleteList(bool isAdded)
{
	_addedToDeleteList = isAdded;
}

bool Channel::getAddedToDeleteList() const
{
	return _addedToDeleteList;
}

const EmaString& Channel::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;
		_toString.set( "\tRsslReactorChannel name " ).append( _name ).append( CR )
		.append( "\tRsslReactor " ).append( ptrToStringAsHex( _pRsslReactor ) ).append( CR )
		.append( "\tRsslReactorChannel " ).append( ptrToStringAsHex( _pRsslChannel ) ).append( CR );

		if (_pRsslSocketList->size() == 1)
		{
			_toString.append("\tRsslSocket ").append((UInt64)getRsslSocket()[0]);
		}
		else
		{
			_toString.append("\tList of RsslSockets ");
			for (UInt32 index = 0; index < _pRsslSocketList->size(); index++)
			{
				_toString.append((UInt64)getRsslSocket()[index]).append(" ");
			}
		}

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

	if ( (getRsslSocket() == other.getRsslSocket()) == false) return false;

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

	channel = _deleteList.pop_back();
	while (channel)
	{
		Channel::destroy(channel);
		channel = _deleteList.pop_back();
	}
}

void ChannelList::addChannel( Channel* pChannel )
{
	_list.push_back( pChannel );
}

void ChannelList::removeChannel( Channel* pChannel )
{
	if (pChannel != NULL && pChannel->getAddedToDeleteList() == false)
	{
		_list.remove(pChannel);
		_deleteList.push_back(pChannel);
		pChannel->setAddedToDeleteList(true);
	}
}

void ChannelList::removeAllChannel()
{
	Channel* pChannel;

	while ((pChannel = _list.pop_front()) != NULL)
	{
		Channel::destroy(pChannel);
	}

	while ((pChannel = _deleteList.pop_front()) != NULL)
	{
		Channel::destroy(pChannel);
	}
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
		if (channel->getParentChannel() == NULL)
		{
			if (index == idx)
			{
				return channel->getRsslChannel();
			}
			else
			{
				index++;
				channel = channel->next();
			}
		}
		else
		{
			/* Warm standby channel get only one RsslReactorChannel and returns*/
			return channel->getParentChannel()->getRsslChannel();
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

Channel* ChannelList::front() const
{
	return _list.front();
}

void ChannelCallbackClient::closeChannels()
{
	UInt32 size = _channelList.size();
	EmaVector<RsslReactorChannel*> tempRemoveChannelList(size);
	RsslBool foundWSBChannel = RSSL_FALSE;

	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		RsslReactorChannel* pReactorChannel = _channelList[idx];

		if (tempRemoveChannelList.getPositionOf(pReactorChannel) == -1)
		{
			_ommBaseImpl.closeChannel(pReactorChannel);
			tempRemoveChannelList.push_back(pReactorChannel);
		}
		else
		{
			/* Found same RsslReactorChannel due to warm standby feature. */
			foundWSBChannel = RSSL_TRUE;
		}
	}

	/* EMA closes the reconnecting channel(if any) as well when the login timeout occurs. */
	if( _pReconnectingReactorChannel)
	{
		_ommBaseImpl.closeChannel( _pReconnectingReactorChannel );
	}

	if (foundWSBChannel)
	{
		_channelList.removeAllChannel();
	}
}

ChannelCallbackClient::ChannelCallbackClient( OmmBaseImpl& ommBaseImpl, RsslReactor* pRsslReactor ) :
	_channelList(),
	_ommBaseImpl( ommBaseImpl ),
	_pRsslReactor( pRsslReactor ),
	_bInitialChannelReadyEventReceived( false ),
	_pReconnectingReactorChannel( NULL )
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
	try
	{
		return new ChannelCallbackClient( ommBaseImpl, pRsslReactor );
	}
	catch ( std::bad_alloc& )
	{
		const char* temp = "Failed to create ChannelCallbackClient";
		if (OmmLoggerClient::ErrorEnum >= ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

		throwMeeException(temp);
	}

	return NULL;
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
		.append( "tcpNodelay " ).append( ( pTempChannelCfg->tcpNodelay ? "true" : "false" ) ).append( CR )
		.append("EnableSessionManagement ").append(pTempChannelCfg->enableSessionMgnt).append(CR);
		break;
	}
	case RSSL_CONN_TYPE_HTTP:
	case RSSL_CONN_TYPE_WEBSOCKET:
	{
		SocketChannelConfig* pTempChannelCfg = static_cast<SocketChannelConfig*>( pChannelCfg );
		strConnectionType = (pChannelCfg->connectionType == RSSL_CONN_TYPE_HTTP) ? "RSSL_CONN_TYPE_HTTP" : "RSSL_CONN_TYPE_WEBSOCKET";
		cfgParameters.append("hostName ").append(pTempChannelCfg->hostName).append(CR)
			.append("port ").append(pTempChannelCfg->serviceName).append(CR)
			.append("CompressionType ").append(compType).append(CR)
			.append("tcpNodelay ").append((pTempChannelCfg->tcpNodelay ? "true" : "false")).append(CR)
			.append("ObjectName ").append(pTempChannelCfg->objectName).append(CR)
			.append("ProxyHost ").append(pTempChannelCfg->proxyHostName).append(CR)
			.append("ProxyPort ").append(pTempChannelCfg->proxyPort).append(CR)
			.append("ProxyConnectionTimeout ").append(pTempChannelCfg->proxyConnectionTimeout).append(CR)
			.append("EnableSessionManagement ").append(pTempChannelCfg->enableSessionMgnt).append(CR);

		if (pChannelCfg->connectionType == RSSL_CONN_TYPE_WEBSOCKET)
		{
			cfgParameters
				.append("WsMaxMsgSize ").append(pTempChannelCfg->wsMaxMsgSize).append(CR)
				.append("WsProtocols ").append(pTempChannelCfg->wsProtocols).append(CR);
		}

		break;
	}
	case RSSL_CONN_TYPE_ENCRYPTED:
	{
		/* TODO: Update for multiple encrypted types */
		SocketChannelConfig* pTempChannelCfg = static_cast<SocketChannelConfig*>( pChannelCfg );
		strConnectionType = "RSSL_CONN_TYPE_ENCRYPTED";
		cfgParameters.append("hostName ").append(pTempChannelCfg->hostName).append(CR)
			.append("port ").append(pTempChannelCfg->serviceName).append(CR)
			.append("CompressionType ").append(compType).append(CR)
			.append("tcpNodelay ").append((pTempChannelCfg->tcpNodelay ? "true" : "false")).append(CR)
			.append("ObjectName ").append(pTempChannelCfg->objectName).append(CR)
			.append("ProxyHost ").append(pTempChannelCfg->proxyHostName).append(CR)
			.append("ProxyPort ").append(pTempChannelCfg->proxyPort).append(CR)
			.append("ProxyConnectionTimeout ").append(pTempChannelCfg->proxyConnectionTimeout).append(CR)
			.append("SecurityProtocol ").append(pTempChannelCfg->securityProtocol).append(CR)
			.append("EnableSessionManagement ").append(pTempChannelCfg->enableSessionMgnt).append(CR)
			.append("Location ").append(pTempChannelCfg->location).append(CR)
			.append("ServiceDiscoveryRetryCount ").append(pTempChannelCfg->serviceDiscoveryRetryCount).append(CR);

		if (pTempChannelCfg->encryptedConnectionType == RSSL_CONN_TYPE_WEBSOCKET)
		{
			cfgParameters
				.append("WsMaxMsgSize ").append(pTempChannelCfg->wsMaxMsgSize).append(CR)
				.append("WsProtocols ").append(pTempChannelCfg->wsProtocols).append(CR);
		}

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
		.append( "connectionPingTimeout " ).append( pChannelCfg->connectionPingTimeout ).append( " msec" ).append( CR )
		.append( "initializationTimeout " ).append( pChannelCfg->initializationTimeout ).append(" sec").append(CR);
	}
}

Channel* ChannelCallbackClient::channelConfigToReactorConnectInfo(ChannelConfig* activeChannelConfig, RsslReactorConnectInfo* reactorConnectInfo, 
	EmaString& componentVersionInfo)
{
	rsslClearReactorConnectInfo(reactorConnectInfo);

	if (activeChannelConfig->connectionType == RSSL_CONN_TYPE_SOCKET ||
		activeChannelConfig->connectionType == RSSL_CONN_TYPE_HTTP ||
		activeChannelConfig->connectionType == RSSL_CONN_TYPE_ENCRYPTED ||
		activeChannelConfig->connectionType == RSSL_CONN_TYPE_RELIABLE_MCAST ||
		activeChannelConfig->connectionType == RSSL_CONN_TYPE_WEBSOCKET)
	{
		Channel* pChannel =  Channel::create(_ommBaseImpl, activeChannelConfig->name, _pRsslReactor);

		reactorConnectInfo->rsslConnectOptions.userSpecPtr = (void*)pChannel;
		activeChannelConfig->pChannel = pChannel;

		reactorConnectInfo->rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		reactorConnectInfo->rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
		reactorConnectInfo->rsslConnectOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;
		reactorConnectInfo->rsslConnectOptions.connectionType = activeChannelConfig->connectionType;
		reactorConnectInfo->rsslConnectOptions.pingTimeout = activeChannelConfig->connectionPingTimeout / 1000;
		reactorConnectInfo->rsslConnectOptions.guaranteedOutputBuffers = activeChannelConfig->guaranteedOutputBuffers;
		reactorConnectInfo->rsslConnectOptions.sysRecvBufSize = activeChannelConfig->sysRecvBufSize;
		reactorConnectInfo->rsslConnectOptions.sysSendBufSize = activeChannelConfig->sysSendBufSize;
		reactorConnectInfo->rsslConnectOptions.numInputBuffers = activeChannelConfig->numInputBuffers;
		reactorConnectInfo->rsslConnectOptions.componentVersion = (char*)componentVersionInfo.c_str();
		reactorConnectInfo->initializationTimeout = activeChannelConfig->initializationTimeout;

		switch (reactorConnectInfo->rsslConnectOptions.connectionType)
		{
		case RSSL_CONN_TYPE_ENCRYPTED:
		{
			if (static_cast<SocketChannelConfig*>(activeChannelConfig)->encryptedConnectionType != RSSL_CONN_TYPE_INIT)
				reactorConnectInfo->rsslConnectOptions.encryptionOpts.encryptedProtocol = static_cast<SocketChannelConfig*>(activeChannelConfig)->encryptedConnectionType;

			if (RSSL_CONN_TYPE_WEBSOCKET == reactorConnectInfo->rsslConnectOptions.encryptionOpts.encryptedProtocol)
			{
				reactorConnectInfo->rsslConnectOptions.wsOpts.maxMsgSize = static_cast<SocketChannelConfig*>(activeChannelConfig)->wsMaxMsgSize;
				reactorConnectInfo->rsslConnectOptions.wsOpts.protocols = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->wsProtocols.c_str());
			}

			reactorConnectInfo->rsslConnectOptions.encryptionOpts.encryptionProtocolFlags = static_cast<SocketChannelConfig*>(activeChannelConfig)->securityProtocol;
			reactorConnectInfo->rsslConnectOptions.encryptionOpts.openSSLCAStore = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->sslCAStore.c_str());
			reactorConnectInfo->enableSessionManagement = static_cast<SocketChannelConfig*>(activeChannelConfig)->enableSessionMgnt;
			reactorConnectInfo->location.length = static_cast<SocketChannelConfig*>(activeChannelConfig)->location.length();
			reactorConnectInfo->location.data = (char*)static_cast<SocketChannelConfig*>(activeChannelConfig)->location.c_str();
			reactorConnectInfo->serviceDiscoveryRetryCount = static_cast<SocketChannelConfig*>(activeChannelConfig)->serviceDiscoveryRetryCount;
			// Fall through to HTTP connection options
		}
		case RSSL_CONN_TYPE_SOCKET:
		case RSSL_CONN_TYPE_HTTP:
		case RSSL_CONN_TYPE_WEBSOCKET:
		{
			reactorConnectInfo->rsslConnectOptions.compressionType = activeChannelConfig->compressionType;
			reactorConnectInfo->rsslConnectOptions.connectionInfo.unified.address = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->hostName.c_str());
			reactorConnectInfo->rsslConnectOptions.connectionInfo.unified.serviceName = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->serviceName.c_str());
			reactorConnectInfo->rsslConnectOptions.tcpOpts.tcp_nodelay = static_cast<SocketChannelConfig*>(activeChannelConfig)->tcpNodelay;
			reactorConnectInfo->rsslConnectOptions.objectName = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->objectName.c_str());
			reactorConnectInfo->rsslConnectOptions.connectionInfo.unified.interfaceName = (char*)(activeChannelConfig->interfaceName.c_str());
			reactorConnectInfo->rsslConnectOptions.connectionInfo.unified.unicastServiceName = (char*) "";
			reactorConnectInfo->rsslConnectOptions.proxyOpts.proxyHostName = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->proxyHostName.c_str());
			reactorConnectInfo->rsslConnectOptions.proxyOpts.proxyPort = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->proxyPort.c_str());
			reactorConnectInfo->rsslConnectOptions.proxyOpts.proxyUserName = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->proxyUserName.c_str());
			reactorConnectInfo->rsslConnectOptions.proxyOpts.proxyPasswd = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->proxyPasswd.c_str());
			reactorConnectInfo->rsslConnectOptions.proxyOpts.proxyDomain = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->proxyDomain.c_str());
			reactorConnectInfo->rsslConnectOptions.proxyOpts.proxyConnectionTimeout = static_cast<SocketChannelConfig*>(activeChannelConfig)->proxyConnectionTimeout;

			if (RSSL_CONN_TYPE_WEBSOCKET == reactorConnectInfo->rsslConnectOptions.connectionType)
			{
				reactorConnectInfo->rsslConnectOptions.wsOpts.maxMsgSize = static_cast<SocketChannelConfig*>(activeChannelConfig)->wsMaxMsgSize;
				reactorConnectInfo->rsslConnectOptions.wsOpts.protocols = (char*)(static_cast<SocketChannelConfig*>(activeChannelConfig)->wsProtocols.c_str());
			}

			reactorConnectInfo->enableSessionManagement = static_cast<SocketChannelConfig*>(activeChannelConfig)->enableSessionMgnt;

			break;
		}
		case RSSL_CONN_TYPE_RELIABLE_MCAST:
		{
			ReliableMcastChannelConfig* relMcastCfg = static_cast<ReliableMcastChannelConfig*>(activeChannelConfig);
			if (activeChannelConfig->interfaceName.empty())
				reactorConnectInfo->rsslConnectOptions.connectionInfo.segmented.interfaceName = 0;
			else
				reactorConnectInfo->rsslConnectOptions.connectionInfo.segmented.interfaceName = (char*)activeChannelConfig->interfaceName.c_str();
			reactorConnectInfo->rsslConnectOptions.connectionInfo.segmented.recvAddress = (char*)relMcastCfg->recvAddress.c_str();
			reactorConnectInfo->rsslConnectOptions.connectionInfo.segmented.recvServiceName = (char*)relMcastCfg->recvServiceName.c_str();
			reactorConnectInfo->rsslConnectOptions.connectionInfo.segmented.unicastServiceName = (char*)relMcastCfg->unicastServiceName.c_str();
			reactorConnectInfo->rsslConnectOptions.connectionInfo.segmented.sendAddress = (char*)relMcastCfg->sendAddress.c_str();
			reactorConnectInfo->rsslConnectOptions.connectionInfo.segmented.sendServiceName = (char*)relMcastCfg->sendServiceName.c_str();
			reactorConnectInfo->rsslConnectOptions.multicastOpts.disconnectOnGaps = relMcastCfg->disconnectOnGap;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.hsmInterface = (char*)relMcastCfg->hsmInterface.c_str();
			reactorConnectInfo->rsslConnectOptions.multicastOpts.hsmMultAddress = (char*)relMcastCfg->hsmMultAddress.c_str();
			reactorConnectInfo->rsslConnectOptions.multicastOpts.hsmPort = (char*)relMcastCfg->hsmPort.c_str();
			reactorConnectInfo->rsslConnectOptions.multicastOpts.hsmInterval = relMcastCfg->hsmInterval;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.packetTTL = relMcastCfg->packetTTL;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.tcpControlPort = (char*)relMcastCfg->tcpControlPort.c_str();
			reactorConnectInfo->rsslConnectOptions.multicastOpts.ndata = relMcastCfg->ndata;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.nmissing = relMcastCfg->nmissing;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.nrreq = relMcastCfg->nrreq;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.tdata = relMcastCfg->tdata;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.trreq = relMcastCfg->trreq;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.twait = relMcastCfg->twait;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.tpphold = relMcastCfg->tpphold;
			reactorConnectInfo->rsslConnectOptions.multicastOpts.tbchold = relMcastCfg->tbchold;
			break;
		}
		default:
			break;
		}

		pChannel->setChannelState(Channel::ChannelDownEnum);
		return pChannel;
	}
	else
	{
		return NULL; /* Indicate invalid channel type. */
	}
}

void ChannelCallbackClient::initialize()
{
	RsslReactorChannelRole role;
	_ommBaseImpl.setRsslReactorChannelRole(role);

	EmaString componentVersionInfo(COMPONENTNAME);
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

	if (activeConfigChannelSet.size() > 0)
	{
		try
		{
			reactorConnectInfo = new RsslReactorConnectInfo[activeConfigChannelSet.size()];
		}
		catch (std::bad_alloc&)
		{
			const char* temp = "Failed to allocate memory in ChannelCallbackClient::initialize()";
			if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

			throwMeeException(temp);
			return;
		}
	}

	EmaVector<WarmStandbyChannelConfig*>& warmStandbyChannelSet = activeConfig.configWarmStandbySet;
	RsslReactorWarmStandbyGroup *warmStandbyChannelGroup = NULL;

	if (warmStandbyChannelSet.size() > 0)
	{
		try
		{
			warmStandbyChannelGroup = new RsslReactorWarmStandbyGroup[warmStandbyChannelSet.size()];
		}
		catch (std::bad_alloc&)
		{
			const char* temp = "Failed to allocate memory in ChannelCallbackClient::initialize() for warm standby group";
			if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

			throwMeeException(temp);
			return;
		}
	}

	RsslReactorConnectOptions connectOpt;
	rsslClearReactorConnectOptions( &connectOpt );
	connectOpt.connectionCount = activeConfigChannelSet.size();
	connectOpt.reactorConnectionList = reactorConnectInfo;
	connectOpt.reconnectAttemptLimit = activeConfig.reconnectAttemptLimit;
	connectOpt.reconnectMinDelay = activeConfig.reconnectMinDelay;
	connectOpt.reconnectMaxDelay = activeConfig.reconnectMaxDelay;

	if (warmStandbyChannelGroup)
	{
		connectOpt.warmStandbyGroupCount = warmStandbyChannelSet.size();
		connectOpt.reactorWarmStandbyGroupList = warmStandbyChannelGroup;
	}

	EmaString channelParams;
	EmaString temp( "Attempt to connect using " );
	if ( connectOpt.connectionCount > 1 )
		temp.set( "Attempt to connect using the following list" );
	UInt32 supportedConnectionTypeChannelCount = 0;
	EmaString errorStrUnsupportedConnectionType( "Unknown connection type. Passed in type is" );

	EmaString channelNames;
	Channel* pChannel = NULL;

	for ( UInt32 i = 0; i < connectOpt.connectionCount; ++i )
	{
		pChannel = channelConfigToReactorConnectInfo(activeConfigChannelSet[i], &reactorConnectInfo[i], componentVersionInfo);

		if(pChannel)
		{
			if (role.ommConsumerRole.pLoginRequest == NULL)
				reactorConnectInfo[i].loginReqIndex = _ommBaseImpl.getLoginArrayIndex(activeConfigChannelSet[i]->name);

			if (reactorConnectInfo[i].enableSessionManagement == RSSL_TRUE)
				reactorConnectInfo[i].oAuthCredentialIndex = _ommBaseImpl.getOAuthArrayIndex(activeConfigChannelSet[i]->name);

			_channelList.addChannel(pChannel);

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

	WarmStandbyChannelConfig* pWarmStandbyChannelConfig = NULL;
	Channel* pWarmStandbyChannel = NULL;
	EmaVector<RsslBuffer*> freeServiceList(10);

	for (UInt32 j = 0; j < connectOpt.warmStandbyGroupCount; ++j)
	{
		rsslClearReactorWarmStandbyGroup(&warmStandbyChannelGroup[j]);

		pWarmStandbyChannelConfig = warmStandbyChannelSet[j];

		if (pWarmStandbyChannelConfig->startingActiveServer)
		{
			if (pWarmStandbyChannel == NULL)
			{
				pWarmStandbyChannel = Channel::create(_ommBaseImpl, pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name,
					_pRsslReactor, Channel::WARM_STANDBY);
				_channelList.addChannel(pWarmStandbyChannel);
			}

			pChannel = channelConfigToReactorConnectInfo(pWarmStandbyChannelConfig->startingActiveServer->channelConfig,
				&warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo, componentVersionInfo);

			if (pChannel)
			{

				if (role.ommConsumerRole.pLoginRequest == NULL)
					warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo.loginReqIndex = _ommBaseImpl.getLoginArrayIndex(pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name);

				if (warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo.enableSessionManagement == RSSL_TRUE)
					warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo.oAuthCredentialIndex = _ommBaseImpl.getOAuthArrayIndex(pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name);

				pChannel->setParentChannel(pWarmStandbyChannel);
				
				_channelList.addChannel(pChannel);
				supportedConnectionTypeChannelCount++;
				channelNames += pChannel->getName();
			}
			else
			{
				warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo.rsslConnectOptions.userSpecPtr = 0;
				errorStrUnsupportedConnectionType.append(pWarmStandbyChannelConfig->startingActiveServer->channelConfig->connectionType)
					.append(" for ")
					.append(pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name);
					errorStrUnsupportedConnectionType.append(", ");
			}

			if (pWarmStandbyChannelConfig->startingActiveServer->perServiceNameSet.size() > 0)
			{
				warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameCount = pWarmStandbyChannelConfig->startingActiveServer->perServiceNameSet.size();

				warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameList = (RsslBuffer*)malloc(sizeof(RsslBuffer) * warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameCount);

				if (warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameList)
				{
					freeServiceList.push_back(warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameList);

					for (RsslUInt32 index = 0; index < warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameCount; index++)
					{
						warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameList[index].data = const_cast<char*>(pWarmStandbyChannelConfig->startingActiveServer->perServiceNameSet[index]->c_str());
						warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameList[index].length = pWarmStandbyChannelConfig->startingActiveServer->perServiceNameSet[index]->length();
					}
				}
			}
		}

		try
		{
			if (pWarmStandbyChannelConfig->standbyServerSet.size() > 0)
			{
				warmStandbyChannelGroup[j].standbyServerList = new RsslReactorWarmStandbyServerInfo[pWarmStandbyChannelConfig->standbyServerSet.size()];
				warmStandbyChannelGroup[j].standbyServerCount = pWarmStandbyChannelConfig->standbyServerSet.size();
			}
		}
		catch (std::bad_alloc&)
		{
			const char* temp = "Failed to allocate memory in ChannelCallbackClient::initialize() for warm standby group";
			if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

			throwMeeException(temp);
			return;
		}

		RsslReactorWarmStandbyServerInfo* warmStandbyServerInfo = NULL;

		for (UInt32 k = 0; k < warmStandbyChannelGroup[j].standbyServerCount; ++k)
		{
			warmStandbyServerInfo = &warmStandbyChannelGroup[j].standbyServerList[k];
			rsslClearReactorWarmStandbyServerInfo(warmStandbyServerInfo);

			if (pWarmStandbyChannel == NULL)
			{
				pWarmStandbyChannel = Channel::create(_ommBaseImpl, pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig->name,
					_pRsslReactor, Channel::WARM_STANDBY);
				_channelList.addChannel(pWarmStandbyChannel);
			}

			pChannel = channelConfigToReactorConnectInfo(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig,
				&warmStandbyServerInfo->reactorConnectInfo, componentVersionInfo);

			if (pChannel)
			{

				if (role.ommConsumerRole.pLoginRequest == NULL)
					warmStandbyServerInfo->reactorConnectInfo.loginReqIndex = _ommBaseImpl.getLoginArrayIndex(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig->name);

				if (warmStandbyServerInfo->reactorConnectInfo.enableSessionManagement == RSSL_TRUE)
					warmStandbyServerInfo->reactorConnectInfo.oAuthCredentialIndex = _ommBaseImpl.getOAuthArrayIndex(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig->name);

				pChannel->setParentChannel(pWarmStandbyChannel);

				_channelList.addChannel(pChannel);
				supportedConnectionTypeChannelCount++;
				channelNames += pChannel->getName();
			}
			else
			{
				warmStandbyServerInfo->reactorConnectInfo.rsslConnectOptions.userSpecPtr = 0;
				errorStrUnsupportedConnectionType.append(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig->connectionType)
					.append(" for ")
					.append(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig->name);
				errorStrUnsupportedConnectionType.append(", ");
			}

			if (pWarmStandbyChannelConfig->standbyServerSet[k]->perServiceNameSet.size() > 0)
			{
				warmStandbyServerInfo->perServiceBasedOptions.serviceNameCount = pWarmStandbyChannelConfig->standbyServerSet[k]->perServiceNameSet.size();

				warmStandbyServerInfo->perServiceBasedOptions.serviceNameList = (RsslBuffer*)malloc(sizeof(RsslBuffer) * warmStandbyServerInfo->perServiceBasedOptions.serviceNameCount);

				if (warmStandbyServerInfo->perServiceBasedOptions.serviceNameList)
				{
					freeServiceList.push_back(warmStandbyServerInfo->perServiceBasedOptions.serviceNameList);

					for (RsslUInt32 index = 0; index < warmStandbyServerInfo->perServiceBasedOptions.serviceNameCount; index++)
					{
						warmStandbyServerInfo->perServiceBasedOptions.serviceNameList[index].data = const_cast<char*>(pWarmStandbyChannelConfig->standbyServerSet[k]->perServiceNameSet[index]->c_str());
						warmStandbyServerInfo->perServiceBasedOptions.serviceNameList[index].length = pWarmStandbyChannelConfig->standbyServerSet[k]->perServiceNameSet[index]->length();
					}
				}
			}
		}

		warmStandbyChannelGroup[j].warmStandbyMode = (RsslReactorWarmStandbyMode)pWarmStandbyChannelConfig->warmStandbyMode;
	}

	if ( supportedConnectionTypeChannelCount > 0 )
	{
		if(warmStandbyChannelGroup == NULL)
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
					// The ChannelList::removeChannel() method also destroys the Channel object as well.
					_channelList.removeChannel( pChannel );
				}
			}

			_channelList.removeAllChannel();

			for (RsslUInt32 index = 0; index < freeServiceList.size(); index++)
			{
				free(freeServiceList[index]);
			}

			freeServiceList.clear();

			delete [] reactorConnectInfo;

			for (UInt32 j = 0; j < connectOpt.warmStandbyGroupCount; ++j)
			{
				if (warmStandbyChannelGroup[j].standbyServerCount > 0)
				{
					delete[] warmStandbyChannelGroup[j].standbyServerList;
				}
			}

			delete [] warmStandbyChannelGroup;
			throwIueException( temp, rsslErrorInfo.rsslError.rsslErrorId );

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
		for (RsslUInt32 index = 0; index < freeServiceList.size(); index++)
		{
			free(freeServiceList[index]);
		}

		freeServiceList.clear();

		delete [] reactorConnectInfo;

		for (UInt32 j = 0; j < connectOpt.warmStandbyGroupCount; ++j)
		{
			if (warmStandbyChannelGroup[j].standbyServerCount > 0)
			{
				delete[] warmStandbyChannelGroup[j].standbyServerList;
			}
		}

		delete [] warmStandbyChannelGroup;

		throwIueException( errorStrUnsupportedConnectionType, OmmInvalidUsageException::InvalidOperationEnum );

		return;
	}

	for (RsslUInt32 index = 0; index < freeServiceList.size(); index++)
	{
		free(freeServiceList[index]);
	}

	freeServiceList.clear();

	delete [] reactorConnectInfo;

	for (UInt32 j = 0; j < connectOpt.warmStandbyGroupCount; ++j)
	{
		if (warmStandbyChannelGroup[j].standbyServerCount > 0)
		{
			delete[] warmStandbyChannelGroup[j].standbyServerList;
		}
	}

	delete [] warmStandbyChannelGroup;
}

void ChannelCallbackClient::removeChannel( RsslReactorChannel* pRsslReactorChannel )
{
	if (pRsslReactorChannel)
	{
		if (_pReconnectingReactorChannel == pRsslReactorChannel)
			_pReconnectingReactorChannel = NULL; // Unset the ReactorChannel of the RECONNECTING event as the channel is being destroyed.

		_ommBaseImpl.getLoginCallbackClient().removeChannel(pRsslReactorChannel);
		_channelList.removeChannel((Channel*)pRsslReactorChannel->userSpecPtr);
	}
}

RsslReactorCallbackRet ChannelCallbackClient::processCallback( RsslReactor* pRsslReactor,
    RsslReactorChannel* pRsslReactorChannel,
    RsslReactorChannelEvent* pEvent )
{
	ChannelConfig* pChannelConfig = NULL;
	Channel* pChannel = ( Channel* )( pEvent->pReactorChannel->userSpecPtr );
	
	if (pChannel != NULL)
	{
		pChannelConfig = _ommBaseImpl.getActiveConfig().findChannelConfig(pChannel);

		if (pChannel->getParentChannel())
		{
			pChannel = pChannel->getParentChannel();
		}
	}

	if ( !pChannelConfig )
	{
		EmaString temp( "Failed to find channel config for channel " );

		temp.append( "that received event type: " )
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
			temp.append( pChannelConfig->name ).append( CR )
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

		// Save the negotiated ping timeout (in milliseconds).
		_ommBaseImpl.saveNegotiatedPingTimeout( channelInfo.rsslChannelInfo.pingTimeout * 1000 );

		_pReconnectingReactorChannel = NULL;
#ifdef WIN32
		if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_SYSTEM_WRITE_BUFFERS, &pChannelConfig->sysSendBufSize, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Failed to set send buffer size on channel " );
				temp.append( pChannelConfig->name ).append( CR )
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

		if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_SYSTEM_READ_BUFFERS, &pChannelConfig->sysRecvBufSize, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Failed to set recv buffer size on channel " );
				temp.append( pChannelConfig->name ).append( CR )
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
		/* Set the compression threshold parameter if it is specified explicitly by users */
		if (pChannelConfig->compressionThresholdSet)
		{
			if (rsslReactorChannelIoctl(pRsslReactorChannel, RSSL_COMPRESSION_THRESHOLD, &pChannelConfig->compressionThreshold, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Failed to set compression threshold on channel ");
					temp.append(pChannelConfig->name).append(CR)
						.append("Consumer Name ").append(_ommBaseImpl.getInstanceName()).append(CR)
						.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
						.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
						.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
						.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
						.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
						.append("Error text ").append(rsslErrorInfo.rsslError.text);

					_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
				}

				_ommBaseImpl.closeChannel(pRsslReactorChannel);

				return RSSL_RC_CRET_SUCCESS;
			}
		}

		if (rsslReactorChannelIoctl(pRsslReactorChannel, RSSL_REACTOR_CHANNEL_IOCTL_DIRECT_WRITE, &pChannelConfig->directWrite, &rsslErrorInfo)
					!= RSSL_RET_SUCCESS)
		{
			if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Failed to set direct write on channel ");
				temp.append(pChannelConfig->name).append(CR)
					.append("Consumer Name ").append(_ommBaseImpl.getInstanceName()).append(CR)
					.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
					.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error text ").append(rsslErrorInfo.rsslError.text);

				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
			}

			_ommBaseImpl.closeChannel(pRsslReactorChannel);

			return RSSL_RC_CRET_SUCCESS;
		}

		if ( OmmLoggerClient::SuccessEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelUp event on channel " );
			temp.append(pChannelConfig->name).append( CR )
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
					temp.append(pChannelConfig->name).append( CR )
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
				temp.append(pChannelConfig->name).append( CR )
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

			if (activeConfig.xmlTracePingOnly)
				traceOptions.traceFlags |= RSSL_TRACE_PING_ONLY;

			if ( activeConfig.xmlTraceHex )
				traceOptions.traceFlags |= RSSL_TRACE_HEX;

			if ( activeConfig.xmlTraceDump )
				traceOptions.traceFlags |= RSSL_TRACE_DUMP;

			traceOptions.traceMsgMaxFileSize = activeConfig.xmlTraceMaxFileSize;

			if ( RSSL_RET_SUCCESS != rsslReactorChannelIoctl( pRsslReactorChannel, ( RsslIoctlCodes )RSSL_TRACE, ( void* )&traceOptions, &rsslErrorInfo ) )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Failed to enable Xml Tracing on channel " );
					temp.append(pChannelConfig->name).append( CR )
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
				temp.append(pChannelConfig->name).append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
		}

		if (pRsslReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
		{
			pChannel->setRsslChannel(pRsslReactorChannel).clearRsslSocket()
				.addRsslSocket(pRsslReactorChannel->socketId)
				.setChannelState(Channel::ChannelUpEnum);

			_ommBaseImpl.removeAllSocket();
			_ommBaseImpl.addCommonSocket();
			_ommBaseImpl.addSocket(pRsslReactorChannel->socketId);
		}
		else if (pRsslReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
		{
			pChannel->setRsslChannel(pRsslReactorChannel).clearRsslSocket().setChannelState(Channel::ChannelUpEnum);
			_ommBaseImpl.removeAllSocket();
			_ommBaseImpl.addCommonSocket();

			if (pRsslReactorChannel->pWarmStandbyChInfo)
			{
				RsslUInt32 index;
				for (index = 0; index < pRsslReactorChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					pChannel->addRsslSocket(pRsslReactorChannel->pWarmStandbyChInfo->socketIdList[index]);
					_ommBaseImpl.addSocket(pRsslReactorChannel->pWarmStandbyChInfo->socketIdList[index]);
				}
			}
			else
			{
				pChannel->addRsslSocket(pRsslReactorChannel->socketId);
				_ommBaseImpl.addSocket(pRsslReactorChannel->socketId);
			}
		}

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_CHANNEL_READY:
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelReady event on channel " );
			temp.append(pChannelConfig->name).append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		pChannel->setChannelState( Channel::ChannelReadyEnum );

		_ommBaseImpl.processChannelEvent( pEvent );

		if ( _bInitialChannelReadyEventReceived )
		{
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
			temp.append(pChannelConfig->name).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		if (pRsslReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_NORMAL)
		{
			pChannel->setRsslChannel(pRsslReactorChannel).clearRsslSocket()
				.addRsslSocket(pRsslReactorChannel->socketId);

			_ommBaseImpl.removeAllSocket();
			_ommBaseImpl.addCommonSocket();
			_ommBaseImpl.addSocket(pRsslReactorChannel->socketId);
		}
		else if (pRsslReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
		{
			pChannel->setRsslChannel(pRsslReactorChannel).clearRsslSocket();

			_ommBaseImpl.removeAllSocket();
			_ommBaseImpl.addCommonSocket();

			if (pRsslReactorChannel->pWarmStandbyChInfo)
			{
				RsslUInt32 index;
				for (index = 0; index < pRsslReactorChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					pChannel->addRsslSocket(pRsslReactorChannel->pWarmStandbyChInfo->socketIdList[index]);
					_ommBaseImpl.addSocket(pRsslReactorChannel->pWarmStandbyChInfo->socketIdList[index]);
				}
			}
			else
			{
				pChannel->setRsslChannel(pRsslReactorChannel).clearRsslSocket()
					.addRsslSocket(pRsslReactorChannel->socketId);

				_ommBaseImpl.addSocket(pRsslReactorChannel->socketId);
			}
		}

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_CHANNEL_DOWN:
	{
		pChannel->setChannelState( Channel::ChannelDownEnum );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelDown event on channel " );
			temp.append(pChannelConfig->name).append( CR )
			.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pEvent->pError->rsslError.rsslErrorId ? pEvent->pError->rsslError.text : "" );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		_pReconnectingReactorChannel = NULL;

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
			temp.append(pChannelConfig->name).append( CR )
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

		pChannel->clearRsslSocket()
		.setChannelState( Channel::ChannelDownReconnectingEnum );

		_ommBaseImpl.processChannelEvent( pEvent );

		_ommBaseImpl.getLoginCallbackClient().processChannelEvent( pEvent );

		_pReconnectingReactorChannel = pEvent->pReactorChannel;

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_WARNING:
	{
		if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received Channel warning event on channel " );
			temp.append(pChannelConfig->name).append( CR )
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
			.append( "channel " ).append(pChannelConfig->name).append( CR )
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
