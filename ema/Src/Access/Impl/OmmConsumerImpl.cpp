/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumer.h"
#include "OmmConsumerImpl.h"
#include "OmmConsumerConfigImpl.h"
#include "ExceptionTranslator.h"
#include "OmmException.h"
#include "ChannelCallbackClient.h"
#include "LoginCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "DictionaryCallbackClient.h"
#include "ItemCallbackClient.h"
#include "TimeOut.h"
#include "OmmConsumerErrorClient.h"
#include "OmmInvalidConfigurationException.h"
#include "OmmInvalidUsageException.h"
#include "OmmOutOfRangeException.h"
#include "OmmSystemException.h"
#include "OmmInvalidHandleException.h"
#include "OmmMemoryExhaustionException.h"
#include "OmmInaccessibleLogFileException.h"
#include "OmmUnsupportedDomainTypeException.h"

#include <new>

#define	EMA_BIG_STR_BUFF_SIZE (1024*4)
using namespace thomsonreuters::ema::access;

EmaList< OmmConsumerImpl > OmmConsumerImplMap::_ommConsumerList;
Mutex OmmConsumerImplMap::_listLock;
UInt64 OmmConsumerImplMap::_id = 0;

#ifndef WIN32
struct sigaction OmmConsumerImplMap::_sigAction;
struct sigaction OmmConsumerImplMap::_oldSigAction;
#endif

#ifdef USING_POLL
int
OmmConsumerImpl::addFd( int fd, short events = POLLIN )
{
	if ( _eventFdsCount == _eventFdsCapacity )
	{
		_eventFdsCapacity *= 2;
		pollfd * tmp( new pollfd[ _eventFdsCapacity ] );
		for ( int i = 0; i < _eventFdsCount; ++i )
			tmp[ i ] = _eventFds[ i ];
		delete [] _eventFds;
		_eventFds = tmp;
	}
	_eventFds[ _eventFdsCount ].fd = fd;
	_eventFds[ _eventFdsCount ].events = events;

	return _eventFdsCount++;
}
void OmmConsumerImpl::removeFd( int fd )
{
#ifdef USING_POLL
  pipeReadEventFdsIdx = -1;
#endif
	for ( int i = 0; i < _eventFdsCount; ++i )
		if ( _eventFds[ i ].fd == fd )
		{
			if ( i < _eventFdsCount - 1 )
				_eventFds[ i ] = _eventFds[ _eventFdsCount - 1 ];
			--_eventFdsCount;
			break;
		}
}
#endif

OmmConsumerImpl::OmmConsumerImpl( const OmmConsumerConfig& config ) :
 _activeConfig(),
 _consumerLock(),
 _pipeLock(),
 _reactorDispatchErrorInfo(),
 _reactorRetCode( RSSL_RET_SUCCESS ),
 _ommConsumerState( NotInitializedEnum ),
 _pRsslReactor( 0 ),
 _pChannelCallbackClient( 0 ),
 _pLoginCallbackClient( 0 ),
 _pDirectoryCallbackClient( 0 ),
 _pDictionaryCallbackClient( 0 ),
 _pItemCallbackClient( 0 ),
 _pLoggerClient( 0 ),
 _pipe(),
 _pipeWriteCount( 0 ),
 _dispatchInternalMsg( false ),
 _atExit( false ),
 _ommConsumerErrorClient( 0 )
{
	initialize( config );
}

OmmConsumerImpl::OmmConsumerImpl( const OmmConsumerConfig& config, OmmConsumerErrorClient& client ) :
 _activeConfig(),
 _consumerLock(),
 _pipeLock(),
 _reactorDispatchErrorInfo(),
 _reactorRetCode( RSSL_RET_SUCCESS ),
 _ommConsumerState( NotInitializedEnum ),
 _pRsslReactor( 0 ),
 _pChannelCallbackClient( 0 ),
 _pLoginCallbackClient( 0 ),
 _pDirectoryCallbackClient( 0 ),
 _pDictionaryCallbackClient( 0 ),
 _pItemCallbackClient( 0 ),
 _pLoggerClient( 0 ),
 _pipe(),
 _pipeWriteCount( 0 ),
 _dispatchInternalMsg( false ),
 _atExit( false ),
 _ommConsumerErrorClient( &client )
{
	initialize( config );
}

OmmConsumerImpl::~OmmConsumerImpl()
{
	uninitialize();
}

