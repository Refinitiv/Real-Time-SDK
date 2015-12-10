/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmBaseImpl_h
#define __thomsonreuters_ema_access_OmmBaseImpl_h

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
#include "Pipe.h"
#include "TimeOut.h"

#include "ActiveConfig.h"
#include "UserErrorHandler.h"
#include "OmmImplMap.h"
#include "OmmException.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class ChannelCallbackClient;
class LoginCallbackClient;
class DirectoryCallbackClient;
class DictionaryCallbackClient;
class ItemCallbackClient;
class OmmLoggerClient;
class TimeOut;
class TunnelStreamRequest;

class OmmBaseImpl : public Thread
{
public :

  static RsslReactorCallbackRet channelCallback( RsslReactor* , RsslReactorChannel* , RsslReactorChannelEvent* );
  static RsslReactorCallbackRet loginCallback( RsslReactor* , RsslReactorChannel* , RsslRDMLoginMsgEvent* );

  static RsslReactorCallbackRet directoryCallback( RsslReactor* , RsslReactorChannel* , RsslRDMDirectoryMsgEvent* );

  static RsslReactorCallbackRet dictionaryCallback( RsslReactor* , RsslReactorChannel* , RsslRDMDictionaryMsgEvent* );

  static RsslReactorCallbackRet itemCallback( RsslReactor* , RsslReactorChannel* , RsslMsgEvent* );

  static RsslReactorCallbackRet channelOpenCallback( RsslReactor* , RsslReactorChannel* , RsslReactorChannelEvent* );

  virtual const EmaString& getUserName() const;

  virtual void reissue( const ReqMsg&, UInt64 );

  virtual void submit( const GenericMsg&, UInt64 ); 

  virtual void submit( const PostMsg&, UInt64 handle = 0 ); 

  virtual void unregister( UInt64 handle );

  virtual void addSocket( RsslSocket ) = 0;

  virtual void removeSocket( RsslSocket ) = 0;

  void closeChannel( RsslReactorChannel* );

  enum ImplState {
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

  void setState( ImplState state );

  void setDispatchInternalMsg();

  ItemCallbackClient& getItemCallbackClient();

  DictionaryCallbackClient& getDictionaryCallbackClient();

  DirectoryCallbackClient& getDirectoryCallbackClient();

  LoginCallbackClient& getLoginCallbackClient();

  ChannelCallbackClient& getChannelCallbackClient();

  OmmLoggerClient& getOmmLoggerClient();

  ActiveConfig& getActiveConfig();

  UserErrorHandler& getUserErrorHandler() { return *userErrorHandler; }

  bool hasUserErrorHandler() { return userErrorHandler != 0; }

  EmaList< TimeOut* > & getTimeOutList() { return theTimeOuts; }
  Mutex & getTimeOutMutex() { return _timeOutLock; }
  void installTimeOut() { pipeWrite(); }

  virtual void downloadDictionary() {}
  virtual void downloadDirectory() {}
  virtual void setRsslReactorChannelRole( RsslReactorChannelRole& ) = 0;



protected:

  template<class T> friend class OmmImplMap;

  OmmBaseImpl( ActiveConfig& );
  OmmBaseImpl( ActiveConfig&, OmmConsumerErrorClient& );
  OmmBaseImpl( ActiveConfig&, OmmNiProviderErrorClient& );
  virtual ~OmmBaseImpl();

  void initialize( EmaConfigImpl* );

  void uninitialize( bool caughtException = false );

  void readConfig( EmaConfigImpl* );

  ChannelConfig* readChannelConfig( EmaConfigImpl*, const EmaString& );

  bool readReliableMcastConfig( EmaConfigImpl*, const EmaString&, ReliableMcastChannelConfig *, EmaString& );

  void useDefaultConfigValues( const EmaString &, const EmaString &, const EmaString & );

  void setAtExit();

  void run();

  void cleanUp();

  int runLog( void*, const char*, unsigned int );

  void pipeWrite();

  void pipeRead();

  bool rsslReactorDispatchLoop( Int64 timeOut, UInt32 count );

  static void terminateIf( void * );

  static void notifyUserErrorHandler( const OmmException&, UserErrorHandler& );

  ActiveConfig& _activeConfig;

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

  Mutex _userLock;
  Mutex							_pipeLock;
  Mutex							_timeOutLock;
  RsslErrorInfo					_reactorDispatchErrorInfo;
  RsslRet							_reactorRetCode;
  ImplState				_state;
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
  UserErrorHandler* userErrorHandler;

  EmaList< TimeOut* > theTimeOuts;

private:

  OmmBaseImpl( const OmmBaseImpl& );
  OmmBaseImpl& operator=( const OmmBaseImpl& );

};

}

}

}

#endif // __thomsonreuters_ema_access_OmmBaseImpl_h
