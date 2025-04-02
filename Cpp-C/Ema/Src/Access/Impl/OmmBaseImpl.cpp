/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2025 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "OmmBaseImpl.h"
#include "LoginCallbackClient.h"
#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "OmmConsumerClient.h"
#include "OmmProviderClient.h"
#include "OmmOAuth2ConsumerClient.h"
#include "OmmOAuth2CredentialImpl.h"

#include "OmmInaccessibleLogFileException.h"
#include "OmmInvalidHandleException.h"
#include "OmmSystemException.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"
#include "OmmJsonConverterException.h"
#include "OmmNiProviderImpl.h"
#include "PreferredHostOptions.h"
#ifndef NO_ETA_CPU_BIND
#include "rtr/rsslBindThread.h"
#endif
#include "GetTime.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define	EMA_BIG_STR_BUFF_SIZE (1024*4)

using namespace refinitiv::ema::access;

/* Dummy no-op consumer class client for initializing handlers */
/* This should never be used */
class DummyConsClient : public refinitiv::ema::access::OmmConsumerClient
{
};

/* Dummy no-op provider class client for initializing handlers */
/* This should never be used */
class DummyProvClient : public refinitiv::ema::access::OmmProviderClient
{
};

/* Dummy no-op OAuth2 Consumer class client for initializing handlers */
/* This should never be used */
class DummyOAuth2ConsClient : public refinitiv::ema::access::OmmOAuth2ConsumerClient
{
};

