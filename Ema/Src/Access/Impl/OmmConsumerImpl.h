/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmConsumerImpl_h
#define __thomsonreuters_ema_access_OmmConsumerImpl_h

#ifdef WIN32
#define USING_SELECT
#else  // todo: Solaris 10 will use USING_POLL
#define USING_POLL
#define USING_PPOLL
#endif

#ifdef USING_PPOLL
	#include <poll.h>
#endif

#include "rtr/rsslReactor.h"
#include "EmaList.h"
#include "EmaVector.h"
#include "Mutex.h"
#include "Thread.h"
#include "OmmLoggerClient.h"
#include "OmmConsumerActiveConfig.h"
#include "Pipe.h"
#include "TimeOut.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmConsumerConfig;
class OmmConsumerClient;
class OmmConsumerErrorClient;
class OmmException;
class ReqMsg;
class RespMsg;
class GenericMsg;
class PostMsg;
class TunnelStreamRequest;

class ChannelCallbackClient;
class LoginCallbackClient;
class DirectoryCallbackClient;
class DictionaryCallbackClient;
class ItemCallbackClient;
class OmmConsumerImpl;
class OmmConsumerConfigImpl;

class OmmConsumerImplMap
{
public :

	static UInt64 add( OmmConsumerImpl* );
	static void remove( OmmConsumerImpl* );

#ifdef WIN32
	static BOOL WINAPI TermHandlerRoutine( DWORD dwCtrlType );
#else
	static void sigAction( int sig, siginfo_t * pSiginfo, void* pv );
#endif

private :

	static void init();
	static void atExit();

	static Mutex							_listLock;
	static EmaVector< OmmConsumerImpl* >	_ommConsumerList;
	static UInt64							_id;
	static bool								_clearSigHandler;

#ifndef WIN32
	static struct sigaction _sigAction;
	static struct sigaction _oldSigAction;
#endif 
};

class OmmConsumerImpl : public Thread
{
public :

	static RsslReactorCallbackRet channelCallback( RsslReactor* , RsslReactorChannel* , RsslReactorChannelEvent* );

	static RsslReactorCallbackRet loginCallback( RsslReactor* , RsslReactorChannel* , RsslRDMLoginMsgEvent* );

	static RsslReactorCallbackRet directoryCallback( RsslReactor* , RsslReactorChannel* , RsslRDMDirectoryMsgEvent* );

	static RsslReactorCallbackRet dictionaryCallback( RsslReactor* , RsslReactorChannel* , RsslRDMDictionaryMsgEvent* );

	static RsslReactorCallbackRet itemCallback( RsslReactor* , RsslReactorChannel* , RsslMsgEvent* );

	static RsslReactorCallbackRet channelOpenCallback( RsslReactor* , RsslReactorChannel* , RsslReactorChannelEvent* );

	static RsslReactorCallbackRet tunnelStreamStatusEventCallback( RsslTunnelStream* , RsslTunnelStreamStatusEvent* );

	static RsslReactorCallbackRet tunnelStreamDefaultMsgCallback( RsslTunnelStream* , RsslTunnelStreamMsgEvent* );

	static RsslReactorCallbackRet tunnelStreamQueueMsgCallback( RsslTunnelStream* , RsslTunnelStreamQueueMsgEvent* );

	OmmConsumerImpl( const OmmConsumerConfig& );

	OmmConsumerImpl( const OmmConsumerConfig& , OmmConsumerErrorClient& );

	virtual ~OmmConsumerImpl();

	const EmaString& getConsumerName() const;

	UInt64 registerClient( const ReqMsg& , OmmConsumerClient& , void* closure = 0, UInt64 parentHandle = 0 ); 

	UInt64 registerClient( const TunnelStreamRequest& , OmmConsumerClient& , void* closure = 0 );

	void reissue( const ReqMsg& reqMsg, UInt64 handle ); 

	void submit( const GenericMsg& genericMsg, UInt64 handle ); 

	void submit( const PostMsg& postMsg, UInt64 handle = 0 ); 

	Int64 dispatch( Int64 timeOut = 0 ); 

	void unregister( UInt64 handle );

	void addSocket( RsslSocket );

	void removeSocket( RsslSocket );

	void closeChannel( RsslReactorChannel* );

	enum OmmConsumerState {
		NotInitializedEnum = 0,
		RsslInitilizedEnum,
		ReactorInitializedEnum,
		RsslChannelDownEnum,
		RsslChannelUpEnum,
		LoginStreamOpenSuspectEnum,
		LoginStreamOpenOkEnum,
		LoginStreamClosedEnum,
		DirectoryStreamOpenSuspectEnum,
		DirectoryStreamOpenOkEnum
	};
		
	void setState( OmmConsumerState );

	void setDispatchInternalMsg();

	ItemCallbackClient& getItemCallbackClient();

	DictionaryCallbackClient& getDictionaryCallbackClient();

	DirectoryCallbackClient& getDirectoryCallbackClient();

	LoginCallbackClient& getLoginCallbackClient();

	ChannelCallbackClient& getChannelCallbackClient();

	OmmLoggerClient& getOmmLoggerClient();

	OmmConsumerActiveConfig& getActiveConfig();

	OmmConsumerErrorClient& getOmmConsumerErrorClient();

	bool hasOmmConnsumerErrorClient();

	EmaList< TimeOut > & getTimeOutList() { return theTimeOuts; }
	Mutex & getTimeOutMutex() { return _timeOutLock; }
	void installTimeOut() { pipeWrite(); }

private :

	friend class OmmConsumerImplMap;

	void initialize( const OmmConsumerConfig& );

	void uninitialize( bool caughtExcep = false );

	void readConfig( const OmmConsumerConfig& );

	void setAtExit();

	void run();

	void cleanUp();

	int runLog( void*, const char*, unsigned int );

	void pipeWrite();

	void pipeRead();

	bool rsslReactorDispatchLoop( Int64 timeOut, UInt32 count );

	static void terminateIf( void * );

	static void notifyOmmConsumerErrorClient( const OmmException&, OmmConsumerErrorClient& );

	OmmConsumerActiveConfig			_activeConfig;

#ifdef USING_SELECT
	fd_set							_readFds;
	fd_set							_exceptFds;
#endif

#ifdef USING_POLL
	pollfd*	_eventFds;
	nfds_t	_eventFdsCount;
	nfds_t	_eventFdsCapacity;
	int     pipeReadEventFdsIdx;
	void removeFd( int );
	int addFd( int, short );
#endif

	Mutex							_consumerLock;
	Mutex							_pipeLock;
	Mutex							_timeOutLock;
	RsslErrorInfo					_reactorDispatchErrorInfo;
	RsslRet							_reactorRetCode;
	OmmConsumerState				_ommConsumerState;
	RsslReactor*					_pRsslReactor;
	ChannelCallbackClient*			_pChannelCallbackClient;
	LoginCallbackClient*			_pLoginCallbackClient;
	DirectoryCallbackClient*		_pDirectoryCallbackClient;
	DictionaryCallbackClient*		_pDictionaryCallbackClient;
	ItemCallbackClient*				_pItemCallbackClient;
	OmmLoggerClient*				_pLoggerClient;
	Pipe							_pipe;
	UInt32							_pipeWriteCount;
	bool							_dispatchInternalMsg;
	bool							_atExit;
	bool							_eventTimedOut;
	OmmConsumerErrorClient*			_ommConsumerErrorClient;

	OmmConsumerImpl( const OmmConsumerImpl& );
	OmmConsumerImpl& operator=( const OmmConsumerImpl& );
	EmaList<TimeOut> theTimeOuts;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerImpl_h
