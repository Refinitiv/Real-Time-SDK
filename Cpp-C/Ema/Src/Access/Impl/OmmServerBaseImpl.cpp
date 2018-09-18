/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#include "OmmServerBaseImpl.h"
#include "LoginHandler.h"
#include "DictionaryHandler.h"
#include "DirectoryHandler.h"
#include "MarketItemHandler.h"
#include "ServerChannelHandler.h"
#include "EmaConfigImpl.h"
#include "ItemInfo.h"
#include "ClientSession.h"
#include "ActiveConfig.h"
#include "EmaBufferInt.h"

#include "OmmInaccessibleLogFileException.h"
#include "OmmInvalidHandleException.h"
#include "OmmSystemException.h"
#include "ExceptionTranslator.h"
#include "../EmaVersion.h"

#include "GetTime.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define	EMA_BIG_STR_BUFF_SIZE (1024*4)

using namespace thomsonreuters::ema::access;

OmmServerBaseImpl::OmmServerBaseImpl(ActiveServerConfig& activeServerConfig, OmmProviderClient& ommProviderClient, void* closure) :
	_activeServerConfig(activeServerConfig),
	_pOmmProviderClient(&ommProviderClient),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state( NotInitializedEnum ),
	_pRsslReactor(0),
	_pServerChannelHandler(0),
	_pLoginHandler(0),
	_pDirectoryHandler(0),
	_pDictionaryHandler(0),
	_pMarketItemHandler(0),
	_pItemCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_pRsslServer(0),
	_pClosure(closure),
	_bApiDispatchThreadStarted(false)
{
	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmServerBaseImpl::OmmServerBaseImpl(ActiveServerConfig& activeServerConfig, OmmProviderClient& ommProviderClient, OmmProviderErrorClient& client, void* closure) :
	_activeServerConfig(activeServerConfig),
	_pOmmProviderClient(&ommProviderClient),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_pRsslReactor(0),
	_pServerChannelHandler(0),
	_pLoginHandler(0),
	_pDirectoryHandler(0),
	_pDictionaryHandler(0),
	_pMarketItemHandler(0),
	_pItemCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_pRsslServer(0),
	_pClosure(closure),
	_bApiDispatchThreadStarted(false)
{
	try
	{
		_pErrorClientHandler = new ErrorClientHandler(client);
	}
	catch (std::bad_alloc)
	{
		client.onMemoryExhaustion("Failed to allocate memory in OmmServerBaseImpl( ActiveConfig& , OmmProviderErrorClient& )");
	}

	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmServerBaseImpl::~OmmServerBaseImpl()
{
	if (_pErrorClientHandler)
		delete _pErrorClientHandler;
}

void OmmServerBaseImpl::readConfig(EmaConfigServerImpl* pConfigServerImpl)
{
	UInt64 id = OmmBaseImplMap<OmmServerBaseImpl>::add(this);

	_activeServerConfig.configuredName = pConfigServerImpl->getConfiguredName();
	_activeServerConfig.instanceName = _activeServerConfig.configuredName;
	_activeServerConfig.instanceName.append("_").append(id);

	const UInt32 maxUInt32( 0xFFFFFFFF );
	UInt64 tmp;
	EmaString instanceNodeName(pConfigServerImpl->getInstanceNodeName());
	instanceNodeName.append(_activeServerConfig.configuredName).append("|");

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "ItemCountHint", tmp))
		_activeServerConfig.itemCountHint = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "ServiceCountHint", tmp))
		_activeServerConfig.serviceCountHint = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "RequestTimeout", tmp))
		_activeServerConfig.requestTimeout = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	pConfigServerImpl->get<Int64>(instanceNodeName + "DispatchTimeoutApiThread", _activeServerConfig.dispatchTimeoutApiThread);

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "CatchUnhandledException", tmp))
		_activeServerConfig.catchUnhandledException = static_cast<UInt32>(tmp > 0 ? true : false);

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "MaxDispatchCountApiThread", tmp))
		_activeServerConfig.maxDispatchCountApiThread = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "MaxDispatchCountUserThread", tmp))
		_activeServerConfig.maxDispatchCountUserThread = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	pConfigServerImpl->get<EmaString>(instanceNodeName + "XmlTraceFileName", _activeServerConfig.xmlTraceFileName);

	Int64 tmp1;
	if (pConfigServerImpl->get<Int64>(instanceNodeName + "XmlTraceMaxFileSize", tmp1) && tmp1 > 0)
	{
		_activeServerConfig.xmlTraceMaxFileSize = tmp1;
	}

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "XmlTraceToFile", tmp))
	{
		_activeServerConfig.xmlTraceToFile = tmp > 0 ? true : false;
	}

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "XmlTraceToStdout", tmp))
	{
		_activeServerConfig.xmlTraceToStdout = tmp > 0 ? true : false;
	}

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "XmlTraceToMultipleFiles", tmp))
	{
		_activeServerConfig.xmlTraceToMultipleFiles = tmp > 0 ? true : false;
	}

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "XmlTraceWrite", tmp))
	{
		_activeServerConfig.xmlTraceWrite = tmp > 0 ? true : false;
	}

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "XmlTraceRead", tmp))
	{
		_activeServerConfig.xmlTraceRead = tmp > 0 ? true : false;
	}

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "XmlTracePing", tmp))
	{
		_activeServerConfig.xmlTracePing = tmp > 0 ? true : false;
	}

	if (pConfigServerImpl->get<UInt64>(instanceNodeName + "XmlTraceHex", tmp))
	{
		_activeServerConfig.xmlTraceHex = tmp > 0 ? true : false;
	}

	pConfigServerImpl->get<Int64>(instanceNodeName + "PipePort", _activeServerConfig.pipePort);

	pConfigServerImpl->getLoggerName(_activeServerConfig.configuredName, _activeServerConfig.loggerConfig.loggerName);

	_activeServerConfig.loggerConfig.minLoggerSeverity = OmmLoggerClient::SuccessEnum;
	_activeServerConfig.loggerConfig.loggerFileName = "emaLog";
	_activeServerConfig.loggerConfig.loggerType = OmmLoggerClient::FileEnum;
	_activeServerConfig.loggerConfig.includeDateInLoggerOutput = false;

	if (_activeServerConfig.loggerConfig.loggerName.length())
	{
		EmaString loggerNodeName("LoggerGroup|LoggerList|Logger.");
		loggerNodeName.append(_activeServerConfig.loggerConfig.loggerName).append("|");

		EmaString name;
		if (!pConfigServerImpl->get< EmaString >(loggerNodeName + "Name", name))
		{
			EmaString errorMsg("no configuration exists for consumer logger [");
			errorMsg.append(loggerNodeName).append("]; will use logger defaults if not config programmatically");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::ErrorEnum);
		}

		pConfigServerImpl->get<OmmLoggerClient::LoggerType>(loggerNodeName + "LoggerType", _activeServerConfig.loggerConfig.loggerType);

		if (_activeServerConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum)
			pConfigServerImpl->get<EmaString>(loggerNodeName + "FileName", _activeServerConfig.loggerConfig.loggerFileName);

		pConfigServerImpl->get<OmmLoggerClient::Severity>(loggerNodeName + "LoggerSeverity", _activeServerConfig.loggerConfig.minLoggerSeverity);

		UInt64 idilo(0);
		if (pConfigServerImpl->get< UInt64 >(loggerNodeName + "IncludeDateInLoggerOutput", idilo))
			_activeServerConfig.loggerConfig.includeDateInLoggerOutput = idilo == 1 ? true : false;
	}
	else
		_activeServerConfig.loggerConfig.loggerName.set("Logger");

	if (ProgrammaticConfigure* ppc = pConfigServerImpl->getProgrammaticConfigure())
	{
		ppc->retrieveLoggerConfig(_activeServerConfig.loggerConfig.loggerName, _activeServerConfig);
	}

	EmaString serverName;
	pConfigServerImpl->getServerName(_activeServerConfig.configuredName, serverName);
	if (serverName.trimWhitespace().length() > 0)
	{
		if (_activeServerConfig.pServerConfig)
		{
			delete _activeServerConfig.pServerConfig;
			_activeServerConfig.pServerConfig = 0;
		}
		_activeServerConfig.pServerConfig = readServerConfig(pConfigServerImpl, serverName.trimWhitespace());
	}
	else
	{
		useDefaultConfigValues(EmaString("Server"), pConfigServerImpl->getUserSpecifiedPort().userSpecifiedValue);
	}

	if (ProgrammaticConfigure* ppc = pConfigServerImpl->getProgrammaticConfigure())
	{
		ppc->retrieveCommonConfig(_activeServerConfig.configuredName, _activeServerConfig);
		bool isProgmaticCfgServerName = ppc->getActiveServerName(_activeServerConfig.configuredName, serverName.trimWhitespace());

		if (isProgmaticCfgServerName)
		{
			if (_activeServerConfig.pServerConfig)
			{
				delete _activeServerConfig.pServerConfig;
				_activeServerConfig.pServerConfig = 0;
			}

			ServerConfig* fileServerConfig = readServerConfig(pConfigServerImpl, serverName.trimWhitespace());

			int serverConfigByFuncCall = 0;
			if (pConfigServerImpl->getUserSpecifiedPort().userSpecifiedValue.length() > 0)
				serverConfigByFuncCall = SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL;
			

			ppc->retrieveServerConfig(serverName.trimWhitespace(), _activeServerConfig, serverConfigByFuncCall, fileServerConfig);
			if (!_activeServerConfig.pServerConfig)
				_activeServerConfig.pServerConfig = fileServerConfig;
			else
			{
				if (fileServerConfig)
				{
					delete fileServerConfig;
					fileServerConfig = 0;
				}
			}
		}
		
	}

	catchUnhandledException(_activeServerConfig.catchUnhandledException);
}