static DummyConsClient defaultConsClient;
static DummyProvClient defaultProvClient;
static DummyOAuth2ConsClient defaultOAuthConsClient;

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_pRsslReactor(0), 
	_consAdminClient(defaultConsClient),
	_consOAuthClient(defaultOAuthConsClient),
	_provAdminClient(defaultProvClient),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount( 0 ),
	_atExit( false ),
	_eventTimedOut( false ),
	_bMsgDispatched( false ),
	_bEventReceived( false ),
	_hasProvAdminClient( false ),
	_hasConsAdminClient( false ),
	_hasConsOAuthClient( false ),
	_inOAuthCallback(false),
	_isInitialized(false),
	_inLoginCredentialCallback(false),
	_pErrorClientHandler( 0 ),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = 0;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif
	clearRsslErrorInfo( &_reactorDispatchErrorInfo );
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmConsumerClient& adminClient, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(adminClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(defaultOAuthConsClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(true),
	_hasProvAdminClient(false),
	_hasConsOAuthClient(false),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif
	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(adminClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(oAuthClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(true),
	_hasProvAdminClient(false),
	_hasConsOAuthClient(true),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif
	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmOAuth2ConsumerClient& oAuthClient, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(defaultConsClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(oAuthClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(false),
	_hasProvAdminClient(false),
	_hasConsOAuthClient(true),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif
	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmProviderClient& adminClient, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(defaultConsClient),
	_provAdminClient(adminClient),
	_consOAuthClient(defaultOAuthConsClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(false),
	_hasProvAdminClient(true),
	_hasConsOAuthClient(false),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif
	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}


OmmBaseImpl::OmmBaseImpl( ActiveConfig& activeConfig, OmmConsumerErrorClient& client ) :
	OmmCommonImpl(),
	_activeConfig( activeConfig ),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state( NotInitializedEnum ),
	_consAdminClient(defaultConsClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(defaultOAuthConsClient),
	_pRsslReactor( 0 ),
	_pChannelCallbackClient( 0 ),
	_pLoginCallbackClient( 0 ),
	_pDirectoryCallbackClient( 0 ),
	_pDictionaryCallbackClient( 0 ),
	_pItemCallbackClient( 0 ),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient( 0 ),
	_pipe(),
	_pipeWriteCount( 0 ),
	_atExit( false ),
	_eventTimedOut( false ),
	_bMsgDispatched( false ),
	_bEventReceived( false ),
	_hasConsAdminClient( false ),
	_hasProvAdminClient( false ),
	_hasConsOAuthClient(false),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler( 0 ),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;
	_adminClosure = 0;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif

	try
	{
		_pErrorClientHandler = new ErrorClientHandler( client );
	}
	catch ( std::bad_alloc& )
	{
		client.onMemoryExhaustion( "Failed to allocate memory in OmmBaseImpl( ActiveConfig& , OmmConsumerErrorClient& )" );
	}

	clearRsslErrorInfo( &_reactorDispatchErrorInfo );
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& client, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(defaultConsClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(oAuthClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(false),
	_hasProvAdminClient(false),
	_hasConsOAuthClient(true),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif

	try
	{
		_pErrorClientHandler = new ErrorClientHandler(client);
	}
	catch (std::bad_alloc&)
	{
		client.onMemoryExhaustion("Failed to allocate memory in OmmBaseImpl( ActiveConfig& , OmmConsumerErrorClient& )");
	}

	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmConsumerClient& adminClient, OmmConsumerErrorClient& errorClient, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(adminClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(defaultOAuthConsClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(true),
	_hasProvAdminClient(false),
	_hasConsOAuthClient(false),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif

	try
	{
		_pErrorClientHandler = new ErrorClientHandler(errorClient);
	}
	catch (std::bad_alloc&)
	{
		errorClient.onMemoryExhaustion("Failed to allocate memory in OmmBaseImpl( ActiveConfig& , OmmConsumerErrorClient& )");
	}

	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(adminClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(oAuthClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(true),
	_hasProvAdminClient(false),
	_hasConsOAuthClient(true),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;
	_LoginReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif

	try
	{
		_pErrorClientHandler = new ErrorClientHandler(errorClient);
	}
	catch (std::bad_alloc&)
	{
		errorClient.onMemoryExhaustion("Failed to allocate memory in OmmBaseImpl( ActiveConfig& , OmmConsumerErrorClient& )");
	}

	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmBaseImpl::OmmBaseImpl( ActiveConfig& activeConfig, OmmProviderErrorClient& client ) :
	OmmCommonImpl(),
	_activeConfig( activeConfig ),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state( NotInitializedEnum ),
	_consAdminClient(defaultConsClient),
	_provAdminClient(defaultProvClient),
	_consOAuthClient(defaultOAuthConsClient),
	_pRsslReactor( 0 ),
	_pChannelCallbackClient( 0 ),
	_pLoginCallbackClient( 0 ),
	_pDirectoryCallbackClient( 0 ),
	_pDictionaryCallbackClient( 0 ),
	_pItemCallbackClient( 0 ),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient( 0 ),
	_pipe(),
	_pipeWriteCount( 0 ),
	_atExit( false ),
	_eventTimedOut( false ),
	_bMsgDispatched( false ),
	_bEventReceived( false ),
	_hasConsAdminClient(false),
	_hasProvAdminClient(false),
	_hasConsOAuthClient(false),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler( 0 ),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = 0;
	_OAuthReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif

	try
	{
		_pErrorClientHandler = new ErrorClientHandler( client );
	}
	catch ( std::bad_alloc& )
	{
		client.onMemoryExhaustion( "Failed to allocate memory in OmmBaseImpl( ActiveConfig& , OmmNiProviderErrorClient& )" );
	}

	clearRsslErrorInfo( &_reactorDispatchErrorInfo );
}

OmmBaseImpl::OmmBaseImpl(ActiveConfig& activeConfig, OmmProviderClient& adminClient, OmmProviderErrorClient& errorClient, void* adminClosure) :
	OmmCommonImpl(),
	_activeConfig(activeConfig),
	_userLock(),
	_dispatchLock(),
	_pipeLock(),
	_reactorDispatchErrorInfo(),
	_state(NotInitializedEnum),
	_consAdminClient(defaultConsClient),
	_provAdminClient(adminClient),
	_consOAuthClient(defaultOAuthConsClient),
	_pRsslReactor(0),
	_pChannelCallbackClient(0),
	_pLoginCallbackClient(0),
	_pDirectoryCallbackClient(0),
	_pDictionaryCallbackClient(0),
	_pItemCallbackClient(0),
	_pRestLoggingCallbackClient(0),
	_pLoggerClient(0),
	_pipe(),
	_pipeWriteCount(0),
	_atExit(false),
	_eventTimedOut(false),
	_bMsgDispatched(false),
	_bEventReceived(false),
	_hasConsAdminClient(false),
	_hasProvAdminClient(true),
	_hasConsOAuthClient(false),
	_inOAuthCallback(false),
	_inLoginCredentialCallback(false),
	_isInitialized(false),
	_pErrorClientHandler(0),
	_theTimeOuts(),
	_bApiDispatchThreadStarted(false),
	_bUninitializeInvoked(false),
	_negotiatedPingTimeout( 0 )
{
	_adminClosure = adminClosure;
	_OAuthReactorConfig = NULL;

#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#endif

	try
	{
		_pErrorClientHandler = new ErrorClientHandler(errorClient);
	}
	catch (std::bad_alloc&)
	{
		errorClient.onMemoryExhaustion("Failed to allocate memory in OmmBaseImpl( ActiveConfig& , OmmConsumerErrorClient& )");
	}

	clearRsslErrorInfo(&_reactorDispatchErrorInfo);
}

OmmBaseImpl::~OmmBaseImpl()
{
	UInt32 i;
	if (_pRestLoggingCallbackClient)
		delete _pRestLoggingCallbackClient;

	if ( _pErrorClientHandler )
		delete _pErrorClientHandler;

	if (_LoginReactorConfig != NULL)
	{
		for (i = 0; i < _LoginRequestMsgs.size(); i++)
		{
			if (_LoginReactorConfig[i] != NULL)
				free(_LoginReactorConfig[i]);
		}

		free(_LoginReactorConfig);
	}

	if (_OAuthReactorConfig != NULL)
	{
		for (i = 0; i < _oAuth2Credentials.size(); i++)
		{
			const_cast<EmaString&>(_oAuth2Credentials[i]->getClientSecret()).secureClear();
			const_cast<EmaString&>(_oAuth2Credentials[i]->getClientJWK()).secureClear();
			const_cast<EmaString&>(_oAuth2Credentials[i]->getPassword()).secureClear();

			if (_OAuthReactorConfig[i] != NULL)
				free(_OAuthReactorConfig[i]);
		}
		free(_OAuthReactorConfig);
	}

	for (i = 0; i < _LoginRequestMsgs.size(); i++)
	{
		delete _LoginRequestMsgs[i];
		_LoginRequestMsgs.removePosition(i);
	}

	for (i = 0; i < _oAuth2Credentials.size(); i++)
	{
		delete _oAuth2Credentials[i];
		_oAuth2Credentials.removePosition(i);
	}

}

UInt8 OmmBaseImpl::getOAuthArrayIndex(EmaString& channelName)
{
	UInt8 index = 0;
	char* pToken = NULL;
	char* pNextToken = NULL;
	EmaString channelListName;
	UInt32 i;


	/* Try to find a match in the login request channel lists */
	for (i = 0; i < _oAuth2Credentials.size(); i++)
	{
		/* Strtok changes the underlying c string(replaces the found token with a NULL character), so we need to make a temporary copy here */
		EmaString tmpChannelList = _oAuth2Credentials[i]->getChannelList();
		pToken = strtok(const_cast<char*>(tmpChannelList.c_str()), ",");
		do
		{
			if (pToken)
			{
				channelListName = pToken;
				pNextToken = strtok(NULL, ",");
				if (channelListName.trimWhitespace().caseInsensitiveCompare(channelName))
					return i;
			}

			pToken = pNextToken;
		} while (pToken != NULL);
	}

	/* No match found, see if a default(no configured channel list) has been set */
	for (i = 0; i < _oAuth2Credentials.size(); i++)
	{
		if (_oAuth2Credentials[i]->getChannelList().empty())
			return i;
	}

	EmaString errorMsg("Unable to match a configured oAuth login for channelName ");
	errorMsg.append(channelName).append(".");
	throwIceException(errorMsg);
	return 0;
}

void OmmBaseImpl::clearSensitiveInfo()
{
	UInt32 i;

	if (_LoginReactorConfig != NULL)
	{
		for (i = 0; i < _LoginRequestMsgs.size(); i++)
		{
			if (_LoginReactorConfig[i] != NULL)
				free(_LoginReactorConfig[i]);
		}

		free(_LoginReactorConfig);
	}

	_LoginReactorConfig = NULL;


	if (_OAuthReactorConfig != NULL)
	{
		for (i = 0; i < _oAuth2Credentials.size(); i++)
		{
			const_cast<EmaString&>(_oAuth2Credentials[i]->getClientSecret()).secureClear();
			const_cast<EmaString&>(_oAuth2Credentials[i]->getClientJWK()).secureClear();
			const_cast<EmaString&>(_oAuth2Credentials[i]->getPassword()).secureClear();

			if (_OAuthReactorConfig[i] != NULL)
				free(_OAuthReactorConfig[i]);
		}
		free(_OAuthReactorConfig);
	}

	_OAuthReactorConfig = NULL;

}

UInt8 OmmBaseImpl::getLoginArrayIndex(EmaString& channelName)
{
	UInt8 index = 0;
	char* pToken = NULL;
	char* pNextToken = NULL;
	EmaString channelListName;
	UInt32 i;


	/* Try to find a match in the login request channel lists */
	for (i = 0; i < _LoginRequestMsgs.size(); i++)
	{
		/* Strtok changes the underlying c string(replaces the found token with a NULL character), so we need to make a temporary copy here */
		EmaString tmpChannelList = _LoginRequestMsgs[i]->channelList();

		pToken = strtok(const_cast<char*>(tmpChannelList.c_str()), ",");
		do
		{
			if (pToken)
			{
				channelListName = pToken;
				pNextToken = strtok(NULL, ",");
				if (channelListName.trimWhitespace().caseInsensitiveCompare(channelName))
					return i;
			}

			pToken = pNextToken;
		} while (pToken != NULL);
	}

	/* No match found, see if a default(no configured channel list) has been set */
	for (i = 0; i < _LoginRequestMsgs.size(); i++)
	{
		if (_LoginRequestMsgs[i]->channelList().empty())
			return i;
	}

	EmaString errorMsg("Unable to match a configured login for channelName ");
	errorMsg.append(channelName).append(".");
	throwIceException(errorMsg);
	return 0;
}

void OmmBaseImpl::readConfig(EmaConfigImpl* pConfigImpl)
{
	UInt64 id = OmmBaseImplMap<OmmBaseImpl>::add(this);
	UInt32 i;

	_activeConfig.configuredName = pConfigImpl->getConfiguredName();
	_activeConfig.instanceName = _activeConfig.configuredName;
	_activeConfig.instanceName.append("_").append(id);

	const UInt32 maxUInt32(0xFFFFFFFF);
	const Int32 maxInt32(0x7FFFFFFF);
	UInt64 tmp;
	EmaString instanceNodeName(pConfigImpl->getInstanceNodeName());
	instanceNodeName.append(_activeConfig.configuredName).append("|");

	if (pConfigImpl->get<UInt64>(instanceNodeName + "ItemCountHint", tmp))
		_activeConfig.itemCountHint = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "ServiceCountHint", tmp))
		_activeConfig.serviceCountHint = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "RequestTimeout", tmp))
		_activeConfig.requestTimeout = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigImpl->get< UInt64 >(instanceNodeName + "LoginRequestTimeOut", tmp))
		_activeConfig.loginRequestTimeOut = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigImpl->get< UInt64 >(instanceNodeName + "RestRequestTimeOut", tmp))
		_activeConfig.restRequestTimeOut = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	pConfigImpl->get<Int64>(instanceNodeName + "DispatchTimeoutApiThread", _activeConfig.dispatchTimeoutApiThread);

	pConfigImpl->get<Double>(instanceNodeName + "TokenReissueRatio", _activeConfig.tokenReissueRatio);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "CatchUnhandledException", tmp))
		_activeConfig.catchUnhandledException = static_cast<UInt32>(tmp > 0 ? true : false);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "MaxDispatchCountApiThread", tmp))
		_activeConfig.maxDispatchCountApiThread = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "MaxDispatchCountUserThread", tmp))
		_activeConfig.maxDispatchCountUserThread = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "SendJsonConvError", tmp))
		_activeConfig.sendJsonConvError = tmp > 0 ? true : false;

	if (pConfigImpl->get<UInt64>(instanceNodeName + "EnablePreferredHostOptions", tmp))
		_activeConfig.enablePreferredHostOptions  = tmp > 0 ? true : false;

	if (pConfigImpl->get<UInt64>(instanceNodeName + "PHDetectionTimeInterval", tmp))
		_activeConfig.phDetectionTimeInterval = static_cast<UInt32>(tmp > maxUInt32 ? maxUInt32 : tmp);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "PHFallBackWithInWSBGroup", tmp))
		_activeConfig.phFallBackWithInWSBGroup = tmp > 0 ? true : false;

	pConfigImpl->get<EmaString>(instanceNodeName + "PHDetectionTimeSchedule", _activeConfig.phDetectionTimeSchedule);
	pConfigImpl->get<EmaString>(instanceNodeName + "PreferredChannelName", _activeConfig.preferredChannelName);
	pConfigImpl->get<EmaString>(instanceNodeName + "PreferredWSBChannelName", _activeConfig.preferredWSBChannelName);

	if (pConfigImpl->isUserSetShouldInitializeCPUIDlib())
	{
		_activeConfig.shouldInitializeCPUIDlib = pConfigImpl->getShouldInitializeCPUIDlib();
	}
	else
	{
		if (pConfigImpl->get<UInt64>(instanceNodeName + "ShouldInitializeCPUIDlib", tmp))
			_activeConfig.shouldInitializeCPUIDlib = tmp > 0 ? true : false;
	}

	if (pConfigImpl->getUserSpecifiedRestProxyHostname().length() > 0)
	{
		_activeConfig.restProxyHostName = pConfigImpl->getUserSpecifiedRestProxyHostname();
	}
	else
	{
		pConfigImpl->get<EmaString>(instanceNodeName + "RestProxyHostName", _activeConfig.restProxyHostName);
	}

	if (pConfigImpl->getUserSpecifiedRestProxyPort().length() > 0)
	{
		_activeConfig.restProxyPort = pConfigImpl->getUserSpecifiedRestProxyPort();
	}
	else
	{
		pConfigImpl->get<EmaString>(instanceNodeName + "RestProxyPort", _activeConfig.restProxyPort);
	}

	if (pConfigImpl->getUserSpecifiedRestProxyUserName().length() > 0)
	{
		_activeConfig.restProxyUserName = pConfigImpl->getUserSpecifiedRestProxyUserName();
	}

	if (pConfigImpl->getUserSpecifiedRestProxyPasswd().length() > 0)
	{
		_activeConfig.restProxyPasswd = pConfigImpl->getUserSpecifiedRestProxyPasswd();
	}

	if (pConfigImpl->getUserSpecifiedRestProxyDomain().length() > 0)
	{
		_activeConfig.restProxyDomain = pConfigImpl->getUserSpecifiedRestProxyDomain();
	}

	Int64 tmp1;
	
	if (pConfigImpl->get<Int64>(instanceNodeName + "MaxEventsInPool", tmp1))
	{
		_activeConfig.maxEventsInPool = static_cast<Int32>(tmp1 > maxInt32 ? maxInt32 : tmp1 < -1 ? -1 : tmp1);
	}

	if (pConfigImpl->getUserSpecifiedLibSslName().length() > 0)
	{
		_activeConfig.libCryptoName = pConfigImpl->getUserSpecifiedLibCryptoName();
		_activeConfig.libSslName = pConfigImpl->getUserSpecifiedLibSslName();
	}

	if (pConfigImpl->getUserSpecifiedLibcurlName().length() > 0)
	{
		_activeConfig.libcurlName = pConfigImpl->getUserSpecifiedLibcurlName();
	}

	if (pConfigImpl->get<Int64>(instanceNodeName + "ReconnectAttemptLimit", tmp1))
	{
		_activeConfig.setReconnectAttemptLimit(tmp1);
	}

	if (pConfigImpl->get<Int64>(instanceNodeName + "ReconnectMinDelay", tmp1))
	{
		_activeConfig.setReconnectMinDelay(tmp1);
	}

	if (pConfigImpl->get<Int64>(instanceNodeName + "ReconnectMaxDelay", tmp1))
	{
		_activeConfig.setReconnectMaxDelay(tmp1);
	}

	pConfigImpl->get<EmaString>(instanceNodeName + "XmlTraceFileName", _activeConfig.xmlTraceFileName);

	if (pConfigImpl->get<Int64>(instanceNodeName + "XmlTraceMaxFileSize", tmp1) && tmp1 > 0)
	{
		_activeConfig.xmlTraceMaxFileSize = tmp1;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTraceToFile", tmp))
	{
		_activeConfig.xmlTraceToFile = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTraceToStdout", tmp))
	{
		_activeConfig.xmlTraceToStdout = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTraceToMultipleFiles", tmp))
	{
		_activeConfig.xmlTraceToMultipleFiles = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTraceWrite", tmp))
	{
		_activeConfig.xmlTraceWrite = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTraceRead", tmp))
	{
		_activeConfig.xmlTraceRead = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTracePing", tmp))
	{
		_activeConfig.xmlTracePing = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTracePingOnly", tmp))
	{
		_activeConfig.xmlTracePingOnly = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTraceHex", tmp))
	{
		_activeConfig.xmlTraceHex = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "XmlTraceDump", tmp))
	{
		_activeConfig.xmlTraceDump = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "MsgKeyInUpdates", tmp))
	{
		_activeConfig.msgKeyInUpdates = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "DefaultServiceID", tmp))
	{
		_activeConfig.defaultServiceIDForConverter = tmp <= 0xFFFF ? (UInt16)tmp: 0xFFFF;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "JsonExpandedEnumFields", tmp))
	{
		_activeConfig.jsonExpandedEnumFields = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "CatchUnknownJsonFids", tmp))
	{
		_activeConfig.catchUnknownJsonFids = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "CatchUnknownJsonKeys", tmp))
	{
		_activeConfig.catchUnknownJsonKeys = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "CloseChannelFromConverterFailure", tmp))
	{
		_activeConfig.closeChannelFromFailure = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "OutputBufferSize", tmp))
	{
		_activeConfig.outputBufferSize = tmp <= maxUInt32 ? (UInt32)tmp : maxUInt32;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "JsonTokenIncrementSize", tmp))
	{
		_activeConfig.jsonTokenIncrementSize = tmp <= maxUInt32 ? (UInt32)tmp : maxUInt32;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "EnableRtt", tmp))
	{
		_activeConfig.enableRtt = tmp > 0 ? true : false;
	}

	pConfigImpl->get<EmaString>(instanceNodeName + "RestLogFileName", _activeConfig.restLogFileName);

	if (pConfigImpl->get<UInt64>(instanceNodeName + "RestEnableLog", tmp))
	{
		_activeConfig.restEnableLog = tmp > 0 ? true : false;
	}

	if (pConfigImpl->get<UInt64>(instanceNodeName + "RestVerboseMode", tmp))
	{
		_activeConfig.restVerboseMode = tmp > 0 ? true : false;
	}


	Int64 tempInt = DEFAULT_REISSUE_TOKEN_ATTEMP_LIMIT;
	if (pConfigImpl->get<Int64>(instanceNodeName + "ReissueTokenAttemptLimit", tempInt))
		_activeConfig.reissueTokenAttemptLimit = tempInt;
	else
		_activeConfig.reissueTokenAttemptLimit = tempInt;

	tempInt = DEFAULT_REISSUE_TOKEN_ATTEMP_INTERVAL;
	if (pConfigImpl->get<Int64>(instanceNodeName + "ReissueTokenAttemptInterval", tempInt))
		_activeConfig.reissueTokenAttemptInterval = tempInt;
	else
		_activeConfig.reissueTokenAttemptInterval = tempInt;

	pConfigImpl->getLoggerName( _activeConfig.configuredName, _activeConfig.loggerConfig.loggerName );

	_activeConfig.loggerConfig.minLoggerSeverity = OmmLoggerClient::SuccessEnum;
	_activeConfig.loggerConfig.loggerFileName = "emaLog";
	_activeConfig.loggerConfig.loggerType = OmmLoggerClient::FileEnum;
	_activeConfig.loggerConfig.includeDateInLoggerOutput = false;
	_activeConfig.loggerConfig.maxFileSize = 0;
	_activeConfig.loggerConfig.maxFileNumber = 0;

	if ( _activeConfig.loggerConfig.loggerName.length() )
	{
		EmaString loggerNodeName( "LoggerGroup|LoggerList|Logger." );
		loggerNodeName.append( _activeConfig.loggerConfig.loggerName ).append( "|" );

		EmaString name;
		if ( !pConfigImpl->get< EmaString >( loggerNodeName + "Name", name ) )
		{
			EmaString errorMsg( "no configuration exists for consumer logger [" );
			errorMsg.append( loggerNodeName ).append( "]; will use logger defaults if not config programmatically" );
			pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::VerboseEnum );
		}

		pConfigImpl->get<OmmLoggerClient::LoggerType>( loggerNodeName + "LoggerType", _activeConfig.loggerConfig.loggerType );

		if ( _activeConfig.loggerConfig.loggerType == OmmLoggerClient::FileEnum )
			pConfigImpl->get<EmaString>( loggerNodeName + "FileName", _activeConfig.loggerConfig.loggerFileName );

		pConfigImpl->get<OmmLoggerClient::Severity>( loggerNodeName + "LoggerSeverity", _activeConfig.loggerConfig.minLoggerSeverity );

		UInt64 idilo( 0 );
		if ( pConfigImpl->get< UInt64 >( loggerNodeName + "IncludeDateInLoggerOutput", idilo ) )
			_activeConfig.loggerConfig.includeDateInLoggerOutput = idilo == 1 ? true : false ;

		if (pConfigImpl->get< UInt64 >(loggerNodeName + "MaxLogFileSize", tmp))
			_activeConfig.loggerConfig.maxFileSize = tmp <= maxUInt32 ? (UInt32)tmp : maxUInt32;

		if (pConfigImpl->get< UInt64 >(loggerNodeName + "NumberOfLogFiles", tmp))
			_activeConfig.loggerConfig.maxFileNumber = tmp <= maxUInt32 ? (UInt32)tmp : maxUInt32;
	}
	else
		_activeConfig.loggerConfig.loggerName.set( "Logger" );

	if ( ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure() )
	{
		ppc->retrieveLoggerConfig( _activeConfig.loggerConfig.loggerName , _activeConfig );
	}

	EmaString channelOrChannelSet;
	EmaString channelName;
	bool setDefaultChannelName = false;
	EmaString warmStandbyChannelSet;
	EmaString warmStandbyChannelName;
	bool foundWSBProgrammaticConfig;

	pConfigImpl->getWarmStandbyChannelName(_activeConfig.configuredName, warmStandbyChannelSet, foundWSBProgrammaticConfig);

	pConfigImpl->getChannelName( _activeConfig.configuredName, channelOrChannelSet);
	if (channelOrChannelSet.trimWhitespace().length() > 0 )
	{
		char* pToken = NULL;
		char* pNextToken = NULL;
		pToken = strtok( const_cast<char*>(channelOrChannelSet.c_str() ), "," );
		do
		{
			if ( pToken )
			{
				channelName = pToken;
				pNextToken = strtok(NULL, ",");
				ChannelConfig* newChannelConfig = readChannelConfig( pConfigImpl, ( channelName.trimWhitespace() ), (pNextToken == NULL ? true : false) );
				_activeConfig.configChannelSet.push_back( newChannelConfig );

			}

			pToken = pNextToken;
		}
		while ( pToken != NULL );
	}
	else if(warmStandbyChannelSet.trimWhitespace().length() == 0) /* Create a default channel where there is no both Channel and warm standby channel */
	{
		useDefaultConfigValues( EmaString( "Channel" ), pConfigImpl->getUserSpecifiedHostname(), pConfigImpl->getUserSpecifiedPort().userSpecifiedValue );
	}

	if ( ProgrammaticConfigure* ppc  = pConfigImpl->getProgrammaticConfigure() )
	{
		ppc->retrieveCommonConfig( _activeConfig.configuredName, _activeConfig );
		bool isProgmaticCfgChannelName = ppc->getActiveChannelName( _activeConfig.configuredName, channelName.trimWhitespace() );
		bool isProgramatiCfgChannelset = ppc->getActiveChannelSet( _activeConfig.configuredName, channelOrChannelSet.trimWhitespace() );
		unsigned int posInProgCfg  = 0;

		if ( isProgmaticCfgChannelName )
		{
			_activeConfig.clearChannelSet();
			ChannelConfig* fileChannelConfig = readChannelConfig( pConfigImpl, ( channelName.trimWhitespace() ), true);

			int chanConfigByFuncCall = 0;
			if (pConfigImpl->getUserSpecifiedHostname().length() > 0)
				chanConfigByFuncCall = SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL;
			if(pConfigImpl->getUserSpecifiedPort().userSet == true && pConfigImpl->getUserSpecifiedPort().userSpecifiedValue.length() > 0)
				chanConfigByFuncCall |= SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL;
			if (pConfigImpl->getUserSpecifiedProxyHostname().length() > 0)
				chanConfigByFuncCall |= PROXY_HOST_CONFIG_BY_FUNCTION_CALL;
			if (pConfigImpl->getUserSpecifiedProxyPort().length() > 0)
				chanConfigByFuncCall |= PROXY_PORT_CONFIG_BY_FUNCTION_CALL;
			if (pConfigImpl->getUserSpecifiedObjectName().length() > 0)
				chanConfigByFuncCall |= TUNNELING_OBJNAME_CONFIG_BY_FUNCTION_CALL;
			if (pConfigImpl->getUserSpecifiedProxyUserName().length() > 0)
				chanConfigByFuncCall |= PROXY_USERNAME_CONFIG_BY_FUNCTION_CALL;
			if (pConfigImpl->getUserSpecifiedProxyPasswd().length() > 0)
				chanConfigByFuncCall |= PROXY_PASSWD_CONFIG_BY_FUNCTION_CALL;
			if (pConfigImpl->getUserSpecifiedProxyDomain().length() > 0)
				chanConfigByFuncCall |= PROXY_DOMAIN_CONFIG_BY_FUNCTION_CALL;


			ppc->retrieveChannelConfig( channelName.trimWhitespace(), _activeConfig, chanConfigByFuncCall, fileChannelConfig );
			if ( !( ActiveConfig::findChannelConfig( _activeConfig.configChannelSet, channelName.trimWhitespace(), posInProgCfg ) ) )
				_activeConfig.configChannelSet.push_back( fileChannelConfig );
			else
			{
				if ( fileChannelConfig )
					delete fileChannelConfig;
			}
		}
		else if ( isProgramatiCfgChannelset )
		{
			_activeConfig.clearChannelSet();
			char* pToken = NULL;
			char* pNextToken = NULL;
			pToken = strtok( const_cast<char*>(channelOrChannelSet.c_str() ), "," );
			while ( pToken != NULL )
			{
				channelName = pToken;
				pNextToken = strtok(NULL, ",");
				ChannelConfig* fileChannelConfig = readChannelConfig( pConfigImpl, ( channelName.trimWhitespace() ), (pNextToken == NULL ? true : false));

				int chanConfigByFuncCall = 0;
				if (pConfigImpl->getUserSpecifiedHostname().length() > 0)
					chanConfigByFuncCall = SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL;
				if (pConfigImpl->getUserSpecifiedPort().userSet == true && pConfigImpl->getUserSpecifiedPort().userSpecifiedValue.length() > 0)
					chanConfigByFuncCall |= SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL;
				if (pConfigImpl->getUserSpecifiedProxyHostname().length() > 0)
					chanConfigByFuncCall |= PROXY_HOST_CONFIG_BY_FUNCTION_CALL;
				if (pConfigImpl->getUserSpecifiedProxyPort().length() > 0)
					chanConfigByFuncCall |= PROXY_PORT_CONFIG_BY_FUNCTION_CALL;
				if (pConfigImpl->getUserSpecifiedObjectName().length() > 0)
					chanConfigByFuncCall |= TUNNELING_OBJNAME_CONFIG_BY_FUNCTION_CALL;

				ppc->retrieveChannelConfig(channelName.trimWhitespace(), _activeConfig, chanConfigByFuncCall, fileChannelConfig );
				if ( !( ActiveConfig::findChannelConfig( _activeConfig.configChannelSet, channelName.trimWhitespace(), posInProgCfg ) ) )
					_activeConfig.configChannelSet.push_back( fileChannelConfig );
				else
				{
					if ( fileChannelConfig )
						delete fileChannelConfig;
				}

				pToken = pNextToken;
			}
		}
	}

	if ( _activeConfig.configChannelSet.size() == 0 )
	{
		if (warmStandbyChannelSet.trimWhitespace().length() == 0)
		{
			EmaString channelName("Channel");
			useDefaultConfigValues(channelName, pConfigImpl->getUserSpecifiedHostname(), pConfigImpl->getUserSpecifiedPort().userSpecifiedValue);
		}
	}
	else
	{
		// Only assigns the default hostname and port for the encrypted connections when the session management feature is disable
		ChannelConfig* pChannelConfig;
		SocketChannelConfig* pSocketChannelConfig;
		for (UInt32 index = 0; index < _activeConfig.configChannelSet.size(); index++)
		{
			pChannelConfig = _activeConfig.configChannelSet[index];
			if (pChannelConfig->connectionType == RSSL_CONN_TYPE_ENCRYPTED)
			{
				pSocketChannelConfig = (SocketChannelConfig*)pChannelConfig;
				if (pSocketChannelConfig->enableSessionMgnt == false)
				{
					if (pSocketChannelConfig->hostName.length() == 0)
						pSocketChannelConfig->hostName = DEFAULT_HOST_NAME;

					if (pSocketChannelConfig->serviceName.length() == 0)
						pSocketChannelConfig->serviceName = _activeConfig.defaultServiceName();
				}
			}
		}
	}

	if (warmStandbyChannelSet.trimWhitespace().length() > 0 && !foundWSBProgrammaticConfig)
	{
		char* pToken = NULL;
		char* pNextToken = NULL;
		EmaVector<EmaString*> warmStandbyNameList;
		pToken = strtok(const_cast<char*>(warmStandbyChannelSet.c_str()), ",");

		while (pToken != NULL)
		{
			warmStandbyChannelName = pToken;
			warmStandbyNameList.push_back(new EmaString(warmStandbyChannelName.trimWhitespace()));
			pNextToken = strtok(NULL, ",");

			pToken = pNextToken;
		}

		EmaString* wsbChannelName;

		for (UInt32 index = 0; index < warmStandbyNameList.size(); index++)
		{
			wsbChannelName = warmStandbyNameList[index];
			WarmStandbyChannelConfig* newWSBChannelConfig = readWSBChannelConfig(pConfigImpl, *wsbChannelName, (pNextToken == NULL ? true : false));
			_activeConfig.configWarmStandbySet.push_back(newWSBChannelConfig);

			delete wsbChannelName;
		}
	}
	else if (ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure())
	{
		ppc->retrieveCommonConfig(_activeConfig.configuredName, _activeConfig);
		bool isProgramatiCfgChannelset = ppc->getActiveWSBChannelSetName(_activeConfig.configuredName, warmStandbyChannelSet.trimWhitespace());
		unsigned int posInProgCfg = 0;

		if (isProgramatiCfgChannelset)
		{
			char* pToken = NULL;
			char* pNextToken = NULL;
			EmaVector<EmaString*> warmStandbyNameList;
			pToken = strtok(const_cast<char*>(warmStandbyChannelSet.c_str()), ",");

			while (pToken != NULL)
			{
				warmStandbyChannelName = pToken;
				warmStandbyNameList.push_back(new EmaString(warmStandbyChannelName.trimWhitespace()));
				pNextToken = strtok(NULL, ",");

				pToken = pNextToken;
			}

			EmaString* wsbChannelName;

			for (UInt32 index = 0; index < warmStandbyNameList.size(); index++)
			{
				wsbChannelName = warmStandbyNameList[index];
				WarmStandbyChannelConfig* fileWsbChannelConfig = readWSBChannelConfig(pConfigImpl, *wsbChannelName, (pNextToken == NULL ? true : false));

				ppc->retrieveWSBChannelConfig(*wsbChannelName, _activeConfig, fileWsbChannelConfig);

				if (!(ActiveConfig::findWsbChannelConfig(_activeConfig.configWarmStandbySet, *wsbChannelName, posInProgCfg)))
					_activeConfig.configWarmStandbySet.push_back(fileWsbChannelConfig);
				else
				{
					if (fileWsbChannelConfig)
						delete fileWsbChannelConfig;
				}

				delete wsbChannelName;
			}
		}
	}

	if (pConfigImpl->getUserSpecifiedChannelType() != RSSL_CONN_TYPE_INIT)
	{
		if (warmStandbyChannelSet.trimWhitespace().length() > 0)
		{
			EmaString temp("Specifying connection type with API call is not applicable for WarmStandby channels.");
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}

		int size = _activeConfig.configChannelSet.size();

		for (int i = 0; i < size; i++)
		{
			_activeConfig.configChannelSet[i]->connectionType = pConfigImpl->getUserSpecifiedChannelType();
		}
	}

	if (pConfigImpl->getUserSpecifiedEncryptedProtocolType() != RSSL_CONN_TYPE_INIT)
	{
		if (warmStandbyChannelSet.trimWhitespace().length() > 0)
		{
			EmaString temp("Specifying encrypted connection type with API call is not applicable for WarmStandby channels.");
			throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
		}

		ChannelConfig* chanConfig = NULL;
		int size = _activeConfig.configChannelSet.size();

		for (int i = 0; i < size; i++)
		{
			chanConfig = (_activeConfig.configChannelSet[i]);

			if (chanConfig->connectionType != RSSL_CONN_TYPE_ENCRYPTED)
			{
				EmaString temp("Encrypted protocol type can not be set for non-encrypted channel type.");
				throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
			}
			static_cast<SocketChannelConfig*>(chanConfig)->encryptedConnectionType = pConfigImpl->getUserSpecifiedEncryptedProtocolType();
		}
	}

	OmmOAuth2CredentialImpl* pOAuth2Impl;
	OAuth2Credential& oAuthCredential = pConfigImpl->getOAuthCredential();
	int oAuthOffset = 0;

	/* Check to see if a clientId has been set, if so, there should be OAuth credentials(if they fail, they will fail at connection) */
	if (oAuthCredential.getClientId().length() != 0)
	{
		if (_hasConsOAuthClient)
			pOAuth2Impl = new OmmOAuth2CredentialImpl(oAuthCredential, _consOAuthClient, _adminClosure);
		else
			pOAuth2Impl = new OmmOAuth2CredentialImpl(oAuthCredential);

		pOAuth2Impl->oAuth2ArrayIndex(0);
		_oAuth2Credentials.push_back(pOAuth2Impl);
		pOAuth2Impl = NULL;
		if(oAuthCredential.getPassword().length() != 0)
			pConfigImpl->getLoginRdmReqMsg().clearPassword();

		oAuthOffset = 1;
	}

	for (i = 0; i < pConfigImpl->getOAuth2CredentialVector().size(); i++)
	{
		pOAuth2Impl = new OmmOAuth2CredentialImpl(*(pConfigImpl->getOAuth2CredentialVector()[i]));
		_oAuth2Credentials.push_back(pOAuth2Impl);
		pOAuth2Impl->oAuth2ArrayIndex(i + oAuthOffset);
		pOAuth2Impl = NULL;
	}

	if (_oAuth2Credentials.size() > 0)
	{
		_OAuthReactorConfig = (RsslReactorOAuthCredential**)malloc(sizeof(RsslReactorOAuthCredential*) * (_oAuth2Credentials.size()));
		if (!_OAuthReactorConfig)
		{
			throwMeeException("Failed to allocate memory for OAuth Credential List structure in OmmBaseImpl::readConfig");
			return;
		}

		memset((void*)_OAuthReactorConfig, 0, _oAuth2Credentials.size()*sizeof(RsslReactorOAuthCredential*));


		for (i = 0; i < _oAuth2Credentials.size(); i++)
		{
			RsslReactorOAuthCredential* pOAuthCredential = (RsslReactorOAuthCredential*)malloc(sizeof(RsslReactorOAuthCredential));
			if (!pOAuthCredential)
			{
				throwMeeException("Failed to allocate memory for OAuth Credential List structure in OmmBaseImpl::readConfig");
				return;
			}

			rsslClearReactorOAuthCredential(pOAuthCredential);

			_OAuthReactorConfig[i] = pOAuthCredential;
			if (_oAuth2Credentials[i]->getUserName().length())
			{
				pOAuthCredential->userName.data = const_cast<char*>(_oAuth2Credentials[i]->getUserName().c_str());
				pOAuthCredential->userName.length = _oAuth2Credentials[i]->getUserName().length();
			}

			if (_oAuth2Credentials[i]->getPassword().length())
			{
				pOAuthCredential->password.data = const_cast<char*>(_oAuth2Credentials[i]->getPassword().c_str());
				pOAuthCredential->password.length = _oAuth2Credentials[i]->getPassword().length();
			}

			if (_oAuth2Credentials[i]->getClientId().length())
			{
				pOAuthCredential->clientId.data = const_cast<char*>(_oAuth2Credentials[i]->getClientId().c_str());
				pOAuthCredential->clientId.length = _oAuth2Credentials[i]->getClientId().length();
			}

			if (_oAuth2Credentials[i]->getClientSecret().length())
			{
				pOAuthCredential->clientSecret.data = const_cast<char*>(_oAuth2Credentials[i]->getClientSecret().c_str());
				pOAuthCredential->clientSecret.length = _oAuth2Credentials[i]->getClientSecret().length();
			}

			if (_oAuth2Credentials[i]->getClientJWK().length())
			{
				pOAuthCredential->clientJWK.data = const_cast<char*>(_oAuth2Credentials[i]->getClientJWK().c_str());
				pOAuthCredential->clientJWK.length = _oAuth2Credentials[i]->getClientJWK().length();
			}

			if (_oAuth2Credentials[i]->getAudience().length())
			{
				pOAuthCredential->audience.data = const_cast<char*>(_oAuth2Credentials[i]->getAudience().c_str());
				pOAuthCredential->audience.length = _oAuth2Credentials[i]->getAudience().length();
			}

			if (_oAuth2Credentials[i]->getTokenScope().length())
			{
				pOAuthCredential->tokenScope.data = const_cast<char*>(_oAuth2Credentials[i]->getTokenScope().c_str());
				pOAuthCredential->tokenScope.length = _oAuth2Credentials[i]->getTokenScope().length();
			}

			pOAuthCredential->takeExclusiveSignOnControl = _oAuth2Credentials[i]->getTakeExclusiveSignOnControl();

			if (_oAuth2Credentials[i]->isOAuth2ClientSet())
			{
				pOAuthCredential->userSpecPtr = (void*)_oAuth2Credentials[i];
				pOAuthCredential->pOAuthCredentialEventCallback = OmmBaseImpl::oAuthCredentialCallback;
			}

		}
	}

	LoginRdmReqMsgImpl& configLoginReqMsgImpl = pConfigImpl->getLoginRdmReqMsg();

	LoginRdmReqMsgImpl* pLoginImpl;

	pLoginImpl = new LoginRdmReqMsgImpl(configLoginReqMsgImpl);

	pLoginImpl->arrayIndex(0);

	_LoginRequestMsgs.push_back(pLoginImpl);

	pLoginImpl = NULL;

	for (i = 0; i < pConfigImpl->getLoginCredentialVector().size(); i++)
	{
		pLoginImpl = new LoginRdmReqMsgImpl(*pConfigImpl->getLoginCredentialVector()[i]);

		pLoginImpl->arrayIndex(i + 1);
		_LoginRequestMsgs.push_back(pLoginImpl);
		pLoginImpl = NULL;
	}

	_LoginReactorConfig = (RsslReactorLoginRequestMsgCredential**)malloc(sizeof(RsslReactorLoginRequestMsgCredential*) * _LoginRequestMsgs.size());

	if (!_LoginReactorConfig)
	{
		throwMeeException("Failed to allocate memory for Login List structure in OmmBaseImpl::readConfig");
		return;
	}

	memset((void*)_LoginReactorConfig, 0, (_LoginRequestMsgs.size()*sizeof(RsslReactorLoginRequestMsgCredential*)));

	for (i = 0; i < _LoginRequestMsgs.size(); i++)
	{
		RsslReactorLoginRequestMsgCredential* pLoginMsgCredential = (RsslReactorLoginRequestMsgCredential*)malloc(sizeof(RsslReactorLoginRequestMsgCredential));

		if (!pLoginMsgCredential)
		{
			throwMeeException("Failed to allocate memory for Login structure in OmmBaseImpl::readConfig");
			return;
		}

		pLoginMsgCredential->loginRequestMsg = _LoginRequestMsgs[i]->get();
		
		pLoginMsgCredential->userSpecPtr = (void*)_LoginRequestMsgs[i];

		// Need to set this no matter what for EMA's bookkeeping. 
		pLoginMsgCredential->pLoginRenewalEventCallback = OmmBaseImpl::loginCredentialCallback;

		_LoginReactorConfig[i] = pLoginMsgCredential;
	}

	if (_activeConfig.configWarmStandbySet.size() != 0)
	{
		if(_activeConfig.configWarmStandbySet[0]->startingActiveServer != NULL)
			_activeConfig.pRsslRDMLoginReq = _LoginRequestMsgs[getLoginArrayIndex(_activeConfig.configWarmStandbySet[0]->startingActiveServer->name)];
		else
			_activeConfig.pRsslRDMLoginReq = _LoginRequestMsgs[0];
	}
	else
	{
		_activeConfig.pRsslRDMLoginReq = _LoginRequestMsgs[getLoginArrayIndex(_activeConfig.configChannelSet[0]->name)];
	}

	catchUnhandledException( _activeConfig.catchUnhandledException );
}

