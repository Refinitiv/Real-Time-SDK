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
#include "OmmConsumerImpl.h"
#include "OmmConsumerErrorClient.h"
#include "Utilities.h"

#include <new>

using namespace thomsonreuters::ema::access;

const EmaString ChannelCallbackClient::_clientName( "ChannelCallbackClient" );

Channel* Channel::create( OmmConsumerImpl& ommConsImpl, const EmaString& name , RsslReactor* pRsslReactor )
{
	Channel* pChannel = 0;

	try {
		pChannel = new Channel( name, pRsslReactor );
	}
	catch( std::bad_alloc ) {}

	if ( !pChannel )
	{
		const char* temp = "Failed to create Channel.";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( "Channel", OmmLoggerClient::ErrorEnum, temp );

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
 _streamId( 4 ),
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

const EmaList< Directory >& Channel::getDirectoryList() const
{
	return _directoryList;
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
	_directoryList.remove( pDirectory );
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

Int32 Channel::getNextStreamId()
{
	// todo ... protect against overflow and assignment of used id
	return ++_streamId;
}

const EmaString& Channel::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;
		_toString.set( "\tRsslReactorChannel name " ).append( _name ).append( CR )
			.append( "\tRsslReactor " ).append( ptrToStringAsHex( _pRsslReactor ) ).append( CR )
			.append( "\tRsslReactorChannel " ).append( ptrToStringAsHex( _pRsslChannel ) ).append( CR )
			.append( "\tRsslSocket " ).append( (UInt64)_rsslSocket );

		if ( ! _directoryList.empty() )
		{
			_toString.append( CR ).append( "\tDirectory " );
			Directory* directory = _directoryList.front();
			while ( directory )
			{
                _toString.append( directory->getName() ).append( " " );
				directory = directory->next();
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

void ChannelCallbackClient::closeChannels()
{
	UInt32 size = _channelList.size();

	for ( UInt32 idx = 0; idx < size; ++idx )
	{
		_ommConsImpl.closeChannel( _channelList[idx] );
	}
}

ChannelCallbackClient::ChannelCallbackClient( OmmConsumerImpl& ommConsImpl,
											 RsslReactor* pRsslReactor ) :
 _channelList(),
 _ommConsImpl( ommConsImpl ),
 _pRsslReactor( pRsslReactor )
{
 	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created ChannelCallbackClient" );
	}
 }

ChannelCallbackClient::~ChannelCallbackClient()
{
	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed ChannelCallbackClient" );
	}
}

ChannelCallbackClient* ChannelCallbackClient::create( OmmConsumerImpl& ommConsImpl,
													 RsslReactor* pRsslReactor )
{
	ChannelCallbackClient* pClient = 0;

	try {
		pClient = new ChannelCallbackClient( ommConsImpl, pRsslReactor );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
	{
		const char* temp = "Failed to create ChannelCallbackClient";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

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

void ChannelCallbackClient::initialize( RsslRDMLoginRequest* loginRequest, RsslRDMDirectoryRequest* dirRequest )
{
	RsslReactorOMMConsumerRole consumerRole;
	rsslClearOMMConsumerRole( &consumerRole );

	consumerRole.pLoginRequest = loginRequest;
	consumerRole.pDirectoryRequest = dirRequest;
	consumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_NONE;
	consumerRole.loginMsgCallback = OmmConsumerImpl::loginCallback;
	consumerRole.directoryMsgCallback = OmmConsumerImpl::directoryCallback;
	consumerRole.dictionaryMsgCallback = OmmConsumerImpl::dictionaryCallback;
	consumerRole.base.channelEventCallback = OmmConsumerImpl::channelCallback;
	consumerRole.base.defaultMsgCallback = OmmConsumerImpl::itemCallback;
	consumerRole.watchlistOptions.channelOpenCallback = OmmConsumerImpl::channelOpenCallback;
	consumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;
	consumerRole.watchlistOptions.itemCountHint = _ommConsImpl.getActiveConfig().itemCountHint;
	consumerRole.watchlistOptions.obeyOpenWindow = _ommConsImpl.getActiveConfig().obeyOpenWindow > 0 ? RSSL_TRUE : RSSL_FALSE;
	consumerRole.watchlistOptions.postAckTimeout = _ommConsImpl.getActiveConfig().postAckTimeout;
	consumerRole.watchlistOptions.requestTimeout = _ommConsImpl.getActiveConfig().requestTimeout;
	consumerRole.watchlistOptions.maxOutstandingPosts = _ommConsImpl.getActiveConfig().maxOutstandingPosts;

	if ( _ommConsImpl.getActiveConfig().channelConfig->connectionType == RSSL_CONN_TYPE_SOCKET   ||
		_ommConsImpl.getActiveConfig().channelConfig->connectionType == RSSL_CONN_TYPE_HTTP ||
		_ommConsImpl.getActiveConfig().channelConfig->connectionType == RSSL_CONN_TYPE_ENCRYPTED)
	{
		Channel* pChannel = Channel::create( _ommConsImpl, _ommConsImpl.getActiveConfig().channelConfig->name, _pRsslReactor );

		RsslReactorConnectOptions connectOpt;
		rsslClearReactorConnectOptions( &connectOpt );

		connectOpt.rsslConnectOptions.userSpecPtr = (void*)pChannel;

		connectOpt.rsslConnectOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
		connectOpt.rsslConnectOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
		connectOpt.rsslConnectOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;

		connectOpt.initializationTimeout = 5;
		connectOpt.reconnectAttemptLimit = _ommConsImpl.getActiveConfig().channelConfig->reconnectAttemptLimit;
		connectOpt.reconnectMinDelay = _ommConsImpl.getActiveConfig().channelConfig->reconnectMinDelay;
		connectOpt.reconnectMaxDelay = _ommConsImpl.getActiveConfig().channelConfig->reconnectMaxDelay;

		connectOpt.rsslConnectOptions.compressionType = _ommConsImpl.getActiveConfig().channelConfig->compressionType;
		connectOpt.rsslConnectOptions.connectionType = _ommConsImpl.getActiveConfig().channelConfig->connectionType;
		connectOpt.rsslConnectOptions.pingTimeout = _ommConsImpl.getActiveConfig().channelConfig->connectionPingTimeout;
		connectOpt.rsslConnectOptions.guaranteedOutputBuffers = _ommConsImpl.getActiveConfig().channelConfig->guaranteedOutputBuffers;
		connectOpt.rsslConnectOptions.sysRecvBufSize = _ommConsImpl.getActiveConfig().channelConfig->sysRecvBufSize;
		connectOpt.rsslConnectOptions.sysSendBufSize = _ommConsImpl.getActiveConfig().channelConfig->sysSendBufSize;
		connectOpt.rsslConnectOptions.numInputBuffers = _ommConsImpl.getActiveConfig().channelConfig->numInputBuffers;

		EmaString strConnectionType;
		switch ( connectOpt.rsslConnectOptions.connectionType )
		{
		case RSSL_CONN_TYPE_SOCKET:
			{
			connectOpt.rsslConnectOptions.connectionInfo.unified.address = (char*)static_cast<SocketChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->hostName.c_str();
			connectOpt.rsslConnectOptions.connectionInfo.unified.serviceName = (char*)static_cast<SocketChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->serviceName.c_str();
			connectOpt.rsslConnectOptions.tcpOpts.tcp_nodelay = static_cast<SocketChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->tcpNodelay;
			strConnectionType = "RSSL_CONN_TYPE_SOCKET";
			break;
			}
		case RSSL_CONN_TYPE_ENCRYPTED:
			{
			connectOpt.rsslConnectOptions.connectionInfo.unified.address = (char*)static_cast<EncryptedChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->hostName.c_str();
			connectOpt.rsslConnectOptions.connectionInfo.unified.serviceName = (char*)static_cast<EncryptedChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->serviceName.c_str();
			connectOpt.rsslConnectOptions.tcpOpts.tcp_nodelay = static_cast<EncryptedChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->tcpNodelay;
			connectOpt.rsslConnectOptions.objectName = (char*) static_cast<EncryptedChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->objectName.c_str();
			strConnectionType = "RSSL_CONN_TYPE_ENCRYPTED";
			break;
			}
		case RSSL_CONN_TYPE_HTTP:
			{
			connectOpt.rsslConnectOptions.connectionInfo.unified.address = (char*)static_cast<HttpChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->hostName.c_str();
			connectOpt.rsslConnectOptions.connectionInfo.unified.serviceName = (char*)static_cast<HttpChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->serviceName.c_str();
			connectOpt.rsslConnectOptions.tcpOpts.tcp_nodelay = static_cast<HttpChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->tcpNodelay;
			connectOpt.rsslConnectOptions.objectName = (char*) static_cast<HttpChannelConfig*>(_ommConsImpl.getActiveConfig().channelConfig)->objectName.c_str();
			strConnectionType = "RSSL_CONN_TYPE_HTTP";
			break;
			}
		default :
			break;
		}

		connectOpt.rsslConnectOptions.connectionInfo.unified.interfaceName = (char*)_ommConsImpl.getActiveConfig().channelConfig->interfaceName.c_str();
		connectOpt.rsslConnectOptions.connectionInfo.unified.unicastServiceName = (char *) "";

		if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString compType;
			switch ( connectOpt.rsslConnectOptions.compressionType )
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

			EmaString temp( "Attempt to connect using ");
			temp.append( strConnectionType ).append( CR )
				.append( "Channel name ").append( pChannel->getName() ).append( CR )
				.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( _pRsslReactor ) ).append( CR )
				.append( "interfaceName " ).append( connectOpt.rsslConnectOptions.connectionInfo.unified.interfaceName ).append( CR )
				.append( "hostName " ).append( connectOpt.rsslConnectOptions.connectionInfo.unified.address ).append( CR )
				.append( "port " ).append( connectOpt.rsslConnectOptions.connectionInfo.unified.serviceName ).append( CR )
				.append( "reconnectAttemptLimit " ).append( connectOpt.reconnectAttemptLimit ).append( CR )
				.append( "reconnectMinDelay " ).append( connectOpt.reconnectMinDelay ).append( " msec" ).append( CR )
				.append( "reconnectMaxDelay " ).append( connectOpt.reconnectMaxDelay).append( " msec" ).append( CR )
				.append( "CompressionType " ).append( compType ).append( CR )
				.append( "connectionPingTimeout " ).append( connectOpt.rsslConnectOptions.pingTimeout ).append( " msec" ).append( CR )
				.append( "tcpNodelay " ).append( (connectOpt.rsslConnectOptions.tcpOpts.tcp_nodelay ? "true" : "false" ) );
				if(connectOpt.rsslConnectOptions.connectionType == RSSL_CONN_TYPE_ENCRYPTED ||
					connectOpt.rsslConnectOptions.connectionType == RSSL_CONN_TYPE_HTTP)
					temp.append( CR ).append("ObjectName ").append(connectOpt.rsslConnectOptions.objectName);
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		RsslErrorInfo rsslErrorInfo;

		if ( RSSL_RET_SUCCESS != rsslReactorConnect( _pRsslReactor, &connectOpt, (RsslReactorChannelRole*)(&consumerRole), &rsslErrorInfo ) )
		{

			EmaString temp( "Failed to add RsslChannel to RsslReactor. Channel name " );
			temp.append( pChannel->getName() ).append( CR )
				.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( _pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( (UInt64)rsslErrorInfo.rsslError.channel ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );

			Channel::destroy( pChannel );

			throwIueException( temp );

			return;
		}

		_ommConsImpl.setState( OmmConsumerImpl::RsslChannelDownEnum );
		pChannel->setChannelState( Channel::ChannelDownEnum );

		if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully created a Reactor Channel" );
            temp.append( CR )
			    .append(" Channel name " ).append( pChannel->getName() ).append( CR )
				.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		_channelList.addChannel( pChannel );
	}
	else
	{
		EmaString temp( "Unknown connection type. Passed in type is " );
		temp.append( _ommConsImpl.getActiveConfig().channelConfig->connectionType );
		throwIueException( temp );
	}
}

void ChannelCallbackClient::removeChannel( RsslReactorChannel* pRsslReactorChannel )
{
	if ( pRsslReactorChannel )
		_channelList.removeChannel( (Channel*)pRsslReactorChannel->userSpecPtr );
}

RsslReactorCallbackRet ChannelCallbackClient::processCallback( RsslReactor* pRsslReactor,
															  RsslReactorChannel* pRsslReactorChannel,
															  RsslReactorChannelEvent* pEvent )
{
	Channel* pChannel = (Channel*)( pRsslReactorChannel->userSpecPtr );

	switch ( pEvent->channelEventType )
	{
		case RSSL_RC_CET_CHANNEL_OPENED :
		{
			if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received ChannelOpened on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() );

				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_UP:
		{
			RsslErrorInfo rsslErrorInfo;
#ifdef WIN32
			int sendBfrSize = 65535;

			if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_SYSTEM_WRITE_BUFFERS, &sendBfrSize, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Failed to set send buffer size on channel " );
					temp.append( pChannel->getName() ).append( CR )
						.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
						.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
						.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
						.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
						.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
						.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
						.append( "Error text " ).append( rsslErrorInfo.rsslError.text );

					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
				}

				_ommConsImpl.closeChannel( pRsslReactorChannel );

				return RSSL_RC_CRET_SUCCESS;
			}

			int rcvBfrSize = 65535;
			if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_SYSTEM_READ_BUFFERS, &rcvBfrSize, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Failed to set recv buffer size on channel " );
					temp.append( pChannel->getName() ).append( CR )
						.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
						.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
						.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
						.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
						.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
						.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
						.append( "Error text " ).append( rsslErrorInfo.rsslError.text );

					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
				}

				_ommConsImpl.closeChannel( pRsslReactorChannel );

				return RSSL_RC_CRET_SUCCESS;
			}
#endif
			if ( rsslReactorChannelIoctl( pRsslReactorChannel, RSSL_COMPRESSION_THRESHOLD, &_ommConsImpl.getActiveConfig().channelConfig->compressionThreshold, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Failed to set compression threshold on channel " );
					temp.append( pChannel->getName() ).append( CR )
						.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
						.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
						.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
						.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
						.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
						.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
						.append( "Error text " ).append( rsslErrorInfo.rsslError.text );

					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
				}

				_ommConsImpl.closeChannel( pRsslReactorChannel );

				return RSSL_RC_CRET_SUCCESS;
			}

			pChannel->setRsslChannel( pRsslReactorChannel )
				.setRsslSocket( pRsslReactorChannel->socketId )
				.setChannelState( Channel::ChannelUpEnum );

			if ( OmmLoggerClient::SuccessEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received ChannelUp event on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::SuccessEnum, temp );
			}

			if ( _ommConsImpl.getActiveConfig().channelConfig->xmlTraceToFile ||
				_ommConsImpl.getActiveConfig().channelConfig->xmlTraceToStdout ) 
			{
				EmaString fileName( _ommConsImpl.getActiveConfig().channelConfig->xmlTraceFileName );
				fileName.append( "_" );

				RsslTraceOptions traceOptions;
				rsslClearTraceOptions( &traceOptions );

				traceOptions.traceMsgFileName = (char*)fileName.c_str();

				if ( _ommConsImpl.getActiveConfig().channelConfig->xmlTraceToFile )
					traceOptions.traceFlags |= RSSL_TRACE_TO_FILE_ENABLE;

				if ( _ommConsImpl.getActiveConfig().channelConfig->xmlTraceToStdout )
					traceOptions.traceFlags |= RSSL_TRACE_TO_STDOUT;

				if ( _ommConsImpl.getActiveConfig().channelConfig->xmlTraceToMultipleFiles )
					traceOptions.traceFlags |= RSSL_TRACE_TO_MULTIPLE_FILES;

				if ( _ommConsImpl.getActiveConfig().channelConfig->xmlTraceWrite )
					traceOptions.traceFlags |= RSSL_TRACE_WRITE;

				if ( _ommConsImpl.getActiveConfig().channelConfig->xmlTraceRead )
					traceOptions.traceFlags |= RSSL_TRACE_READ;
		
				traceOptions.traceMsgMaxFileSize = _ommConsImpl.getActiveConfig().channelConfig->xmlTraceMaxFileSize;

				if ( RSSL_RET_SUCCESS != rsslReactorChannelIoctl( pRsslReactorChannel, (RsslIoctlCodes)RSSL_TRACE, (void *)&traceOptions, &rsslErrorInfo ) )
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Failed to enable Xml Tracing on channel " );
						temp.append( pChannel->getName() ).append( CR )
							.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
							.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
							.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
							.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
							.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
							.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
							.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
					}
				}
				else if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Xml Tracing enabled on channel " );
					temp.append( pChannel->getName() ).append( CR )
						.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
				}
			}

			_ommConsImpl.addSocket( pRsslReactorChannel->socketId );
			_ommConsImpl.setState( OmmConsumerImpl::RsslChannelUpEnum );
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_READY:
		{
			pChannel->setChannelState( Channel::ChannelReadyEnum );

			if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received ChannelReady event on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_FD_CHANGE:
		{
			if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received FD Change event on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}

			pChannel->setRsslChannel( pRsslReactorChannel )
				.setRsslSocket( pRsslReactorChannel->socketId );

			_ommConsImpl.removeSocket( pRsslReactorChannel->oldSocketId );
			_ommConsImpl.addSocket( pRsslReactorChannel->socketId );
			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN:
		{
			pChannel->setChannelState( Channel::ChannelDownEnum );

			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received ChannelDown event on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
					.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
					.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
					.append( "Error Text " ).append( pEvent->pError->rsslError.text );
				
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}

			_ommConsImpl.setState( OmmConsumerImpl::RsslChannelDownEnum );

			_ommConsImpl.closeChannel( pRsslReactorChannel );

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
		{
			if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received ChannelDownReconnecting event on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
					.append( "RsslReactor ").append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append(CR)
					.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
					.append( "Error Text " ).append( pEvent->pError->rsslError.text );

				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace() );
			}

			if ( pRsslReactorChannel->socketId != REACTOR_INVALID_SOCKET )
				_ommConsImpl.removeSocket( pRsslReactorChannel->socketId );

			pChannel->setRsslSocket( 0 )
				.setChannelState( Channel::ChannelDownReconnectingEnum );

			return RSSL_RC_CRET_SUCCESS;
		}
		case RSSL_RC_CET_WARNING:
		{
			if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received Channel warning event on channel " );
				temp.append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
					.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
					.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
					.append( "Error Text " ).append( pEvent->pError->rsslError.text );

				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace() );
			}
			return RSSL_RC_CRET_SUCCESS;
		}
		default:
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unknown channel event type " );
				temp.append( pEvent->channelEventType ).append( CR )
					.append( "channel " ).append( pChannel->getName() ).append( CR )
					.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
					.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( pEvent->pError->rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( pEvent->pError->rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( pEvent->pError->rsslError.sysError ).append( CR )
					.append( "Error Location " ).append( pEvent->pError->errorLocation ).append( CR )
					.append( "Error Text " ).append( pEvent->pError->rsslError.text );

				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}
			return RSSL_RC_CRET_FAILURE;
		}
	}
}

const ChannelList & ChannelCallbackClient::getChannelList()
{
    return _channelList;
}
