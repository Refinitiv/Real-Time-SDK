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
#include "TunnelStreamRequest.h"

#define	EMA_BIG_STR_BUFF_SIZE (1024*4)
using namespace thomsonreuters::ema::access;

EmaVector< OmmConsumerImpl* > OmmConsumerImplMap::_ommConsumerList;
Mutex OmmConsumerImplMap::_listLock;
UInt64 OmmConsumerImplMap::_id = 0;
bool OmmConsumerImplMap::_clearSigHandler = true;

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
	clearRsslErrorInfo( &_reactorDispatchErrorInfo );
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
	clearRsslErrorInfo( &_reactorDispatchErrorInfo );
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

	UInt64 id = OmmConsumerImplMap::add( this );

	_activeConfig.consumerName = pConfigImpl->getConsumerName();

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
	
	EmaString channelSet;
	EmaString channelName = pConfigImpl->getChannelName( _activeConfig.consumerName );

	if ( channelName.trimWhitespace().empty() )
	{
		EmaString nodeName("ConsumerGroup|ConsumerList|Consumer.");
                              nodeName.append( _activeConfig.consumerName );
                              nodeName.append("|ChannelSet");

		if( pConfigImpl->get<EmaString>( nodeName, channelSet ))
		 {
			char *pToken = NULL;
			pToken = strtok(const_cast<char *>(channelSet.c_str()), ",");
			do {				
				if(pToken)
				{
					channelName = pToken;
					ChannelConfig *newChannelConfig= readChannelConfig(pConfigImpl, (channelName.trimWhitespace()));
					_activeConfig.configChannelSet.push_back(newChannelConfig);

				}
				pToken = strtok(NULL, ",");

			} while(pToken != NULL);
		}
		else
		{
			EmaString channelName = "Channel";
			useDefaultConfigValues(channelName, pConfigImpl->getUserSpecifiedHostname(), pConfigImpl->getUserSpecifiedPort());
		}
	}
	else
	{
		ChannelConfig *newChannelConfig = readChannelConfig(pConfigImpl, (channelName.trimWhitespace()));
		_activeConfig.configChannelSet.push_back(newChannelConfig);
	}
	
	if ( ProgrammaticConfigure *  const ppc  = pConfigImpl->pProgrammaticConfigure() ) {
		ppc->retrieveConsumerConfig( _activeConfig.consumerName, _activeConfig );
		bool isProgmaticCfgChannelName = ppc->getActiveChannelName(_activeConfig.consumerName, channelName.trimWhitespace());
		bool isProgramatiCfgChannelset = ppc->getActiveChannelSet(_activeConfig.consumerName, channelSet.trimWhitespace());
		unsigned int posInProgCfg  = 0;

		if( isProgmaticCfgChannelName )
		{
			_activeConfig.clearChannelSet();
			ChannelConfig *fileChannelConfig = readChannelConfig(pConfigImpl, (channelName.trimWhitespace()));
			ppc->retrieveChannelConfig( channelName.trimWhitespace(), _activeConfig, pConfigImpl->getUserSpecifiedHostname().length() > 0, fileChannelConfig );
			if( !(OmmConsumerActiveConfig::findChannelConfig(_activeConfig.configChannelSet, channelName.trimWhitespace(), posInProgCfg) ) )
				_activeConfig.configChannelSet.push_back( fileChannelConfig );
			else
			{
				if( fileChannelConfig )
					delete fileChannelConfig;
			}
		}
		else if(isProgramatiCfgChannelset)
		{
			_activeConfig.configChannelSet.clear();
			char *pToken = NULL;
			pToken = strtok(const_cast<char *>(channelSet.c_str()), ",");
			while(pToken != NULL)
			{
				channelName = pToken;
				ChannelConfig *fileChannelConfig = readChannelConfig(pConfigImpl, (channelName.trimWhitespace()));
				ppc->retrieveChannelConfig( channelName.trimWhitespace(), _activeConfig, pConfigImpl->getUserSpecifiedHostname().length() > 0, fileChannelConfig );
				if( !(OmmConsumerActiveConfig::findChannelConfig(_activeConfig.configChannelSet, channelName.trimWhitespace(), posInProgCfg) ) )
					_activeConfig.configChannelSet.push_back( fileChannelConfig );
				else
				{
					if( fileChannelConfig )
						delete fileChannelConfig;
				}

				pToken = strtok(NULL, ",");
			}
		}

		ppc->retrieveLoggerConfig( _activeConfig.loggerConfig.loggerName , _activeConfig );
		ppc->retrieveDictionaryConfig( _activeConfig.dictionaryConfig.dictionaryName, _activeConfig );
	}

	if( _activeConfig.configChannelSet.size() == 0 ) {
		EmaString channelName("Channel");
		useDefaultConfigValues(channelName, pConfigImpl->getUserSpecifiedHostname(), pConfigImpl->getUserSpecifiedPort());
	}
	_activeConfig.userDispatch = pConfigImpl->getOperationModel();
	_activeConfig.pRsslRDMLoginReq = pConfigImpl->getLoginReq();
	_activeConfig.pRsslDirectoryRequestMsg = pConfigImpl->getDirectoryReq();
	_activeConfig.pRsslEnumDefRequestMsg = pConfigImpl->getEnumDefDictionaryReq();
	_activeConfig.pRsslRdmFldRequestMsg = pConfigImpl->getRdmFldDictionaryReq();
	catchUnhandledException( _activeConfig.catchUnhandledException );
}