void OmmBaseImpl::useDefaultConfigValues( const EmaString& channelName, const EmaString& host, const EmaString& port )
{
	SocketChannelConfig* newChannelConfig =  0;
	try
	{
		newChannelConfig = new SocketChannelConfig(DEFAULT_HOST_NAME, getActiveConfig().defaultServiceName(), RSSL_CONN_TYPE_SOCKET );
		if ( host.length() )
			newChannelConfig->hostName = host;

		if ( port.length() )
			newChannelConfig->serviceName = port;

		newChannelConfig->name.set( channelName );
		_activeConfig.configChannelSet.push_back( newChannelConfig );
	}
	catch ( std::bad_alloc& )
	{
		const char* temp( "Failed to allocate memory for SocketChannelConfig." );
		throwMeeException( temp );
		return;
	}
}

WarmStandbyServerInfoConfig* OmmBaseImpl::readWarmStandbyServerInfoConfig(EmaConfigImpl* pConfigImpl, const EmaString& wsbServerInfoName)
{
	WarmStandbyServerInfoConfig* wsbServerInfoConfig = NULL;
	EmaString wsbServerInfoNodeName("WarmStandbyServerInfoGroup|WarmStandbyServerInfoList|WarmStandbyServerInfo.");
	wsbServerInfoNodeName.append(wsbServerInfoName).append("|");
	EmaString channelName;
	EmaString PerServiceNameSet;

	wsbServerInfoConfig = new WarmStandbyServerInfoConfig(wsbServerInfoName);

	if (pConfigImpl->get<EmaString>(wsbServerInfoNodeName + "Channel", channelName))
	{
		/* Don't reuse the existing config with the same name as a channel name can be used by multiple server info. */
		wsbServerInfoConfig->channelConfig = readChannelConfig(pConfigImpl, channelName, true);
		_activeConfig.configChannelSetForWSB.push_back(wsbServerInfoConfig->channelConfig);
	}

	if (pConfigImpl->get<EmaString>(wsbServerInfoNodeName + "PerServiceNameSet", PerServiceNameSet))
	{
		char* pToken = NULL;
		char* pNextToken = NULL;
		EmaString *pServiceName;
		EmaString serviceName;
		pToken = strtok(const_cast<char*>(PerServiceNameSet.c_str()), ",");
		do
		{
			if (pToken)
			{
				serviceName = pToken;
				pNextToken = strtok(NULL, ",");
				pServiceName = new EmaString(serviceName.trimWhitespace());
				wsbServerInfoConfig->perServiceNameSet.push_back(pServiceName);
			}

			pToken = pNextToken;
		} while (pToken != NULL);
	}

	return wsbServerInfoConfig;
}

