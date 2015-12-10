/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmBaseImpl.h"
#include "LoginCallbackClient.h"
#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "OmmClient.h"

#include "OmmInaccessibleLogFileException.h"
#include "OmmInvalidConfigurationException.h"
#include "OmmInvalidHandleException.h"
#include "OmmInvalidUsageException.h"
#include "OmmMemoryExhaustionException.h"
#include "OmmOutOfRangeException.h"
#include "OmmSystemException.h"
#include "OmmUnsupportedDomainTypeException.h"

#define	EMA_BIG_STR_BUFF_SIZE (1024*4)

using namespace thomsonreuters::ema::access;

#ifdef USING_POLL
int OmmBaseImpl::addFd( int fd, short events = POLLIN )
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

void OmmBaseImpl::removeFd( int fd )
{
  pipeReadEventFdsIdx = -1;
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

OmmBaseImpl::OmmBaseImpl( ActiveConfig& _a ) :
  _activeConfig( _a ),
  _userLock(),
  _pipeLock(),
  _reactorDispatchErrorInfo(),
  _reactorRetCode( RSSL_RET_SUCCESS ),
  _state( NotInitializedEnum ),
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
  userErrorHandler( 0 )  
{
}

OmmBaseImpl::OmmBaseImpl( ActiveConfig& _a, OmmConsumerErrorClient& client ) :
  _activeConfig( _a ),
 _userLock(),
 _pipeLock(),
 _reactorDispatchErrorInfo(),
  _reactorRetCode( RSSL_RET_SUCCESS ),
 _state( NotInitializedEnum ),
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
 _atExit( false )
{
  userErrorHandler = new UserErrorHandler( client );
}

OmmBaseImpl::OmmBaseImpl( ActiveConfig& _a, OmmNiProviderErrorClient& client ) :
  _activeConfig( _a ),
 _userLock(),
 _pipeLock(),
 _reactorDispatchErrorInfo(),
  _reactorRetCode( RSSL_RET_SUCCESS ),
 _state( NotInitializedEnum ),
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
 _atExit( false )
{
  userErrorHandler = new UserErrorHandler( client );
}

OmmBaseImpl::~OmmBaseImpl()
{
}

void OmmBaseImpl::readConfig( EmaConfigImpl* configImpl )
{
  UInt64 id( OmmImplMap< OmmBaseImpl >::add( this ) );
  _activeConfig.userName = configImpl->getUserName();
  _activeConfig.loggerConfig.loggerName = configImpl->getLoggerName( _activeConfig.userName );
  _activeConfig.dictionaryConfig.dictionaryName = configImpl->getDictionaryName( _activeConfig.userName );
  _activeConfig.instanceName = _activeConfig.userName;
  _activeConfig.instanceName.append("_").append(id);

  UInt32 maxUInt32( 0xFFFFFFFF );
  UInt64 tmp;
  EmaString userNodeName( configImpl->getUserNodeName() );
  userNodeName.append( _activeConfig.userName ).append( "|" );
  if ( configImpl->get<UInt64>( userNodeName + "ItemCountHint", tmp ) )
    _activeConfig.itemCountHint = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if (configImpl->get<UInt64>(userNodeName + "ServiceCountHint", tmp ) )
		_activeConfig.serviceCountHint = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( configImpl->get<UInt64>(userNodeName + "ObeyOpenWindow", tmp ) )
		_activeConfig.obeyOpenWindow = static_cast<UInt32>( tmp > 0 ? 1 : 0 );
	if ( configImpl->get<UInt64>(userNodeName + "PostAckTimeout", tmp ) )
		_activeConfig.postAckTimeout = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( configImpl->get<UInt64>(userNodeName + "RequestTimeout", tmp ) )
		_activeConfig.requestTimeout = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( configImpl->get< UInt64 >( userNodeName + "LoginRequestTimeOut", tmp ) )
		_activeConfig.loginRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( configImpl->get< UInt64 >( userNodeName + "DirectoryRequestTimeOut", tmp ) )
		_activeConfig.directoryRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( configImpl->get< UInt64 >( userNodeName + "DictionaryRequestTimeOut", tmp ) )
		_activeConfig.dictionaryRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( configImpl->get<UInt64>(userNodeName + "MaxOutstandingPosts", tmp ) )
		_activeConfig.maxOutstandingPosts = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );

	configImpl->get<Int64>( userNodeName + "DispatchTimeoutApiThread", _activeConfig.dispatchTimeoutApiThread );

	if ( configImpl->get<UInt64>( userNodeName + "CatchUnhandledException", tmp ) )
		_activeConfig.catchUnhandledException = static_cast<UInt32>( tmp > 0 ? true : false );
	if ( configImpl->get<UInt64>( userNodeName + "MaxDispatchCountApiThread", tmp ) )
		_activeConfig.maxDispatchCountApiThread = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	if ( configImpl->get<UInt64>(userNodeName + "MaxDispatchCountUserThread", tmp ) )
		_activeConfig.maxDispatchCountUserThread = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );
	configImpl->get<Int64>( userNodeName + "PipePort", _activeConfig.pipePort );

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
        if ( !configImpl->get< EmaString >( dictionaryNodeName + "Name", name ) )
		{
			EmaString errorMsg( "no configuration exists for consumer dictionary [" );
			errorMsg.append( dictionaryNodeName ).append( "]; will use dictionary defaults" );
            configImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
		}

		if ( !configImpl->get<Dictionary::DictionaryType>( dictionaryNodeName + "DictionaryType", _activeConfig.dictionaryConfig.dictionaryType ))
			_activeConfig.dictionaryConfig.dictionaryType = Dictionary::ChannelDictionaryEnum;

		if ( _activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum )
		{
			if ( !configImpl->get<EmaString>( dictionaryNodeName + "RdmFieldDictionaryFileName", _activeConfig.dictionaryConfig.rdmfieldDictionaryFileName))
				_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName.set( "./RDMFieldDictionary" );
			if ( !configImpl->get<EmaString>( dictionaryNodeName + "EnumTypeDefFileName", _activeConfig.dictionaryConfig.enumtypeDefFileName ) )
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
      if (!configImpl->get< EmaString >( loggerNodeName + "Name", name ) )
	{
	  EmaString errorMsg( "no configuration exists for consumer logger [" );
	  errorMsg.append( loggerNodeName ).append( "]; will use logger defaults" );
	  configImpl->appendConfigError( errorMsg, OmmLoggerClient::ErrorEnum );
	}

      configImpl->get<OmmLoggerClient::LoggerType>( loggerNodeName + "LoggerType", _activeConfig.loggerConfig.loggerType );

      if ( _activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum )
	configImpl->get<EmaString>( loggerNodeName + "FileName", _activeConfig.loggerConfig.loggerFileName );

      configImpl->get<OmmLoggerClient::Severity>(loggerNodeName + "LoggerSeverity", _activeConfig.loggerConfig.minLoggerSeverity);

      UInt64 idilo(0);
      if ( configImpl->get< UInt64 >( loggerNodeName + "IncludeDateInLoggerOutput", idilo) )
	_activeConfig.loggerConfig.includeDateInLoggerOutput = idilo == 1 ? true : false ;
    }
  else
    _activeConfig.loggerConfig.loggerName.set( "Logger" );

	EmaString channelSet;
	EmaString channelName = configImpl->getChannelName( _activeConfig.userName );

	if ( channelName.trimWhitespace().empty() )
	{
		EmaString nodeName("ConsumerGroup|ConsumerList|Consumer.");
                              nodeName.append( _activeConfig.userName );
                              nodeName.append("|ChannelSet");

		if( configImpl->get<EmaString>( nodeName, channelSet ))
		 {
			char *pToken = NULL;
			pToken = strtok(const_cast<char *>(channelSet.c_str()), ",");
			do {				
				if(pToken)
				{
					channelName = pToken;
					ChannelConfig *newChannelConfig= readChannelConfig(configImpl, (channelName.trimWhitespace()));
					_activeConfig.configChannelSet.push_back(newChannelConfig);

				}
				pToken = strtok(NULL, ",");

			} while(pToken != NULL);
		}
		else
		{
			EmaString channelName = "Channel";
			useDefaultConfigValues(channelName, configImpl->getUserSpecifiedHostname(), configImpl->getUserSpecifiedPort());
		}
	}
	else
	{
		ChannelConfig *newChannelConfig = readChannelConfig(configImpl, (channelName.trimWhitespace()));
		_activeConfig.configChannelSet.push_back(newChannelConfig);
	}
	
	if ( ProgrammaticConfigure *  const ppc  = configImpl->pProgrammaticConfigure() ) {
	  ppc->retrieveUserConfig( _activeConfig.userName, _activeConfig );
	  bool isProgmaticCfgChannelName = ppc->getActiveChannelName(_activeConfig.userName, channelName.trimWhitespace());
	  bool isProgramatiCfgChannelset = ppc->getActiveChannelSet(_activeConfig.userName, channelSet.trimWhitespace());
	  unsigned int posInProgCfg  = 0;

		if( isProgmaticCfgChannelName )
		{
			_activeConfig.clearChannelSet();
			ChannelConfig *fileChannelConfig = readChannelConfig(configImpl, (channelName.trimWhitespace()));
			ppc->retrieveChannelConfig( channelName.trimWhitespace(), _activeConfig, configImpl->getUserSpecifiedHostname().length() > 0, fileChannelConfig );
			if( !(ActiveConfig::findChannelConfig(_activeConfig.configChannelSet, channelName.trimWhitespace(), posInProgCfg) ) )
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
				ChannelConfig *fileChannelConfig = readChannelConfig(configImpl, (channelName.trimWhitespace()));
				ppc->retrieveChannelConfig( channelName.trimWhitespace(), _activeConfig, configImpl->getUserSpecifiedHostname().length() > 0, fileChannelConfig );
				if( !(ActiveConfig::findChannelConfig(_activeConfig.configChannelSet, channelName.trimWhitespace(), posInProgCfg) ) )
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
		useDefaultConfigValues(channelName, configImpl->getUserSpecifiedHostname(), configImpl->getUserSpecifiedPort());
	}
	_activeConfig.userDispatch = configImpl->getOperationModel();
	_activeConfig.pRsslRDMLoginReq = configImpl->getLoginReq();
	_activeConfig.pRsslDirectoryRequestMsg = configImpl->getDirectoryReq();
	_activeConfig.pRsslEnumDefRequestMsg = configImpl->getEnumDefDictionaryReq();
	_activeConfig.pRsslRdmFldRequestMsg = configImpl->getRdmFldDictionaryReq();
	catchUnhandledException( _activeConfig.catchUnhandledException );
}