void OmmConsumerImpl::readConfig( const OmmConsumerConfig& config )
{
	OmmConsumerConfigImpl* pConfigImpl = 0;

	pConfigImpl = config.getConfigImpl();

	UInt64 id = OmmConsumerImplMap::push_back( this );

	_activeConfig.consumerName = pConfigImpl->getConsumerName();

	EmaString channelName = pConfigImpl->getChannelName( _activeConfig.consumerName );
	_activeConfig.loggerConfig.loggerName = pConfigImpl->getLoggerName( _activeConfig.consumerName );
	_activeConfig.dictionaryConfig.dictionaryName = pConfigImpl->getDictionaryName( _activeConfig.consumerName );

	_activeConfig.instanceName = _activeConfig.consumerName;
	_activeConfig.instanceName.append("_").append(id);

	EmaString consumerNodeName("ConsumerGroup|ConsumerList|Consumer.");
	consumerNodeName.append(_activeConfig.consumerName);
	consumerNodeName.append("|");
        
	UInt32 maxUInt32( 0xFFFFFFFF );
	UInt64 tmp;
	if ( pConfigImpl->get<UInt64>( consumerNodeName + "ItemCountHint", tmp ) )
		_activeConfig.itemCountHint = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if (pConfigImpl->get<UInt64>(consumerNodeName + "ServiceCountHint", tmp ) )
		_activeConfig.serviceCountHint = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( pConfigImpl->get<UInt64>(consumerNodeName + "ObeyOpenWindow", tmp ) )
		_activeConfig.obeyOpenWindow = static_cast<UInt32>( tmp > 0 ? 1 : 0 );
	if ( pConfigImpl->get<UInt64>(consumerNodeName + "PostAckTimeout", tmp ) )
		_activeConfig.postAckTimeout = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( pConfigImpl->get<UInt64>(consumerNodeName + "RequestTimeout", tmp ) )
		_activeConfig.requestTimeout = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( pConfigImpl->get< UInt64 >( consumerNodeName + "LoginRequestTimeOut", tmp ) )
		_activeConfig.loginRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( pConfigImpl->get< UInt64 >( consumerNodeName + "DirectoryRequestTimeOut", tmp ) )
		_activeConfig.directoryRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( pConfigImpl->get< UInt64 >( consumerNodeName + "DictionaryRequestTimeOut", tmp ) )
		_activeConfig.dictionaryRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( pConfigImpl->get<UInt64>(consumerNodeName + "MaxOutstandingPosts", tmp ) )
		_activeConfig.maxOutstandingPosts = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );

	pConfigImpl->get<Int64>( consumerNodeName + "DispatchTimeoutApiThread", _activeConfig.dispatchTimeoutApiThread );

	if ( pConfigImpl->get<UInt64>( consumerNodeName + "CatchUnhandledException", tmp ) )
		_activeConfig.catchUnhandledException = static_cast<UInt32>( tmp > 0 ? true : false );
	if ( pConfigImpl->get<UInt64>( consumerNodeName + "MaxDispatchCountApiThread", tmp ) )
		_activeConfig.maxDispatchCountApiThread = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( pConfigImpl->get<UInt64>(consumerNodeName + "MaxDispatchCountUserThread", tmp ) )
		_activeConfig.maxDispatchCountUserThread = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	pConfigImpl->get<Int64>( consumerNodeName + "PipePort", _activeConfig.pipePort );

	if ( _activeConfig.dictionaryConfig.dictionaryName.empty() )
	{
		_activeConfig.dictionaryConfig.dictionaryName.set("Dictionary");
		_activeConfig.dictionaryConfig.dictionaryType = Dictionary::ChannelDictionaryEnum;
		_activeConfig.dictionaryConfig.enumtypeDefFileName.clear();
		_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName.clear();
	}
	else
	{
		EmaString dictionaryNodeName( "DictionaryGroup|DictionaryList|Dictionary." );
		dictionaryNodeName.append( _activeConfig.dictionaryConfig.dictionaryName ).append("|");

        EmaString name;
        if ( !pConfigImpl->get< EmaString >( dictionaryNodeName + "Name", name ) )
		{
			EmaString errorMsg( "no configuration exists for consumer dictionary [" );
			errorMsg.append( dictionaryNodeName ).append( "]; will use dictionary defaults" );
            pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
		}

		if ( !pConfigImpl->get<Dictionary::DictionaryType>( dictionaryNodeName + "DictionaryType", _activeConfig.dictionaryConfig.dictionaryType ))
			_activeConfig.dictionaryConfig.dictionaryType = Dictionary::ChannelDictionaryEnum;

		if ( _activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum )
		{
			if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "RdmFieldDictionaryFileName", _activeConfig.dictionaryConfig.rdmfieldDictionaryFileName))
				_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName.set( "./RDMFieldDictionary" );
			if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "EnumTypeDefFileName", _activeConfig.dictionaryConfig.enumtypeDefFileName ) )
				_activeConfig.dictionaryConfig.enumtypeDefFileName.set( "./enumtype.def" );
		}
	}
	
	_activeConfig.loggerConfig.minLoggerSeverity = OmmLoggerClient::SuccessEnum;
	_activeConfig.loggerConfig.loggerFileName = "emaLog";
	_activeConfig.loggerConfig.loggerType = OmmLoggerClient::FileEnum;
	_activeConfig.loggerConfig.includeDateInLoggerOutput = false;

	if ( _activeConfig.loggerConfig.loggerName.length() )
	{
		EmaString loggerNodeName( "LoggerGroup|LoggerList|Logger." );
		loggerNodeName.append( _activeConfig.loggerConfig.loggerName ).append( "|" );
        
		EmaString name;
        if ( !pConfigImpl->get< EmaString >( loggerNodeName + "Name", name ) )
		{
			EmaString errorMsg( "no configuration exists for consumer logger [" );
			errorMsg.append( loggerNodeName ).append( "]; will use logger defaults" );
            pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
		}

		pConfigImpl->get<OmmLoggerClient::LoggerType>( loggerNodeName + "LoggerType", _activeConfig.loggerConfig.loggerType );

		if ( _activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum )
			pConfigImpl->get<EmaString>( loggerNodeName + "FileName", _activeConfig.loggerConfig.loggerFileName );

		pConfigImpl->get<OmmLoggerClient::Severity>(loggerNodeName + "LoggerSeverity", _activeConfig.loggerConfig.minLoggerSeverity);

		UInt64 idilo(0);
		if ( pConfigImpl->get< UInt64 >( loggerNodeName + "IncludeDateInLoggerOutput", idilo) )
			_activeConfig.loggerConfig.includeDateInLoggerOutput = idilo == 1 ? true : false ;
	}
	else
		_activeConfig.loggerConfig.loggerName.set( "Logger" );

	if ( channelName.empty() )
	{
		try {
			_activeConfig.channelConfig = new SocketChannelConfig();
			if ( _activeConfig.channelConfig->getType() == ChannelConfig::SocketChannelEnum)
			{
				EmaString & tmp( pConfigImpl->getUserSpecifiedHostname() );
				if ( tmp.length() )
					static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->hostName = tmp;
				tmp = pConfigImpl->getUserSpecifiedPort();
				if ( tmp.length() )
					static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->serviceName = tmp;
			}
		}
		catch ( std::bad_alloc )
		{
			const char* temp( "Failed to allocate memory for SocketChannelConfig." );
			throwMeeException( temp );
			return;
		}

		if ( !_activeConfig.channelConfig )
		{
			const char* temp = "Failed to allocate memory for SocketChannelConfig.";
			throwMeeException(temp);
			return;
		}

		_activeConfig.channelConfig->name.set( "Channel" );
	}
	else
	{
		EmaString channelNodeName( "ChannelGroup|ChannelList|Channel." );
		channelNodeName.append( channelName ).append("|");

		RsslConnectionTypes channelType;
		if ( !pConfigImpl->get<RsslConnectionTypes>( channelNodeName + "ChannelType", channelType ) ||
			pConfigImpl->getUserSpecifiedHostname().length() > 0 )
			channelType = RSSL_CONN_TYPE_SOCKET;

		switch ( channelType )
		{
		case RSSL_CONN_TYPE_SOCKET:
			{
				try {
					_activeConfig.channelConfig = new SocketChannelConfig();
				}
				catch ( std::bad_alloc )
				{
					const char* temp( "Failed to allocate memory for SocketChannelConfig. (std::bad_alloc)" );
					throwMeeException( temp );
					return;
				}

				if ( !_activeConfig.channelConfig )
				{
					const char* temp = "Failed to allocate memory for SocketChannelConfig. (null ptr)";
					throwMeeException(temp);
					return;
				}

				EmaString & tmp( pConfigImpl->getUserSpecifiedHostname() );
				if ( tmp.length() )
					static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->hostName = tmp;
				else
					pConfigImpl->get< EmaString >( channelNodeName + "Host", static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->hostName );
				
				tmp = pConfigImpl->getUserSpecifiedPort();
				if ( tmp.length() )
					static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->serviceName = tmp;
				else
					pConfigImpl->get< EmaString >( channelNodeName + "Port", static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->serviceName );

				UInt64 tempUInt = 1;
				pConfigImpl->get<UInt64>( channelNodeName + "TcpNodelay", tempUInt );
				if ( tempUInt )
					static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->tcpNodelay = RSSL_TRUE;
				else
					static_cast<SocketChannelConfig*>(_activeConfig.channelConfig)->tcpNodelay = RSSL_FALSE;
			
				break;
			}
		case RSSL_CONN_TYPE_HTTP:
			{
				try {
					_activeConfig.channelConfig = new HttpChannelConfig();
				}
				catch ( std::bad_alloc )
				{
					const char* temp( "Failed to allocate memory for HttpChannelConfig. (std::bad_alloc)" );
					throwMeeException( temp );
					return;
				}

				if ( !_activeConfig.channelConfig )
				{
					const char* temp = "Failed to allocate memory for HttpChannelConfig. (null ptr)";
					throwMeeException(temp);
					return;
				}

				pConfigImpl->get<EmaString>(channelNodeName + "Host", static_cast<HttpChannelConfig*>(_activeConfig.channelConfig)->hostName);

				pConfigImpl->get<EmaString>(channelNodeName + "Port", static_cast<HttpChannelConfig*>(_activeConfig.channelConfig)->serviceName);

				UInt64 tempUInt = 1;
				pConfigImpl->get<UInt64>(channelNodeName + "TcpNodelay", tempUInt);
				if (!tempUInt)
					static_cast<HttpChannelConfig*>(_activeConfig.channelConfig)->tcpNodelay = RSSL_FALSE;
				else
					static_cast<HttpChannelConfig*>(_activeConfig.channelConfig)->tcpNodelay = RSSL_TRUE;

				break;
			}
		case RSSL_CONN_TYPE_ENCRYPTED:
			{
				try {
					_activeConfig.channelConfig = new EncryptedChannelConfig();
				}
				catch ( std::bad_alloc )
				{
					const char* temp( "Failed to allocate memory for EncryptedChannelConfig. (std::bad_alloc)" );
					throwMeeException( temp );
					return;
				}

				if ( !_activeConfig.channelConfig )
				{
					const char* temp = "Failed to allocate memory for EncryptedChannelConfig. (null ptr)";
					throwMeeException(temp);
					return;
				}

				pConfigImpl->get<EmaString>( channelNodeName + "Host", static_cast<EncryptedChannelConfig*>(_activeConfig.channelConfig)->hostName);

				pConfigImpl->get<EmaString>( channelNodeName + "Port", static_cast<EncryptedChannelConfig*>(_activeConfig.channelConfig)->serviceName);

				UInt64 tempUInt = 1;
				pConfigImpl->get<UInt64>( channelNodeName + "TcpNodelay", tempUInt );
				if ( tempUInt )
					static_cast<EncryptedChannelConfig*>(_activeConfig.channelConfig)->tcpNodelay = RSSL_TRUE;
				else
					static_cast<EncryptedChannelConfig*>(_activeConfig.channelConfig)->tcpNodelay = RSSL_FALSE;

				break;
			}
		default:
			{
			EmaString temp( "Not supported channel type. Type = " );
			temp.append( (UInt32)channelType );
			throwIueException(temp);
			return;
			}
		}

		_activeConfig.channelConfig->name = channelName;

		pConfigImpl->get<EmaString>( channelNodeName + "InterfaceName", _activeConfig.channelConfig->interfaceName );

		pConfigImpl->get<RsslCompTypes>( channelNodeName + "CompressionType", _activeConfig.channelConfig->compressionType );
				
		UInt64 tempUInt = 0;
		if (pConfigImpl->get<UInt64>( channelNodeName + "GuaranteedOutputBuffers", tempUInt ) )
		{
			_activeConfig.channelConfig->setGuaranteedOutputBuffers( tempUInt );
		}

		tempUInt = 0;
		if ( pConfigImpl->get<UInt64>( channelNodeName + "ConnectionPingTimeout", tempUInt ) )
		{
			_activeConfig.channelConfig->connectionPingTimeout = (UInt32)tempUInt;
		}

		pConfigImpl->get<Int64>( channelNodeName + "ReconnectAttemptLimit", _activeConfig.channelConfig->reconnectAttemptLimit );

		pConfigImpl->get<Int64>( channelNodeName + "ReconnectMinDelay", _activeConfig.channelConfig->reconnectMinDelay );
				
		pConfigImpl->get<Int64>( channelNodeName + "ReconnectMaxDelay", _activeConfig.channelConfig->reconnectMaxDelay );

		pConfigImpl->get<EmaString>( channelNodeName + "XmlTraceFileName", _activeConfig.channelConfig->xmlTraceFileName );

		Int64 tempInt = 0;
		pConfigImpl->get<Int64>( channelNodeName + "XmlTraceMaxFileSize", tempInt );
		if ( tempInt > 0 )
			_activeConfig.channelConfig->xmlTraceMaxFileSize = tempInt;

		tempUInt = 0;
		pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceToFile", tempUInt );
		if ( tempUInt > 0 )
			_activeConfig.channelConfig->xmlTraceToFile = true;

		tempUInt = 0;
		pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceToStdout", tempUInt );
		if ( tempUInt > 0 )
			_activeConfig.channelConfig->xmlTraceToStdout = true;

		tempUInt = 0;
		pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceToMultipleFiles", tempUInt );
		if ( tempUInt > 0 )
			_activeConfig.channelConfig->xmlTraceToMultipleFiles = true;

		tempUInt = 1;
		pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceWrite", tempUInt );
		if (tempUInt == 0)
			_activeConfig.channelConfig->xmlTraceWrite = false;

		tempUInt = 1;
		pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceRead", tempUInt );
		if ( tempUInt == 0 )
			_activeConfig.channelConfig->xmlTraceRead = false;

		tempUInt = 1;
		pConfigImpl->get<UInt64>( channelNodeName + "MsgKeyInUpdates", tempUInt );
		if ( tempUInt == 0 )
			_activeConfig.channelConfig->msgKeyInUpdates = false;
	}
	if ( ProgrammaticConfigure *  const ppc  = pConfigImpl->pProgrammaticConfigure() ) {
		ppc->retrieveConsumerConfig( _activeConfig.consumerName, _activeConfig );
		ppc->retrieveChannelConfig( channelName, _activeConfig, pConfigImpl->getUserSpecifiedHostname().length() > 0 );
		ppc->retrieveLoggerConfig( _activeConfig.loggerConfig.loggerName , _activeConfig );
		ppc->retrieveDictionaryConfig( _activeConfig.dictionaryConfig.dictionaryName, _activeConfig );
	}

	_activeConfig.userDispatch = pConfigImpl->getOperationModel();
	_activeConfig.pRsslRDMLoginReq = pConfigImpl->getLoginReq();
	_activeConfig.pRsslDirectoryRequestMsg = pConfigImpl->getDirectoryReq();
	_activeConfig.pRsslEnumDefRequestMsg = pConfigImpl->getEnumDefDictionaryReq();
	_activeConfig.pRsslRdmFldRequestMsg = pConfigImpl->getRdmFldDictionaryReq();
	catchUnhandledException( _activeConfig.catchUnhandledException );
}