void OmmBaseImpl::readChannelConfigForWSBChannel(EmaConfigImpl* pConfigImpl, const EmaString& wsbChannelName)
{
	EmaString wsbChannelNodeName("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.");
	wsbChannelNodeName.append(wsbChannelName).append("|");
	UInt64 tempUInt = 0;
	EmaString wsbServerInfoName, standbyServerSet;
	EmaString wsbServerInfoNodeName("WarmStandbyServerInfoGroup|WarmStandbyServerInfoList|WarmStandbyServerInfo.");
	EmaString channelName;
	EmaString xmlPath;
	EmaString channelNodeName("ChannelGroup|ChannelList|Channel.");
	ChannelConfig *pChannelConfig;

	if (pConfigImpl->get<EmaString>(wsbChannelNodeName + "StartingActiveServer", wsbServerInfoName))
	{
		xmlPath.set(wsbServerInfoNodeName).append(wsbServerInfoName).append("|");
		if (pConfigImpl->get<EmaString>(xmlPath + "Channel", channelName))
		{
			xmlPath.set(channelNodeName).append(channelName);
			
			/* Ensure that this channel name exists */
			if (pConfigImpl->getNode(xmlPath) != NULL)
			{
				pChannelConfig = readChannelConfig(pConfigImpl, channelName, true);
				_activeConfig.configChannelSetForWSB.push_back(pChannelConfig);
			}
		}
	}

	if (pConfigImpl->get<EmaString>(wsbChannelNodeName + "StandbyServerSet", standbyServerSet))
	{
		char* pToken = NULL;
		char* pNextToken = NULL;

		EmaString standbyServerInfoName;
		pToken = strtok(const_cast<char*>(standbyServerSet.c_str()), ",");
		while (pToken)
		{
			standbyServerInfoName = pToken;
			pNextToken = strtok(NULL, ",");
			wsbServerInfoName = standbyServerInfoName.trimWhitespace();
			xmlPath.set(wsbServerInfoNodeName).append(wsbServerInfoName).append("|");
			if (pConfigImpl->get<EmaString>(xmlPath + "Channel", channelName))
			{
				xmlPath.set(channelNodeName).append(channelName);
				/* Ensure that this channel name exists */
				if (pConfigImpl->getNode(xmlPath) != NULL)
				{
					pChannelConfig = readChannelConfig(pConfigImpl, channelName, true);
					_activeConfig.configChannelSetForWSB.push_back(pChannelConfig);
				}
			}

			pToken = pNextToken;
		}
	}
}

WarmStandbyChannelConfig* OmmBaseImpl::readWSBChannelConfig(EmaConfigImpl* pConfigImpl, const EmaString& wsbChannelName, bool readLastChannel)
{
	WarmStandbyChannelConfig* newWSBChannelConfig = NULL;
	EmaString wsbChannelNodeName("WarmStandbyGroup|WarmStandbyList|WarmStandbyChannel.");
	wsbChannelNodeName.append(wsbChannelName).append("|");
	UInt64 tempUInt = 0;
	EmaString startingActiveServerName, standbyServerSet;
	RsslReactorWarmStandbyMode warmStandbyMode;
	WarmStandbyServerInfoConfig *warmStandbyServerInfoConfig;

	try
	{
		newWSBChannelConfig = new WarmStandbyChannelConfig(wsbChannelName);

		if (pConfigImpl->get<EmaString>(wsbChannelNodeName + "StartingActiveServer", startingActiveServerName))
		{
			newWSBChannelConfig->startingActiveServer = readWarmStandbyServerInfoConfig(pConfigImpl, startingActiveServerName);
		}

		if (pConfigImpl->get<EmaString>(wsbChannelNodeName + "StandbyServerSet", standbyServerSet))
		{
			char* pToken = NULL;
			char* pNextToken = NULL;

			EmaString standbyServerInfoName;
			EmaVector< EmaString*> standbyServerInfoList;
			pToken = strtok(const_cast<char*>(standbyServerSet.c_str()), ",");
			do
			{
				if (pToken)
				{
					standbyServerInfoName = pToken;
					pNextToken = strtok(NULL, ",");
					standbyServerInfoList.push_back(new EmaString(standbyServerInfoName.trimWhitespace()));
				}

				pToken = pNextToken;
			} while (pToken != NULL);

			for (UInt32 index = 0; index < standbyServerInfoList.size(); index++)
			{
				warmStandbyServerInfoConfig = readWarmStandbyServerInfoConfig(pConfigImpl, *standbyServerInfoList[index]);
			    newWSBChannelConfig->standbyServerSet.push_back(warmStandbyServerInfoConfig);

				delete standbyServerInfoList[index];
			}
		}

		//if (pConfigImpl->get<UInt64>(wsbChannelNodeName + "DownloadConnectionConfig", tempUInt))
		//{
		//	newWSBChannelConfig->downloadConnectionConfig = tempUInt > 0 ? true : false;
		//}

		if (pConfigImpl->get<RsslReactorWarmStandbyMode>(wsbChannelNodeName + "WarmStandbyMode", warmStandbyMode))
		{
			newWSBChannelConfig->warmStandbyMode = (WarmStandbyChannelConfig::WarmStandbyMode)warmStandbyMode;
		}
	}
	catch (std::bad_alloc&)
	{
		const char* temp("Failed to allocate memory for WarmStandbyChannelConfig.");
		throwMeeException(temp);
		return NULL;
	}

	return newWSBChannelConfig;
}