void OmmBaseImpl::useDefaultConfigValues( const EmaString &channelName, const EmaString &host, const EmaString &port )
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

ChannelConfig*  OmmBaseImpl::readChannelConfig(EmaConfigImpl* pConfigImpl, const EmaString&  channelName)
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

	tempUInt = 1;
	pConfigImpl->get<UInt64>( channelNodeName + "MsgKeyInUpdates", tempUInt );
	if ( tempUInt == 0 )
		newChannelConfig->msgKeyInUpdates = false;	

	return newChannelConfig;
}

bool OmmBaseImpl::readReliableMcastConfig(EmaConfigImpl* pConfigImpl, const EmaString& channNodeName, ReliableMcastChannelConfig *relMcastChannelCfg, EmaString& errorText)
{
	EmaString channelNodeName(channNodeName);

	pConfigImpl->get<EmaString>( channelNodeName + "RecvAddress", relMcastChannelCfg->recvAddress);
	if(	 relMcastChannelCfg->recvAddress.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [RecvAddress]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "RecvServiceName", relMcastChannelCfg->recvServiceName);
	if( relMcastChannelCfg->recvServiceName.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [RecvServiceName]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "UnicastServiceName", relMcastChannelCfg->unicastServiceName);
	if( relMcastChannelCfg->unicastServiceName.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [UnicastServiceName]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "SendAddress", relMcastChannelCfg->sendAddress);
	if( relMcastChannelCfg->sendAddress.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [SendAddress]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "SendServiceName", relMcastChannelCfg->sendServiceName);
	if( relMcastChannelCfg->sendServiceName.empty())
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [SendServiceName]." );
		return false;
	}

	UInt64 tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "DisconnectOnGap", tempUIntval );
	relMcastChannelCfg->disconnectOnGap = (tempUIntval) ? true : false;
		
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "PacketTTL", tempUIntval );
	relMcastChannelCfg->setPacketTTL(tempUIntval);
			
	pConfigImpl->get<EmaString>( channelNodeName + "HsmInterface", relMcastChannelCfg->hsmInterface);
	pConfigImpl->get<EmaString>( channelNodeName + "HsmMultAddress", relMcastChannelCfg->hsmMultAddress);
	pConfigImpl->get<EmaString>( channelNodeName + "HsmPort", relMcastChannelCfg->hsmPort);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "HsmInterval", tempUIntval );
	relMcastChannelCfg->setHsmInterval(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "ndata", tempUIntval );
	relMcastChannelCfg->setNdata(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "nmissing", tempUIntval );
	relMcastChannelCfg->setNmissing(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "nrreq", tempUIntval );
	relMcastChannelCfg->setNrreq(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "tdata", tempUIntval );
	relMcastChannelCfg->setTdata(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "trreq", tempUIntval );
	relMcastChannelCfg->setTrreq(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "pktPoolLimitHigh", tempUIntval );
	relMcastChannelCfg->setPktPoolLimitHigh(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "pktPoolLimitLow", tempUIntval );
	relMcastChannelCfg->setPktPoolLimitLow(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "twait", tempUIntval );
	relMcastChannelCfg->setTwait(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "tbchold", tempUIntval );
	relMcastChannelCfg->setTbchold(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "tpphold", tempUIntval );
	relMcastChannelCfg->setTpphold(tempUIntval);
	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "userQLimit", tempUIntval );
	relMcastChannelCfg->setUserQLimit(tempUIntval);

	return true;
}