void OmmConsumerImpl::useDefaultConfigValues( const EmaString &channelName, const EmaString &host, const EmaString &port )
{
	SocketChannelConfig *newChannelConfig =  0;
	try {
		newChannelConfig = new SocketChannelConfig();
		if ( host.length() )
			newChannelConfig->hostName = host;
		if ( port.length() )
			newChannelConfig->serviceName = port;
		newChannelConfig->name.set( channelName );
		_activeConfig.configChannelSet.push_back(newChannelConfig);
	}
	catch ( std::bad_alloc )
	{
		const char* temp( "Failed to allocate memory for SocketChannelConfig." );
		throwMeeException( temp );
		return;
	}
}

ChannelConfig*  OmmConsumerImpl::readChannelConfig(OmmConsumerConfigImpl* pConfigImpl, const EmaString&  channelName)
{
	ChannelConfig *newChannelConfig = NULL;
	UInt32 maxUInt32( 0xFFFFFFFF );
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
			SocketChannelConfig *socketChannelCfg = NULL;
			try {
				socketChannelCfg = new SocketChannelConfig();
				newChannelConfig = socketChannelCfg;
			}
			catch ( std::bad_alloc )
			{
				const char* temp( "Failed to allocate memory for SocketChannelConfig. (std::bad_alloc)" );
				throwMeeException( temp );
				return 0;
			}

			if ( !socketChannelCfg)
			{
				const char* temp = "Failed to allocate memory for SocketChannelConfig. (null ptr)";
				throwMeeException(temp);
				return 0;
			}

			EmaString  tmp = pConfigImpl->getUserSpecifiedHostname() ;
			if ( tmp.length() )
				socketChannelCfg->hostName = tmp;
			else
				pConfigImpl->get< EmaString >( channelNodeName + "Host", socketChannelCfg->hostName );
				
			tmp = pConfigImpl->getUserSpecifiedPort();
			if ( tmp.length() )
				socketChannelCfg->serviceName = tmp;
			else
				pConfigImpl->get< EmaString >( channelNodeName + "Port", socketChannelCfg->serviceName );

			UInt64 tempUInt = 1;
			pConfigImpl->get<UInt64>( channelNodeName + "TcpNodelay", tempUInt );
			if ( tempUInt )
				socketChannelCfg->tcpNodelay = RSSL_TRUE;
			else
				socketChannelCfg->tcpNodelay = RSSL_FALSE;
			
			break;
		}
	case RSSL_CONN_TYPE_HTTP:
		{
			HttpChannelConfig *httpChannelCfg = NULL;
			try {
				httpChannelCfg = new HttpChannelConfig();
				newChannelConfig = httpChannelCfg;
			}
			catch ( std::bad_alloc )
			{
				const char* temp( "Failed to allocate memory for HttpChannelConfig. (std::bad_alloc)" );
				throwMeeException( temp );
				return 0;
			}

			if ( !httpChannelCfg )
			{
				const char* temp = "Failed to allocate memory for HttpChannelConfig. (null ptr)";
				throwMeeException(temp);
				return 0;
			}

			pConfigImpl->get<EmaString>(channelNodeName + "Host", httpChannelCfg->hostName);

			pConfigImpl->get<EmaString>(channelNodeName + "Port", httpChannelCfg->serviceName);

			UInt64 tempUInt = 1;
			pConfigImpl->get<UInt64>(channelNodeName + "TcpNodelay", tempUInt);
			if (!tempUInt)
				httpChannelCfg->tcpNodelay = RSSL_FALSE;
			else
				httpChannelCfg->tcpNodelay = RSSL_TRUE;

			pConfigImpl->get<EmaString>(channelNodeName + "ObjectName", httpChannelCfg->objectName);

			break;
		}
	case RSSL_CONN_TYPE_ENCRYPTED:
		{
			EncryptedChannelConfig *encriptedChannelCfg = NULL;
			try {
				encriptedChannelCfg = new EncryptedChannelConfig();
				newChannelConfig = encriptedChannelCfg;
			}
			catch ( std::bad_alloc )
			{
				const char* temp( "Failed to allocate memory for EncryptedChannelConfig. (std::bad_alloc)" );
				throwMeeException( temp );
				return 0;
			}

			if ( !newChannelConfig)
			{
				const char* temp = "Failed to allocate memory for EncryptedChannelConfig. (null ptr)";
				throwMeeException(temp);
				return 0;
			}

			pConfigImpl->get<EmaString>( channelNodeName + "Host", encriptedChannelCfg->hostName);

			pConfigImpl->get<EmaString>( channelNodeName + "Port", encriptedChannelCfg->serviceName);

			UInt64 tempUInt = 1;
			pConfigImpl->get<UInt64>( channelNodeName + "TcpNodelay", tempUInt );
			if ( tempUInt )
				encriptedChannelCfg->tcpNodelay = RSSL_TRUE;
			else
				encriptedChannelCfg->tcpNodelay = RSSL_FALSE;

			pConfigImpl->get<EmaString>(channelNodeName + "ObjectName", encriptedChannelCfg->objectName);

			break;
		}
	case RSSL_CONN_TYPE_RELIABLE_MCAST:
	{
			ReliableMcastChannelConfig *relMcastChannelCfg = NULL;
			try {
				relMcastChannelCfg = new ReliableMcastChannelConfig();
				newChannelConfig = relMcastChannelCfg;
			}
			catch ( std::bad_alloc )
			{
				const char* temp( "Failed to allocate memory for ReliableMcastChannelConfig. (std::bad_alloc)" );
				throwMeeException( temp );
				return 0;
			}

			if ( !newChannelConfig)
			{
				const char* temp = "Failed to allocate memory for ReliableMcastChannelConfig. (null ptr)";
				throwMeeException(temp);
				return 0;
			}
			EmaString errorMsg;
			if(!readReliableMcastConfig(pConfigImpl, channelNodeName, relMcastChannelCfg, errorMsg))
			{
				throwIceException(errorMsg);
				return 0;
			}
			break;
		}
	default:
		{
		EmaString temp( "Not supported channel type. Type = " );
		temp.append( (UInt32)channelType );
		throwIueException(temp);
		return 0;
		}
	}
	

	newChannelConfig->name = channelName;

	pConfigImpl->get<EmaString>( channelNodeName + "InterfaceName", newChannelConfig->interfaceName );

	UInt64 tempUInt = 0;
	if(channelType != RSSL_CONN_TYPE_RELIABLE_MCAST)
	{
		pConfigImpl->get<RsslCompTypes>( channelNodeName + "CompressionType", newChannelConfig->compressionType );
	
		tempUInt = 0;
		if (pConfigImpl->get<UInt64>( channelNodeName + "CompressionThreshold", tempUInt ) )
		{
			newChannelConfig->compressionThreshold = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;
		}		
	}

	tempUInt = 0;
	if (pConfigImpl->get<UInt64>( channelNodeName + "GuaranteedOutputBuffers", tempUInt ) )
	{
		newChannelConfig->setGuaranteedOutputBuffers( tempUInt );
	}

	tempUInt = 0;
	if (pConfigImpl->get<UInt64>( channelNodeName + "NumInputBuffers", tempUInt ) )
	{
		newChannelConfig->setNumInputBuffers( tempUInt );
	}

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "ConnectionPingTimeout", tempUInt ) )
	{
		newChannelConfig->connectionPingTimeout = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;
	}

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "SysRecvBufSize", tempUInt ) )
	{
		newChannelConfig->sysRecvBufSize = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;
	}

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "SysSendBufSize", tempUInt ) )
	{
		newChannelConfig->sysSendBufSize = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;
	}

	Int64 tempInt = -1;
	if ( pConfigImpl->get<Int64>( channelNodeName + "ReconnectAttemptLimit", tempInt ) )
	{
		newChannelConfig->setReconnectAttemptLimit( tempInt );
	}

	tempInt = 0;
	if ( pConfigImpl->get<Int64>( channelNodeName + "ReconnectMinDelay", tempInt ) )
	{
		newChannelConfig->setReconnectMinDelay( tempInt );
	}
		
	tempInt = 0;
	if ( pConfigImpl->get<Int64>( channelNodeName + "ReconnectMaxDelay", tempInt ) )
	{
		newChannelConfig->setReconnectMaxDelay( tempInt );
	}

	pConfigImpl->get<EmaString>( channelNodeName + "XmlTraceFileName", newChannelConfig->xmlTraceFileName );

	tempInt = 0;
	pConfigImpl->get<Int64>( channelNodeName + "XmlTraceMaxFileSize", tempInt );
	if ( tempInt > 0 )
		newChannelConfig->xmlTraceMaxFileSize = tempInt;

	tempUInt = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceToFile", tempUInt );
	if ( tempUInt > 0 )
		newChannelConfig->xmlTraceToFile = true;

	tempUInt = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceToStdout", tempUInt );
	if ( tempUInt > 0 )
		newChannelConfig->xmlTraceToStdout = true;

	tempUInt = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceToMultipleFiles", tempUInt );
	if ( tempUInt > 0 )
		newChannelConfig->xmlTraceToMultipleFiles = true;

	tempUInt = 1;
	pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceWrite", tempUInt );
	if (tempUInt == 0)
		newChannelConfig->xmlTraceWrite = false;

	tempUInt = 1;
	pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceRead", tempUInt );
	if ( tempUInt == 0 )
		newChannelConfig->xmlTraceRead = false;

	tempUInt = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "XmlTracePing", tempUInt );
	newChannelConfig->xmlTracePing = tempUInt == 0 ? false : true;

	tempUInt = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "XmlTraceHex", tempUInt );
	newChannelConfig->xmlTraceHex = tempUInt == 0 ? false : true;

	tempUInt = 1;
	pConfigImpl->get<UInt64>( channelNodeName + "MsgKeyInUpdates", tempUInt );
	if ( tempUInt == 0 )
		newChannelConfig->msgKeyInUpdates = false;	

	return newChannelConfig;
}