ChannelConfig* OmmBaseImpl::readChannelConfig(EmaConfigImpl* pConfigImpl, const EmaString&  channelName, bool readLastChannel)
{
	ChannelConfig* newChannelConfig = NULL;
	SocketChannelConfig* socketChannelCfg = NULL;
	UInt32 maxUInt32( 0xFFFFFFFF );
	EmaString channelNodeName( "ChannelGroup|ChannelList|Channel." );
	channelNodeName.append( channelName ).append( "|" );
	UInt64 tempUInt = 0;
	Int64 tempInt = 0;

	RsslConnectionTypes channelType = RSSL_CONN_TYPE_INIT;
	if (pConfigImpl->getUserSpecifiedHostname().length() > 0)
	{
		channelType = RSSL_CONN_TYPE_SOCKET;
	}
	else
	{
		bool foundChannelType = false;
		if (ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure())
			channelType = (RsslConnectionTypes)ppc->retrieveChannelTypeConfig(channelName);

		if (channelType < 0)
		{
			if (!pConfigImpl->get<RsslConnectionTypes>(channelNodeName + "ChannelType", channelType))
				channelType = RSSL_CONN_TYPE_SOCKET;
		}
	}

	switch ( channelType )
	{
	case RSSL_CONN_TYPE_ENCRYPTED:
	{
		try
		{
			/*	Both host and port is set as empty string by default to support the Reactor's session management
				to query them from RDP service discovery when the SocketChannelConfig.enableSessionMgnt is set to true.
			*/
			socketChannelCfg = new SocketChannelConfig("","", channelType);
			socketChannelCfg->initializationTimeout = DEFAULT_INITIALIZATION_TIMEOUT_ENCRYPTED_CON;
			newChannelConfig = socketChannelCfg;
		}
		catch ( std::bad_alloc& )
		{
			const char* temp("Failed to allocate memory for HttpChannelConfig. (std::bad_alloc)");
			throwMeeException(temp);
			return 0;
		}

		if (pConfigImpl->getUserSpecifiedSecurityProtocol() > 0)
		{
			socketChannelCfg->securityProtocol = pConfigImpl->getUserSpecifiedSecurityProtocol();
		}
		else
		{
			if ( pConfigImpl->get<UInt64>(channelNodeName + "SecurityProtocol", tempUInt) )
				socketChannelCfg->securityProtocol = (int)tempUInt;
		}

		pConfigImpl->get<RsslConnectionTypes>(channelNodeName + "EncryptedProtocolType", socketChannelCfg->encryptedConnectionType);

		if (RSSL_CONN_TYPE_WEBSOCKET == socketChannelCfg->encryptedConnectionType)
		{
			pConfigImpl->get<UInt64>(channelNodeName + "WsMaxMsgSize", socketChannelCfg->wsMaxMsgSize);
			pConfigImpl->get<EmaString>(channelNodeName + "WsProtocols", socketChannelCfg->wsProtocols);
		}

		if (pConfigImpl->getUserSpecifiedSslCAStore().length() > 0)
		{
			socketChannelCfg->sslCAStore = pConfigImpl->getUserSpecifiedSslCAStore();
		}
		else
			pConfigImpl->get<EmaString>(channelNodeName + "OpenSSLCAStore", socketChannelCfg->sslCAStore);

		if (!pConfigImpl->get< EmaString >(channelNodeName + "Location", socketChannelCfg->location))
			socketChannelCfg->location = DEFAULT_RDP_RT_LOCATION;

		if (pConfigImpl->get<UInt64>(channelNodeName + "ServiceDiscoveryRetryCount", tempUInt))
		{
			socketChannelCfg->setServiceDiscoveryRetryCount(tempUInt);
		}

		// Fall through to HTTP for common configurations
	}
	case RSSL_CONN_TYPE_SOCKET:
	case RSSL_CONN_TYPE_HTTP:
	case RSSL_CONN_TYPE_WEBSOCKET:
	{
		if (socketChannelCfg == 0)
		{
			try
			{
				socketChannelCfg = new SocketChannelConfig(DEFAULT_HOST_NAME, getActiveConfig().defaultServiceName(), channelType);
				newChannelConfig = socketChannelCfg;
			}
			catch (std::bad_alloc&)
			{
				const char* temp("Failed to allocate memory for HttpChannelConfig. (std::bad_alloc)");
				throwMeeException(temp);
				return 0;
			}
		}

		UInt64 tempUInt = 0;
		pConfigImpl->get<UInt64>(channelNodeName + "EnableSessionManagement", tempUInt);
		if (!tempUInt)
			socketChannelCfg->enableSessionMgnt = RSSL_FALSE;
		else
			socketChannelCfg->enableSessionMgnt = RSSL_TRUE;

		EmaString  tmp = pConfigImpl->getUserSpecifiedHostname();
		if (tmp.length())
			socketChannelCfg->hostName = tmp;
		else
			pConfigImpl->get< EmaString >(channelNodeName + "Host", socketChannelCfg->hostName);

		PortSetViaFunctionCall psvfc(pConfigImpl->getUserSpecifiedPort());
		if (psvfc.userSet) {
			if (psvfc.userSpecifiedValue.length())
				socketChannelCfg->serviceName = psvfc.userSpecifiedValue;
			else
				socketChannelCfg->serviceName = _activeConfig.defaultServiceName();
		}
		else
			pConfigImpl->get< EmaString >(channelNodeName + "Port", socketChannelCfg->serviceName);

		tmp = pConfigImpl->getUserSpecifiedProxyHostname();
		if (tmp.length())
			socketChannelCfg->proxyHostName = tmp;
		else
			pConfigImpl->get< EmaString >(channelNodeName + "ProxyHost", socketChannelCfg->proxyHostName);

		tmp = pConfigImpl->getUserSpecifiedProxyPort();
		if (tmp.length())
			socketChannelCfg->proxyPort = tmp;
		else
			pConfigImpl->get< EmaString >(channelNodeName + "ProxyPort", socketChannelCfg->proxyPort);

		if (pConfigImpl->getUserSpecifiedProxyUserName().length())
		{
			socketChannelCfg->proxyUserName = pConfigImpl->getUserSpecifiedProxyUserName();
		}

		if (pConfigImpl->getUserSpecifiedProxyPasswd().length())
		{
			socketChannelCfg->proxyPasswd = pConfigImpl->getUserSpecifiedProxyPasswd();
		}

		if (pConfigImpl->getUserSpecifiedProxyDomain().length())
		{
			socketChannelCfg->proxyDomain = pConfigImpl->getUserSpecifiedProxyDomain();
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "ProxyConnectionTimeout", tempUInt))
		{
			socketChannelCfg->setProxyConnectionTimeout(tempUInt);
		}

		if (pConfigImpl->getUserSpecifiedSslCAStore().length())
		{
			socketChannelCfg->sslCAStore = pConfigImpl->getUserSpecifiedSslCAStore();
		}

		tempUInt = 1;
		pConfigImpl->get<UInt64>( channelNodeName + "TcpNodelay", tempUInt );
		if ( !tempUInt )
			socketChannelCfg->tcpNodelay = RSSL_FALSE;
		else
			socketChannelCfg->tcpNodelay = RSSL_TRUE;

		tmp = pConfigImpl->getUserSpecifiedObjectName();
		if (tmp.length())
			socketChannelCfg->objectName = tmp;
		else
			pConfigImpl->get<EmaString>( channelNodeName + "ObjectName", socketChannelCfg->objectName );

		if (RSSL_CONN_TYPE_WEBSOCKET == channelType)
		{
			pConfigImpl->get<UInt64>(channelNodeName + "WsMaxMsgSize", socketChannelCfg->wsMaxMsgSize);
			pConfigImpl->get<EmaString>(channelNodeName + "WsProtocols", socketChannelCfg->wsProtocols);
		}

		break;
	}
	case RSSL_CONN_TYPE_RELIABLE_MCAST:
	{
		ReliableMcastChannelConfig* relMcastChannelCfg = NULL;
		try
		{
			relMcastChannelCfg = new ReliableMcastChannelConfig();
			newChannelConfig = relMcastChannelCfg;
		}
		catch ( std::bad_alloc& )
		{
			const char* temp( "Failed to allocate memory for ReliableMcastChannelConfig. (std::bad_alloc)" );
			throwMeeException( temp );
			return 0;
		}

		if ( !newChannelConfig )
		{
			const char* temp = "Failed to allocate memory for ReliableMcastChannelConfig. (null ptr)";
			throwMeeException( temp );
			return 0;
		}
		EmaString errorMsg;
		if ( !readReliableMcastConfig( pConfigImpl, channelNodeName, relMcastChannelCfg, errorMsg ) )
		{
			throwIceException( errorMsg );
			return 0;
		}
		break;
	}
	default:
	{
		EmaString temp( "Not supported channel type. Type = " );
		temp.append( ( UInt32 )channelType );
		throwIueException(temp, OmmInvalidUsageException::UnSupportedChannelTypeEnum );
		return 0;
	}
	}

	newChannelConfig->name = channelName;

	pConfigImpl->get<EmaString>( channelNodeName + "InterfaceName", newChannelConfig->interfaceName );

	tempUInt = 0;
	if ( channelType != RSSL_CONN_TYPE_RELIABLE_MCAST )
	{
		bool setCompressionThresholdFromConfigFile(false);
		tempUInt = 0;
		if ( pConfigImpl->get<UInt64>( channelNodeName + "CompressionThreshold", tempUInt ) )
		{
			newChannelConfig->compressionThreshold = tempUInt > maxUInt32 ? maxUInt32 : ( UInt32 )tempUInt;
			newChannelConfig->compressionThresholdSet = true;
			setCompressionThresholdFromConfigFile = true;
		}

		pConfigImpl->get<RsslCompTypes>( channelNodeName + "CompressionType", newChannelConfig->compressionType );
		if ( newChannelConfig->compressionType == RSSL_COMP_LZ4 &&
			 !setCompressionThresholdFromConfigFile )
		  newChannelConfig->compressionThreshold = DEFAULT_COMPRESSION_THRESHOLD_LZ4;
	}

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "GuaranteedOutputBuffers", tempUInt ) )
		newChannelConfig->setGuaranteedOutputBuffers( tempUInt );

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "NumInputBuffers", tempUInt ) )
		newChannelConfig->setNumInputBuffers( tempUInt );

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "ConnectionPingTimeout", tempUInt ) )
		newChannelConfig->connectionPingTimeout = tempUInt > maxUInt32 ? maxUInt32 : ( UInt32 )tempUInt;

	tempUInt = 0;
	if (pConfigImpl->get<UInt64>(channelNodeName + "InitializationTimeout", tempUInt))
		newChannelConfig->initializationTimeout = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "SysRecvBufSize", tempUInt ) )
		newChannelConfig->sysRecvBufSize = tempUInt > maxUInt32 ? maxUInt32 : ( UInt32 )tempUInt;

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "SysSendBufSize", tempUInt ) )
		newChannelConfig->sysSendBufSize = tempUInt > maxUInt32 ? maxUInt32 : ( UInt32 )tempUInt;

	tempUInt = 0;
	if ( pConfigImpl->get<UInt64>( channelNodeName + "HighWaterMark", tempUInt ) )
		newChannelConfig->highWaterMark = tempUInt > maxUInt32 ? maxUInt32 : (UInt32) tempUInt;

	tempUInt = 0;
	if (pConfigImpl->get<UInt64>(channelNodeName + "DirectWrite", tempUInt))
		newChannelConfig->directWrite = tempUInt > maxUInt32 ? maxUInt32 : (UInt32)tempUInt;

	/* @deprecated DEPRECATED:
	 *ReconnectAttemptLimit,ReconnectMinDelay,ReconnectMaxDelay,MsgKeyInUpdates,XmlTrace is per consumer/niprov/iprov instance based now. 
	  The following code will be removed in the future.
	 */
	if ( !_activeConfig.parameterConfigGroup && readLastChannel )
	{
		Int64 tmp = -1;
		if (pConfigImpl->get<Int64>(channelNodeName + "ReconnectAttemptLimit", tmp))
		{
			_activeConfig.setReconnectAttemptLimit(tmp);
			EmaString errorMsg("ReconnectAttemptLimit is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<Int64>(channelNodeName + "ReconnectMinDelay", tmp))
		{
			_activeConfig.setReconnectMinDelay(tmp);
			EmaString errorMsg("ReconnectMinDelay is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<Int64>(channelNodeName + "ReconnectMaxDelay", tmp))
		{
			_activeConfig.setReconnectMaxDelay(tmp);
			EmaString errorMsg("ReconnectMaxDelay is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}
		
		if (pConfigImpl->get<EmaString>(channelNodeName + "XmlTraceFileName", _activeConfig.xmlTraceFileName))
		{
			EmaString errorMsg("XmlTraceFileName is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}
		
		if (pConfigImpl->get<Int64>(channelNodeName + "XmlTraceMaxFileSize", tmp))
		{
			if (tmp > 0)
				_activeConfig.xmlTraceMaxFileSize = tmp;

			EmaString errorMsg("XmlTraceMaxFileSize is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTraceToFile", tempUInt))
		{
			if (tempUInt > 0)
				_activeConfig.xmlTraceToFile = true;

			EmaString errorMsg("XmlTraceToFile is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTraceToStdout", tempUInt))
		{
			_activeConfig.xmlTraceToStdout = tempUInt > 0 ? true : false;

			EmaString errorMsg("XmlTraceToStdout is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTraceToMultipleFiles", tempUInt))
		{
			if (tempUInt > 0)
				_activeConfig.xmlTraceToMultipleFiles = true;

			EmaString errorMsg("XmlTraceToMultipleFiles is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTraceWrite", tempUInt))
		{
			if (tempUInt == 0)
				_activeConfig.xmlTraceWrite = false;

			EmaString errorMsg("XmlTraceWrite is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTraceRead", tempUInt))
		{
			if (tempUInt == 0)
				_activeConfig.xmlTraceRead = false;

			EmaString errorMsg("XmlTraceRead is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTracePing", tempUInt))
		{
			_activeConfig.xmlTracePing = tempUInt == 0 ? false : true;

			EmaString errorMsg("XmlTracePing is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTracePingOnly", tempUInt))
		{
			_activeConfig.xmlTracePingOnly = tempUInt == 0 ? false : true;

			EmaString errorMsg("XmlTracePingOnly is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "XmlTraceHex", tempUInt))
		{
			_activeConfig.xmlTraceHex = tempUInt == 0 ? false : true;

			EmaString errorMsg("XmlTraceHex is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}

		if (pConfigImpl->get<UInt64>(channelNodeName + "MsgKeyInUpdates", tempUInt))
		{
			if (tempUInt == 0)
				_activeConfig.msgKeyInUpdates = false;

			EmaString errorMsg("MsgKeyInUpdates is no longer configured on a per-channel basis; configure it instead in the Consumer instance.");
			pConfigImpl->appendConfigError(errorMsg, OmmLoggerClient::WarningEnum);
		}
	}

	return newChannelConfig;
}

bool OmmBaseImpl::readReliableMcastConfig( EmaConfigImpl* pConfigImpl, const EmaString& channNodeName, ReliableMcastChannelConfig* relMcastChannelCfg, EmaString& errorText )
{
	EmaString channelNodeName( channNodeName );

	pConfigImpl->get<EmaString>( channelNodeName + "RecvAddress", relMcastChannelCfg->recvAddress );
	if ( relMcastChannelCfg->recvAddress.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [RecvAddress]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "RecvPort", relMcastChannelCfg->recvServiceName );
	if ( relMcastChannelCfg->recvServiceName.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [RecvPort]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "UnicastPort", relMcastChannelCfg->unicastServiceName );
	if ( relMcastChannelCfg->unicastServiceName.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [UnicastPort]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "SendAddress", relMcastChannelCfg->sendAddress );
	if ( relMcastChannelCfg->sendAddress.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [SendAddress]." );
		return false;
	}

	pConfigImpl->get<EmaString>( channelNodeName + "SendPort", relMcastChannelCfg->sendServiceName );
	if ( relMcastChannelCfg->sendServiceName.empty() )
	{
		errorText.clear();
		errorText.append( "Invalid Channel Configuration for ChannelType [RSSL_RELIABLE_MCAST]. Missing required parameter [SendPort]." );
		return false;
	}

	UInt64 tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "DisconnectOnGap", tempUIntval );
	relMcastChannelCfg->disconnectOnGap = ( tempUIntval ) ? true : false;

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "PacketTTL", tempUIntval );
	relMcastChannelCfg->setPacketTTL( tempUIntval );

	pConfigImpl->get<EmaString>( channelNodeName + "HsmInterface", relMcastChannelCfg->hsmInterface );
	pConfigImpl->get<EmaString>( channelNodeName + "HsmMultAddress", relMcastChannelCfg->hsmMultAddress );
	pConfigImpl->get<EmaString>( channelNodeName + "HsmPort", relMcastChannelCfg->hsmPort );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "HsmInterval", tempUIntval );
	relMcastChannelCfg->setHsmInterval( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "ndata", tempUIntval );
	relMcastChannelCfg->setNdata( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "nmissing", tempUIntval );
	relMcastChannelCfg->setNmissing( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "nrreq", tempUIntval );
	relMcastChannelCfg->setNrreq( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "tdata", tempUIntval );
	relMcastChannelCfg->setTdata( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "trreq", tempUIntval );
	relMcastChannelCfg->setTrreq( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "pktPoolLimitHigh", tempUIntval );
	relMcastChannelCfg->setPktPoolLimitHigh( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "pktPoolLimitLow", tempUIntval );
	relMcastChannelCfg->setPktPoolLimitLow( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "twait", tempUIntval );
	relMcastChannelCfg->setTwait( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "tbchold", tempUIntval );
	relMcastChannelCfg->setTbchold( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "tpphold", tempUIntval );
	relMcastChannelCfg->setTpphold( tempUIntval );

	tempUIntval = 0;
	pConfigImpl->get<UInt64>( channelNodeName + "userQLimit", tempUIntval );
	relMcastChannelCfg->setUserQLimit( tempUIntval );

	return true;
}

void OmmBaseImpl::initialize( EmaConfigImpl* configImpl )
{
	try
	{
		_userLock.lock();

		readConfig( configImpl );

		_pLoggerClient = OmmLoggerClient::create(
			_activeConfig.loggerConfig.loggerType,
			_activeConfig.loggerConfig.includeDateInLoggerOutput,
			_activeConfig.loggerConfig.minLoggerSeverity,
			_activeConfig.loggerConfig.loggerFileName,
			_activeConfig.loggerConfig.maxFileSize,
			_activeConfig.loggerConfig.maxFileNumber );

		readCustomConfig(configImpl);

		configImpl->configErrors().log(_pLoggerClient, _activeConfig.loggerConfig.minLoggerSeverity);

		if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Print out active configuration detail.");
			temp.append(_activeConfig.configTrace());
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		if ( !_pipe.create() )
		{
			EmaString temp( "Failed to create communication Pipe." );
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
			throwIueException( temp, OmmInvalidUsageException::InternalErrorEnum );
		}
		else if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully initialized communication Pipe." );
			_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		RsslError rsslError;
		RsslInitializeExOpts rsslInitOpts = RSSL_INIT_INITIALIZE_EX_OPTS;
		rsslInitOpts.rsslLocking = RSSL_LOCK_GLOBAL_AND_CHANNEL;
		if ( _activeConfig.libSslName.length() > 0)
			rsslInitOpts.jitOpts.libsslName = (char*)_activeConfig.libSslName.c_str();
		if (_activeConfig.libCryptoName.length() > 0)
			rsslInitOpts.jitOpts.libcryptoName = (char*)_activeConfig.libCryptoName.c_str();
		if (_activeConfig.libcurlName.length() > 0)
			rsslInitOpts.jitOpts.libcurlName = (char*)_activeConfig.libcurlName.c_str();
		if (_activeConfig.shouldInitializeCPUIDlib != DEFAULT_SHOULD_INIT_CPUID_LIB)
			rsslInitOpts.shouldInitializeCPUIDlib = _activeConfig.shouldInitializeCPUIDlib;

		RsslRet retCode = rsslInitializeEx(&rsslInitOpts, &rsslError);
		if ( retCode != RSSL_RET_SUCCESS )
		{
			EmaString temp( "rsslInitializeEx() failed while initializing OmmBaseImpl." );
			temp.append( " Internal sysError='" ).append( rsslError.sysError )
			.append( "' Error text='" ).append( rsslError.text ).append( "'. " );

			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
			throwIueException( temp, OmmInvalidUsageException::InternalErrorEnum );
		}
		else if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully initialized Rssl." );

			_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		_state = RsslInitilizedEnum;

		RsslCreateReactorOptions reactorOpts;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );

		rsslClearCreateReactorOptions( &reactorOpts );

		// Overrides the default service discovery URL if specified by user
		if ( configImpl->getUserSpecifiedServiceDiscoveryUrl().length() > 0 )
		{
			reactorOpts.serviceDiscoveryURL.length = configImpl->getUserSpecifiedServiceDiscoveryUrl().length();
			reactorOpts.serviceDiscoveryURL.data = (char*)configImpl->getUserSpecifiedServiceDiscoveryUrl().c_str();
		}

		// Overrides the default token service URL if specified by user
		if ( configImpl->getUserSpecifiedTokenServiceUrlV1().length() > 0 )
		{
			reactorOpts.tokenServiceURL_V1.length = configImpl->getUserSpecifiedTokenServiceUrlV1().length();
			reactorOpts.tokenServiceURL_V1.data = (char*)configImpl->getUserSpecifiedTokenServiceUrlV1().c_str();
		}
		// Overrides the default token service URL if specified by user
		if (configImpl->getUserSpecifiedTokenServiceUrlV2().length() > 0)
		{
			reactorOpts.tokenServiceURL_V2.length = configImpl->getUserSpecifiedTokenServiceUrlV2().length();
			reactorOpts.tokenServiceURL_V2.data = (char*)configImpl->getUserSpecifiedTokenServiceUrlV2().c_str();
		}

		if( _activeConfig.tokenReissueRatio != DEFAULT_TOKEN_REISSUE_RATIO )
			reactorOpts.tokenReissueRatio = _activeConfig.tokenReissueRatio;

		if (_activeConfig.reissueTokenAttemptLimit != DEFAULT_REISSUE_TOKEN_ATTEMP_LIMIT)
			reactorOpts.reissueTokenAttemptLimit = (Int32)_activeConfig.reissueTokenAttemptLimit;

		if (_activeConfig.reissueTokenAttemptInterval != DEFAULT_REISSUE_TOKEN_ATTEMP_INTERVAL)
			reactorOpts.reissueTokenAttemptInterval = (Int32)_activeConfig.reissueTokenAttemptInterval;

		if (_activeConfig.restRequestTimeOut != DEFAULT_REST_REQUEST_TIMEOUT)
			reactorOpts.restRequestTimeOut = _activeConfig.restRequestTimeOut;

        if (_activeConfig.maxEventsInPool != DEFAULT_MAX_EVENT_IN_POOL)
            reactorOpts.maxEventsInPool = _activeConfig.maxEventsInPool;

		if (_activeConfig.restEnableLog != DEFAULT_REST_ENABLE_LOG)
			reactorOpts.restEnableLog = _activeConfig.restEnableLog;

		if (_activeConfig.restVerboseMode != DEFAULT_REST_VERBOSE_MODE)
			reactorOpts.restVerboseMode = _activeConfig.restVerboseMode;

		if (!_activeConfig.restLogFileName.empty())
		{
			reactorOpts.restLogOutputStream = fopen(_activeConfig.restLogFileName, "w");
			if (!reactorOpts.restLogOutputStream)
			{
				EmaString temp("Failed to open specified file: ");
				temp.append(_activeConfig.restLogFileName);
				throwIueException(temp, OmmInvalidUsageException::InternalErrorEnum);
			}
		}

		if (configImpl->getOmmRestLoggingClient())
		{
			_pRestLoggingCallbackClient = new RestLoggingCallbackClient(configImpl->getOmmRestLoggingClient(), configImpl->getRestLoggingClosure());
			reactorOpts.pRestLoggingCallback = restLoggingCallback;
		}

		if ( !configImpl->getCpuWorkerThreadBind().empty() )
		{
			reactorOpts.cpuBindWorkerThread.length = configImpl->getCpuWorkerThreadBind().length();
			reactorOpts.cpuBindWorkerThread.data = (char*)configImpl->getCpuWorkerThreadBind().c_str();
		}

		if ( !_activeConfig.restProxyHostName.empty() )
		{
			reactorOpts.restProxyOptions.proxyHostName = (char*)_activeConfig.restProxyHostName.c_str();
		}

		if ( !_activeConfig.restProxyPort.empty() )
		{
			reactorOpts.restProxyOptions.proxyPort = (char*)_activeConfig.restProxyPort.c_str();
		}

		if ( !_activeConfig.restProxyUserName.empty() )
		{
			reactorOpts.restProxyOptions.proxyUserName = (char*)_activeConfig.restProxyUserName.c_str();
		}

		if ( !_activeConfig.restProxyPasswd.empty() )
		{
			reactorOpts.restProxyOptions.proxyPasswd = (char*)_activeConfig.restProxyPasswd.c_str();
		}

		if ( !_activeConfig.restProxyDomain.empty() )
		{
			reactorOpts.restProxyOptions.proxyDomain = (char*)_activeConfig.restProxyDomain.c_str();
		}

		reactorOpts.userSpecPtr = ( void* )this;

		_pRsslReactor = rsslCreateReactor( &reactorOpts, &rsslErrorInfo );
		if ( !_pRsslReactor )
		{
			EmaString temp( "Failed to initialize OmmBaseImpl (rsslCreateReactor)." );
			temp.append( "' Error Id='" ).append( rsslErrorInfo.rsslError.rsslErrorId )
			.append( "' Internal sysError='" ).append( rsslErrorInfo.rsslError.sysError )
			.append( "' Error Location='" ).append( rsslErrorInfo.errorLocation )
			.append( "' Error Text='" ).append( rsslErrorInfo.rsslError.text ).append( "'. " );
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
			throwIueException( temp, OmmInvalidUsageException::InternalErrorEnum );
		}
		else if ( OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Successfully created Reactor." );
			_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp );
		}

		_state = ReactorInitializedEnum;

#ifdef USING_SELECT
		FD_SET( _pipe.readFD(), &_readFds );
		FD_SET( _pipe.readFD(), &_exceptFds );
		FD_SET( _pRsslReactor->eventFd, &_readFds );
		FD_SET( _pRsslReactor->eventFd, &_exceptFds );
#else
		_eventFdsCapacity = 8;
		_eventFds = new pollfd[ _eventFdsCapacity ];
		_eventFdsCount = 0;
		_pipeReadEventFdsIdx = addFd( _pipe.readFD() );
		addFd( _pRsslReactor->eventFd );
#endif

		_pLoginCallbackClient = LoginCallbackClient::create( *this );
		_pLoginCallbackClient->initialize();

		createDictionaryCallbackClient( _pDictionaryCallbackClient, *this );

		createDirectoryCallbackClient( _pDirectoryCallbackClient, *this );

		_pItemCallbackClient = ItemCallbackClient::create( *this );
		_pItemCallbackClient->initialize();

		if (!_atExit && getImplType() == OmmCommonImpl::ConsumerEnum)
		{
			Dictionary* dictionary = getDictionaryCallbackClient().getDefaultDictionary();
			RsslReactorJsonConverterOptions jsonConverterOptions;
			rsslClearReactorJsonConverterOptions(&jsonConverterOptions);
			jsonConverterOptions.pDictionary = const_cast<RsslDataDictionary*>(dictionary->getRsslDictionary());
			jsonConverterOptions.pServiceNameToIdCallback = serviceNameToIdCallback;
			jsonConverterOptions.pJsonConversionEventCallback = jsonConversionEventCallback;
			jsonConverterOptions.defaultServiceId = _activeConfig.defaultServiceIDForConverter;
			jsonConverterOptions.jsonExpandedEnumFields = (RsslBool)_activeConfig.jsonExpandedEnumFields;
			jsonConverterOptions.catchUnknownJsonKeys = (RsslBool)_activeConfig.catchUnknownJsonKeys;
			jsonConverterOptions.catchUnknownJsonFids = (RsslBool)_activeConfig.catchUnknownJsonFids;
			jsonConverterOptions.closeChannelFromFailure = (RsslBool)_activeConfig.closeChannelFromFailure;
			jsonConverterOptions.outputBufferSize = _activeConfig.outputBufferSize;
			jsonConverterOptions.jsonTokenIncrementSize = _activeConfig.jsonTokenIncrementSize;
			jsonConverterOptions.sendJsonConvError = _activeConfig.sendJsonConvError;

			if (rsslReactorInitJsonConverter(_pRsslReactor, &jsonConverterOptions, &rsslErrorInfo) != RSSL_RET_SUCCESS)
			{
				EmaString temp("Failed to initialize OmmBaseImpl (RWF/JSON Converter).");
				temp.append("' Error Id='").append(rsslErrorInfo.rsslError.rsslErrorId)
					.append("' Internal sysError='").append(rsslErrorInfo.rsslError.sysError)
					.append("' Error Location='").append(rsslErrorInfo.errorLocation)
					.append("' Error Text='").append(rsslErrorInfo.rsslError.text).append("'. ");
				if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
					_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				throwIueException(temp, OmmInvalidUsageException::InternalErrorEnum);
			}
		}

		/* Now that all the handlers are setup, initialize the login stream handle, if set */
		if (!_atExit && _hasConsAdminClient)
		{
			ReqMsg loginRequest;
			loginRequest.clear().domainType(ema::rdm::MMT_LOGIN);
			_pItemCallbackClient->registerClient(loginRequest, _consAdminClient, _adminClosure, 0);
		}

		/* Consumer and Provider side should be mutually exclusive */
		if (!_atExit && _hasProvAdminClient)
		{
			ReqMsg loginRequest;
			loginRequest.clear().domainType(ema::rdm::MMT_LOGIN);
			_pItemCallbackClient->registerClient(loginRequest, _provAdminClient, _adminClosure, 0);
		}

		if (!_atExit)
		{
			_pChannelCallbackClient = ChannelCallbackClient::create( *this, _pRsslReactor );
			_pChannelCallbackClient->initialize();
		}

		UInt64 timeOutLengthInMicroSeconds = _activeConfig.loginRequestTimeOut * 1000;
		_eventTimedOut = false;
		TimeOut* loginWatcher( new TimeOut( *this, timeOutLengthInMicroSeconds, &OmmBaseImpl::terminateIf, reinterpret_cast< void* >( this ), true ) );
		while ( ! _atExit && ! _eventTimedOut &&
		        ( _state < LoginStreamOpenOkEnum ) &&
		        ( _state != RsslChannelUpStreamNotOpenEnum ) )
		{
			rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread, _bEventReceived );
		}

	    if ( !_atExit )
		{
			if (_eventTimedOut)
			{
				Channel *pChannel = _pLoginCallbackClient->getActiveChannel();
				ChannelConfig* pChannelcfg = NULL;
				if (pChannel != NULL && pChannel->getReactorChannelType() == Channel::NORMAL)
				{
					pChannelcfg = _activeConfig.findChannelConfig(pChannel);
					if (!pChannelcfg && _activeConfig.configChannelSet.size() > 0)
						pChannelcfg = _activeConfig.configChannelSet[_activeConfig.configChannelSet.size() - 1];
				}

				EmaString failureMsg( "login failed (timed out after waiting " );
				failureMsg.append( _activeConfig.loginRequestTimeOut ).append( " milliseconds)" );
				if (pChannelcfg != NULL && pChannelcfg->getType() == ChannelConfig::SocketChannelEnum )
				{
					SocketChannelConfig* channelConfig( reinterpret_cast< SocketChannelConfig* >( pChannelcfg ) );
					failureMsg.append(" for ").append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append( ")" );
				}

				throwIueException( failureMsg, OmmInvalidUsageException::LoginRequestTimeOutEnum );
			}
			else if ( _state == RsslChannelUpStreamNotOpenEnum )
			{
				if ( timeOutLengthInMicroSeconds != 0 ) loginWatcher->cancel();
				throwIueException( getLoginCallbackClient().getLoginFailureMessage(), OmmInvalidUsageException::LoginRequestRejectedEnum );
			}
			else
			{
				if ( timeOutLengthInMicroSeconds != 0 ) loginWatcher->cancel();
				loginWatcher = 0;
			}
		}
		else
		{
			throwIueException( "Application or user initiated exit while waiting for login response.", OmmInvalidUsageException::InvalidOperationEnum );
		}

		loadDirectory();

		if (getImplType() == OmmCommonImpl::ConsumerEnum)
		{
			if (!getDictionaryCallbackClient().getDefaultDictionary()->isLoaded())
				loadDictionary();
		}
		else
		{
			loadDictionary();
		}

		clearSensitiveInfo();
		_isInitialized = true;

		if ( isApiDispatching() && !_atExit )
		{
			if ( !configImpl->getCpuApiThreadBind().empty() )
			{
				_cpuApiThreadBind = configImpl->getCpuApiThreadBind();
			}

			start();

			/* Waits until the API dispatch thread started */
			while ( !_bApiDispatchThreadStarted && !_atExit ) OmmBaseImplMap<OmmBaseImpl>::sleep(100);
		}
		
		if (_atExit)
		{
			throwIueException("Application or user initiated exit while running initialize.", OmmInvalidUsageException::InvalidOperationEnum);
		}

		_userLock.unlock();
	}
	catch ( const OmmException& ommException )
	{
		if (OmmBaseImplMap<OmmBaseImpl>::has(this))
		{
			uninitialize(false, true);
		}

		_userLock.unlock();

		/* Acquire the cleanup lock to ensure that the interrupted thread can cleanup this object properly */
		OmmBaseImplMap<OmmBaseImpl>::acquireCleanupLock();

		if (hasErrorClientHandler())
		{
			OmmBaseImplMap<OmmBaseImpl>::releaseCleanupLock();
			notifErrorClientHandler(ommException, getErrorClientHandler());
		}
		else
		{
			OmmBaseImplMap<OmmBaseImpl>::releaseCleanupLock();
			throw;
		}
	}
}

//only for unit test, internal use
void OmmBaseImpl::initializeForTest(EmaConfigImpl* configImpl)
{
	try
	{
		_userLock.lock();

		readConfig(configImpl);

		_pLoggerClient = OmmLoggerClient::create(
			_activeConfig.loggerConfig.loggerType,
			_activeConfig.loggerConfig.includeDateInLoggerOutput,
			_activeConfig.loggerConfig.minLoggerSeverity,
			_activeConfig.loggerConfig.loggerFileName,
			_activeConfig.loggerConfig.maxFileSize,
			_activeConfig.loggerConfig.maxFileNumber );

		readCustomConfig(configImpl);

		configImpl->configErrors().log(_pLoggerClient, _activeConfig.loggerConfig.minLoggerSeverity);
		
		if (OmmLoggerClient::VerboseEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Print out active configuration detail.");
			temp.append(_activeConfig.configTrace());
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, temp);
		}

		_userLock.unlock();
	}
	catch (const OmmException& ommException)
	{
		_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::VerboseEnum, ommException.getText());
		_userLock.unlock();
	}
}

ErrorClientHandler& OmmBaseImpl::getErrorClientHandler()
{
	return *_pErrorClientHandler;
}

bool OmmBaseImpl::hasErrorClientHandler() const
{
	return _pErrorClientHandler != 0 ? true : false;
}

EmaList< TimeOut* >& OmmBaseImpl::getTimeOutList()
{
	return _theTimeOuts;
}

Mutex& OmmBaseImpl::getTimeOutMutex()
{
	return _timeOutLock;
}

void OmmBaseImpl::installTimeOut()
{
	pipeWrite();
}

bool OmmBaseImpl::isPipeWritten()
{
	MutexLocker lock( _pipeLock );

	return ( _pipeWriteCount > 0 ? true : false );
}

bool OmmBaseImpl::isInitialized()
{
	return _isInitialized;
}


void OmmBaseImpl::pipeWrite()
{
	MutexLocker lock( _pipeLock );

	if ( ++_pipeWriteCount == 1 )
		_pipe.write( "0", 1 );
}

void OmmBaseImpl::pipeRead()
{
	MutexLocker lock( _pipeLock );

	if ( --_pipeWriteCount == 0 )
	{
		char temp[10];
		_pipe.read( temp, 1 );
	}
}

void OmmBaseImpl::cleanUp()
{
	uninitialize( true, false );
}

void OmmBaseImpl::uninitialize( bool caughtExcep, bool calledFromInit )
{
	OmmBaseImplMap<OmmBaseImpl>::remove(this);

	// prevents invoking uninitialize twice
	if ( _bUninitializeInvoked )
	{
		return;
	}
	_bUninitializeInvoked = true;

	_atExit = true;

	if (isApiDispatching() && !caughtExcep)
	{
		stop();
		eventReceived();
		msgDispatched();
		pipeWrite();

		if (!calledFromInit)
		{
			_dispatchLock.lock();
			_userLock.lock();
			if (_bApiDispatchThreadStarted) wait();
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

	if ( _pRsslReactor )
	{
		if ( _pLoginCallbackClient && !caughtExcep )
			rsslReactorDispatchLoop( 10000, _pLoginCallbackClient->sendLoginClose(), _bEventReceived );

		if ( _pChannelCallbackClient )
			_pChannelCallbackClient->closeChannels();

		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo( &rsslErrorInfo );

		if ( RSSL_RET_SUCCESS != rsslDestroyReactor( _pRsslReactor, &rsslErrorInfo ) )
		{
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Failed to uninitialize OmmBaseImpl( rsslDestroyReactor )." );
				temp.append( "' Error Id='" ).append( rsslErrorInfo.rsslError.rsslErrorId )
				.append( "' Internal sysError='" ).append( rsslErrorInfo.rsslError.sysError )
				.append( "' Error Location='" ).append( rsslErrorInfo.errorLocation )
				.append( "' Error Text='" ).append( rsslErrorInfo.rsslError.text ).append( "'. " );

				_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
			}
		}

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
			_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
		}
	}

#ifdef USING_SELECT
	if ( _pipe.isInitialized() )
	{
		FD_CLR( _pipe.readFD(), &_readFds );
		FD_CLR( _pipe.readFD(), &_exceptFds );
	}
#else
	if ( _pipe.isInitialized() )
	{
		removeFd( _pipe.readFD() );
		_pipeReadEventFdsIdx = -1;
	}
#endif
	_pipe.close();

	OmmLoggerClient::destroy( _pLoggerClient );

	_state = NotInitializedEnum;

	if ( !calledFromInit ) _userLock.unlock();

#ifdef USING_POLL
	if ( _eventFds )
		delete[] _eventFds;
#endif
}

void OmmBaseImpl::setAtExit()
{
	_atExit = true;
	eventReceived();
	msgDispatched();
	pipeWrite();
}

bool OmmBaseImpl::isAtExit()
{
	return _atExit;
}

/* Add these common FDs after clearing all sockets */
void OmmBaseImpl::addCommonSocket()
{
#ifdef USING_SELECT
	FD_SET(_pipe.readFD(), &_readFds);
	FD_SET(_pipe.readFD(), &_exceptFds);
	FD_SET(_pRsslReactor->eventFd, &_readFds);
	FD_SET(_pRsslReactor->eventFd, &_exceptFds);
#else
	_pipeReadEventFdsIdx = addFd(_pipe.readFD());
	addFd(_pRsslReactor->eventFd);
#endif
}

Int64 OmmBaseImpl::rsslReactorDispatchLoop( Int64 timeOut, UInt32 count, bool& bMsgDispRcvd )
{
	bMsgDispRcvd = false;

	Int64 startTime = GetTime::getMicros();
	Int64 endTime = 0;
	Int64 nextTimer = 0;

	bool userTimeoutExists( TimeOut::getTimeOutInMicroSeconds( *this, nextTimer ) );
	if ( userTimeoutExists )
	{
		if ( timeOut >= 0 && timeOut < nextTimer )
			userTimeoutExists = false;
		else
			timeOut = nextTimer;
	}

	// Get the negotiated ping timeout
	Int64 pingTimeout = (_negotiatedPingTimeout > 0) ? _negotiatedPingTimeout : DEFAULT_CONNECTION_PINGTIMEOUT;
	Int64 pingTimeoutInMicroSeconds = pingTimeout * 1000 / 2;

	RsslReactorDispatchOptions dispatchOpts;
	dispatchOpts.pReactorChannel = NULL;
	dispatchOpts.maxMessages = count;

	RsslRet reactorRetCode = RSSL_RET_SUCCESS;
	UInt64 loopCount = 0;

	endTime = GetTime::getMicros();

	if ( ( timeOut >= 0 ) && ( endTime - startTime >= timeOut ) )
	{
		if ( userTimeoutExists )
			TimeOut::execute( *this );
	}

	if ( timeOut >= 0 )
	{
		timeOut -= endTime - startTime;
		if ( timeOut < 0 )
			timeOut = 0;
	}

	do
	{
		startTime = endTime;

		Int64 selectRetCode = 1;

		// Do not wait infinitely in the select if there is a timeout event in the list.
		if ( ( timeOut < 0 ) && getTimeOutList().size() != 0 )
		{
			return bMsgDispRcvd ? 0 : -1;
		}

		// to account ping timeout
		Int64 selectTimeOut = timeOut;
		if ( timeOut < 0 || pingTimeoutInMicroSeconds < timeOut )
		{
			selectTimeOut = pingTimeoutInMicroSeconds;
		}

#if defined( USING_SELECT )

		fd_set useReadFds = _readFds;
		fd_set useExceptFds = _exceptFds;

		struct timeval selectTime;
		if ( selectTimeOut > 0 )
		{
			selectTime.tv_sec = static_cast<long>( selectTimeOut / 1000000 );
			selectTime.tv_usec = selectTimeOut % 1000000;

			selectRetCode = select( FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime );
		}
		else if ( selectTimeOut == 0 )
		{
			selectTime.tv_sec = 0;
			selectTime.tv_usec = 0;

			selectRetCode = select(FD_SETSIZE, &useReadFds, NULL, &useExceptFds, &selectTime);
		}
		else if ( selectTimeOut < 0 )
			selectRetCode = select( FD_SETSIZE, &useReadFds, NULL, &useExceptFds, NULL );

		if ( selectRetCode > 0 && FD_ISSET( _pipe.readFD(), &useReadFds ) )
		{
			pipeRead();
			--selectRetCode;
		}

#elif defined( USING_PPOLL )

		struct timespec ppollTime;

		if ( selectTimeOut > 0 )
		{
			ppollTime.tv_sec = selectTimeOut / static_cast<long long>( 1e6 );
			ppollTime.tv_nsec = selectTimeOut % static_cast<long long>( 1e6 ) * static_cast<long long>( 1e3 );
			selectRetCode = ppoll( _eventFds, _eventFdsCount, &ppollTime, 0 );
		}
		else if ( selectTimeOut == 0 )
		{
			ppollTime.tv_sec = 0;
			ppollTime.tv_nsec = 0;

			selectRetCode = ppoll(_eventFds, _eventFdsCount, &ppollTime, 0);
		}
		else if ( selectTimeOut < 0 )
			selectRetCode = ppoll( _eventFds, _eventFdsCount, 0, 0 );

		if ( selectRetCode > 0 )
		{
			if ( _pipeReadEventFdsIdx == -1 )
				for ( int i = 0; i < _eventFdsCount; ++i )
					if ( _eventFds[i].fd == _pipe.readFD() )
					{
						_pipeReadEventFdsIdx = i;
						break;
					}

			if ( _pipeReadEventFdsIdx != -1 )
				if ( _eventFds[_pipeReadEventFdsIdx].revents & POLLIN )
				{
					pipeRead();
					--selectRetCode;
				}
		}

#else
#error "No Implementation for Operating System That Does Not Implement ppoll"
#endif

		if ( selectRetCode > 0 )
		{
			loopCount = 0;
			do
			{
				_userLock.lock();
				reactorRetCode = _pRsslReactor ? rsslReactorDispatch( _pRsslReactor, &dispatchOpts, &_reactorDispatchErrorInfo ) : RSSL_RET_SUCCESS;
				_userLock.unlock();
				++loopCount;
			}
			while ( reactorRetCode > RSSL_RET_SUCCESS && !bMsgDispRcvd && loopCount < 10 );

			if ( reactorRetCode < RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity || hasErrorClientHandler() )
				{
					EmaString temp( "Call to rsslReactorDispatch() failed. Internal sysError='" );
					temp.append( _reactorDispatchErrorInfo.rsslError.sysError )
						.append( "' Error Id " ).append( _reactorDispatchErrorInfo.rsslError.rsslErrorId ).append( "' " )
						.append( "' Error Location='" ).append( _reactorDispatchErrorInfo.errorLocation ).append( "' " )
						.append( "' Error text='" ).append( _reactorDispatchErrorInfo.rsslError.text ).append( "'. " );

					_userLock.lock();

					if (hasErrorClientHandler())
					{
						getErrorClientHandler().onDispatchError(temp, _reactorDispatchErrorInfo.rsslErrorInfoCode);
					}

					if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
					{
						if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
					}

					_userLock.unlock();
				}

				return -2;
			}

			if ( bMsgDispRcvd ) return 0;

			TimeOut::execute( *this );

			if ( bMsgDispRcvd ) return 0;

			endTime = GetTime::getMicros();

			if ( timeOut >= 0 )
			{
				if ( endTime > startTime + timeOut ) return -1;

				timeOut -= ( endTime - startTime );
			}
		}
		else if ( selectRetCode == 0 )
		{
			// if select/ppoll breaks by timeout, it calls rsslReactorDispatch to allow check the channel Ping timeout
			_userLock.lock();
			reactorRetCode = _pRsslReactor ? rsslReactorDispatch( _pRsslReactor, &dispatchOpts, &_reactorDispatchErrorInfo ) : RSSL_RET_SUCCESS;
			_userLock.unlock();

			if ( reactorRetCode < RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity || hasErrorClientHandler() )
				{
					EmaString temp( "Call to rsslReactorDispatch() failed. Internal sysError='" );
					temp.append( _reactorDispatchErrorInfo.rsslError.sysError )
						.append( "' Error Id " ).append( _reactorDispatchErrorInfo.rsslError.rsslErrorId ).append( "' " )
						.append( "' Error Location='" ).append( _reactorDispatchErrorInfo.errorLocation ).append( "' " )
						.append( "' Error text='" ).append( _reactorDispatchErrorInfo.rsslError.text ).append( "'. " );

					_userLock.lock();

					if (hasErrorClientHandler())
					{
						getErrorClientHandler().onDispatchError(temp, _reactorDispatchErrorInfo.rsslErrorInfoCode);
					}

					if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
					{
						if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
					}

					_userLock.unlock();
				}

				return -2;
			}

			TimeOut::execute( *this );

			if ( bMsgDispRcvd ) return 0;

			endTime = GetTime::getMicros();

			if ( timeOut >= 0 )
			{
				if ( endTime > startTime + timeOut ) return -1;

				timeOut -= ( endTime - startTime );
			}
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
					if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
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
			return -2;
		}
	} while ( true );
}

void OmmBaseImpl::setState( ImplState state )
{
	_state = state;
}

void OmmBaseImpl::closeChannel( RsslReactorChannel* pRsslReactorChannel )
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
			temp.append( "' RsslChannel='" ).append( ( UInt64 )rsslErrorInfo.rsslError.channel )
			.append( "' Error Id='" ).append( rsslErrorInfo.rsslError.rsslErrorId )
			.append( "' Internal sysError='" ).append( rsslErrorInfo.rsslError.sysError )
			.append( "' Error Location='" ).append( rsslErrorInfo.errorLocation )
			.append( "' Error Text='" ).append( rsslErrorInfo.rsslError.text ).append( "'. " );

			_userLock.lock();

			if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );

			_userLock.unlock();
		}
	}

	_pChannelCallbackClient->removeChannel( pRsslReactorChannel );
}