void OmmBaseImpl::initialize( EmaConfigImpl* configImpl )
{
	try {
		_userLock.lock();

		readConfig( configImpl );

		_pLoggerClient = OmmLoggerClient::create( _activeConfig );

		configImpl->configErrors().log( _pLoggerClient, _activeConfig.loggerConfig.minLoggerSeverity );

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

		_state = RsslInitilizedEnum;

		RsslCreateReactorOptions reactorOpts;
		RsslErrorInfo rsslErrorInfo;

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

		_state = ReactorInitializedEnum;

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
		TimeOut * loginWatcher( new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmBaseImpl::terminateIf, reinterpret_cast< void * >( this ), true ) );
		while ( ! _atExit && ! _eventTimedOut && ( _state < LoginStreamOpenOkEnum ) )
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
		else
			loginWatcher->cancel();
	
		downloadDirectory();
		downloadDictionary();

		if ( !_atExit && _activeConfig.userDispatch == OmmConsumerConfig::ApiDispatchEnum ) start();

		_userLock.unlock();
	}
	catch ( const OmmException& ommException )
	{
		_userLock.unlock();

 		uninitialize();

		if ( hasUserErrorHandler() )
			notifyUserErrorHandler( ommException, getUserErrorHandler() );
		else
			throw;
	}
}