void OmmConsumerImpl::initialize( const OmmConsumerConfig& config )
{
	try {
		_consumerLock.lock();

		readConfig( config );

		_pLoggerClient = OmmLoggerClient::create( *this );

		config.getConfigImpl()->configErrors().log( _pLoggerClient, _activeConfig.loggerConfig.minLoggerSeverity );

		if ( !_pipe.create() )
		{
			EmaString temp( "Failed to create communication Pipe." );
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
			throwIueException( temp );
		}
		else if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully initialized communication Pipe." );
			_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		RsslError rsslError;

		RsslRet retCode = rsslInitialize( RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError );
		if ( retCode != RSSL_RET_SUCCESS )
		{
			_consumerLock.unlock();
			EmaString temp( "rsslInitialize() failed while initializing OmmConsumer." );
			temp.append( " Internal sysError='" ).append( rsslError.sysError )
				.append( "' Error text='" ).append( rsslError.text ).append( "'. " );

			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException( temp );
			return;
		}
		else if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully initialized Rssl." );
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		_ommConsumerState = RsslInitilizedEnum;

		RsslCreateReactorOptions reactorOpts;
		RsslErrorInfo rsslErrorInfo;

		rsslClearCreateReactorOptions( &reactorOpts );

		reactorOpts.userSpecPtr = (void*)this;

		_pRsslReactor = rsslCreateReactor( &reactorOpts, &rsslErrorInfo );
		if ( !_pRsslReactor )
		{
			_consumerLock.unlock();
			EmaString temp( "Failed to initialize OmmConsumer (rsslCreateReactor)." );
			temp.append( "' Error Id='" ).append( rsslErrorInfo.rsslError.rsslErrorId )
				.append( "' Internal sysError='" ).append( rsslErrorInfo.rsslError.sysError )
				.append( "' Error Location='" ).append( rsslErrorInfo.errorLocation )
				.append( "' Error Text='" ).append( rsslErrorInfo.rsslError.text ).append( "'. " );
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException( temp );
			return;
		}
		else if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully created Reactor." );
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		_ommConsumerState = ReactorInitializedEnum;

#ifdef USING_SELECT
		FD_ZERO( &_readFds );
		FD_ZERO( &_exceptFds );

		FD_SET( _pipe.readFD(), &_readFds );
		FD_SET( _pRsslReactor->eventFd, &_readFds );
#else
		_eventFdsCapacity = 8;
		_eventFds = new pollfd[ _eventFdsCapacity ];
		_eventFdsCount = 0;
		pipeReadEventFdsIdx = addFd( _pipe.readFD() );
		addFd( _pRsslReactor->eventFd );
#endif

		_pLoginCallbackClient = LoginCallbackClient::create( *this );
		_pLoginCallbackClient->initialize();

		_pDictionaryCallbackClient = DictionaryCallbackClient::create( *this );
		_pDictionaryCallbackClient->initialize();

		_pDirectoryCallbackClient = DirectoryCallbackClient::create( *this );
		_pDirectoryCallbackClient->initialize();

		_pItemCallbackClient = ItemCallbackClient::create( *this );
		_pItemCallbackClient->initialize();

		_pChannelCallbackClient = ChannelCallbackClient::create( *this, _pRsslReactor );
		_pChannelCallbackClient->initialize( _pLoginCallbackClient->getLoginRequest(), _pDirectoryCallbackClient->getDirectoryRequest() );

		UInt64 timeOutLengthInMicroSeconds = _activeConfig.loginRequestTimeOut * 1000;
		_eventTimedOut = false;
		TimeOut * loginWatcher( new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmConsumerImpl::terminateIf, reinterpret_cast< void * >( this ) ) );
		while ( ! _atExit && ! _eventTimedOut && ( _ommConsumerState < LoginStreamOpenOkEnum ) )
			rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );

		if ( _eventTimedOut )
		{
			EmaString failureMsg( "login failed (timed out after waiting " );
			failureMsg.append( _activeConfig.loginRequestTimeOut ).append( " milliseconds) for " );
			if ( _activeConfig.channelConfig->getType() == ChannelConfig::SocketChannelEnum )
			{
				SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( _activeConfig.channelConfig ) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}
			else if ( _activeConfig.channelConfig->getType() == ChannelConfig::HttpChannelEnum )
			{
				HttpChannelConfig * channelConfig( reinterpret_cast< HttpChannelConfig * >( _activeConfig.channelConfig ) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}
			else if ( _activeConfig.channelConfig->getType() == ChannelConfig::EncryptedChannelEnum )
			{
				EncryptedChannelConfig * channelConfig( reinterpret_cast< EncryptedChannelConfig * >( _activeConfig.channelConfig ) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}

			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg);
			throwIueException( failureMsg );
			return;
		}
		else
			loginWatcher->cancel();

		timeOutLengthInMicroSeconds = _activeConfig.directoryRequestTimeOut * 1000;
		_eventTimedOut = false;
		loginWatcher = new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmConsumerImpl::terminateIf, reinterpret_cast< void * >( this ) );
		while ( ! _atExit && ! _eventTimedOut && ( _ommConsumerState < DirectoryStreamOpenOkEnum ) )
			rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );

		if ( _eventTimedOut )
		{
			EmaString failureMsg( "directory retrieval failed (timed out after waiting " );
			failureMsg.append( _activeConfig.directoryRequestTimeOut ).append( " milliseconds) for " );
			if ( _activeConfig.channelConfig->getType() == ChannelConfig::SocketChannelEnum )
			{
				SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( _activeConfig.channelConfig ) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg);
			throwIueException( failureMsg );
			return;
		}
		else
			loginWatcher->cancel();

		timeOutLengthInMicroSeconds = _activeConfig.dictionaryRequestTimeOut * 1000;
		_eventTimedOut = false;
		loginWatcher = new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmConsumerImpl::terminateIf, reinterpret_cast< void * >( this ) );
		while ( !_atExit && ! _eventTimedOut && !_pDictionaryCallbackClient->isDictionaryReady() )
			rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );

		if ( _eventTimedOut )
		{
			EmaString failureMsg( "dictionary retrieval failed (timed out after waiting " );
			failureMsg.append( _activeConfig.dictionaryRequestTimeOut ).append( " milliseconds) for " );
			if ( _activeConfig.channelConfig->getType() == ChannelConfig::SocketChannelEnum )
			{
				SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( _activeConfig.channelConfig ) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg);
			throwIueException( failureMsg );
			return;
		}
		else
			loginWatcher->cancel();

		if ( !_atExit && _activeConfig.userDispatch == OmmConsumerConfig::ApiDispatchEnum ) start();

		_consumerLock.unlock();
	}
	catch ( const OmmException& ommException )
	{
		_consumerLock.unlock();

		uninitialize();

		if ( hasOmmConnsumerErrorClient() )
			notifyOmmConsumerErrorClient( ommException, getOmmConsumerErrorClient() );
		else
			throw;
	}
}

