/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmBaseImpl_h
#define __refinitiv_ema_access_OmmBaseImpl_h

#ifdef WIN32
#define USING_SELECT
#else
#define USING_POLL
#define USING_PPOLL
#endif

#include "rtr/rsslReactor.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "Mutex.h"
#include "Thread.h"
#include "OmmLoggerClient.h"
#include "OmmConsumerClient.h"
#include "OmmProviderClient.h"
#include "Pipe.h"
#include "TimeOut.h"
#include "ActiveConfig.h"
#include "ErrorClientHandler.h"
#include "OmmException.h"
#include "OmmBaseImplMap.h"
#include "IOCtlReactorCode.h"
#include "RestLoggingCallbackClient.h"

namespace refinitiv {

namespace ema {

namespace access {

class ChannelCallbackClient;
class LoginCallbackClient;
class DirectoryCallbackClient;
class DictionaryCallbackClient;
class ItemCallbackClient;
class OmmConsumerClient;
class OmmOAuth2ConsumerClient;
class OmmProviderClient;
class OmmLoggerClient;
class TimeOut;
class TunnelStreamRequest;
class EmaConfigImpl;
class OmmProvider;

class OmmBaseImpl : public OmmCommonImpl, public Thread, public TimeOutClient
{
public :

	static RsslReactorCallbackRet channelCallback( RsslReactor*, RsslReactorChannel*, RsslReactorChannelEvent* );

	static RsslReactorCallbackRet loginCallback( RsslReactor*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );

	static RsslReactorCallbackRet directoryCallback( RsslReactor*, RsslReactorChannel*, RsslRDMDirectoryMsgEvent* );

	static RsslReactorCallbackRet dictionaryCallback( RsslReactor*, RsslReactorChannel*, RsslRDMDictionaryMsgEvent* );

	static RsslReactorCallbackRet itemCallback( RsslReactor*, RsslReactorChannel*, RsslMsgEvent* );

	static RsslReactorCallbackRet channelOpenCallback( RsslReactor*, RsslReactorChannel*, RsslReactorChannelEvent* );

	static RsslReactorCallbackRet jsonConversionEventCallback(RsslReactor *pReactor, RsslReactorChannel *pReactorChannel, RsslReactorJsonConversionEvent *pEvent);

	static RsslRet serviceNameToIdCallback(RsslReactor *pReactor, RsslBuffer* pServiceName, RsslUInt16* pServiceId, RsslReactorServiceNameToIdEvent* pEvent);

	static RsslReactorCallbackRet restLoggingCallback(RsslReactor* pReactor, RsslReactorRestLoggingEvent* pLogEvent);

	static RsslReactorCallbackRet oAuthCredentialCallback(RsslReactor* pRsslReactor, RsslReactorOAuthCredentialEvent* oAuthEvent);

	virtual const EmaString& getInstanceName() const;

	virtual void reissue( const ReqMsg&, UInt64 );

	virtual void submit( const GenericMsg&, UInt64 );

	virtual void submit( const PostMsg&, UInt64 handle = 0 );

	virtual void unregister( UInt64 handle );

	virtual void addSocket( RsslSocket ) = 0;

	virtual void removeSocket( RsslSocket ) = 0;

	virtual void removeAllSocket() = 0;

	void closeChannel( RsslReactorChannel* );

	enum ImplState
	{
		NotInitializedEnum = 0,
		RsslInitilizedEnum,
		ReactorInitializedEnum,
		RsslChannelDownEnum,
		RsslChannelUpEnum,
		RsslChannelUpStreamNotOpenEnum,
		LoginStreamOpenSuspectEnum,
		LoginStreamOpenOkEnum,
		LoginStreamClosedEnum,
		DirectoryStreamOpenSuspectEnum,
		DirectoryStreamOpenOkEnum
	};

	void setState( ImplState state );

	void msgDispatched( bool value = true );

	void eventReceived( bool value = true );

	ItemCallbackClient& getItemCallbackClient();

	DictionaryCallbackClient& getDictionaryCallbackClient();

	DirectoryCallbackClient& getDirectoryCallbackClient();

	LoginCallbackClient& getLoginCallbackClient();

	ChannelCallbackClient& getChannelCallbackClient();

	RestLoggingCallbackClient& getRestLoggingCallbackClient();

	bool hasRestLoggingCallbackClient() const;

	OmmLoggerClient& getOmmLoggerClient();

	OmmOAuth2ConsumerClient& getOAuth2ConsumerClient();

	ActiveConfig& getActiveConfig();

	LoggerConfig& getActiveLoggerConfig();

	ErrorClientHandler& getErrorClientHandler();

	void* getAdminClosure();

	bool hasErrorClientHandler() const;

	EmaList< TimeOut* >& getTimeOutList();

	Mutex& getTimeOutMutex();

	void installTimeOut();

	virtual void loadDictionary() = 0;

	virtual void reLoadDirectory() = 0;

	virtual void loadDirectory() = 0;

	virtual void setRsslReactorChannelRole( RsslReactorChannelRole&, RsslReactorOAuthCredential* ) = 0;

	virtual void createDictionaryCallbackClient( DictionaryCallbackClient*&, OmmBaseImpl& ) = 0;

	virtual void createDirectoryCallbackClient( DirectoryCallbackClient*&, OmmBaseImpl& ) = 0;

	virtual void processChannelEvent( RsslReactorChannelEvent* ) = 0;

	virtual void setActiveRsslReactorChannel( Channel* activeChannel ) {};

