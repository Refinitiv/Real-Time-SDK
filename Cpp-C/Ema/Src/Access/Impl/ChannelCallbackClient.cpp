/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
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
#include "ConsumerRoutingSession.h"
#include "ConsumerRoutingChannel.h"

#include <new>

#define EMA_INIT_NUMBER_OF_SOCKET 5

using namespace refinitiv::ema::access;

const EmaString ChannelCallbackClient::_clientName( "ChannelCallbackClient" );

Channel* Channel::create( OmmBaseImpl& ommBaseImpl, const EmaString& name , RsslReactor* pRsslReactor, ReactorChannelType reactorChannelType, RsslReactorWarmStandbyMode warmStandbyMode)
{
	try
	{
		return new Channel(ommBaseImpl, name, pRsslReactor, reactorChannelType, warmStandbyMode);
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

Channel::Channel(OmmBaseImpl& baseImpl, const EmaString& name, RsslReactor* pRsslReactor, ReactorChannelType reactorChannelType, RsslReactorWarmStandbyMode warmStandbyMode) :
	_ommBaseImpl(baseImpl),
	_name( name ),
	_pRsslReactor( pRsslReactor ),
	_toString(),
	_state( ChannelDownEnum ),
	_pDictionary( 0 ),
	_toStringSet( false ),
	_reactorChannelType( reactorChannelType ),
	_warmStandbyMode(warmStandbyMode),
	_pParentChannel( NULL ),
	_inOAuthCallback( false ),
	_addedToDeleteList( false ),
	_pChannelConfig(NULL),
	_pRoutingChannel(NULL),
	hasCalledRenewal(false),
	pChannelConfig(NULL)
{
}

Channel::~Channel()
{
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


RsslReactor* Channel::getRsslReactor() const
{
	return _pRsslReactor;
}

const EmaString& Channel::getName() const
{
	return _name;
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

RsslReactorWarmStandbyMode Channel::getWarmStandbyMode() const
{
	return _warmStandbyMode;
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

Channel& Channel::setConsumerRoutingChannel(ConsumerRoutingSessionChannel* pRoutingChannel)
{
	_pRoutingChannel = pRoutingChannel;
	return *this;
}
ConsumerRoutingSessionChannel* Channel::getConsumerRoutingChannel()
{
	return _pRoutingChannel;
}

Channel& Channel::setChannelConfig(ChannelConfig* pChannelConfig)
{
	_pChannelConfig = pChannelConfig;
	return *this;
}

ChannelConfig* Channel::getChannelConfig()
{
	return _pChannelConfig;
}

OmmBaseImpl* Channel::getBaseImpl()
{
	return &_ommBaseImpl;
		}


const EmaString& Channel::toString() const
{
	if ( !_toStringSet )
		{
		_toStringSet = true;
		_toString.set("\tRsslReactorChannel name ").append(_name).append(CR);

		if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
			{
			_toString.append("\tRsslReactor ").append(ptrToStringAsHex(_ommBaseImpl.getRsslReactor())).append(CR)
				.append("\tRsslReactorChannel ").append(ptrToStringAsHex(_ommBaseImpl.getRsslReactorChannel())).append(CR);
			}
		else
		{
			_toString.append("\tRsslReactor ").append(ptrToStringAsHex(_ommBaseImpl.getRsslReactor())).append(CR)
				.append("\tRsslReactorChannel ").append(ptrToStringAsHex(_pRoutingChannel->pReactorChannel)).append(CR);
		}
	}
	return _toString;
}

bool Channel::operator==( const Channel& other )
{
	if ( this == &other ) return true;

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

Channel* ChannelList::front() const
{
	return _list.front();
}

// This is only used in the final shutdown, so we're in a terminal state no matter what.
void ChannelCallbackClient::closeChannels()
{
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{
		UInt32 size = _channelList.size();

		_ommBaseImpl.closeChannel(_ommBaseImpl.getRsslReactorChannel());
		_channelList.removeAllChannel();
	}
	else
	{
		// This will close out all of the currently active channels and close them.  Everything will be cleaned up with the destructor, called later.
		_ommBaseImpl.getConsumerRoutingSession()->closeReactorChannels();		
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
		pChannel->pChannelConfig = activeChannelConfig;

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

void ChannelCallbackClient::freeReactorWarmStandbyGroup(RsslReactorConnectOptions& connectOpt, RsslReactorWarmStandbyGroup* warmStandbyChannelGroup)
{
	if (warmStandbyChannelGroup != nullptr)
	{
		for (UInt32 j = 0; j < connectOpt.warmStandbyGroupCount; ++j)
		{
			if (warmStandbyChannelGroup[j].standbyServerCount > 0)
			{
				RsslReactorWarmStandbyServerInfo* warmStandbyServerInfo = NULL;
				for (UInt32 k = 0; k < warmStandbyChannelGroup[j].standbyServerCount; ++k)
				{
					warmStandbyServerInfo = &warmStandbyChannelGroup[j].standbyServerList[k];

					if (warmStandbyServerInfo->perServiceBasedOptions.serviceNameCount > 0)
						free(warmStandbyServerInfo->perServiceBasedOptions.serviceNameList);
				}

				delete[] warmStandbyChannelGroup[j].standbyServerList;
			}

			if (warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameCount > 0)
				free(warmStandbyChannelGroup[j].startingActiveServer.perServiceBasedOptions.serviceNameList);
		}

		delete[] warmStandbyChannelGroup;
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

	if (activeConfig.consumerRoutingSessionSet.size() == 0)
	{
		// No routing session, so setup the single reactor channel here.
		EmaVector< ChannelConfig* >& activeConfigChannelSet = activeConfig.configChannelSet;
			EmaVector<WarmStandbyChannelConfig*>& warmStandbyChannelSet = activeConfig.configWarmStandbySet;
		UInt32 channelCfgSetLastIndex = activeConfigChannelSet.size() - 1;

		RsslReactorConnectInfo* reactorConnectInfo = nullptr;
		RsslReactorWarmStandbyGroup* warmStandbyChannelGroup = nullptr;

		RsslReactorConnectOptions connectOpt;
		rsslClearReactorConnectOptions(&connectOpt);
		connectOpt.connectionCount = activeConfigChannelSet.size();
		connectOpt.warmStandbyGroupCount = warmStandbyChannelSet.size();
		connectOpt.reconnectAttemptLimit = activeConfig.reconnectAttemptLimit;
		connectOpt.reconnectMinDelay = activeConfig.reconnectMinDelay;
		connectOpt.reconnectMaxDelay = activeConfig.reconnectMaxDelay;
				
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

			for (UInt32 i = 0; i < activeConfigChannelSet.size(); ++i)
			{
				rsslClearReactorConnectInfo(&reactorConnectInfo[i]);
			}

			connectOpt.reactorConnectionList = reactorConnectInfo;
		}


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

			for (UInt32 i = 0; i < warmStandbyChannelSet.size(); ++i)
			{
				rsslClearReactorWarmStandbyGroup(&warmStandbyChannelGroup[i]);
			}

			connectOpt.reactorWarmStandbyGroupList = warmStandbyChannelGroup;
		}
		

		if (activeConfig.enablePreferredHostOptions == true)
		{
			connectOpt.preferredHostOptions.enablePreferredHostOptions = true;
			connectOpt.preferredHostOptions.detectionTimeInterval = activeConfig.phDetectionTimeInterval;
			if (!activeConfig.phDetectionTimeSchedule.empty())
			{
				connectOpt.preferredHostOptions.detectionTimeSchedule.data = (char*)activeConfig.phDetectionTimeSchedule.c_str();
				connectOpt.preferredHostOptions.detectionTimeSchedule.length = activeConfig.phDetectionTimeSchedule.length();
			}
			connectOpt.preferredHostOptions.fallBackWithInWSBGroup = activeConfig.phFallBackWithInWSBGroup;

			if (!activeConfig.preferredChannelName.empty())
			{
				for (unsigned i = 0; i < activeConfigChannelSet.size(); i++)
				{
					if (activeConfigChannelSet[i]->name == activeConfig.preferredChannelName)
					{
						connectOpt.preferredHostOptions.connectionListIndex = i;
						break;
					}
					if ( i == activeConfigChannelSet.size() - 1)
					{
						EmaString temp("Preferred host channel name: ");
						temp.append(activeConfig.preferredChannelName);
						temp.append(" is not present in configuration.");
						throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
					}
				}
			}

			if (warmStandbyChannelGroup)
			{
				if (!activeConfig.preferredWSBChannelName.empty())
				{
					for (unsigned i = 0; i < warmStandbyChannelSet.size(); i++)
					{
						if (warmStandbyChannelSet[i]->name == activeConfig.preferredWSBChannelName)
						{
							connectOpt.preferredHostOptions.warmStandbyGroupListIndex = i;
							break;
						}

						if (i == warmStandbyChannelSet.size() - 1)
						{
							EmaString temp("Preferred host WSB channel name: ");
							temp.append(activeConfig.preferredWSBChannelName);
							temp.append(" is not present in configuration.");
							throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
						}
					}
				}
			}
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
				pChannel->setChannelConfig(activeConfigChannelSet[i]);

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
				warmStandbyChannelGroup[j].warmStandbyMode = (RsslReactorWarmStandbyMode)pWarmStandbyChannelConfig->warmStandbyMode;

			if (pWarmStandbyChannelConfig->startingActiveServer)
			{
				if (pWarmStandbyChannel == NULL)
				{
					pWarmStandbyChannel = Channel::create(_ommBaseImpl, pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name,
							_pRsslReactor, Channel::WARM_STANDBY, (RsslReactorWarmStandbyMode)pWarmStandbyChannelConfig->warmStandbyMode);
					_channelList.addChannel(pWarmStandbyChannel);
				}

				pChannel = channelConfigToReactorConnectInfo(pWarmStandbyChannelConfig->startingActiveServer->channelConfig,
					&warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo, componentVersionInfo);

				if (pChannel)
				{
					pChannel->setChannelConfig(pWarmStandbyChannelConfig->startingActiveServer->channelConfig);

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
							_pRsslReactor, Channel::WARM_STANDBY, (RsslReactorWarmStandbyMode)pWarmStandbyChannelConfig->warmStandbyMode);
					_channelList.addChannel(pWarmStandbyChannel);
				}

				pChannel = channelConfigToReactorConnectInfo(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig,
					&warmStandbyServerInfo->reactorConnectInfo, componentVersionInfo);

				if (pChannel)
				{
						pChannel->setChannelConfig(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig);

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
		}

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

			delete [] reactorConnectInfo;

			freeReactorWarmStandbyGroup(connectOpt, warmStandbyChannelGroup);

			throwIueException( temp, rsslErrorInfo.rsslError.rsslErrorId );

			return;
		}

		if (reactorConnectInfo != nullptr)
			delete [] reactorConnectInfo;

		freeReactorWarmStandbyGroup(connectOpt, warmStandbyChannelGroup);
	}
	else
	{
		// Iterate through the consumer routing session set and connect to each one.
		for (UInt32 sessionIter = 0; sessionIter < activeConfig.consumerRoutingSessionSet.size(); ++sessionIter)
		{
			EmaVector< ChannelConfig* >& channelSet = activeConfig.consumerRoutingSessionSet[sessionIter]->configChannelSet;
			UInt32 channelCfgSetLastIndex = channelSet.size() - 1;

			RsslReactorConnectInfo* reactorConnectInfo = 0;

			ConsumerRoutingSessionChannel* routingSessionChannel;

			RsslReactorConnectOptions& connectOpt = activeConfig.consumerRoutingSessionSet[sessionIter]->connectOpts;
			EmaVector<WarmStandbyChannelConfig*>& warmStandbyChannelSet = activeConfig.consumerRoutingSessionSet[sessionIter]->configWarmStandbySet;

			rsslClearReactorConnectOptions(&connectOpt);
			connectOpt.connectionCount = channelSet.size();
			connectOpt.warmStandbyGroupCount = warmStandbyChannelSet.size();
			connectOpt.reconnectAttemptLimit = activeConfig.consumerRoutingSessionSet[sessionIter]->reconnectAttemptLimit;
			connectOpt.reconnectMinDelay = activeConfig.consumerRoutingSessionSet[sessionIter]->reconnectMinDelay;
			connectOpt.reconnectMaxDelay = activeConfig.consumerRoutingSessionSet[sessionIter]->reconnectMaxDelay;

			try
			{
				routingSessionChannel = new ConsumerRoutingSessionChannel(_ommBaseImpl, activeConfig.consumerRoutingSessionSet[sessionIter]->name, *activeConfig.consumerRoutingSessionSet[sessionIter]);
			}
			catch (std::bad_alloc&)
			{
				const char* temp = "Failed to allocate memory in ChannelCallbackClient::initialize() for consumer routing channel";
				if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
					_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

				throwMeeException(temp);
				return;
			}

			if (channelSet.size() > 0)
			{
				try
				{
					reactorConnectInfo = new RsslReactorConnectInfo[channelSet.size()];
				}
				catch (std::bad_alloc&)
				{
					const char* temp = "Failed to allocate memory in ChannelCallbackClient::initialize()";
					if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
						_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

					throwMeeException(temp);
					return;
				}

				for (UInt32 i = 0; i < channelSet.size(); ++i)
				{
					rsslClearReactorConnectInfo(&reactorConnectInfo[i]);
				}

				connectOpt.reactorConnectionList = reactorConnectInfo;
			}

			RsslReactorWarmStandbyGroup* warmStandbyChannelGroup = NULL;
			ChannelList& sessionChannelList = routingSessionChannel->channelList;

			if (warmStandbyChannelSet.size() > 0)
			{
				try
				{
					warmStandbyChannelGroup = new RsslReactorWarmStandbyGroup[warmStandbyChannelSet.size()];
				}
				catch (std::bad_alloc&)
				{
					if (reactorConnectInfo)
						delete [] reactorConnectInfo;

					const char* temp = "Failed to allocate memory in ChannelCallbackClient::initialize() for warm standby group";
					if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
						_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

					throwMeeException(temp);
					return;
				}

				for (UInt32 i = 0; i < warmStandbyChannelSet.size(); ++i)
				{
					rsslClearReactorWarmStandbyGroup(&warmStandbyChannelGroup[i]);
				}

				connectOpt.reactorWarmStandbyGroupList = warmStandbyChannelGroup;
			}

			if (activeConfig.consumerRoutingSessionSet[sessionIter]->enablePreferredHostOptions == true)
			{
				connectOpt.preferredHostOptions.enablePreferredHostOptions = true;
				connectOpt.preferredHostOptions.detectionTimeInterval = activeConfig.consumerRoutingSessionSet[sessionIter]->phDetectionTimeInterval;

				if (!activeConfig.consumerRoutingSessionSet[sessionIter]->phDetectionTimeSchedule.empty())
				{
					connectOpt.preferredHostOptions.detectionTimeSchedule.data = (char*)activeConfig.consumerRoutingSessionSet[sessionIter]->phDetectionTimeSchedule.c_str();
					connectOpt.preferredHostOptions.detectionTimeSchedule.length = activeConfig.consumerRoutingSessionSet[sessionIter]->phDetectionTimeSchedule.length();
				}

				connectOpt.preferredHostOptions.fallBackWithInWSBGroup = activeConfig.consumerRoutingSessionSet[sessionIter]->phFallBackWithInWSBGroup;

				if (!activeConfig.consumerRoutingSessionSet[sessionIter]->preferredChannelName.empty())
				{
					for (unsigned i = 0; i < activeConfig.consumerRoutingSessionSet[sessionIter]->configChannelSet.size(); i++)
					{
						if (activeConfig.consumerRoutingSessionSet[sessionIter]->configChannelSet[i]->name == activeConfig.consumerRoutingSessionSet[sessionIter]->preferredChannelName)
						{
							connectOpt.preferredHostOptions.connectionListIndex = i;
							break;
						}
						if (i == activeConfig.consumerRoutingSessionSet[sessionIter]->configChannelSet.size() - 1)
						{
							EmaString temp("Preferred host channel name: ");
							temp.append(activeConfig.preferredChannelName);
							temp.append(" is not present in configuration.");
							throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
						}
					}
				}

				if (warmStandbyChannelGroup)
				{
					if (!activeConfig.consumerRoutingSessionSet[sessionIter]->preferredWSBChannelName.empty())
					{
						for (unsigned i = 0; i < warmStandbyChannelSet.size(); i++)
						{
							if (warmStandbyChannelSet[i]->name == activeConfig.consumerRoutingSessionSet[sessionIter]->preferredWSBChannelName)
							{
								connectOpt.preferredHostOptions.warmStandbyGroupListIndex = i;
								break;
							}

							if (i == warmStandbyChannelSet.size() - 1)
							{
								EmaString temp("Preferred host WSB channel name: ");
								temp.append(activeConfig.consumerRoutingSessionSet[sessionIter]->preferredWSBChannelName);
								temp.append(" is not present in configuration.");
								throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
							}
						}
					}
				}
			}

			EmaString channelParams;
			EmaString temp("Attempt to connect using ");
			if (connectOpt.connectionCount > 1)
				temp.set("Attempt to connect using the following list");
			UInt32 supportedConnectionTypeChannelCount = 0;
			EmaString errorStrUnsupportedConnectionType("Unknown connection type. Passed in type is");

			EmaString channelNames;
			Channel* pChannel = NULL;

			for (UInt32 i = 0; i < connectOpt.connectionCount; ++i)
			{
				pChannel = channelConfigToReactorConnectInfo(channelSet[i], &reactorConnectInfo[i], componentVersionInfo);

				if (pChannel)
				{
					pChannel->setChannelConfig( channelSet[i]);
					pChannel->setConsumerRoutingChannel(routingSessionChannel);
					if (role.ommConsumerRole.pLoginRequest == NULL)
						reactorConnectInfo[i].loginReqIndex = _ommBaseImpl.getLoginArrayIndex(channelSet[i]->name);

					if (reactorConnectInfo[i].enableSessionManagement == RSSL_TRUE)
						reactorConnectInfo[i].oAuthCredentialIndex = _ommBaseImpl.getOAuthArrayIndex(channelSet[i]->name);

					routingSessionChannel->channelList.addChannel(pChannel);

					supportedConnectionTypeChannelCount++;
					channelNames += pChannel->getName();

					if (OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
					{
						channelParams.clear();
						channelParametersToString(activeConfig, channelSet[i], channelParams);
						temp.append(CR).append(i + 1).append("] ").append(channelParams);
						if (i == (connectOpt.connectionCount - 1))
							_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
					}
				}
				else
				{
					reactorConnectInfo[i].rsslConnectOptions.userSpecPtr = 0;
					errorStrUnsupportedConnectionType.append(channelSet[i]->connectionType)
						.append(" for ")
						.append(channelSet[i]->name);
					if (i < channelCfgSetLastIndex)
						errorStrUnsupportedConnectionType.append(", ");
				}
			}

			WarmStandbyChannelConfig* pWarmStandbyChannelConfig = NULL;
			Channel* pWarmStandbyChannel = NULL;

			for (UInt32 j = 0; j < connectOpt.warmStandbyGroupCount; ++j)
			{
				rsslClearReactorWarmStandbyGroup(&warmStandbyChannelGroup[j]);

				pWarmStandbyChannelConfig = warmStandbyChannelSet[j];
				warmStandbyChannelGroup[j].warmStandbyMode = (RsslReactorWarmStandbyMode)pWarmStandbyChannelConfig->warmStandbyMode;

				if (pWarmStandbyChannelConfig->startingActiveServer)
				{
					if (pWarmStandbyChannel == NULL)
					{
						pWarmStandbyChannel = Channel::create(_ommBaseImpl, pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name,
							_pRsslReactor, Channel::WARM_STANDBY, (RsslReactorWarmStandbyMode)pWarmStandbyChannelConfig->warmStandbyMode);
						routingSessionChannel->channelList.addChannel(pWarmStandbyChannel);
						pWarmStandbyChannel->setConsumerRoutingChannel(routingSessionChannel);
					}

					pChannel = channelConfigToReactorConnectInfo(pWarmStandbyChannelConfig->startingActiveServer->channelConfig,
						&warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo, componentVersionInfo);

					if (pChannel)
					{
						pChannel->setChannelConfig(pWarmStandbyChannelConfig->startingActiveServer->channelConfig);
						pChannel->setConsumerRoutingChannel(routingSessionChannel);

						if (role.ommConsumerRole.pLoginRequest == NULL)
							warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo.loginReqIndex = _ommBaseImpl.getLoginArrayIndex(pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name);

						if (warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo.enableSessionManagement == RSSL_TRUE)
							warmStandbyChannelGroup[j].startingActiveServer.reactorConnectInfo.oAuthCredentialIndex = _ommBaseImpl.getOAuthArrayIndex(pWarmStandbyChannelConfig->startingActiveServer->channelConfig->name);

						pChannel->setParentChannel(pWarmStandbyChannel);

						routingSessionChannel->channelList.addChannel(pChannel);
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
							_pRsslReactor, Channel::WARM_STANDBY, (RsslReactorWarmStandbyMode)pWarmStandbyChannelConfig->warmStandbyMode);
						routingSessionChannel->channelList.addChannel(pWarmStandbyChannel);
						pWarmStandbyChannel->setConsumerRoutingChannel(routingSessionChannel);
					}

					pChannel = channelConfigToReactorConnectInfo(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig,
						&warmStandbyServerInfo->reactorConnectInfo, componentVersionInfo);

					if (pChannel)
					{
						pChannel->setChannelConfig(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig);
						pChannel->setConsumerRoutingChannel(routingSessionChannel);

						if (role.ommConsumerRole.pLoginRequest == NULL)
							warmStandbyServerInfo->reactorConnectInfo.loginReqIndex = _ommBaseImpl.getLoginArrayIndex(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig->name);

						if (warmStandbyServerInfo->reactorConnectInfo.enableSessionManagement == RSSL_TRUE)
							warmStandbyServerInfo->reactorConnectInfo.oAuthCredentialIndex = _ommBaseImpl.getOAuthArrayIndex(pWarmStandbyChannelConfig->standbyServerSet[k]->channelConfig->name);

						pChannel->setParentChannel(pWarmStandbyChannel);

						routingSessionChannel->channelList.addChannel(pChannel);
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
							for (RsslUInt32 index = 0; index < warmStandbyServerInfo->perServiceBasedOptions.serviceNameCount; index++)
							{
								warmStandbyServerInfo->perServiceBasedOptions.serviceNameList[index].data = const_cast<char*>(pWarmStandbyChannelConfig->standbyServerSet[k]->perServiceNameSet[index]->c_str());
								warmStandbyServerInfo->perServiceBasedOptions.serviceNameList[index].length = pWarmStandbyChannelConfig->standbyServerSet[k]->perServiceNameSet[index]->length();
							}
						}
					}
				}
			}

			RsslErrorInfo rsslErrorInfo;
			clearRsslErrorInfo(&rsslErrorInfo);

			// Turn ON SingleOpen for all login messages with request routing.
			// This is intentional, because it gives us more consistent behaviors for handling the data.  This is also required for any warm standby requests
			for (UInt32 i = 0; i < _ommBaseImpl.getLoginRequestList().size(); ++i)
			{
				LoginRdmReqMsgImpl* pRequestMsgImpl = _ommBaseImpl.getLoginRequestList()[i];

				if (pRequestMsgImpl != NULL)
				{
					RsslRDMLoginRequest* pLoginMsg = pRequestMsgImpl->get();
					pLoginMsg->flags |= RDM_LG_RQF_HAS_SINGLE_OPEN;
					pLoginMsg->singleOpen = 1;
				}
			}

			// Set the starting login here
			if (warmStandbyChannelSet.size() != 0)
			{

				if (warmStandbyChannelSet[0]->startingActiveServer != NULL)
				{
					if (activeConfig.consumerRoutingSessionSet[sessionIter]->enablePreferredHostOptions == true)
					{
						routingSessionChannel->loginInfo.pLoginRequestMsg = _ommBaseImpl.getLoginRequestList()[_ommBaseImpl.getLoginArrayIndex(warmStandbyChannelSet[connectOpt.preferredHostOptions.warmStandbyGroupListIndex]->startingActiveServer->name)];
					}
					else
					{
						routingSessionChannel->loginInfo.pLoginRequestMsg = _ommBaseImpl.getLoginRequestList()[_ommBaseImpl.getLoginArrayIndex(warmStandbyChannelSet[0]->startingActiveServer->name)];
					}
				}
				else
				{
					if (activeConfig.consumerRoutingSessionSet[sessionIter]->enablePreferredHostOptions == true)
					{
						routingSessionChannel->loginInfo.pLoginRequestMsg = _ommBaseImpl.getLoginRequestList()[_ommBaseImpl.getLoginArrayIndex(channelSet[connectOpt.preferredHostOptions.connectionListIndex]->name)];
					}
					else
					{
						routingSessionChannel->loginInfo.pLoginRequestMsg = _ommBaseImpl.getLoginRequestList()[_ommBaseImpl.getLoginArrayIndex(channelSet[0]->name)];
					}
				}
			}
			else
			{
				routingSessionChannel->loginInfo.pLoginRequestMsg = _ommBaseImpl.getLoginRequestList()[_ommBaseImpl.getLoginArrayIndex(channelSet[0]->name)];
			}

			// Log the request for this channel
			if (OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("RDMLogin request message was populated with this info: ");
				temp.append(CR).append("Routing Session Channel name: ").append(routingSessionChannel->name);
				temp.append(CR)
					.append(routingSessionChannel->loginInfo.pLoginRequestMsg->toString());
				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			// We've setup everything, now connect.
			if (RSSL_RET_SUCCESS != rsslReactorConnect(_pRsslReactor, &connectOpt, (RsslReactorChannelRole*)&role, &rsslErrorInfo))
			{

				EmaString temp("Failed to add RsslChannel(s) to RsslReactor. Channel name(s) ");
				temp.append(channelNames).append(CR)
					.append("Instance Name ").append(_ommBaseImpl.getInstanceName()).append(CR)
					.append("RsslReactor ").append(ptrToStringAsHex(_pRsslReactor)).append(CR)
					.append("RsslChannel ").append((UInt64)rsslErrorInfo.rsslError.channel).append(CR)
					.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
					.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
					.append("Error Text ").append(rsslErrorInfo.rsslError.text);

				if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
					_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());

				for (UInt32 i = 0; i < connectOpt.connectionCount; ++i)
				{
					Channel* pChannel = (Channel*)reactorConnectInfo[i].rsslConnectOptions.userSpecPtr;
					if (pChannel)
					{
						// The ChannelList::removeChannel() method also destroys the Channel object as well.
						_channelList.removeChannel(pChannel);
					}
				}

				routingSessionChannel->channelList.removeAllChannel();

				freeReactorWarmStandbyGroup(connectOpt, warmStandbyChannelGroup);

				throwIueException(temp, rsslErrorInfo.rsslError.rsslErrorId);

				return;
			}

			freeReactorWarmStandbyGroup(connectOpt, warmStandbyChannelGroup);

			routingSessionChannel->channelState = OmmBaseImpl::RsslChannelDownEnum;

			if (OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Successfully created a Reactor and Channel(s)");
				temp.append(CR)
					.append("Channel name(s) ").append(channelNames).append(CR)
					.append("Instance Name ").append(_ommBaseImpl.getInstanceName());
				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
			}

			_ommBaseImpl.getConsumerRoutingSession()->activeChannelCount++;
			routingSessionChannel->sessionIndex = _ommBaseImpl.getConsumerRoutingSession()->routingChannelList.size();
			_ommBaseImpl.getConsumerRoutingSession()->routingChannelList.push_back(routingSessionChannel);
		}
	}
}

void ChannelCallbackClient::removeChannel( Channel* pChannel)
{
	if (pChannel)
	{
		_channelList.removeChannel(pChannel);
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
		pChannelConfig = pChannel->getChannelConfig();

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

		// Set the channel here
		if (pChannel->getConsumerRoutingChannel() != NULL)
		{
			pChannel->getConsumerRoutingChannel()->pReactorChannel = pRsslReactorChannel;
		}
		else
		{
			_ommBaseImpl.setRsslReactorChannel(pRsslReactorChannel);
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

		if (_ommBaseImpl.getImplType() == OmmCommonImpl::NiProviderEnum)
		{
			_ommBaseImpl.setRsslReactorChannel(pRsslReactorChannel);
		}

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
		ConsumerRoutingSessionChannelConfig *pSessionChannelConfig;

		if (pChannel->getConsumerRoutingChannel() != NULL)
		{
			pSessionChannelConfig = &pChannel->getConsumerRoutingChannel()->routingChannelConfig;

			if (pSessionChannelConfig->xmlTraceToFile || pSessionChannelConfig->xmlTraceToStdout)
			{
				EmaString fileName(pSessionChannelConfig->xmlTraceFileName);
				fileName.append("_").append(pSessionChannelConfig->name);

				RsslTraceOptions traceOptions;
				rsslClearTraceOptions(&traceOptions);

				traceOptions.traceMsgFileName = (char*)fileName.c_str();

				if (pSessionChannelConfig->xmlTraceToFile)
					traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE;

				if (pSessionChannelConfig->xmlTraceToStdout)
					traceOptions.traceFlags |= RSSL_TRACE_TO_STDOUT;

				if (pSessionChannelConfig->xmlTraceToMultipleFiles)
					traceOptions.traceFlags |= RSSL_TRACE_TO_MULTIPLE_FILES;

				if (pSessionChannelConfig->xmlTraceWrite)
					traceOptions.traceFlags |= RSSL_TRACE_WRITE;

				if (pSessionChannelConfig->xmlTraceRead)
					traceOptions.traceFlags |= RSSL_TRACE_READ;

				if (pSessionChannelConfig->xmlTracePing)
					traceOptions.traceFlags |= RSSL_TRACE_PING;

				if (pSessionChannelConfig->xmlTracePingOnly)
					traceOptions.traceFlags |= RSSL_TRACE_PING_ONLY;

				if (pSessionChannelConfig->xmlTraceHex)
					traceOptions.traceFlags |= RSSL_TRACE_HEX;

				if (pSessionChannelConfig->xmlTraceDump)
					traceOptions.traceFlags |= RSSL_TRACE_DUMP;

				traceOptions.traceMsgMaxFileSize = pSessionChannelConfig->xmlTraceMaxFileSize;

				if (RSSL_RET_SUCCESS != rsslReactorChannelIoctl(pRsslReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void*)&traceOptions, &rsslErrorInfo))
				{
					if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
					{
						EmaString temp("Failed to enable Xml Tracing on channel ");
						temp.append(pChannelConfig->name).append(CR)
							.append("Instance Name ").append(_ommBaseImpl.getInstanceName()).append(CR)
							.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
							.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
							.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
							.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
							.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
							.append("Error Text ").append(rsslErrorInfo.rsslError.text);
						_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
					}
				}
				else if (OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Xml Tracing enabled on channel ");
					temp.append(pChannelConfig->name).append(CR)
						.append("Instance Name ").append(_ommBaseImpl.getInstanceName());
					_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
				}
			}
		}
		else if ( activeConfig.xmlTraceToFile || activeConfig.xmlTraceToStdout )
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
			pChannel->setChannelState(Channel::ChannelUpEnum);

			_ommBaseImpl.addSocket(pRsslReactorChannel->socketId);
		}
		else if (pRsslReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
		{
			pChannel->setChannelState(Channel::ChannelUpEnum);

			if (pRsslReactorChannel->pWarmStandbyChInfo)
			{
				RsslUInt32 index;
				for (index = 0; index < pRsslReactorChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					_ommBaseImpl.addSocket(pRsslReactorChannel->pWarmStandbyChInfo->socketIdList[index]);
				}
			}
			else
			{
				_ommBaseImpl.addSocket(pRsslReactorChannel->socketId);
			}
		}

		// Set the reactor channel and the current active channel
		if (pChannel->getConsumerRoutingChannel() != NULL)
		{
			pChannel->getConsumerRoutingChannel()->pReactorChannel = pRsslReactorChannel;
			pChannel->getConsumerRoutingChannel()->reconnecting = false;
			pChannel->getConsumerRoutingChannel()->pCurrentActiveChannel = pChannel;

			pChannel->getConsumerRoutingChannel()->pRoutingSession->processChannelEvent(pChannel->getConsumerRoutingChannel(), pEvent);
		}
		else
		{
			_ommBaseImpl.setRsslReactorChannel(pRsslReactorChannel);
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

		if (pChannel->getConsumerRoutingChannel() != NULL)
		{
			pChannel->getConsumerRoutingChannel()->pRoutingSession->processChannelEvent(pChannel->getConsumerRoutingChannel(), pEvent);
		}
		else if ( _bInitialChannelReadyEventReceived )
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
			// Remove the old socket from the list, then add the new one.
			_ommBaseImpl.removeSocket(pRsslReactorChannel->oldSocketId);
			_ommBaseImpl.addSocket(pRsslReactorChannel->socketId);
		}
		else if (pRsslReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
		{
			if (pRsslReactorChannel->pWarmStandbyChInfo)
			{
				RsslUInt32 index;

				// Remove all of the old sockets from the channel, then add the new ones.

				for (index = 0; index < pRsslReactorChannel->pWarmStandbyChInfo->oldSocketIdCount; index++)
				{
					_ommBaseImpl.removeSocket(pRsslReactorChannel->pWarmStandbyChInfo->oldSocketIdList[index]);
				}

				for (index = 0; index < pRsslReactorChannel->pWarmStandbyChInfo->socketIdCount; index++)
				{
					_ommBaseImpl.addSocket(pRsslReactorChannel->pWarmStandbyChInfo->socketIdList[index]);
				}
			}
			else
			{
				_ommBaseImpl.removeSocket(pRsslReactorChannel->oldSocketId);
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
			temp.append(pChannelConfig->name).append(CR)
				.append("Instance Name ").append(_ommBaseImpl.getInstanceName()).append(CR)
				.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR);
			
			if (pEvent->pError)
			{
				temp.append("RsslChannel ").append(ptrToStringAsHex(pEvent->pError->rsslError.channel)).append(CR)
					.append("Error Id ").append(pEvent->pError->rsslError.rsslErrorId).append(CR)
					.append("Internal sysError ").append(pEvent->pError->rsslError.sysError).append(CR)
					.append("Error Location ").append(pEvent->pError->errorLocation).append(CR)
					.append("Error Text ").append(pEvent->pError->rsslError.rsslErrorId ? pEvent->pError->rsslError.text : "");
			}

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		_ommBaseImpl.processChannelEvent( pEvent );

		if (pChannel->getConsumerRoutingChannel() != NULL)
		{
			pChannel->getConsumerRoutingChannel()->pRoutingSession->processChannelEvent(pChannel->getConsumerRoutingChannel(), pEvent);
		}
		else
		{
			_ommBaseImpl.getLoginCallbackClient().processChannelEvent( pEvent );
		}

		_ommBaseImpl.closeChannel( pRsslReactorChannel );

		if (_ommBaseImpl.getConsumerRoutingSession() != NULL)
		{
			ConsumerRoutingSession* pRoutingSession = _ommBaseImpl.getConsumerRoutingSession();

			if (pRoutingSession->activeChannelCount == 0)
				_ommBaseImpl.setState(OmmBaseImpl::RsslChannelDownEnum);

			// Re-route any items on this channel. They should have gotten a OPEN/SUSPECT status from the underlying watchlist previous to this.
			EmaList<Item*>& pendingList = pChannel->getConsumerRoutingChannel()->routedRequestList.getList();
			SingleItem* pItem = (SingleItem*)pendingList.pop_front();

			while (pItem != NULL)
			{
				pItem->setItemList(NULL);

				// Reroute and submit the item.  This will move the item to it's appropriate list.
				pItem->reSubmit(true);

				pItem = (SingleItem*)pendingList.pop_front();
			}
		}
		else
		{
			_ommBaseImpl.setState(OmmBaseImpl::RsslChannelDownEnum);
		}

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
	{
		if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received ChannelDownReconnecting event on channel " );
			temp.append(pChannelConfig->name).append(CR)
				.append("Instance Name ").append(_ommBaseImpl.getInstanceName()).append(CR)
				.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR);
			
			if (pEvent->pError)
			{
				temp.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
				.append( "Error Text " ).append( pEvent->pError->rsslError.rsslErrorId ? pEvent->pError->rsslError.text : "" );
			}
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace() );
		}

		// Clean out any set FD's in the notifiers
		if (pRsslReactorChannel->reactorChannelType == RSSL_REACTOR_CHANNEL_TYPE_WARM_STANDBY)
		{
			if (pRsslReactorChannel->pWarmStandbyChInfo != NULL)
			{
				for (UInt32 i = 0; i < pRsslReactorChannel->pWarmStandbyChInfo->oldSocketIdCount; ++i)
				{
					_ommBaseImpl.removeSocket(pRsslReactorChannel->pWarmStandbyChInfo->oldSocketIdList[i]);
				}
			}
			else
			{
				if (pRsslReactorChannel->socketId != REACTOR_INVALID_SOCKET)
					_ommBaseImpl.removeSocket(pRsslReactorChannel->socketId);
			}
		}
		else
		{
		if ( pRsslReactorChannel->socketId != REACTOR_INVALID_SOCKET )
			_ommBaseImpl.removeSocket( pRsslReactorChannel->socketId );
		}


		// Since this may be the first time we see a reactorChannel, set it on the associated Channel object.
		pChannel->setChannelState(Channel::ChannelDownReconnectingEnum);

	
		if (pChannel->getConsumerRoutingChannel())
		{
			// closedOnDownReconnecting gets set when the login is denied.  Do not send out an additional channel event here, and close the channel.
			// We should have received all fanout by this point.
			if (pChannel->getConsumerRoutingChannel()->closeOnDownReconnecting != true)
			{	
				// Set this to true on the routing channel if it exists
				pChannel->getConsumerRoutingChannel()->reconnecting = true;

				pChannel->getConsumerRoutingChannel()->pRoutingSession->processChannelEvent(pChannel->getConsumerRoutingChannel(), pEvent);
			}
			else
			{
				_ommBaseImpl.closeChannel(pRsslReactorChannel);
			}

			// Re-route any items on this list only if preferred host is not currently happening and enhanced item recovery is turned on.  They should have gotten a OPEN/SUSPECT status from the underlying watchlist previous to this.
			if (pChannel->getConsumerRoutingChannel()->inPreferredHost == false && 
				(_ommBaseImpl.getActiveConfig().consumerRoutingSessionEnhancedItemRecovery == true || pChannel->getConsumerRoutingChannel()->closeOnDownReconnecting == true))
			{
				EmaList<Item*>& pendingList = pChannel->getConsumerRoutingChannel()->routedRequestList.getList();
				SingleItem* pItem = (SingleItem*)pendingList.pop_front();

				while (pItem != NULL)
				{
					pItem->setItemList(NULL);
					pItem->sendClose();

					// Reroute and submit the item.  This will move the item to it's appropriate list.
					pItem->reSubmit(true);

					pItem = (SingleItem*)pendingList.pop_front();
				}
			}

			if (pChannel->getConsumerRoutingChannel()->closeOnDownReconnecting == true)
			{
				if (_ommBaseImpl.getConsumerRoutingSession()->activeChannelCount == 0)
					_ommBaseImpl.setState(OmmBaseImpl::RsslChannelDownEnum);

			}
		}

		_ommBaseImpl.processChannelEvent( pEvent );

		if(pChannel->getConsumerRoutingChannel() == NULL)
			_ommBaseImpl.getLoginCallbackClient().processChannelEvent( pEvent );

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
	case RSSL_RC_CET_PREFERRED_HOST_COMPLETE:
		if (OmmLoggerClient::SuccessEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Received Channel preferred host on channel ");
			temp.append(pChannelConfig->name);
			temp.append(" complete.");
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::SuccessEnum, temp.trimWhitespace());
		}

		if (pChannel->getConsumerRoutingChannel() != NULL)
		{
			pChannel->getConsumerRoutingChannel()->pRoutingSession->processChannelEvent(pChannel->getConsumerRoutingChannel(), pEvent);
		}
		else
		{
		_ommBaseImpl.getLoginCallbackClient().processChannelEvent(pEvent);
		}	

		break;
	case RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK:
		if (OmmLoggerClient::SuccessEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Received Channel preferred host starting fallback on channel ");
			temp.append(pChannelConfig->name);
			temp.append(".");
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::SuccessEnum, temp.trimWhitespace());
		}

		if (pChannel->getConsumerRoutingChannel() != NULL)
		{
			pChannel->getConsumerRoutingChannel()->pRoutingSession->processChannelEvent(pChannel->getConsumerRoutingChannel(), pEvent);
		}
		else
		{
			_ommBaseImpl.getLoginCallbackClient().processChannelEvent(pEvent);
		}

		break;
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
	return RSSL_RC_CRET_SUCCESS;
}

const ChannelList& ChannelCallbackClient::getChannelList()
{
	return _channelList;
}