void OmmConsumerImpl::setDispatchInternalMsg()
{
	_dispatchInternalMsg = true;
	pipeWrite();
}

void OmmConsumerImpl::pipeWrite()
{
	_pipeLock.lock();

	if ( ++_pipeWriteCount == 1 )
		_pipe.write( "0", 1 );

	_pipeLock.unlock();
}

void OmmConsumerImpl::pipeRead()
{
	_pipeLock.lock();

	if ( --_pipeWriteCount == 0 )
	{
		char temp[10];
		_pipe.read( temp, 1 );
	}

	_pipeLock.unlock();
}

void OmmConsumerImpl::uninitialize()
{
	_consumerLock.lock();

	if ( _ommConsumerState == NotInitializedEnum )
	{
		_consumerLock.unlock();
		return;
	}

	if ( _activeConfig.userDispatch == OmmConsumerConfig::ApiDispatchEnum )
	{
		pipeWrite();
		stop();
		wait();
	}

	pipeRead();

	if ( _pRsslReactor )
	{
		if ( _pLoginCallbackClient )
			rsslReactorDispatchLoop( 10000, _pLoginCallbackClient->sendLoginClose() );

		_pChannelCallbackClient->closeChannels();

		RsslErrorInfo rsslErrorInfo;
		
		if ( RSSL_RET_SUCCESS != rsslDestroyReactor( _pRsslReactor, &rsslErrorInfo ) ) 
		{
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Failed to uninitialize OmmConsumer (rsslDestroyReactor)." );
				temp.append( "' Error Id='" ).append( rsslErrorInfo.rsslError.rsslErrorId )
					.append( "' Internal sysError='" ).append( rsslErrorInfo.rsslError.sysError )
					.append( "' Error Location='" ).append( rsslErrorInfo.errorLocation )
					.append( "' Error Text='" ).append( rsslErrorInfo.rsslError.text ).append( "'. " );

				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			}
		}

#ifdef USING_SELECT
		FD_CLR( _pRsslReactor->eventFd, &_readFds );
#else
		removeFd( _pRsslReactor->eventFd );
#endif
		_pRsslReactor = 0;
	}
	
	ItemCallbackClient::destroy( _pItemCallbackClient );

	DictionaryCallbackClient::destroy( _pDictionaryCallbackClient );

	DirectoryCallbackClient::destroy( _pDirectoryCallbackClient );

	LoginCallbackClient::destroy( _pLoginCallbackClient );

	ChannelCallbackClient::destroy( _pChannelCallbackClient );

	if ( RSSL_RET_SUCCESS != rsslUninitialize() )
	{
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "rsslUninitialize() failed while unintializing OmmConsumer." );
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
		}
	}