	virtual void unsetActiveRsslReactorChannel(Channel* cancelChannel) {};

	void handleIue( const EmaString&, Int32 errorCode );

	void handleIue( const char*, Int32 errorCode );

	void handleIhe( UInt64 , const EmaString& );

	void handleIhe( UInt64 , const char* );

	void handleMee( const char* );

	void handleJce(const char* text, Int32 errorCode, RsslReactorChannel* reactorChannel, OmmProvider* provider);

	Mutex& getUserMutex();

	bool isAtExit();

	void addCommonSocket();

	void modifyReactorIOCtl(Int32 code, Int32 value);

protected:

	friend class OmmBaseImplMap<OmmBaseImpl>;
	friend class LoginItem;
	friend class NiProviderLoginItem;

	OmmBaseImpl( ActiveConfig& );
	OmmBaseImpl(ActiveConfig&, OmmConsumerClient&, void* = 0);
	OmmBaseImpl(ActiveConfig&, OmmConsumerClient&, OmmOAuth2ConsumerClient&, void* = 0);
	OmmBaseImpl(ActiveConfig&, OmmProviderClient&, void* = 0);
	OmmBaseImpl(ActiveConfig&, OmmOAuth2ConsumerClient&, void* = 0);
	OmmBaseImpl( ActiveConfig&, OmmConsumerErrorClient& );
	OmmBaseImpl(ActiveConfig&, OmmOAuth2ConsumerClient&, OmmConsumerErrorClient&, void* = 0);
	OmmBaseImpl(ActiveConfig&, OmmConsumerClient&, OmmConsumerErrorClient&, void* = 0);
	OmmBaseImpl(ActiveConfig&, OmmConsumerClient&, OmmOAuth2ConsumerClient&, OmmConsumerErrorClient&, void* = 0);
	OmmBaseImpl( ActiveConfig&, OmmProviderErrorClient& );
	OmmBaseImpl(ActiveConfig&, OmmProviderClient&, OmmProviderErrorClient&, void* = 0);

	virtual ~OmmBaseImpl();

	void initialize( EmaConfigImpl* );

	void uninitialize( bool caughtException, bool calledFromInit );

	void readConfig( EmaConfigImpl* );

	virtual void readCustomConfig( EmaConfigImpl* ) = 0;

	ChannelConfig* readChannelConfig( EmaConfigImpl*, const EmaString& , bool);

	WarmStandbyChannelConfig* readWSBChannelConfig(EmaConfigImpl*, const EmaString&, bool);

	WarmStandbyServerInfoConfig* readWarmStandbyServerInfoConfig(EmaConfigImpl*, const EmaString&);

	void readChannelConfigForWSBChannel(EmaConfigImpl*, const EmaString&);

	bool readReliableMcastConfig( EmaConfigImpl*, const EmaString&, ReliableMcastChannelConfig*, EmaString& );

	void useDefaultConfigValues( const EmaString&, const EmaString&, const EmaString& );

	void setAtExit();

	void run();

	void cleanUp();

	int runLog( void*, const char*, unsigned int );

	void pipeWrite();

	void pipeRead();

	bool isPipeWritten();

	// return values:
	// -2 -> error
	// -1 -> timeout expired ( nothing dispatched )
	//  0 -> message(s) was dispatched
	Int64 rsslReactorDispatchLoop( Int64 timeOut, UInt32 count, bool& bMsgDispRcvd );

	static void terminateIf( void* );

	static void notifErrorClientHandler( const OmmException&, ErrorClientHandler& );

	void initializeForTest(EmaConfigImpl*); //only for unit test, internal use 

	ActiveConfig&	_activeConfig;

#ifdef USING_SELECT
	fd_set			_readFds;
	fd_set			_exceptFds;
#endif

	Mutex						_userLock;
	Mutex						_dispatchLock;
	Mutex						_pipeLock;
	Mutex						_timeOutLock;
	RsslErrorInfo				_reactorDispatchErrorInfo;
	ImplState					_state;
	RsslReactor*				_pRsslReactor;
	ChannelCallbackClient*		_pChannelCallbackClient;
	LoginCallbackClient*		_pLoginCallbackClient;
	DirectoryCallbackClient*	_pDirectoryCallbackClient;
	DictionaryCallbackClient*	_pDictionaryCallbackClient;
	ItemCallbackClient*			_pItemCallbackClient;
	RestLoggingCallbackClient*	_pRestLoggingCallbackClient;
	OmmConsumerClient&			_consAdminClient;
	OmmProviderClient&			_provAdminClient;
	OmmOAuth2ConsumerClient&	_consOAuthClient;
	void*						_adminClosure;
	OmmLoggerClient*			_pLoggerClient;
	Pipe						_pipe;
	UInt32						_pipeWriteCount;
	bool						_atExit;
	bool						_eventTimedOut;
	bool						_bMsgDispatched;
	bool						_bEventReceived;
	bool						_hasConsAdminClient;
	bool						_hasProvAdminClient;
	bool						_hasConsOAuthClient;
	ErrorClientHandler*			_pErrorClientHandler;
	EmaList< TimeOut* >			_theTimeOuts;
	bool						_bApiDispatchThreadStarted;
	bool						_bUninitializeInvoked;

	

private:

	OmmBaseImpl( const OmmBaseImpl& );
	OmmBaseImpl& operator=( const OmmBaseImpl& );
	virtual bool isApiDispatching() const = 0;
};

}

}

}

#endif // __refinitiv_ema_access_OmmBaseImpl_h