void OmmBaseImpl::handleIue( const EmaString& text, Int32 errorCode )
{
	if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, text );

	if ( hasErrorClientHandler() )
	{
		getErrorClientHandler().onInvalidUsage( text );
		getErrorClientHandler().onInvalidUsage( text, errorCode );
	}
	else
		throwIueException( text, errorCode );
}

void OmmBaseImpl::handleIue( const char* text, Int32 errorCode )
{
	if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, text );

	if (hasErrorClientHandler())
	{
		getErrorClientHandler().onInvalidUsage( text );
		getErrorClientHandler().onInvalidUsage( text, errorCode );
	}
	else
		throwIueException( text, errorCode );
}

void OmmBaseImpl::handleIhe( UInt64 handle, const EmaString& text )
{
	if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, text );

	if ( hasErrorClientHandler() )
		getErrorClientHandler().onInvalidHandle( handle, text );
	else
		throwIheException( handle, text );
}

void OmmBaseImpl::handleIhe( UInt64 handle, const char* text )
{
	if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, text );

	if ( hasErrorClientHandler() )
		getErrorClientHandler().onInvalidHandle( handle, text );
	else
		throwIheException( handle, text );
}

void OmmBaseImpl::handleMee( const char* text )
{
	if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		getOmmLoggerClient().log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, text );

	if ( hasErrorClientHandler() )
		getErrorClientHandler().onMemoryExhaustion( text );
	else
		throwMeeException( text );
}