#ifdef USING_SELECT
	FD_CLR( _pipe.readFD(), &_readFds );
#else
	removeFd( _pipe.readFD() );
	pipeReadEventFdsIdx = -1;
#endif
	_pipe.close();

	OmmLoggerClient::destroy( _pLoggerClient );

	OmmConsumerImplMap::removeValue( this );

	_ommConsumerState = NotInitializedEnum;

	_consumerLock.unlock();
	
#ifdef USING_POLL
	delete[] _eventFds;
#endif
}

void OmmConsumerImpl::setAtExit()
{
	_atExit = true;
	pipeWrite();
}

bool OmmConsumerImpl::rsslReactorDispatchLoop( Int64 timeOut, UInt32 count )
{
	Int64 selectRetCode = 1;

	Int64 userTimeoutInMicroSeconds( 0 );
	bool userTimeoutExists( TimeOut::getTimeOutInMicroSeconds( *this, userTimeoutInMicroSeconds ) );
	if ( userTimeoutExists )
	{
	    if ( timeOut >= 0 && timeOut < userTimeoutInMicroSeconds )
	      userTimeoutExists = false;
	    else
	      timeOut = userTimeoutInMicroSeconds;
	}

	if ( _reactorRetCode <= RSSL_RET_SUCCESS || userTimeoutExists )
	{
#if defined( USING_SELECT )
		fd_set useReadFds = _readFds;
		fd_set useExceptFds = _exceptFds;

		struct timeval selectTime;
		if ( timeOut >= 0 )
		{
			selectTime.tv_sec = timeOut / 1000000;
			selectTime.tv_usec = timeOut % 1000000;			
			selectRetCode = select( FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime );
		}
		else
			selectRetCode = select( FD_SETSIZE, &useReadFds, NULL, &useExceptFds, NULL );

		if ( selectRetCode > 0 && FD_ISSET( _pipe.readFD(), &useReadFds ) )
			pipeRead();

#elif defined( USING_PPOLL )
		struct timespec ppollTime;

		if ( timeOut >= 0 )
		{
			ppollTime.tv_sec = timeOut / static_cast<long long>(1e6);
			ppollTime.tv_nsec = timeOut % static_cast<long long>(1e6) * static_cast<long long>(1e3);
			selectRetCode = ppoll( _eventFds, _eventFdsCount, &ppollTime, 0);
		}
		else
			selectRetCode = ppoll( _eventFds, _eventFdsCount, 0, 0);

		if ( selectRetCode > 0 )
		{
			if ( pipeReadEventFdsIdx == -1 )
				for( int i = 0; i < _eventFdsCount; ++i )
					if ( _eventFds[ i ].fd == _pipe.readFD() )
					{
						pipeReadEventFdsIdx = i;
						break;
					}

			if ( pipeReadEventFdsIdx != -1 )
				if ( _eventFds[ pipeReadEventFdsIdx ].revents & POLLIN )
					pipeRead();
		}
#else
#error "No Implementation for Operating System That Does Not Implement ppoll"
#endif
	}

	if ( selectRetCode > 0 )
	{
		RsslReactorDispatchOptions dispatchOpts;
		dispatchOpts.pReactorChannel = NULL;
		dispatchOpts.maxMessages = count;

		_reactorRetCode = _pRsslReactor ? rsslReactorDispatch( _pRsslReactor, &dispatchOpts, &_reactorDispatchErrorInfo ) : RSSL_RET_SUCCESS;

		if ( _reactorRetCode > RSSL_RET_SUCCESS )
		{
			if ( userTimeoutExists && timeOut == 0 )
				TimeOut::execute( *this, theTimeOuts );
			return true;
		}
		else if ( _reactorRetCode == RSSL_RET_SUCCESS )
			return false;
		else
		{
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Call to rsslReactorDispatch() failed. Internal sysError='" );
				temp.append( _reactorDispatchErrorInfo.rsslError.sysError )
					.append( "' Error text='" ).append( _reactorDispatchErrorInfo.rsslError.text ).append( "'. " );

				_consumerLock.lock();
				if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				_consumerLock.unlock();
			}

			return false;
		}
	}
	else if ( selectRetCode == 0 && userTimeoutExists )
	{
		TimeOut::execute( *this, theTimeOuts );
	}
	else if ( selectRetCode < 0 )
	{
#ifdef WIN32
		Int64 lastError = WSAGetLastError();
		if ( lastError != WSAEINTR )
		{
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal Error. Call to select() failed. LastError='" );
				temp.append( lastError ).append( "'. " );

				_consumerLock.lock();
				if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				_consumerLock.unlock();
			}
		}