bool OmmConsumerImpl::readReliableMcastConfig(OmmConsumerConfigImpl* pConfigImpl, const EmaString& channNodeName, ReliableMcastChannelConfig *relMcastChannelCfg, EmaString& errorText)
{
	EmaString channelNodeName(channNodeName);

	pConfigImpl->get<EmaString>( channelNodeName + "RecvAddress", relMcastChannelCfg->recvAddress);
	if(	 relMcastChannelCfg->recvAddress.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [RecvAddress]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "RecvPort", relMcastChannelCfg->recvServiceName);
	if( relMcastChannelCfg->recvServiceName.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [RecvPort]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "UnicastPort", relMcastChannelCfg->unicastServiceName);
	if( relMcastChannelCfg->unicastServiceName.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [UnicastPort]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "SendAddress", relMcastChannelCfg->sendAddress);
	if( relMcastChannelCfg->sendAddress.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [SendAddress]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "SendPort", relMcastChannelCfg->sendServiceName);
	if( relMcastChannelCfg->sendServiceName.empty())
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [SendPort]." );
		return false;
	}

	UInt64 tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "DisconnectOnGap", tempUIntval ) )
		relMcastChannelCfg->disconnectOnGap = (tempUIntval) ? true : false;
		
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "PacketTTL", tempUIntval ) )
		relMcastChannelCfg->setPacketTTL(tempUIntval);
			
	pConfigImpl->get<EmaString>( channelNodeName + "HsmInterface", relMcastChannelCfg->hsmInterface);
	pConfigImpl->get<EmaString>( channelNodeName + "HsmMultAddress", relMcastChannelCfg->hsmMultAddress);
	pConfigImpl->get<EmaString>( channelNodeName + "HsmPort", relMcastChannelCfg->hsmPort);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "HsmInterval", tempUIntval ) )
		relMcastChannelCfg->setHsmInterval(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "ndata", tempUIntval ) )
		relMcastChannelCfg->setNdata(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "nmissing", tempUIntval ) )
		relMcastChannelCfg->setNmissing(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "nrreq", tempUIntval ) )
		relMcastChannelCfg->setNrreq(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "tdata", tempUIntval ) )
		relMcastChannelCfg->setTdata(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "trreq", tempUIntval ) )
		relMcastChannelCfg->setTrreq(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "pktPoolLimitHigh", tempUIntval ) )
		relMcastChannelCfg->setPktPoolLimitHigh(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "pktPoolLimitLow", tempUIntval ) )
		relMcastChannelCfg->setPktPoolLimitLow(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "twait", tempUIntval ) )
		relMcastChannelCfg->setTwait(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "tbchold", tempUIntval ) )
		relMcastChannelCfg->setTbchold(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "tpphold", tempUIntval ) )
		relMcastChannelCfg->setTpphold(tempUIntval);
	tempUIntval = 0;
	if( pConfigImpl->get<UInt64>( channelNodeName + "userQLimit", tempUIntval ) )
		relMcastChannelCfg->setUserQLimit(tempUIntval);
	pConfigImpl->get<EmaString>( channelNodeName + "tcpControlPort", relMcastChannelCfg->tcpControlPort);

	return true;
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
		clearRsslErrorInfo( &rsslErrorInfo );

		rsslClearCreateReactorOptions( &reactorOpts );

		reactorOpts.userSpecPtr = (void*)this;

		_pRsslReactor = rsslCreateReactor( &reactorOpts, &rsslErrorInfo );
		if ( !_pRsslReactor )
		{
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
		TimeOut * loginWatcher( new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmConsumerImpl::terminateIf, reinterpret_cast< void * >( this ), true ) );
		while ( ! _atExit && ! _eventTimedOut &&
			( _ommConsumerState < LoginStreamOpenOkEnum ) &&
			( _ommConsumerState != RsslChannelUpStreamNotOpenEnum ) )
			rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );
		
		ChannelConfig *pChannelcfg = _activeConfig.configChannelSet[0];

		if ( _eventTimedOut )
		{
			EmaString failureMsg( "login failed (timed out after waiting " );
			failureMsg.append( _activeConfig.loginRequestTimeOut ).append( " milliseconds) for " );
			if ( pChannelcfg->getType() == ChannelConfig::SocketChannelEnum )
			{
				SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( pChannelcfg ) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}
			else if (pChannelcfg->getType() == ChannelConfig::HttpChannelEnum )
			{
				HttpChannelConfig * channelConfig( reinterpret_cast< HttpChannelConfig * >( pChannelcfg ) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}
			else if ( pChannelcfg->getType() == ChannelConfig::EncryptedChannelEnum )
			{
				EncryptedChannelConfig * channelConfig( reinterpret_cast< EncryptedChannelConfig * >(pChannelcfg) );
				failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
			}

			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg);
			throwIueException( failureMsg );
			return;
		}
		else if ( _ommConsumerState == RsslChannelUpStreamNotOpenEnum )
		{
			throwIueException( getLoginCallbackClient().getLoginFailureMessage() );
			return;
		}
		else
			loginWatcher->cancel();

		timeOutLengthInMicroSeconds = _activeConfig.directoryRequestTimeOut * 1000;
		_eventTimedOut = false;
		loginWatcher = new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmConsumerImpl::terminateIf, reinterpret_cast< void * >( this ), true );
		while ( ! _atExit && ! _eventTimedOut && ( _ommConsumerState < DirectoryStreamOpenOkEnum ) )
			rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );

		if ( _eventTimedOut )
		{
			EmaString failureMsg( "directory retrieval failed (timed out after waiting " );
			failureMsg.append( _activeConfig.directoryRequestTimeOut ).append( " milliseconds) for " );
			if ( pChannelcfg->getType() == ChannelConfig::SocketChannelEnum )
			{
				SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( pChannelcfg ) );
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
		loginWatcher = new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmConsumerImpl::terminateIf, reinterpret_cast< void * >( this ), true );
		while ( !_atExit && ! _eventTimedOut && !_pDictionaryCallbackClient->isDictionaryReady() )
			rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );

		if ( _eventTimedOut )
		{
			EmaString failureMsg( "dictionary retrieval failed (timed out after waiting " );
			failureMsg.append( _activeConfig.dictionaryRequestTimeOut ).append( " milliseconds) for " );
			if ( pChannelcfg->getType() == ChannelConfig::SocketChannelEnum )
			{
				SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( pChannelcfg ) );
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

void OmmConsumerImpl::cleanUp()
{
	uninitialize( true );
}

void OmmConsumerImpl::uninitialize( bool caughtExcep )
{
	OmmConsumerImplMap::remove( this );

	_consumerLock.lock();

	if ( _ommConsumerState == NotInitializedEnum )
	{
		_consumerLock.unlock();
		return;
	}

	if ( _activeConfig.userDispatch == OmmConsumerConfig::ApiDispatchEnum && !caughtExcep )
	{
		pipeWrite();
		stop();
		wait();
	}

	pipeRead();

	if ( _pRsslReactor )
	{
		if ( _pLoginCallbackClient && !caughtExcep )
			rsslReactorDispatchLoop( 10000, _pLoginCallbackClient->sendLoginClose() );

		if ( _pChannelCallbackClient )
			_pChannelCallbackClient->closeChannels();

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );
		
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
			selectTime.tv_sec = static_cast< long >( timeOut / 1000000 );
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
					.append( "' Error Id " ).append( _reactorDispatchErrorInfo.rsslError.rsslErrorId ).append( "' " )
					.append( "' Error Location='" ).append( _reactorDispatchErrorInfo.errorLocation ).append( "' " )
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
	clearRsslErrorInfo( &rsslErrorInfo );

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
										void* closure, UInt64 parentHandle )
{
	_consumerLock.lock();

	UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( reqMsg, ommConsClient, closure, parentHandle ) : 0;

	_consumerLock.unlock();

	return handle;
}

UInt64 OmmConsumerImpl::registerClient( const TunnelStreamRequest& tunnelStreamRequest,
									   OmmConsumerClient& ommConsClient, void* closure )
{
	_consumerLock.lock();

	UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( tunnelStreamRequest, ommConsClient, closure ) : 0;

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
	if ( _activeConfig.userDispatch == OmmConsumerConfig::UserDispatchEnum && !_atExit )
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
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->getChannelCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::loginCallback( RsslReactor *pRsslReactor, RsslReactorChannel *pRsslReactorChannel, RsslRDMLoginMsgEvent *pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->getLoginCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::directoryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDirectoryMsgEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->getDirectoryCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::dictionaryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDictionaryMsgEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->getDictionaryCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::itemCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->_pItemCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::channelOpenCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pEvent )
{
	return static_cast<OmmConsumerImpl*>( pRsslReactor->userSpecPtr )->getChannelCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamStatusEventCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pTunnelStreamStatusEvent )
{
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getOmmConsumerImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamStatusEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamDefaultMsgCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent )
{
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getOmmConsumerImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamMsgEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamQueueMsgCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamQueueMsgEvent* pTunnelStreamQueueMsgEvent )
{
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getOmmConsumerImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamQueueMsgEvent );
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

	_clearSigHandler = false;
}

UInt64 OmmConsumerImplMap::add( OmmConsumerImpl* consumer )
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

void OmmConsumerImplMap::remove( OmmConsumerImpl* consumer )
{
	_listLock.lock();

	_ommConsumerList.removeValue( consumer );

	if ( !_ommConsumerList.empty() || _clearSigHandler )
	{
		_listLock.unlock();
		return;
	}
#ifdef WIN32
	SetConsoleCtrlHandler( &OmmConsumerImplMap::TermHandlerRoutine, FALSE );
#else
	sigaction( SIGINT, &_oldSigAction, NULL );
#endif

	_clearSigHandler = true;

	_listLock.unlock();
}

void OmmConsumerImplMap::atExit()
{
	_listLock.lock();

	UInt32 size = _ommConsumerList.size();

	while ( size )
	{
		OmmConsumerImpl* temp = _ommConsumerList[size - 1];
		temp->setAtExit();
		temp->uninitialize();
		size = _ommConsumerList.size();
	}

	_listLock.unlock();
}

void OmmConsumerImpl::terminateIf( void* object )
{
	OmmConsumerImpl * consumer = reinterpret_cast<OmmConsumerImpl*>( object );
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
		break;
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