void OmmBaseImpl::handleJce(const char* text, Int32 errorCode, RsslReactorChannel* reactorChannel, OmmProvider* provider)
{
	if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
		getOmmLoggerClient().log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, text);

	if (hasErrorClientHandler())
	{
		getErrorClientHandler().onJsonConverter(text, errorCode, reactorChannel, NULL, provider);
	}
	else if (!isApiDispatching())
		throwJConverterException(text, errorCode, reactorChannel, NULL, provider);
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

bool OmmBaseImpl::hasRestLoggingCallbackClient() const
{
	return _pRestLoggingCallbackClient != 0 ? true : false;
}

RestLoggingCallbackClient& OmmBaseImpl::getRestLoggingCallbackClient()
{
	return *_pRestLoggingCallbackClient;
}

OmmLoggerClient& OmmBaseImpl::getOmmLoggerClient()
{
	return *_pLoggerClient;
}

void OmmBaseImpl::reissue( const ReqMsg& reqMsg, UInt64 handle )
{
	try
	{
		_userLock.lock();
		if (_pItemCallbackClient) _pItemCallbackClient->reissue(reqMsg, handle);
		_userLock.unlock();
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

void OmmBaseImpl::unregister( UInt64 handle )
{
	try
	{
		_userLock.lock();
		if (_pItemCallbackClient) _pItemCallbackClient->unregister(handle);
		_userLock.unlock();
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

void OmmBaseImpl::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	try
	{
		_userLock.lock();
		if (_pItemCallbackClient) _pItemCallbackClient->submit(genericMsg, handle);
		_userLock.unlock();
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

void OmmBaseImpl::submit( const PostMsg& postMsg, UInt64 handle )
{
	try
	{
		_userLock.lock();
		if (_pItemCallbackClient) _pItemCallbackClient->submit(postMsg, handle);
		_userLock.unlock();
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

ActiveConfig& OmmBaseImpl::getActiveConfig()
{
	return _activeConfig;
}

LoggerConfig& OmmBaseImpl::getActiveLoggerConfig()
{
	return _activeConfig.loggerConfig;
}

Mutex& OmmBaseImpl::getUserMutex()
{
	return _userLock;
}

void OmmBaseImpl::setInOAuthCallback(bool inCallback)
{
	_inOAuthCallback = inCallback;
}

bool OmmBaseImpl::getInOAuthCallback()
{
	return _inOAuthCallback;
}

void OmmBaseImpl::setInLoginCredentialCallback(bool inCallback)
{
	_inLoginCredentialCallback = inCallback;
}


bool OmmBaseImpl::getInLoginCredentialCallback()
{
	return _inLoginCredentialCallback;
}


void OmmBaseImpl::run()
{
	_dispatchLock.lock();
	_bApiDispatchThreadStarted = true;


	/* Bind cpu for the API thread. */
	if ( !_cpuApiThreadBind.empty() )
	{
#ifdef NO_ETA_CPU_BIND
		_dispatchLock.unlock();
		EmaString temp("CPU Binding is not supported by this EMA library build. OmmBaseImpl::run().");
		
		if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
		setAtExit();
		return;
#else
		RsslRet ret;
		RsslErrorInfo rsslErrorInfo;
		clearRsslErrorInfo(&rsslErrorInfo);

		if ( (ret = rsslBindThread(_cpuApiThreadBind.c_str(), &rsslErrorInfo)) != RSSL_RET_SUCCESS )
		{
			_dispatchLock.unlock();
			EmaString temp( "Failed to bind Cpu for API thread. OmmBaseImpl::run()." );
			temp.append( " CPU='" ).append( _cpuApiThreadBind )
				.append( "' Error Id='" ).append( rsslErrorInfo.rsslError.rsslErrorId )
				.append( "' Internal sysError='" ).append( rsslErrorInfo.rsslError.sysError )
				.append( "' Error Location='" ).append (rsslErrorInfo.errorLocation )
				.append( "' Error Text='" ).append( rsslErrorInfo.rsslError.text ).append( "'." );
			if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp );
			setAtExit();
			return;
		}

		if ( OmmLoggerClient::SuccessEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "EMA Api thread bound to CPU: " );
			temp.append( _cpuApiThreadBind ).append( "." );

			if ( _pLoggerClient ) _pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::SuccessEnum, temp );
		}
#endif
	}

	while ( !Thread::isStopping() && !_atExit )
		rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread, _bEventReceived );

	_dispatchLock.unlock();
}

int OmmBaseImpl::runLog( void* pExceptionStructure, const char* file, unsigned int line )
{
	char reportBuf[EMA_BIG_STR_BUFF_SIZE * 10];
	if ( retrieveExceptionContext( pExceptionStructure, file, line, reportBuf, EMA_BIG_STR_BUFF_SIZE * 10 ) > 0 )
	{
		try
		{
			_userLock.lock();
			if (_pLoggerClient) _pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, reportBuf);
			_userLock.unlock();
		}
		catch (...)
		{
			_userLock.unlock();
			throw;
		}
	}

	return 1;
}

RsslReactorCallbackRet OmmBaseImpl::channelCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pEvent )
{
	static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->eventReceived();
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getChannelCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::loginCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMLoginMsgEvent* pEvent )
{
	static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->eventReceived();
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getLoginCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::directoryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDirectoryMsgEvent* pEvent )
{
	static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->eventReceived();
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getDirectoryCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::dictionaryCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslRDMDictionaryMsgEvent* pEvent )
{
	static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->eventReceived();
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getDictionaryCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::itemCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->eventReceived();
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->_pItemCallbackClient->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::channelOpenCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel, RsslReactorChannelEvent* pEvent )
{
	static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->eventReceived();
	return static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr )->getChannelCallbackClient().processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
}

