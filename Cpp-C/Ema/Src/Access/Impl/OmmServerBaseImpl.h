/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright Thomson Reuters 2016. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_access_OmmServerBaseImpl_h
#define __thomsonreuters_ema_access_OmmServerBaseImpl_h

#ifdef WIN32
#define USING_SELECT
#else
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
#include "ErrorClientHandler.h"
#include "OmmException.h"
#include "OmmBaseImplMap.h"
#include "ReqMsg.h"
#include "PostMsg.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class LoginHandler;
class DictionaryHandler;
class DirectoryHandler;
class ServerChannelHandler;
class MarketItemHandler;
class OmmLoggerClient;
class OmmProviderErrorClient;
class OmmProviderClient;
class TimeOut;
class ClientSession;
class ItemInfo;
class EmaConfigServerImpl;
class DirectoryServiceStore;
class ItemCallbackClient;

typedef ItemInfo* ItemInfoPtr;

typedef const ClientSession* ClientSessionPtr;

class OmmServerBaseImpl : public OmmCommonImpl, public Thread, public TimeOutClient
{
public:

	enum ServerImplState
	{
		NotInitializedEnum = 0,
		RsslInitilizedEnum,
		ReactorInitializedEnum,
		UnInitializingEnum
	};

	virtual const EmaString& getInstanceName() const;

	virtual OmmProvider* getProvider() const = 0;

	virtual DirectoryServiceStore& getDirectoryServiceStore() = 0;

	virtual void addSocket(RsslSocket);

	virtual void removeSocket(RsslSocket);

	void setState(ServerImplState state);

	ServerImplState getState();

	void msgDispatched(bool value = true);

	void eventReceived(bool value = true);

	ItemCallbackClient& getItemCallbackClient();

	MarketItemHandler& getMarketItemHandler();

	DictionaryHandler& getDictionaryHandler();

	DirectoryHandler& getDirectoryHandler();

	LoginHandler& getLoginHandler();

	ServerChannelHandler& getServerChannelHandler();

	OmmLoggerClient& getOmmLoggerClient();

	ActiveServerConfig& getActiveConfig();

	LoggerConfig& getActiveLoggerConfig();

	ErrorClientHandler& getErrorClientHandler();

	bool hasErrorClientHandler() const;

	EmaList< TimeOut* >& getTimeOutList();

	Mutex& getTimeOutMutex();

	Mutex& getUserMutex();

	RsslReactor* getRsslReactor();

	void installTimeOut();

	void handleIue(const EmaString&);

	void handleIue(const char*);

	void handleIhe(UInt64, const EmaString&);

	void handleIhe(UInt64, const char*);

	void handleMee(const char*);

	virtual void processChannelEvent(RsslReactorChannelEvent*) = 0;

	bool isAtExit();

protected:

	friend class OmmBaseImplMap<OmmServerBaseImpl>;

	OmmServerBaseImpl(ActiveServerConfig&, OmmProviderClient&, void* closure);
	OmmServerBaseImpl(ActiveServerConfig&, OmmProviderClient&, OmmProviderErrorClient&, void* closure);

	virtual ~OmmServerBaseImpl();

	void initialize(EmaConfigServerImpl*);

	void uninitialize(bool caughtException, bool calledFromInit);

	void readConfig(EmaConfigServerImpl*);

	virtual void readCustomConfig(EmaConfigServerImpl*) = 0;

	ServerConfig* readServerConfig(EmaConfigServerImpl*, const EmaString&);

	void addItemGroup(ItemInfo* itemInfo, const EmaBuffer& groupId);

	void updateItemGroup(ItemInfo* itemInfo, const EmaBuffer& newGroupId);

	void removeItemGroup(ItemInfo* itemInfo);

	void removeServiceId(ClientSession* clientSession, UInt64 serviceId);

	void removeGroupId(ClientSession* clientSession, UInt64 serviceId, const RsslBuffer& groupId);

	void mergeToGroupId(ClientSession* clientSession, UInt64 serviceId, const RsslBuffer& groupId, const RsslBuffer& newGroupId);

	void setAtExit();

	void run();

	void cleanUp();

	int runLog(void*, const char*, unsigned int);

	void pipeWrite();

	void pipeRead();

	bool isPipeWritten();

	Int64 rsslReactorDispatchLoop(Int64 timeOut, UInt32 count, bool& bMsgDispRcvd);

	static void terminateIf(void*);

	static void notifErrorClientHandler(const OmmException&, ErrorClientHandler&);

	virtual bool isApiDispatching() const = 0;

	ItemInfoPtr getItemInfo(UInt64);

	void addItemInfo(ItemInfo *);

	void removeItemInfo(ItemInfo *, bool eraseItemGroup);

	void bindServerOptions(RsslBindOptions& bindOptions, const EmaString& componentVersion);

	ActiveServerConfig&	_activeServerConfig;

#ifdef USING_SELECT
	fd_set			_readFds;
	fd_set			_exceptFds;
#endif

#ifdef USING_POLL
	pollfd*			_eventFds;
	nfds_t			_eventFdsCount;
	nfds_t			_eventFdsCapacity;
	int				_pipeReadEventFdsIdx;
	int				_serverReadEventFdsIdx;

	void removeFd(int);
	int addFd(int, short);
#endif
	Mutex						_userLock;
	Mutex						_pipeLock;
	Mutex						_timeOutLock;
	RsslErrorInfo				_reactorDispatchErrorInfo;
	ServerImplState				_state;
	RsslReactor*				_pRsslReactor;
	ServerChannelHandler*		_pServerChannelHandler;
	LoginHandler*				_pLoginHandler;
	DirectoryHandler*			_pDirectoryHandler;
	DictionaryHandler*			_pDictionaryHandler;
	MarketItemHandler*			_pMarketItemHandler;
	ItemCallbackClient*			_pItemCallbackClient;
	OmmLoggerClient*			_pLoggerClient;
	Pipe						_pipe;
	UInt32						_pipeWriteCount;
	bool						_atExit;
	bool						_eventTimedOut;
	bool						_bMsgDispatched;
	bool						_bEventReceived;
	ErrorClientHandler*			_pErrorClientHandler;
	EmaList< TimeOut* >			_theTimeOuts;
	OmmProviderClient*			_pOmmProviderClient;
	OmmProviderEvent			ommProviderEvent;
	void*						_pClosure;

	ReqMsg                      _reqMsg;
	StatusMsg					_statusMsg;
	GenericMsg					_genericMsg;
	PostMsg						_postMsg;

private:

	friend class LoginHandler;
	friend class DirectoryHandler;
	friend class DictionaryHandler;
	friend class MarketItemHandler;
	friend class ClientSession;
	friend class ItemCallbackClient;

	OmmServerBaseImpl( const OmmServerBaseImpl& );
	OmmServerBaseImpl& operator=( const OmmServerBaseImpl& );

	class UInt64rHasher
	{
	public:
		size_t operator()(const UInt64&) const;
	};

	class UInt64Equal_To
	{
	public:
		bool operator()(const UInt64&, const UInt64&) const;
	};

	typedef HashTable< UInt64, ItemInfoPtr, UInt64rHasher, UInt64Equal_To > ItemInfoHash;

	ItemInfoHash _itemInfoHash;

	RsslReactorOMMProviderRole	_providerRole;
	RsslServer*					_pRsslServer;
	RsslReactorAcceptOptions	_reactorAcceptOptions;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmServerBaseImpl_h