void OmmBaseImpl::setDispatchInternalMsg()
{
	_dispatchInternalMsg = true;
	pipeWrite();
}

void OmmBaseImpl::pipeWrite()
{
	_pipeLock.lock();

	if ( ++_pipeWriteCount == 1 )
		_pipe.write( "0", 1 );

	_pipeLock.unlock();
}

void OmmBaseImpl::pipeRead()
{
	_pipeLock.lock();

	if ( --_pipeWriteCount == 0 )
	{
		char temp[10];
		_pipe.read( temp, 1 );
	}

	_pipeLock.unlock();
}

void OmmBaseImpl::cleanUp()
{
	uninitialize( true );
}

void OmmBaseImpl::uninitialize( bool caughtExcep )
{
  OmmImplMap< OmmBaseImpl >::remove( this );

	_userLock.lock();

	if ( _state == NotInitializedEnum )
	{
		_userLock.unlock();
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

	_state = NotInitializedEnum;

	_userLock.unlock();
	
#ifdef USING_POLL
	delete[] _eventFds;
#endif
}

void OmmBaseImpl::setAtExit()
{
	_atExit = true;
	pipeWrite();
}

bool OmmBaseImpl::rsslReactorDispatchLoop( Int64 timeOut, UInt32 count )
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
					.append( "' Error text='" ).append( _reactorDispatchErrorInfo.rsslError.text ).append( "'. " );

				_userLock.lock();
				if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				_userLock.unlock();
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

				_userLock.lock();
				if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				_userLock.unlock();
			}
		}