RsslReactorCallbackRet OmmBaseImpl::oAuthCredentialCallback(RsslReactor* pRsslReactor, RsslReactorOAuthCredentialEvent* oAuthEvent)
{
	OmmBaseImpl* pBaseImpl = static_cast<OmmBaseImpl*>(pRsslReactor->userSpecPtr);
	OmmConsumerEvent event;
	OmmOAuth2CredentialImpl* oAuthCredentialImpl = (OmmOAuth2CredentialImpl*)oAuthEvent->userSpecPtr;

	pBaseImpl->setInOAuthCallback(true);

	/* Handle and parent handle are set to 0 here, as there isn't any associated item handle */
	event._channel = NULL;
	event._closure = oAuthCredentialImpl->getClosure();

	/* Call back the user with the event */
	if(pBaseImpl->isInitialized())
		oAuthCredentialImpl->getClient().onCredentialRenewal(event);
	else
	{
		RsslErrorInfo rsslErrorInfo;
		RsslReactorOAuthCredentialRenewal credentialRenewal;
		RsslReactorOAuthCredentialRenewalOptions renewalOpts;
		
		rsslClearReactorOAuthCredentialRenewal(&credentialRenewal);
		rsslClearReactorOAuthCredentialRenewalOptions(&renewalOpts);

		renewalOpts.renewalMode = RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD;

		if (!oAuthCredentialImpl->getUserName().empty())
		{
			credentialRenewal.userName.data = (char*)oAuthCredentialImpl->getUserName().c_str();
			credentialRenewal.userName.length = oAuthCredentialImpl->getUserName().length();
		}

		if (!oAuthCredentialImpl->getPassword().empty())
		{
			credentialRenewal.password.data = (char*)oAuthCredentialImpl->getPassword().c_str();
			credentialRenewal.password.length = oAuthCredentialImpl->getPassword().length();
		}

		if (!oAuthCredentialImpl->getClientId().empty())
		{
			credentialRenewal.clientId.data = (char*)oAuthCredentialImpl->getClientId().c_str();
			credentialRenewal.clientId.length = oAuthCredentialImpl->getClientId().length();
		}

		if (!oAuthCredentialImpl->getClientSecret().empty())
		{
			credentialRenewal.clientSecret.data = (char*)oAuthCredentialImpl->getClientSecret().c_str();
			credentialRenewal.clientSecret.length = oAuthCredentialImpl->getClientSecret().length();
		}

		if (!oAuthCredentialImpl->getClientJWK().empty())
		{
			credentialRenewal.clientJWK.data = (char*)oAuthCredentialImpl->getClientJWK().c_str();
			credentialRenewal.clientJWK.length = oAuthCredentialImpl->getClientJWK().length();
		}

		if (!oAuthCredentialImpl->getTokenScope().empty())
		{
			credentialRenewal.tokenScope.data = (char*)oAuthCredentialImpl->getTokenScope().c_str();
			credentialRenewal.tokenScope.length = oAuthCredentialImpl->getTokenScope().length();
		}

		credentialRenewal.takeExclusiveSignOnControl = oAuthCredentialImpl->getTakeExclusiveSignOnControl();


		rsslReactorSubmitOAuthCredentialRenewal(pRsslReactor, &renewalOpts, &credentialRenewal, &rsslErrorInfo);
	}

	pBaseImpl->setInOAuthCallback(false);

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet OmmBaseImpl::loginCredentialCallback(RsslReactor* pRsslReactor, RsslReactorChannel* pReactorChannel, RsslReactorLoginRenewalEvent* pEvent)
{
	OmmBaseImpl* pBaseImpl = static_cast<OmmBaseImpl*>(pRsslReactor->userSpecPtr);
	OmmConsumerEvent event;
	LoginRdmReqMsgImpl* pLoginCredentialImpl = (LoginRdmReqMsgImpl*)pEvent->userSpecPtr;

	pBaseImpl->setInLoginCredentialCallback(true);

	/* Handle and parent handle are set to 0 here, as there isn't any associated item handle */
	event._channel = pReactorChannel;
	event._closure = pLoginCredentialImpl->getClosure();
	pBaseImpl->_tmpLoginMsg = pLoginCredentialImpl;
	pBaseImpl->_tmpChnl = pReactorChannel;

	/* Call back the user with the event */
	if(pBaseImpl->isInitialized() && pLoginCredentialImpl->hasLoginClient())
		pLoginCredentialImpl->getClient().onLoginCredentialRenewal(event);
	else
	{
		RsslReactorLoginCredentialRenewalOptions opts;
		RsslErrorInfo Error;
		rsslClearReactorLoginCredentialRenewalOptions(&opts);
		opts.userName = pEvent->pLoginMsg->userName;
		opts.authenticationExtended = pEvent->pLoginMsg->authenticationExtended;

		rsslReactorSubmitLoginCredentialRenewal(pRsslReactor, pReactorChannel, &opts, &Error);
	}

	pBaseImpl->setInLoginCredentialCallback(false);
	pBaseImpl->_activeConfig.pRsslRDMLoginReq = pLoginCredentialImpl;
	pBaseImpl->getLoginCallbackClient().setLoginRequest(pLoginCredentialImpl);
	return RSSL_RC_CRET_SUCCESS;
}


RsslReactorCallbackRet OmmBaseImpl::jsonConversionEventCallback(RsslReactor *pRsslReactor, RsslReactorChannel *pRsslReactorChannel, RsslReactorJsonConversionEvent *pEvent)
{
	if (pEvent->pError)
	{
		OmmBaseImpl* pOmmBaseImpl = static_cast<OmmBaseImpl*>( pRsslReactor->userSpecPtr );
		pOmmBaseImpl->getUserMutex().unlock();

		OmmProvider* pProvider = NULL;

		if (pOmmBaseImpl->getImplType() == OmmCommonImpl::NiProviderEnum)
		{
			OmmNiProviderImpl* pOmmNiProviderImpl = static_cast<OmmNiProviderImpl*>(pOmmBaseImpl);
			pProvider = pOmmNiProviderImpl->getProvider();
		}

		pOmmBaseImpl->handleJce(EmaString(pEvent->pError->rsslError.text), pEvent->pError->rsslError.rsslErrorId, pRsslReactorChannel, pProvider);
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslRet OmmBaseImpl::serviceNameToIdCallback(RsslReactor *pRsslReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent)
{
	const Directory* pDirectory = static_cast<OmmBaseImpl*>(pRsslReactor->userSpecPtr)->getDirectoryCallbackClient().getDirectory( EmaString( pServiceName->data, pServiceName->length ) );
	if ( pDirectory )
	{
		*pServiceId = (RsslUInt16)pDirectory->getId();
		return RSSL_RET_SUCCESS;
	}

	return RSSL_RET_FAILURE;
}

RsslReactorCallbackRet OmmBaseImpl::restLoggingCallback(RsslReactor* pRsslReactor, RsslReactorRestLoggingEvent* pLogEvent)
{
	OmmBaseImpl* pOmmBaseImpl = static_cast<OmmBaseImpl*>(pRsslReactor->userSpecPtr);

	pOmmBaseImpl->eventReceived();
	if (pOmmBaseImpl->hasRestLoggingCallbackClient())
	{
		RestLoggingCallbackClient& restLoggingCallbackClient = pOmmBaseImpl->getRestLoggingCallbackClient();
		OmmConsumerRestLoggingEvent
			ommLogRestEvent(pLogEvent->pRestLoggingMessage->data, pLogEvent->pRestLoggingMessage->length, restLoggingCallbackClient.getRestLoggingClosure());
		restLoggingCallbackClient.onRestLoggingEvent(ommLogRestEvent);
	}

	return RSSL_RC_CRET_SUCCESS;
}

const EmaString& OmmBaseImpl::getInstanceName() const
{
	return _activeConfig.instanceName;
}

void OmmBaseImpl::msgDispatched( bool value )
{
	_bMsgDispatched = value;
}

void OmmBaseImpl::eventReceived( bool value )
{
	_bEventReceived = value;
}

void OmmBaseImpl::notifErrorClientHandler( const OmmException& ommException, ErrorClientHandler& errorClient )
{
	switch ( ommException.getExceptionType() )
	{
	case OmmException::OmmSystemExceptionEnum :
		errorClient.onSystemError( static_cast<const OmmSystemException&>( ommException ).getSystemExceptionCode(),
		                           static_cast<const OmmSystemException&>( ommException ).getSystemExceptionAddress(),
		                           ommException.getText() );
		break;
	case OmmException::OmmInvalidHandleExceptionEnum:
		errorClient.onInvalidHandle( static_cast<const OmmInvalidHandleException&>( ommException ).getHandle(),
		                             ommException.getText() );
		break;
	case OmmException::OmmInvalidUsageExceptionEnum:
		errorClient.onInvalidUsage( ommException.getText() );
		errorClient.onInvalidUsage( ommException.getText(), static_cast<const OmmInvalidUsageException&>( ommException ).getErrorCode() );
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
	OmmBaseImpl* user = reinterpret_cast<OmmBaseImpl*>( object );
	user->_eventTimedOut = true;
}

// Allows modifying some I/O values programmatically for Reactor to override the default values.
void OmmBaseImpl::modifyReactorIOCtl(Int32 code, Int32 value)
{
	_userLock.lock();

	if ( _pRsslReactor == NULL )
	{
		_userLock.unlock();
		EmaString temp("No Reactor to modify I/O option.");
		handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);

	RsslRet ret = rsslReactorIoctl(_pRsslReactor, (RsslReactorIoctlCodes)code, &value, &rsslErrorInfo);

	if (ret != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Failed to modify I/O option for code = ");
		temp.append(code).append(".").append(CR)
			.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);
		handleIue(temp, ret);
		return;
	}

	_userLock.unlock();
	return;
}

// Allows modifying some I/O values programmatically for Reactor to override the default values.
void OmmBaseImpl::modifyReactorChannelIOCtl(Int32 code, void* value)
{
	if (code != RsslReactorChannelIoctlCodes::RSSL_REACTOR_CHANNEL_IOCTL_DIRECT_WRITE &&
		code != RsslReactorChannelIoctlCodes::RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS)
	{
		EmaString temp("Income IOCtl code: ");
		temp.append(code);
		temp.append(" is not valid.");
		handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}
	
	if (value == NULL)
	{
		EmaString temp("Income IOCtl value is NULL.");
		handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return;
	}

	_userLock.lock();

	Channel* pChannel;
	if ((pChannel = getLoginCallbackClient().getActiveChannel()) == NULL || (pChannel->getRsslChannel() == NULL))
	{
		_userLock.unlock();
		EmaString temp("No active channel to modify I/O option.");
		handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		return;
	}

	RsslErrorInfo rsslErrorInfo;
	RsslPreferredHostOptions rsslPreferredHost;
	void *valueIOCtl = NULL;

	clearRsslErrorInfo(&rsslErrorInfo);

	if (code == RSSL_REACTOR_CHANNEL_IOCTL_PREFERRED_HOST_OPTIONS)
	{
		PreferredHostOptions *prefHost = (PreferredHostOptions *)value;

		rsslClearRsslPreferredHostOptions(&rsslPreferredHost);

		rsslPreferredHost.enablePreferredHostOptions = prefHost->getEnablePreferredHostOptions();
		rsslPreferredHost.detectionTimeSchedule.data = const_cast<char*>(prefHost->getPHDetectionTimeSchedule().c_str());
		rsslPreferredHost.detectionTimeSchedule.length = prefHost->getPHDetectionTimeSchedule().length();
		rsslPreferredHost.detectionTimeInterval = prefHost->getPHDetectionTimeInterval();
		rsslPreferredHost.fallBackWithInWSBGroup = prefHost->getPHFallBackWithInWSBGroup();

		if (!prefHost->getPreferredChannelName().empty())
		{
			for (unsigned i = 0; i < _activeConfig.configChannelSet.size(); i++)
			{
				if (_activeConfig.configChannelSet[i]->name == prefHost->getPreferredChannelName())
				{
					rsslPreferredHost.connectionListIndex = i;
					break;
				}
				if (i == _activeConfig.configChannelSet.size() - 1) 
				{
					_userLock.unlock();
					EmaString temp("Preferred host channel name: ");
						temp.append(prefHost->getPreferredChannelName());
						temp.append(" is not present in configuration.");
					handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
					return;
				}
			}
		}

		if (!prefHost->getPreferredWSBChannelName().empty())
		{
			for (unsigned i = 0; i < _activeConfig.configWarmStandbySet.size(); i++)
			{
				if (_activeConfig.configWarmStandbySet[i]->name == prefHost->getPreferredWSBChannelName())
				{
					rsslPreferredHost.warmStandbyGroupListIndex = i;
					break;
				}
				if (i == _activeConfig.configWarmStandbySet.size() - 1)
				{
					_userLock.unlock();
					EmaString temp("Preferred host WSB channel name: ");
					temp.append(prefHost->getPreferredWSBChannelName());
					temp.append(" is not present in configuration.");
					handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
					return;
				}
			}
		}

		valueIOCtl = &rsslPreferredHost;
	}
	else
		valueIOCtl = value;

	RsslRet ret = rsslReactorChannelIoctl(pChannel->getRsslChannel(), (RsslReactorChannelIoctlCodes)code, valueIOCtl, &rsslErrorInfo);

	if (ret != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Failed to modify I/O option for code = ");
		temp.append(code).append(".").append(CR)
			.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);
		handleIue(temp, ret);
		return;
	}

	_userLock.unlock();
	return;
}


void OmmBaseImpl::saveNegotiatedPingTimeout(UInt32 timeoutMs)
{
	if ( timeoutMs > 0 )
	{
		if ( _negotiatedPingTimeout == 0 || timeoutMs < _negotiatedPingTimeout )
			_negotiatedPingTimeout = timeoutMs;
	}
}