void OmmServerBaseImpl::useDefaultConfigValues(const EmaString& serverName, const EmaString& port)
{
	SocketServerConfig* newServerConfig = 0;
	try
	{
		newServerConfig = new SocketServerConfig(_activeServerConfig.defaultServiceName());

		if (port.length())
			newServerConfig->serviceName = port;

		newServerConfig->name.set(serverName);
		if (_activeServerConfig.pServerConfig)
		{
			delete _activeServerConfig.pServerConfig;
			_activeServerConfig.pServerConfig = 0;
		}
		_activeServerConfig.pServerConfig = newServerConfig;
	}
	catch (std::bad_alloc)
	{
		const char* temp("Failed to allocate memory for SocketServerConfig.");
		throwMeeException(temp);
		return;
	}
}

ServerConfig* OmmServerBaseImpl::readServerConfig( EmaConfigServerImpl* pConfigServerImpl, const EmaString& serverName )
{
	ServerConfig* newServerConfig = NULL;
	UInt32 maxUInt32(0xFFFFFFFF);
	EmaString serverNodeName("ServerGroup|ServerList|Server.");
	serverNodeName.append(serverName).append("|");

	RsslConnectionTypes serverType;
	if ( !pConfigServerImpl->get<RsslConnectionTypes>( serverNodeName + "ServerType", serverType )  ||
		pConfigServerImpl->getUserSpecifiedPort().userSpecifiedValue.length() > 0 )
	{
		serverType = RSSL_CONN_TYPE_SOCKET;
	}

	switch (serverType)
	{
		case RSSL_CONN_TYPE_SOCKET:
		{
			SocketServerConfig* socketServerConfig = 0;
			try
			{
				socketServerConfig = new SocketServerConfig(_activeServerConfig.defaultServiceName());
				newServerConfig = socketServerConfig;
			}
			catch (std::bad_alloc) {}

			if ( !socketServerConfig )
			{
				const char* temp = "Failed to allocate memory for SocketServerConfig.";
				throwMeeException(temp);
				return 0;
			}

			PortSetViaFunctionCall portSetViaFncCall(pConfigServerImpl->getUserSpecifiedPort());
			if (portSetViaFncCall.userSet)
			{
				if (portSetViaFncCall.userSpecifiedValue.length())
				{
					socketServerConfig->serviceName = portSetViaFncCall.userSpecifiedValue;
				}
				else
				{
					socketServerConfig->serviceName = _activeServerConfig.defaultServiceName();
				}
			}
			else
				pConfigServerImpl->get< EmaString >( serverNodeName + "Port", socketServerConfig->serviceName );

			UInt64 tempUInt = DEFAULT_TCP_NODELAY;
			pConfigServerImpl->get<UInt64>( serverNodeName + "TcpNodelay", tempUInt );
			if (tempUInt)
				socketServerConfig->tcpNodelay = RSSL_TRUE;
			else
				socketServerConfig->tcpNodelay = RSSL_FALSE;

			break;
		}
		default:
		{
			EmaString temp("Not supported server type. Type = ");
			temp.append((UInt32)serverType);
			throwIueException(temp);
			return 0;
		}
	}

	newServerConfig->name = serverName;

	pConfigServerImpl->get<EmaString>(serverNodeName + "InterfaceName", newServerConfig->interfaceName);

	bool setCompressionThresholdFromConfigFile(false);
	UInt64 tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "CompressionThreshold", tempUInt))
	{
		newServerConfig->compressionThreshold = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;
		setCompressionThresholdFromConfigFile = true;
	}

	pConfigServerImpl->get<RsslCompTypes>(serverNodeName + "CompressionType", newServerConfig->compressionType);
	if ( newServerConfig->compressionType == RSSL_COMP_LZ4 &&
		 !setCompressionThresholdFromConfigFile )
	  newServerConfig->compressionThreshold = DEFAULT_COMPRESSION_THRESHOLD_LZ4;

	tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "GuaranteedOutputBuffers", tempUInt))
		newServerConfig->setGuaranteedOutputBuffers(tempUInt);

	tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "NumInputBuffers", tempUInt))
		newServerConfig->setNumInputBuffers(tempUInt);

	tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "ConnectionPingTimeout", tempUInt))
		newServerConfig->connectionPingTimeout = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;

	tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "ConnectionMinPingTimeout", tempUInt))
		newServerConfig->connectionMinPingTimeout = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;

	tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "SysRecvBufSize", tempUInt))
		newServerConfig->sysRecvBufSize = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;

	tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "SysSendBufSize", tempUInt))
		newServerConfig->sysSendBufSize = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;

	tempUInt = 0;
	if (pConfigServerImpl->get<UInt64>(serverNodeName + "HighWaterMark", tempUInt))
		newServerConfig->highWaterMark = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;

	EmaString instanceNodeName(pConfigServerImpl->getInstanceNodeName());
	instanceNodeName.append(_activeServerConfig.configuredName).append("|");

	/* @deprecated DEPRECATED:
	 * All Xml trace configuration is per provider instance based now.
	 * The following code will be removed in the future.
	 */
	if (!_activeServerConfig.parameterConfigGroup)
	{
		if (pConfigServerImpl->get<EmaString>(serverNodeName + "XmlTraceFileName", _activeServerConfig.xmlTraceFileName))
		{
			EmaString errorMsg("XmlTraceFileName is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		Int64 tmp = 0;
		if (pConfigServerImpl->get<Int64>(serverNodeName + "XmlTraceMaxFileSize", tmp))
		{
			if (tmp > 0)
				_activeServerConfig.xmlTraceMaxFileSize = tmp;

			EmaString errorMsg("XmlTraceMaxFileSize is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigServerImpl->get<UInt64>(serverNodeName + "XmlTraceToFile", tempUInt))
		{ 
			if (tempUInt > 0)
				_activeServerConfig.xmlTraceToFile = true;

			EmaString errorMsg("XmlTraceToFile is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigServerImpl->get<UInt64>(serverNodeName + "XmlTraceToStdout", tempUInt))
		{
			_activeServerConfig.xmlTraceToStdout = tempUInt > 0 ? true : false;

			EmaString errorMsg("XmlTraceToStdout is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigServerImpl->get<UInt64>(serverNodeName + "XmlTraceToMultipleFiles", tempUInt))
		{
			if (tempUInt > 0)
				_activeServerConfig.xmlTraceToMultipleFiles = true;

			EmaString errorMsg("XmlTraceToMultipleFiles is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigServerImpl->get<UInt64>(serverNodeName + "XmlTraceWrite", tempUInt))
		{
			if (tempUInt == 0)
				_activeServerConfig.xmlTraceWrite = false;

			EmaString errorMsg("XmlTraceWrite is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigServerImpl->get<UInt64>(serverNodeName + "XmlTraceRead", tempUInt))
		{
			if (tempUInt == 0)
				_activeServerConfig.xmlTraceRead = false;

			EmaString errorMsg("XmlTraceRead is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigServerImpl->get<UInt64>(serverNodeName + "XmlTracePing", tempUInt))
		{
			_activeServerConfig.xmlTracePing = tempUInt == 0 ? false : true;

			EmaString errorMsg("XmlTracePing is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigServerImpl->get<UInt64>(serverNodeName + "XmlTraceHex", tempUInt))
		{
			_activeServerConfig.xmlTraceHex = tempUInt == 0 ? false : true;

			EmaString errorMsg("XmlTraceHex is no longer configured on a per-server basis; configure it instead in the IProvider instance.");
			pConfigServerImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}
	}

	return newServerConfig;
}

void OmmServerBaseImpl::initialize(EmaConfigServerImpl* serverConfigImpl)
{
	try
	{
		_userLock.lock();

		readConfig(serverConfigImpl);

		_pLoggerClient = OmmLoggerClient::create(_activeServerConfig.loggerConfig.loggerType, _activeServerConfig.loggerConfig.includeDateInLoggerOutput,
			_activeServerConfig.loggerConfig.minLoggerSeverity, _activeServerConfig.loggerConfig.loggerFileName);

		serverConfigImpl->configErrors().log(_pLoggerClient, _activeServerConfig.loggerConfig.minLoggerSeverity);

		readCustomConfig(serverConfigImpl);

		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Print out active configuration detail.");
			temp.append(_activeServerConfig.configTrace());
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		if (!_pipe.create())
		{
			EmaString temp("Failed to create communication Pipe.");
			if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException(temp);
		}
		else if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Successfully initialized communication Pipe.");
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		RsslError rsslError;
		RsslRet retCode = rsslInitialize(RSSL_LOCK_GLOBAL_AND_CHANNEL, &rsslError);
		if (retCode != RSSL_RET_SUCCESS)
		{
			EmaString temp("rsslInitialize() failed while initializing OmmServerBaseImpl.");
			temp.append(" Internal sysError='").append(rsslError.sysError)
				.append("' Error text='").append(rsslError.text).append("'. ");

			if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException(temp);
			return;
		}
		else if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Successfully initialized Rssl.");
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		_state = RsslInitilizedEnum;

		RsslCreateReactorOptions reactorOpts;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);

		rsslClearCreateReactorOptions(&reactorOpts);

		reactorOpts.userSpecPtr = (void*)this;

		_pRsslReactor = rsslCreateReactor(&reactorOpts, &rsslErrorInfo);
		if (!_pRsslReactor)
		{
			EmaString temp("Failed to initialize OmmServerBaseImpl (rsslCreateReactor).");
			temp.append("' Error Id='").append(rsslErrorInfo.rsslError.rsslErrorId)
				.append("' Internal sysError='").append(rsslErrorInfo.rsslError.sysError)
				.append("' Error Location='").append(rsslErrorInfo.errorLocation)
				.append("' Error Text='").append(rsslErrorInfo.rsslError.text).append("'. ");
			if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException(temp);
			return;
		}
		else if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Successfully created Reactor.");
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		_state = ReactorInitializedEnum;

#ifdef USING_SELECT
		FD_ZERO(&_readFds);
		FD_ZERO(&_exceptFds);

		FD_SET(_pipe.readFD(), &_readFds);
		FD_SET(_pipe.readFD(), &_exceptFds);
		FD_SET(_pRsslReactor->eventFd, &_readFds);
		FD_SET(_pRsslReactor->eventFd, &_exceptFds);
#else
		_eventFdsCapacity = 8;
		_eventFds = new pollfd[_eventFdsCapacity];
		_eventFdsCount = 0;
		_pipeReadEventFdsIdx = addFd(_pipe.readFD());
		_serverReadEventFdsIdx = addFd(_pRsslReactor->eventFd);
#endif
		_pServerChannelHandler = ServerChannelHandler::create(this);
		_pServerChannelHandler->initialize();

		_pLoginHandler = LoginHandler::create(this);
		_pLoginHandler->initialize();

		_pDirectoryHandler = DirectoryHandler::create(this);
		_pDirectoryHandler->initialize(serverConfigImpl);

		_pDictionaryHandler = DictionaryHandler::create(this);
		_pDictionaryHandler->initialize();

		_pMarketItemHandler = MarketItemHandler::create(this);
		_pMarketItemHandler->initialize();

		_pItemCallbackClient = ItemCallbackClient::create( *this );
		_pItemCallbackClient->initialize();

		rsslClearOMMProviderRole(&_providerRole);
		_providerRole.base.channelEventCallback = ServerChannelHandler::channelEventCallback;
		_providerRole.base.defaultMsgCallback = MarketItemHandler::itemCallback;
		_providerRole.loginMsgCallback = LoginHandler::loginCallback;
		_providerRole.directoryMsgCallback = DirectoryHandler::directoryCallback;
		_providerRole.dictionaryMsgCallback = DictionaryHandler::dictionaryCallback;

		RsslBindOptions bindOptions = RSSL_INIT_BIND_OPTS;

		EmaString componentVersionInfo(COMPONENTNAME);
		componentVersionInfo.append(EMA_COMPONENT_VER_PLATFORM);
		componentVersionInfo.append(COMPILE_BITS_STR);
		componentVersionInfo.append(emaComponentLinkType);
		componentVersionInfo.append("(");
		componentVersionInfo.append(emaComponentBldtype);
		componentVersionInfo.append(")");

		bindServerOptions(bindOptions, componentVersionInfo);

		clearRsslErrorInfo(&rsslErrorInfo);

		_pRsslServer = rsslBind(&bindOptions, &rsslErrorInfo.rsslError);
		if (!_pRsslServer)
		{
			EmaString temp("Failed to initialize OmmServerBaseImpl (Unable to bind RSSL server).");
			temp.append("' Error Id='").append(rsslErrorInfo.rsslError.rsslErrorId)
				.append("' Internal sysError='").append(rsslErrorInfo.rsslError.sysError)
				.append("' Error Location='").append(rsslErrorInfo.errorLocation)
				.append("' Error Text='").append(rsslErrorInfo.rsslError.text).append("'. ");
			if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException(temp);
			return;
		}

		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Successfully binded Rssl Server for protocol type ");
			temp.append(bindOptions.protocolType).append(" on port ")
				.append(bindOptions.serviceName).append(".");
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

#ifdef USING_SELECT
		FD_SET(_pRsslServer->socketId, &_readFds);
#else
		_serverReadEventFdsIdx = addFd(_pRsslServer->socketId);
#endif

		if ( isApiDispatching() && !_atExit )
		{
			start();

			/* Waits until the API dispatch thread started */
			while (!_bApiDispatchThreadStarted) OmmBaseImplMap<OmmServerBaseImpl>::sleep(100);
		}

		_userLock.unlock();
	}
	catch (const OmmException& ommException)
	{
		uninitialize(false, true);

		_userLock.unlock();

		if (hasErrorClientHandler())
			notifErrorClientHandler(ommException, getErrorClientHandler());
		else
			throw;
	}
}

//only for unit test, internal use
void OmmServerBaseImpl::initializeForTest(EmaConfigServerImpl* serverConfigImpl)
{
	try
	{
		_userLock.lock();

		readConfig(serverConfigImpl);

		_pLoggerClient = OmmLoggerClient::create(_activeServerConfig.loggerConfig.loggerType, _activeServerConfig.loggerConfig.includeDateInLoggerOutput,
			_activeServerConfig.loggerConfig.minLoggerSeverity, _activeServerConfig.loggerConfig.loggerFileName);

		serverConfigImpl->configErrors().log(_pLoggerClient, _activeServerConfig.loggerConfig.minLoggerSeverity);

		readCustomConfig(serverConfigImpl);

		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Print out active configuration detail.");
			temp.append(_activeServerConfig.configTrace());
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		_userLock.unlock();
	}
	catch (const OmmException& ommException)
	{
		_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, ommException.getText());
		_userLock.unlock();
	}
}

void OmmServerBaseImpl::bindServerOptions(RsslBindOptions& bindOptions, const EmaString& componentVersion)
{
	bindOptions.majorVersion = RSSL_RWF_MAJOR_VERSION;
	bindOptions.minorVersion = RSSL_RWF_MINOR_VERSION;
	bindOptions.protocolType = RSSL_RWF_PROTOCOL_TYPE;
	bindOptions.guaranteedOutputBuffers = _activeServerConfig.pServerConfig->guaranteedOutputBuffers;
	bindOptions.compressionType = _activeServerConfig.pServerConfig->compressionType;
	bindOptions.connectionType = _activeServerConfig.pServerConfig->connectionType;
	bindOptions.numInputBuffers = _activeServerConfig.pServerConfig->numInputBuffers;
	bindOptions.sysRecvBufSize = _activeServerConfig.pServerConfig->sysRecvBufSize;
	bindOptions.sysSendBufSize = _activeServerConfig.pServerConfig->sysSendBufSize;
	bindOptions.interfaceName = const_cast<char*>(_activeServerConfig.pServerConfig->interfaceName.c_str());
	bindOptions.pingTimeout = _activeServerConfig.pServerConfig->connectionPingTimeout;
	bindOptions.minPingTimeout = _activeServerConfig.pServerConfig->connectionMinPingTimeout;
	bindOptions.componentVersion = (char *)componentVersion.c_str();

	switch (_activeServerConfig.pServerConfig->getType())
	{
		case ServerConfig::SocketChannelEnum:
		{
			SocketServerConfig *socketServerConfig = static_cast<SocketServerConfig *>(_activeServerConfig.pServerConfig);
			bindOptions.tcpOpts.tcp_nodelay = socketServerConfig->tcpNodelay;
			bindOptions.serviceName = const_cast<char *>(socketServerConfig->serviceName.c_str());
		}
		break;
		default:
		{
			EmaString temp("Failed to initialize OmmServerBaseImpl(ServerType = ");
			temp.append(_activeServerConfig.pServerConfig->getType())
				.append(" is not supported)").append("'. ");

			if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException(temp);
			return;
		}
	}
}

ErrorClientHandler& OmmServerBaseImpl::getErrorClientHandler()
{
	return *_pErrorClientHandler;
}

bool OmmServerBaseImpl::hasErrorClientHandler() const
{
	return _pErrorClientHandler != 0 ? true : false;
}

EmaList< TimeOut* >& OmmServerBaseImpl::getTimeOutList()
{
	return _theTimeOuts;
}

Mutex& OmmServerBaseImpl::getTimeOutMutex()
{
	return _timeOutLock;
}

Mutex& OmmServerBaseImpl::getUserMutex()
{
	return _userLock;
}

void OmmServerBaseImpl::installTimeOut()
{
	pipeWrite();
}

RsslReactor* OmmServerBaseImpl::getRsslReactor()
{
	return _pRsslReactor;
}

bool OmmServerBaseImpl::isPipeWritten()
{
	MutexLocker lock(_pipeLock);

	return (_pipeWriteCount > 0 ? true : false);
}

void OmmServerBaseImpl::pipeWrite()
{
	MutexLocker lock(_pipeLock);

	if (++_pipeWriteCount == 1)
		_pipe.write("0", 1);
}

void OmmServerBaseImpl::pipeRead()
{
	MutexLocker lock(_pipeLock);

	if (--_pipeWriteCount == 0)
	{
		char temp[10];
		_pipe.read(temp, 1);
	}
}

ItemInfoPtr OmmServerBaseImpl::getItemInfo(UInt64 handle)
{
	_userLock.lock();

	ItemInfoPtr* itemInfoPtr = _itemInfoHash.find(handle);

	_userLock.unlock();

	return itemInfoPtr ? *itemInfoPtr : 0;
}

void OmmServerBaseImpl::addItemInfo(ItemInfo* itemInfo)
{
	_userLock.lock();

	UInt64 handle = (UInt64)itemInfo;

	ItemInfoPtr* itemInfoPtr = _itemInfoHash.find(handle);

	if (!itemInfoPtr)
	{
		if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Added ItemInfo ");
			temp.append(ptrToStringAsHex(itemInfo)).append(" to ItemInfoHash").append(CR)
				.append("Client handle ").append(itemInfo->getClientSession()->getClientHandle()).append(CR)
				.append("Instance name ").append(getInstanceName());
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		_itemInfoHash.insert(handle, itemInfo);
		itemInfo->getClientSession()->addItemInfo(itemInfo);
	}

	_userLock.unlock();
}

void OmmServerBaseImpl::removeItemInfo(ItemInfo* itemInfo, bool eraseItemGroup)
{
	_userLock.lock();

	if (OmmLoggerClient::VerboseEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
	{
		EmaString temp("Removed ItemInfo ");
		temp.append(ptrToStringAsHex(itemInfo)).append(" from ItemInfoHash").append(CR)
			.append("Client handle ").append(itemInfo->getClientSession()->getClientHandle()).append(CR)
			.append("Instance name ").append(getInstanceName());
		_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
	}

	_itemInfoHash.erase((UInt64)itemInfo);
	itemInfo->getClientSession()->removeItemInfo(itemInfo);

	if (eraseItemGroup && itemInfo->hasItemGroup() )
	{
		removeItemGroup(itemInfo);
	}

	ItemInfo::destroy(itemInfo);

	_userLock.unlock();
}

void OmmServerBaseImpl::cleanUp()
{
	uninitialize(true, false);
}

void OmmServerBaseImpl::uninitialize(bool caughtException, bool calledFromInit)
{
	OmmBaseImplMap<OmmServerBaseImpl>::remove(this);

	_atExit = true;

	if (isApiDispatching() && !caughtException)
	{
		stop();
		eventReceived();
		msgDispatched();
		pipeWrite();

		if (!calledFromInit)
		{
			_dispatchLock.lock();
			_userLock.lock();
			if (_bApiDispatchThreadStarted ) wait();
			_dispatchLock.unlock();
		}
		else
		{
			if (_bApiDispatchThreadStarted) wait();
		}
	}
	else
	{
		if (!calledFromInit) _userLock.lock();
	}

	if ( _state == NotInitializedEnum )
	{
		if ( !calledFromInit ) _userLock.unlock();
		return;
	}

	_state = OmmServerBaseImpl::UnInitializingEnum;

	if (_pRsslReactor)
	{
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);

		if (RSSL_RET_SUCCESS != rsslDestroyReactor(_pRsslReactor, &rsslErrorInfo))
		{
			if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
			{
				EmaString temp("Failed to uninitialize OmmServerBaseImpl( rsslDestroyReactor ).");
				temp.append("' Error Id='").append(rsslErrorInfo.rsslError.rsslErrorId)
					.append("' Internal sysError='").append(rsslErrorInfo.rsslError.sysError)
					.append("' Error Location='").append(rsslErrorInfo.errorLocation)
					.append("' Error Text='").append(rsslErrorInfo.rsslError.text).append("'. ");

				_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			}
		}

		if (_pServerChannelHandler)
			_pServerChannelHandler->closeActiveSessions();

		_pRsslReactor = 0;
	}

	ItemCallbackClient::destroy(_pItemCallbackClient);

	MarketItemHandler::destroy(_pMarketItemHandler);

	DictionaryHandler::destroy(_pDictionaryHandler);

	DirectoryHandler::destroy(_pDirectoryHandler);

	LoginHandler::destroy(_pLoginHandler);

	ServerChannelHandler::destroy(_pServerChannelHandler);

	if (RSSL_RET_SUCCESS != rsslUninitialize())
	{
		if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("rsslUninitialize() failed while unintializing OmmServerBaseImpl.");
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
		}
	}

#ifdef USING_SELECT
	if (_pipe.isInitialized())
	{
		FD_CLR(_pipe.readFD(), &_readFds);
		FD_CLR(_pipe.readFD(), &_exceptFds);
		_pipe.close();
	}
#else
	if (_pipe.isInitialized())
	{
		removeFd(_pipe.readFD());
		_pipe.close();
	}

	if ( _pRsslServer )
	{
		removeFd(_pRsslServer->socketId);
	}

	_pipeReadEventFdsIdx = -1;
	_serverReadEventFdsIdx = -1;
#endif

	OmmLoggerClient::destroy(_pLoggerClient);

	_state = NotInitializedEnum;

	if (!calledFromInit) _userLock.unlock();

#ifdef USING_POLL
	delete[] _eventFds;
#endif
}

void OmmServerBaseImpl::setAtExit()
{
	_atExit = true;
	eventReceived();
	msgDispatched();
	pipeWrite();
}

bool OmmServerBaseImpl::isAtExit()
{
	return _atExit;
}

Int64 OmmServerBaseImpl::rsslReactorDispatchLoop(Int64 timeOut, UInt32 count, bool& bMsgDispRcvd)
{
	bMsgDispRcvd = false;

	Int64 startTime = GetTime::getMicros();
	Int64 endTime = 0;
	Int64 nextTimer = 0;

	bool userTimeoutExists(TimeOut::getTimeOutInMicroSeconds(*this, nextTimer));
	if (userTimeoutExists)
	{
		if (timeOut >= 0 && timeOut < nextTimer)
			userTimeoutExists = false;
		else
			timeOut = nextTimer;
	}

	RsslReactorDispatchOptions dispatchOpts;
	dispatchOpts.pReactorChannel = NULL;
	dispatchOpts.maxMessages = count;

	RsslRet reactorRetCode = RSSL_RET_SUCCESS;

	UInt64 loopCount = 0;
	do
	{
		_userLock.lock();
		reactorRetCode = _pRsslReactor ? rsslReactorDispatch(_pRsslReactor, &dispatchOpts, &_reactorDispatchErrorInfo) : RSSL_RET_SUCCESS;
		_userLock.unlock();

		++loopCount;
	} while (reactorRetCode > RSSL_RET_SUCCESS && !bMsgDispRcvd && loopCount < 5);

	if (reactorRetCode < RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Call to rsslReactorDispatch() failed. Internal sysError='");
			temp.append(_reactorDispatchErrorInfo.rsslError.sysError)
				.append("' Error Id ").append(_reactorDispatchErrorInfo.rsslError.rsslErrorId).append("' ")
				.append("' Error Location='").append(_reactorDispatchErrorInfo.errorLocation).append("' ")
				.append("' Error text='").append(_reactorDispatchErrorInfo.rsslError.text).append("'. ");

			_userLock.lock();
			if (_pLoggerClient) _pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			_userLock.unlock();
		}

		return -2;
	}

	if (bMsgDispRcvd)
		return 0;

	endTime = GetTime::getMicros();

	if ((timeOut >= 0) && (endTime - startTime >= timeOut))
	{
		if (userTimeoutExists)
			TimeOut::execute(*this);

		return bMsgDispRcvd ? 0 : -1;
	}

	if (timeOut >= 0)
	{
		timeOut -= endTime - startTime;
		if (timeOut < 0)
			timeOut = 0;
	}

	do
	{
		startTime = endTime;

		Int64 selectRetCode = 1;

#if defined( USING_SELECT )

		fd_set useReadFds = _readFds;
		fd_set useExceptFds = _exceptFds;

		struct timeval selectTime;
		if (timeOut >= 0)
		{
			selectTime.tv_sec = static_cast<long>(timeOut / 1000000);
			selectTime.tv_usec = timeOut % 1000000;
			selectRetCode = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime);
		}
		else if (timeOut < 0)
			selectRetCode = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, NULL);

		if (selectRetCode > 0 && FD_ISSET(_pRsslServer->socketId, &useReadFds))
		{
			rsslClearReactorAcceptOptions(&_reactorAcceptOptions);
			clearRsslErrorInfo(&_reactorDispatchErrorInfo);

			ClientSession* clientSession = ClientSession::create(this);

			_reactorAcceptOptions.rsslAcceptOptions.userSpecPtr = clientSession;

			if (rsslReactorAccept(_pRsslReactor, _pRsslServer, &_reactorAcceptOptions, (RsslReactorChannelRole*)&_providerRole, &_reactorDispatchErrorInfo) != RSSL_RET_SUCCESS)
			{
				ClientSession::destroy(clientSession);

				if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Call to rsslReactorAccept() failed. Internal sysError='");
					temp.append(_reactorDispatchErrorInfo.rsslError.sysError)
						.append("' Error Id ").append(_reactorDispatchErrorInfo.rsslError.rsslErrorId).append("' ")
						.append("' Error Location='").append(_reactorDispatchErrorInfo.errorLocation).append("' ")
						.append("' Error text='").append(_reactorDispatchErrorInfo.rsslError.text).append("'. ");

					_userLock.lock();
					if (_pLoggerClient) _pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
					_userLock.unlock();
				}
			}

			--selectRetCode;
		}

		if (selectRetCode > 0 && FD_ISSET(_pipe.readFD(), &useReadFds))
		{
			pipeRead();
			--selectRetCode;
		}

#elif defined( USING_PPOLL )

		struct timespec ppollTime;

		if (timeOut >= 0)
		{
			ppollTime.tv_sec = timeOut / static_cast<long long>(1e6);
			ppollTime.tv_nsec = timeOut % static_cast<long long>(1e6) * static_cast<long long>(1e3);
			selectRetCode = ppoll(_eventFds, _eventFdsCount, &ppollTime, 0);
		}
		else if (timeOut < 0)
			selectRetCode = ppoll(_eventFds, _eventFdsCount, 0, 0);

		if (selectRetCode > 0)
		{
			if(_eventFds[_serverReadEventFdsIdx].revents & POLLIN)
			{
				rsslClearReactorAcceptOptions(&_reactorAcceptOptions);
				clearRsslErrorInfo(&_reactorDispatchErrorInfo);

				ClientSession* clientSession = ClientSession::create(this);

				_reactorAcceptOptions.rsslAcceptOptions.userSpecPtr = clientSession;

				if (rsslReactorAccept(_pRsslReactor, _pRsslServer, &_reactorAcceptOptions, (RsslReactorChannelRole*)&_providerRole, &_reactorDispatchErrorInfo) != RSSL_RET_SUCCESS)
				{
					ClientSession::destroy(clientSession);

					if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
					{
						EmaString temp("Call to rsslReactorAccept() failed. Internal sysError='");
						temp.append(_reactorDispatchErrorInfo.rsslError.sysError)
							.append("' Error Id ").append(_reactorDispatchErrorInfo.rsslError.rsslErrorId).append("' ")
							.append("' Error Location='").append(_reactorDispatchErrorInfo.errorLocation).append("' ")
							.append("' Error text='").append(_reactorDispatchErrorInfo.rsslError.text).append("'. ");

						_userLock.lock();
						if (_pLoggerClient) _pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
						_userLock.unlock();
					}
				}
				--selectRetCode;
			}

			if (_pipeReadEventFdsIdx == -1)
				for (int i = 0; i < _eventFdsCount; ++i)
					if (_eventFds[i].fd == _pipe.readFD())
					{
						_pipeReadEventFdsIdx = i;
						break;
					}

			if (_pipeReadEventFdsIdx != -1)
				if (_eventFds[_pipeReadEventFdsIdx].revents & POLLIN)
				{
					pipeRead();
					--selectRetCode;
				}
		}

#else
#error "No Implementation for Operating System That Does Not Implement ppoll"
#endif

		if (selectRetCode > 0)
		{
			loopCount = 0;
			do
			{
				_userLock.lock();
				reactorRetCode = _pRsslReactor ? rsslReactorDispatch(_pRsslReactor, &dispatchOpts, &_reactorDispatchErrorInfo) : RSSL_RET_SUCCESS;
				_userLock.unlock();

				++loopCount;
			} while (reactorRetCode > RSSL_RET_SUCCESS && !bMsgDispRcvd && loopCount < 5);

			if (reactorRetCode < RSSL_RET_SUCCESS)
			{
				if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Call to rsslReactorDispatch() failed. Internal sysError='");
					temp.append(_reactorDispatchErrorInfo.rsslError.sysError)
						.append("' Error Id ").append(_reactorDispatchErrorInfo.rsslError.rsslErrorId).append("' ")
						.append("' Error Location='").append(_reactorDispatchErrorInfo.errorLocation).append("' ")
						.append("' Error text='").append(_reactorDispatchErrorInfo.rsslError.text).append("'. ");

					_userLock.lock();
					if (_pLoggerClient) _pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
					_userLock.unlock();
				}

				return -2;
			}

			if (bMsgDispRcvd) return 0;

			TimeOut::execute(*this);

			if (bMsgDispRcvd) return 0;

			endTime = GetTime::getMicros();

			if (timeOut >= 0)
			{
				if (endTime > startTime + timeOut) return -1;

				timeOut -= (endTime - startTime);
			}
		}
		else if (selectRetCode == 0)
		{
			TimeOut::execute(*this);

			if (bMsgDispRcvd) return 0;

			endTime = GetTime::getMicros();

			if (timeOut >= 0)
			{
				if (endTime > startTime + timeOut) return -1;

				timeOut -= (endTime - startTime);
			}
		}
		else if (selectRetCode < 0)
		{
#ifdef WIN32
			Int64 lastError = WSAGetLastError();
			if (lastError != WSAEINTR)
			{
				if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Internal Error. Call to select() failed. LastError='");
					temp.append(lastError).append("'. ");

					_userLock.lock();
					if (_pLoggerClient) _pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
					_userLock.unlock();
				}
			}
#else
			if (errno != EINTR)
			{
				if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
				{
					EmaString temp("Internal Error. Call to select() failed. LastError='");
					temp.append(errno).append("'. ");

					_userLock.lock();
					if (_pLoggerClient) _pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
					_userLock.unlock();
				}
			}
#endif
			return -2;
		}
	} while (true);
}

void OmmServerBaseImpl::setState(ServerImplState state)
{
	_state = state;
}

OmmServerBaseImpl::ServerImplState OmmServerBaseImpl::getState()
{
	return _state;
}

void OmmServerBaseImpl::addSocket(RsslSocket fd)
{
#ifdef USING_SELECT
	FD_SET(fd, &_readFds);
	FD_SET(fd, &_exceptFds);
#else
	addFd(fd, POLLIN | POLLERR | POLLHUP);
#endif
}

void OmmServerBaseImpl::removeSocket(RsslSocket fd)
{
#ifdef USING_SELECT
	FD_CLR(fd, &_readFds);
	FD_CLR(fd, &_exceptFds);
#else
	removeFd(fd);
#endif
}

void OmmServerBaseImpl::handleIue(const EmaString& text)
{
	if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
	{
		if (_pLoggerClient)
		{
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, text);
		}
	}

	if (hasErrorClientHandler())
		getErrorClientHandler().onInvalidUsage(text);
	else
		throwIueException(text);
}

void OmmServerBaseImpl::handleIue(const char* text)
{
	if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
	{
		if (_pLoggerClient)
		{
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, text);
		}
	}

	if (hasErrorClientHandler())
		getErrorClientHandler().onInvalidUsage(text);
	else
		throwIueException(text);
}

void OmmServerBaseImpl::handleIhe(UInt64 handle, const EmaString& text)
{
	if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
	{
		if (_pLoggerClient)
		{
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, text);
		}
	}

	if (hasErrorClientHandler())
		getErrorClientHandler().onInvalidHandle(handle, text);
	else
		throwIheException(handle, text);
}

void OmmServerBaseImpl::handleIhe(UInt64 handle, const char* text)
{
	if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
	{
		if (_pLoggerClient)
		{
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, text);
		}
	}

	if (hasErrorClientHandler())
		getErrorClientHandler().onInvalidHandle(handle, text);
	else
		throwIheException(handle, text);
}

void OmmServerBaseImpl::handleMee(const char* text)
{
	if (OmmLoggerClient::ErrorEnum >= _activeServerConfig.loggerConfig.minLoggerSeverity)
	{
		if (_pLoggerClient)
		{
			_pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, text);
		}
	}

	if (hasErrorClientHandler())
		getErrorClientHandler().onMemoryExhaustion(text);
	else
		throwMeeException(text);
}

ItemCallbackClient& OmmServerBaseImpl::getItemCallbackClient()
{
	return *_pItemCallbackClient;
}

MarketItemHandler& OmmServerBaseImpl::getMarketItemHandler()
{
	return *_pMarketItemHandler;
}

DictionaryHandler& OmmServerBaseImpl::getDictionaryHandler()
{
	return *_pDictionaryHandler;
}

DirectoryHandler& OmmServerBaseImpl::getDirectoryHandler()
{
	return *_pDirectoryHandler;
}

LoginHandler& OmmServerBaseImpl::getLoginHandler()
{
	return *_pLoginHandler;
}

ServerChannelHandler& OmmServerBaseImpl::getServerChannelHandler()
{
	return *_pServerChannelHandler;
}

OmmLoggerClient& OmmServerBaseImpl::getOmmLoggerClient()
{
	return *_pLoggerClient;
}

const EmaString& OmmServerBaseImpl::getInstanceName() const
{
	return _activeServerConfig.instanceName;
}

void OmmServerBaseImpl::msgDispatched(bool value)
{
	_bMsgDispatched = value;
}

void OmmServerBaseImpl::eventReceived(bool value)
{
	_bEventReceived = value;
}

ActiveServerConfig& OmmServerBaseImpl::getActiveConfig()
{
	return _activeServerConfig;
}

LoggerConfig& OmmServerBaseImpl::getActiveLoggerConfig()
{
	return _activeServerConfig.loggerConfig;
}

void OmmServerBaseImpl::run()
{
	_dispatchLock.lock();
	_bApiDispatchThreadStarted = true;

	while (!Thread::isStopping() && !_atExit)
		rsslReactorDispatchLoop(_activeServerConfig.dispatchTimeoutApiThread, _activeServerConfig.maxDispatchCountApiThread, _bEventReceived);

	_dispatchLock.unlock();
}

int OmmServerBaseImpl::runLog(void* pExceptionStructure, const char* file, unsigned int line)
{
	char reportBuf[EMA_BIG_STR_BUFF_SIZE * 10];
	if (retrieveExceptionContext(pExceptionStructure, file, line, reportBuf, EMA_BIG_STR_BUFF_SIZE * 10) > 0)
	{
		_userLock.lock();
		if (_pLoggerClient) _pLoggerClient->log(_activeServerConfig.instanceName, OmmLoggerClient::ErrorEnum, reportBuf);
		_userLock.unlock();
	}

	return 1;
}


void OmmServerBaseImpl::terminateIf(void* object)
{
	OmmServerBaseImpl* user = reinterpret_cast<OmmServerBaseImpl*>(object);
	user->_eventTimedOut = true;
}

void OmmServerBaseImpl::notifErrorClientHandler(const OmmException& ommException, ErrorClientHandler& errorClient)
{
	switch (ommException.getExceptionType())
	{
	case OmmException::OmmSystemExceptionEnum:
		errorClient.onSystemError(static_cast<const OmmSystemException&>(ommException).getSystemExceptionCode(),
			static_cast<const OmmSystemException&>(ommException).getSystemExceptionAddress(),
			ommException.getText());
		break;
	case OmmException::OmmInvalidHandleExceptionEnum:
		errorClient.onInvalidHandle(static_cast<const OmmInvalidHandleException&>(ommException).getHandle(),
			ommException.getText());
		break;
	case OmmException::OmmInvalidUsageExceptionEnum:
		errorClient.onInvalidUsage(ommException.getText());
		break;
	case OmmException::OmmInaccessibleLogFileExceptionEnum:
		errorClient.onInaccessibleLogFile(static_cast<const OmmInaccessibleLogFileException&>(ommException).getFilename(),
			ommException.getText());
		break;
	case OmmException::OmmMemoryExhaustionExceptionEnum:
		errorClient.onMemoryExhaustion(ommException.getText());
		break;
	}
}

void OmmServerBaseImpl::addItemGroup(ItemInfo* itemInfo, const EmaBuffer& groupId)
{
	try
	{
		ClientSession::GroupIdToInfoHash** groupIdToInfoHashPtr = itemInfo->getClientSession()->getServiceGroupToItemInfoHash().find(itemInfo->getServiceId());

		if (groupIdToInfoHashPtr)
		{
			ClientSession::GroupIdToInfoHash* pGroupIdToInfoHash = *groupIdToInfoHashPtr;

			EmaVector<ItemInfo*>** itemInfoListPtr = pGroupIdToInfoHash->find(groupId);

			if (itemInfoListPtr)
			{
				EmaVector<ItemInfo*>* pItemInfoList = *itemInfoListPtr;
				pItemInfoList->push_back(itemInfo);
			}
			else
			{
				EmaVector<ItemInfo*>* pItemInfoList = new EmaVector<ItemInfo*>();
				pItemInfoList->push_back(itemInfo);

				itemInfo->getClientSession()->getItemInfoVectorList().push_back(pItemInfoList);
				pGroupIdToInfoHash->insert(groupId, pItemInfoList);
			}
		}
		else
		{
			ClientSession::GroupIdToInfoHash* pGroupIdToInfo = new ClientSession::GroupIdToInfoHash();

			EmaVector<ItemInfo*>* pItemInfoList = new EmaVector<ItemInfo*>(100);
			pItemInfoList->push_back(itemInfo);

			itemInfo->getClientSession()->getItemInfoVectorList().push_back(pItemInfoList);
			pGroupIdToInfo->insert(groupId, pItemInfoList);

			itemInfo->getClientSession()->getServiceGroupToItemInfoList().push_back(pGroupIdToInfo);
			itemInfo->getClientSession()->getServiceGroupToItemInfoHash().insert(itemInfo->getServiceId(), pGroupIdToInfo);
		}
	}
	catch (std::bad_alloc)
	{
		throwMeeException("Failed to allocate memory in OmmServerBaseImpl::addItemGroup()");
	}
}

void OmmServerBaseImpl::updateItemGroup(ItemInfo* itemInfo, const EmaBuffer& newGroupId)
{
	removeItemGroup(itemInfo);
	addItemGroup(itemInfo, newGroupId);
}

void OmmServerBaseImpl::removeItemGroup(ItemInfo* itemInfo)
{
	ClientSession::GroupIdToInfoHash** groupIdToInfoHashPtr = itemInfo->getClientSession()->getServiceGroupToItemInfoHash().find(itemInfo->getServiceId());

	if (groupIdToInfoHashPtr)
	{
		ClientSession::GroupIdToInfoHash* pGroupIdToInfoHash = *groupIdToInfoHashPtr;

		EmaVector<ItemInfo*>** itemInfoListPtr = pGroupIdToInfoHash->find(itemInfo->getItemGroup());

		if (itemInfoListPtr)
		{
			EmaVector<ItemInfo*>* pItemInfoList = *itemInfoListPtr;
			pItemInfoList->removeValue(itemInfo);

			if (pItemInfoList->size() == 0)
			{
				itemInfo->getClientSession()->getItemInfoVectorList().removeValue(pItemInfoList);
				delete pItemInfoList;

				pGroupIdToInfoHash->erase(itemInfo->getItemGroup());
			}
		}
	}

	UInt32 flags = itemInfo->getFlags();
	flags &= ~ItemInfo::ItemGroupFlag;
	itemInfo->setFlags(flags);
}

void OmmServerBaseImpl::mergeToGroupId(ClientSession* clientSession, UInt64 serviceId, const RsslBuffer& rsslGroupId, const RsslBuffer& rsslNewGroupId)
{
	EmaBufferInt groupId;
	groupId.setFromInt(rsslGroupId.data, rsslGroupId.length);

	EmaBufferInt newGroupId;
	newGroupId.setFromInt(rsslNewGroupId.data, rsslNewGroupId.length);

	if (groupId.toBuffer() == newGroupId.toBuffer())
	{
		return;
	}

	ClientSession::GroupIdToInfoHash** groupIdToInfoHashPtr = clientSession->getServiceGroupToItemInfoHash().find(serviceId);

	try
	{
		if (groupIdToInfoHashPtr)
		{
			ClientSession::GroupIdToInfoHash* pGroupIdToInfoHash = *groupIdToInfoHashPtr;

			EmaVector<ItemInfo*>** oldItemInfoListPtr = pGroupIdToInfoHash->find(groupId.toBuffer());

			if (oldItemInfoListPtr)
			{
				EmaVector<ItemInfo*>& oldItemInfoList = *(*oldItemInfoListPtr);
				EmaVector<ItemInfo*>** mergeToItemInfoListPtr = pGroupIdToInfoHash->find(newGroupId.toBuffer());

				if (mergeToItemInfoListPtr)
				{
					EmaVector<ItemInfo*>* pMergeToItemInfoList = *mergeToItemInfoListPtr;

					for (UInt32 index = 0; index < oldItemInfoList.size(); index++)
					{
						pMergeToItemInfoList->push_back(oldItemInfoList[index]);
					}

					clientSession->getItemInfoVectorList().removeValue(*oldItemInfoListPtr);
					delete (*oldItemInfoListPtr);

					pGroupIdToInfoHash->erase(groupId.toBuffer());
				}
				else
				{
					EmaVector<ItemInfo*>* pMergeToItemInfoList = new EmaVector<ItemInfo*>(oldItemInfoList.size() + 100);

					for (UInt32 index = 0; index < oldItemInfoList.size(); index++)
					{
						pMergeToItemInfoList->push_back(oldItemInfoList[index]);
					}

					clientSession->getItemInfoVectorList().removeValue(*oldItemInfoListPtr);
					delete (*oldItemInfoListPtr);

					pGroupIdToInfoHash->erase(groupId.toBuffer());

					clientSession->getItemInfoVectorList().push_back(pMergeToItemInfoList);
					pGroupIdToInfoHash->insert(newGroupId.toBuffer(), pMergeToItemInfoList);
				}
			}
		}
	}
	catch (std::bad_alloc)
	{
		throwMeeException("Failed to allocate memory in OmmServerBaseImpl::mergeToGroupId()");
	}
}

void OmmServerBaseImpl::removeGroupId(ClientSession* clientSession, UInt64 serviceId, const RsslBuffer& rsslGroupId)
{
	ClientSession::GroupIdToInfoHash** groupIdToInfoHashPtr = clientSession->getServiceGroupToItemInfoHash().find(serviceId);

	ItemInfo* pItemInfo;
	EmaBufferInt groupId;
	groupId.setFromInt(rsslGroupId.data, rsslGroupId.length);

	if (groupIdToInfoHashPtr)
	{
		ClientSession::GroupIdToInfoHash* pGroupIdToInfoHash = *groupIdToInfoHashPtr;

		EmaVector<ItemInfo*>** itemInfoListPtr = pGroupIdToInfoHash->find(groupId.toBuffer());

		if (itemInfoListPtr)
		{
			EmaVector<ItemInfo*>& itemInfoList = *(*itemInfoListPtr);

			for (UInt32 index = 0; index < itemInfoList.size(); index++)
			{
				pItemInfo = itemInfoList[index];
				removeItemInfo(pItemInfo, false);
			}

			clientSession->getItemInfoVectorList().removeValue(*itemInfoListPtr);
			delete (*itemInfoListPtr);

			pGroupIdToInfoHash->erase(groupId.toBuffer());
		}
	}
}

void OmmServerBaseImpl::removeServiceId(ClientSession* clientSession, UInt64 serviceId)
{
	ClientSession::GroupIdToInfoHash** groupIdToInfoHashPtr = clientSession->getServiceGroupToItemInfoHash().find(serviceId);

	if (groupIdToInfoHashPtr)
	{
		ClientSession::GroupIdToInfoHash* pGroupIdToInfoHash = *groupIdToInfoHashPtr;

		for (UInt32 index = 0; index < clientSession->getItemInfoVectorList().size(); index++)
		{
			EmaVector<ItemInfo*>* pItemInfoList = clientSession->getItemInfoVectorList()[index];

			if ((*pItemInfoList)[0]->getServiceId() == serviceId)
			{
				clientSession->getItemInfoVectorList().removePosition(index);
				delete pItemInfoList;
			}
		}

		clientSession->getServiceGroupToItemInfoHash().erase(serviceId);
		clientSession->getServiceGroupToItemInfoList().removeValue(pGroupIdToInfoHash);
	}

	ItemInfo* pItemInfo, *pNextItemInfo;

	pNextItemInfo = clientSession->getItemInfoList().front();

	while (pNextItemInfo)
	{
		pItemInfo = pNextItemInfo;

		if (  pItemInfo->hasServiceId() && (pItemInfo->getServiceId() == serviceId) )
		{
			if (pItemInfo->getDomainType() > 5 )
			{
				pNextItemInfo = pItemInfo->next();
				removeItemInfo(pItemInfo, false);
				continue;
			}
		}

		pNextItemInfo = pItemInfo->next();
	}
}

size_t OmmServerBaseImpl::UInt64rHasher::operator()(const UInt64& value) const
{
	return value;
}

bool OmmServerBaseImpl::UInt64Equal_To::operator()(const UInt64& x, const UInt64& y) const
{
	return x == y ? true : false;
}