#else
		if ( errno != EINTR )
		{
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal Error. Call to select() failed. LastError='" );
				temp.append( errno ).append( "'. " );

				_consumerLock.lock();
				if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
				_consumerLock.unlock();
			}
		}
#endif
		return false;
	}

	return true;
}

void OmmConsumerImpl::setState( OmmConsumerState state )
{
	_ommConsumerState = state;
}

void OmmConsumerImpl::addSocket( RsslSocket fd )
{
#ifdef USING_SELECT	
	FD_SET( fd, &_readFds );
	FD_SET( fd, &_exceptFds );
#else
	addFd( fd , POLLIN|POLLERR|POLLHUP);
#endif
}

void OmmConsumerImpl::removeSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_CLR( fd, &_readFds );
	FD_CLR( fd, &_exceptFds );
#else
	removeFd( fd );
#endif
}

void OmmConsumerImpl::closeChannel( RsslReactorChannel* pRsslReactorChannel )
{
	if ( !pRsslReactorChannel ) return;

	RsslErrorInfo rsslErrorInfo;

	if ( pRsslReactorChannel->socketId != REACTOR_INVALID_SOCKET )
		removeSocket( pRsslReactorChannel->socketId );

	if ( rsslReactorCloseChannel( _pRsslReactor, pRsslReactorChannel, &rsslErrorInfo ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Failed to close reactor channel (rsslReactorCloseChannel)." );
			temp.append( "' RsslChannel='" ).append( (UInt64)rsslErrorInfo.rsslError.channel )
				.append( "' Error Id='" ).append( rsslErrorInfo.rsslError.rsslErrorId )
				.append( "' Internal sysError='" ).append( rsslErrorInfo.rsslError.sysError )
				.append( "' Error Location='" ).append( rsslErrorInfo.errorLocation )
				.append( "' Error Text='" ).append( rsslErrorInfo.rsslError.text ).append( "'. " );

			_consumerLock.lock();

			if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			
			_consumerLock.unlock();
		}
	}

	_pChannelCallbackClient->removeChannel( pRsslReactorChannel );
}