#else
		if ( errno != EINTR )
		{
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal Error. Call to select() failed. LastError='" );
				temp.append( errno ).append( "'. " );

				_userLock.lock();
				if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
				_userLock.unlock();
			}
		}
#endif
		return false;
	}

	return true;
}

void OmmBaseImpl::setState( ImplState state )
{
	_state = state;
}

void OmmBaseImpl::closeChannel( RsslReactorChannel* pRsslReactorChannel )
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

			_userLock.lock();

			if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			
			_userLock.unlock();
		}
	}

	_pChannelCallbackClient->removeChannel( pRsslReactorChannel );
}

ItemCallbackClient& OmmBaseImpl::getItemCallbackClient()
{
	return *_pItemCallbackClient;
}

DictionaryCallbackClient& OmmBaseImpl::getDictionaryCallbackClient()
{
	return *_pDictionaryCallbackClient;
}

DirectoryCallbackClient& OmmBaseImpl::getDirectoryCallbackClient()
{
	return *_pDirectoryCallbackClient;
}

LoginCallbackClient& OmmBaseImpl::getLoginCallbackClient()
{
	return *_pLoginCallbackClient;
}

ChannelCallbackClient& OmmBaseImpl::getChannelCallbackClient()
{
	return *_pChannelCallbackClient;
}

OmmLoggerClient& OmmBaseImpl::getOmmLoggerClient()
{
	return *_pLoggerClient;
}

void OmmBaseImpl::reissue( const ReqMsg& reqMsg, UInt64 handle )
{
	_userLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->reissue( reqMsg, handle );

	_userLock.unlock();
}

void OmmBaseImpl::unregister( UInt64 handle )
{
	_userLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->unregister( handle );

	_userLock.unlock();
}

void OmmBaseImpl::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	_userLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->submit( genericMsg, handle );

	_userLock.unlock();
}

void OmmBaseImpl::submit( const PostMsg& postMsg, UInt64 handle )
{
	_userLock.lock();

	if ( _pItemCallbackClient ) _pItemCallbackClient->submit( postMsg, handle );

	_userLock.unlock();
}

ActiveConfig& OmmBaseImpl::getActiveConfig()
{
	return _activeConfig;
}

void OmmBaseImpl::run()
{
  while ( !Thread::isStopping() && !_atExit )
    rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );
}

int OmmBaseImpl::runLog( void* pExceptionStructure, const char* file, unsigned int line )
{
	char reportBuf[EMA_BIG_STR_BUFF_SIZE*10];
	if ( retrieveExceptionContext( pExceptionStructure, file, line, reportBuf, EMA_BIG_STR_BUFF_SIZE*10 ) > 0 )
	{
		_userLock.lock();
		if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, reportBuf );
		_userLock.unlock();
	}

	return 1;
}

RsslReactorCallbackRet OmmBaseImpl::channelCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pEvent )
{
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getChannelCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::loginCallback( RsslReactor *pRsslReactor, RsslReactorChannel *pRsslReactorChannel, RsslRDMLoginMsgEvent *pEvent )
{
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getLoginCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::directoryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDirectoryMsgEvent* pEvent )
{
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getDirectoryCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::dictionaryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDictionaryMsgEvent* pEvent )
{
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getDictionaryCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::itemCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->_pItemCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::channelOpenCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pEvent )
{
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getChannelCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

const EmaString& OmmBaseImpl::getUserName() const
{
  return _activeConfig.instanceName;
}

void OmmBaseImpl::notifyUserErrorHandler( const OmmException& ommException, UserErrorHandler& errorClient )
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

void OmmBaseImpl::terminateIf( void* object )
{
	OmmBaseImpl * user = reinterpret_cast<OmmBaseImpl*>( object );
	user->_eventTimedOut = true;
}