const EmaString& OmmConsumerImpl::getConsumerName() const
{
	return _activeConfig.instanceName;
}

ItemCallbackClient& OmmConsumerImpl::getItemCallbackClient()
{
	return *_pItemCallbackClient;
}

DictionaryCallbackClient& OmmConsumerImpl::getDictionaryCallbackClient()
{
	return *_pDictionaryCallbackClient;
}

DirectoryCallbackClient& OmmConsumerImpl::getDirectoryCallbackClient()
{
	return *_pDirectoryCallbackClient;
}

LoginCallbackClient& OmmConsumerImpl::getLoginCallbackClient()
{
	return *_pLoginCallbackClient;
}

ChannelCallbackClient& OmmConsumerImpl::getChannelCallbackClient()
{
	return *_pChannelCallbackClient;
}

OmmLoggerClient& OmmConsumerImpl::getOmmLoggerClient()
{
	return *_pLoggerClient;
}

UInt64 OmmConsumerImpl::registerClient( const ReqMsg& reqMsg, OmmConsumerClient& ommConsClient,
										void* closure )
{
	_consumerLock.lock();

	UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( reqMsg, ommConsClient, closure ) : 0;

	_consumerLock.unlock();

	return handle;
}

void OmmConsumerImpl::reissue( const ReqMsg& reqMsg, UInt64 handle )
{
	_consumerLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->reissue( reqMsg, handle );

	_consumerLock.unlock();
}

void OmmConsumerImpl::unregister( UInt64 handle )
{
	_consumerLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->unregister( handle );

	_consumerLock.unlock();
}

void OmmConsumerImpl::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	_consumerLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->submit( genericMsg, handle );

	_consumerLock.unlock();
}

void OmmConsumerImpl::submit( const PostMsg& postMsg, UInt64 handle )
{
	_consumerLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->submit( postMsg, handle );

	_consumerLock.unlock();
}

OmmConsumerActiveConfig& OmmConsumerImpl::getActiveConfig()
{
	return _activeConfig;
}

Int64 OmmConsumerImpl::dispatch( Int64 timeOut )
{
	if ( _activeConfig.userDispatch == OmmConsumerConfig::UserDispatchEnum )
		return rsslReactorDispatchLoop( timeOut, _activeConfig.maxDispatchCountUserThread ) ? OmmConsumer::DispatchedEnum : OmmConsumer::TimeoutEnum;

	return OmmConsumer::TimeoutEnum;
}

void OmmConsumerImpl::run()
{
	while ( !Thread::isStopping() && !_atExit )
		rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );
}

int OmmConsumerImpl::runLog( void* pExceptionStructure, const char* file, unsigned int line )
{
	char reportBuf[EMA_BIG_STR_BUFF_SIZE*10];
	if ( retrieveExceptionContext( pExceptionStructure, file, line, reportBuf, EMA_BIG_STR_BUFF_SIZE*10 ) > 0 )
	{
		_consumerLock.lock();
		if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, reportBuf );
		_consumerLock.unlock();
	}

	return 1;
}

RsslReactorCallbackRet OmmConsumerImpl::channelCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->_pChannelCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::loginCallback( RsslReactor *pRsslReactor, RsslReactorChannel *pRsslReactorChannel, RsslRDMLoginMsgEvent *pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->_pLoginCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::directoryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDirectoryMsgEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->_pDirectoryCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::dictionaryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDictionaryMsgEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->_pDictionaryCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::itemCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->_pItemCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::channelOpenCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->_pChannelCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

bool OmmConsumerImpl::hasOmmConnsumerErrorClient()
{
	return ( _ommConsumerErrorClient != 0 );
}

OmmConsumerErrorClient& OmmConsumerImpl::getOmmConsumerErrorClient()
{
	return *_ommConsumerErrorClient;
}

void OmmConsumerImpl::notifyOmmConsumerErrorClient( const OmmException& ommException, OmmConsumerErrorClient& errorClient )
{
	switch( ommException.getExceptionType() )
	{
	case OmmException::OmmSystemExceptionEnum :
		errorClient.onSystemError( static_cast<const OmmSystemException&>( ommException ).getSystemExceptionCode(), 
			static_cast<const OmmSystemException&>( ommException ).getSystemExceptionAddress(),
			ommException.getText() );
		break;
	case OmmException::OmmInvalidHandleExceptionEnum:
		errorClient.onInvalidHandle ( static_cast<const OmmInvalidHandleException&>( ommException ).getHandle(),
			ommException.getText() );
		break;
	case OmmException::OmmInvalidUsageExceptionEnum:
		errorClient.onInvalidUsage( ommException.getText() );
		break;
	case OmmException::OmmInaccessibleLogFileExceptionEnum:
		errorClient.onInaccessibleLogFile( static_cast<const OmmInaccessibleLogFileException&>( ommException ).getFilename(),
			ommException.getText() );
		break;
	case OmmException::OmmMemoryExhaustionExceptionEnum:
		errorClient.onMemoryExhaustion( ommException.getText() );
		break;
	}
}

void OmmConsumerImplMap::init()
{
#ifdef WIN32
	SetConsoleCtrlHandler( &OmmConsumerImplMap::TermHandlerRoutine, TRUE ) ;
#else
	bzero( &_sigAction, sizeof( _sigAction ) );
	bzero( &_oldSigAction, sizeof( _oldSigAction ) );

	_sigAction.sa_sigaction = sigAction;
	_sigAction.sa_flags = SA_SIGINFO;

	sigaction( SIGINT, &_sigAction, &_oldSigAction );
#endif
}

UInt64 OmmConsumerImplMap::push_back( OmmConsumerImpl* consumer )
{
	_listLock.lock();

	if ( _ommConsumerList.empty() )
		OmmConsumerImplMap::init();

	_ommConsumerList.push_back( consumer );

	UInt64 count = _ommConsumerList.size();

	++_id;

	_listLock.unlock();

	return _id;
}

void OmmConsumerImplMap::removeValue( OmmConsumerImpl* consumer )
{
	_listLock.lock();

	_ommConsumerList.remove( consumer );

	if ( !_ommConsumerList.empty() )
	{
		_listLock.unlock();
		return;
	}
#ifdef WIN32
	SetConsoleCtrlHandler( &OmmConsumerImplMap::TermHandlerRoutine, FALSE );
#else
	sigaction( SIGINT, &_oldSigAction, NULL );
#endif

	_listLock.unlock();
}

void OmmConsumerImplMap::atExit()
{
	_listLock.lock();

	OmmConsumerImpl* temp = _ommConsumerList.pop_back();

	while ( temp )
	{
		temp->setAtExit();
		temp->uninitialize();
		temp = _ommConsumerList.pop_back();
	}


#ifdef WIN32
	Sleep( 1000 );
#endif

#ifdef WIN32
	SetConsoleCtrlHandler( &OmmConsumerImplMap::TermHandlerRoutine, FALSE );
#else
	sigaction( SIGINT, &_oldSigAction, NULL );
#endif

	_listLock.unlock();
}

void
OmmConsumerImpl::terminateIf( void * object )
{
	OmmConsumerImpl * consumer = reinterpret_cast<OmmConsumerImpl *>( object );
	consumer->_eventTimedOut = true;
}

#ifdef WIN32
BOOL WINAPI OmmConsumerImplMap::TermHandlerRoutine( DWORD dwCtrlType )
{
	switch ( dwCtrlType )
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
	case CTRL_SHUTDOWN_EVENT:	
	case CTRL_C_EVENT:
		OmmConsumerImplMap::atExit();
		return TRUE;
	}
	return FALSE;
}
#else
extern "C" {
	void OmmConsumerImplMap::sigAction( int sig, siginfo_t* pSiginfo, void* pv ) 
	{	
		OmmConsumerImplMap::atExit();
	}
}
#endif 
