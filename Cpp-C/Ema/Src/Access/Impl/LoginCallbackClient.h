/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_LoginCallbackClient_h
#define __refinitiv_ema_access_LoginCallbackClient_h

#include "rtr/rsslReactor.h"
#include "EmaList.h"
#include "OmmLoggerClient.h"
#include "ItemCallbackClient.h"
#include "ChannelCallbackClient.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "RefreshMsg.h"
#include "StatusMsg.h"
#include "ReqMsg.h"
#include "PostMsg.h"
#include "OmmBaseImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

#define	EMA_LOGIN_STREAM_ID 1

class Channel;
class OmmBaseImpl;

class LoginRdmRefreshMsgImpl
{
public :

	LoginRdmRefreshMsgImpl();
	virtual ~LoginRdmRefreshMsgImpl();

	LoginRdmRefreshMsgImpl(const LoginRdmRefreshMsgImpl&);
	LoginRdmRefreshMsgImpl& operator=(const LoginRdmRefreshMsgImpl&);

	static LoginRdmRefreshMsgImpl* create( OmmBaseImpl& );

	static void destroy( LoginRdmRefreshMsgImpl*& );

	LoginRdmRefreshMsgImpl& set( RsslRDMLoginRefresh* );

	bool aggregateForRequestRouting(RsslRDMLoginRefresh*, ConsumerRoutingSession*);

	const EmaString& toString();

	Channel* getChannel() const;

	LoginRdmRefreshMsgImpl& setChannel( Channel* );

	RsslRDMLoginRefresh* getRefreshMsg();

	bool populate(RsslRefreshMsg&, RsslBuffer&, UInt8 majorVersion = RSSL_RWF_MAJOR_VERSION, UInt8 minorVersion = RSSL_RWF_MINOR_VERSION );

	bool populate(RsslStatusMsg&, RsslBuffer&, UInt8 majorVersion = RSSL_RWF_MAJOR_VERSION, UInt8 minorVersion = RSSL_RWF_MINOR_VERSION);

	void clear();

private :

	EmaString		_username;
	EmaString		_position;
	EmaString		_applicationId;
	EmaString		_applicationName;
	EmaString		_toString;
	EmaString		_authenticationErrorText;
	EmaString		_stateText;
	EmaBuffer		_authenticationExtendedResp;
	Channel*		_pChannel;
	bool			_toStringSet;
	bool			_initialSet;				// Internal flag 
	RsslRDMLoginRefresh _refreshMsg;

};

// This represents the handle and associated routing information for a Login Request.
// EMA aggregates the login, and that's handled solely by the LoginCallbackClient.
class LoginItem : public SingleItem
{
public :

	static LoginItem* create( OmmBaseImpl&, OmmConsumerClient& , void* );

	bool open( RsslRDMLoginRequest*);
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();

	ItemType getType() const { return Item::LoginItemEnum; }

	void setEventChannel( void* channel ) { _event._channel = channel; }

private :

	bool submit( RsslRDMLoginRequest*, RsslReactorChannel* );
	bool submit( RsslGenericMsg*, RsslReactorChannel* );
	bool submit( RsslPostMsg*, RsslBuffer*, RsslReactorChannel* );

	static const EmaString		_clientName;

	LoginItem( OmmBaseImpl&, OmmConsumerClient& , void*);
	LoginItem();
	virtual ~LoginItem();
	LoginItem( const LoginItem& );
	LoginItem& operator=( const LoginItem& );
};

class NiProviderLoginItem : public NiProviderSingleItem
{
public:

	static NiProviderLoginItem* create( OmmBaseImpl&, OmmProviderClient&, void*);

	bool open( RsslRDMLoginRequest*);
	bool modify( const ReqMsg& );
	bool submit( const PostMsg& );
	bool submit( const GenericMsg& );
	bool close();

	ItemType getType() const { return Item::NiProviderLoginItemEnum; }

	void setEventChannel( void* channel ) { _event._channel = channel; }

private:

	bool submit( RsslRDMLoginRequest * );
	bool submit( RsslGenericMsg* );
	bool submit( RsslPostMsg* );

	static const EmaString		_clientName;

	NiProviderLoginItem( OmmBaseImpl&, OmmProviderClient&, void*);
	NiProviderLoginItem();
	virtual ~NiProviderLoginItem();
	NiProviderLoginItem( const NiProviderLoginItem& );
	NiProviderLoginItem& operator=( const NiProviderLoginItem& );
};

// This class stores all of the associated login information for a currently connected Reactor Channel
class LoginInfo
{
public:
	LoginRdmReqMsgImpl*				pLoginRequestMsg;

	LoginRdmRefreshMsgImpl			loginRefreshMsg;

	LoginInfo();
	LoginInfo(const LoginInfo&);
	~LoginInfo();
};

class LoginCallbackClient
{
public :

	static LoginCallbackClient* create( OmmBaseImpl& );

	static void destroy( LoginCallbackClient*& );

	void initialize();

	RsslReactorCallbackRet processCallback( RsslReactor*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );

	RsslRDMLoginRequest* getLoginRequest();

	RsslRDMLoginRefresh* getLoginRefresh();

	void setLoginRequest(LoginRdmReqMsgImpl*);

	LoginItem* getLoginItem( const ReqMsg&, OmmConsumerClient& , void* );

	NiProviderLoginItem* getLoginItem( const ReqMsg&, OmmProviderClient&, void* );

	LoginInfo& getLoginInfo();

	void sendInternalMsg( LoginItem* );

	RsslReactorCallbackRet processAckMsg( RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent* );

	RsslReactorCallbackRet processGenericMsg(RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent*);

	static void handleLoginItemCallback( void* );

	const EmaString& getLoginFailureMessage();

	void processChannelEvent( RsslReactorChannelEvent* );

	void overlayLoginRequest(RsslRDMLoginRequest* pRequest);

	static void sendLoginClose(RsslReactor*, RsslReactorChannel* );

	void aggregateLoginsAfterClose();

	EmaVector< Item* >& getLoginItems();

	RsslReactorCallbackRet processRefreshMsg(RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent*);
	RsslReactorCallbackRet processStatusMsg(RsslMsg*, RsslReactorChannel*, RsslRDMLoginMsgEvent*);


private :

	static const EmaString			_clientName;

	Mutex							_loginItemLock;

	LoginInfo						_loginInfo;				// Contains the login request and refresh information for the current reactor connection, if request routing is not turned on.

	char*							_loginRefreshBuffer;

	bool							_refreshReceived;

	RefreshMsg						_refreshMsg;

	StatusMsg						_statusMsg;

	GenericMsg						_genericMsg;

	AckMsg							_ackMsg;

	OmmBaseImpl&					_ommBaseImpl;

	EmaVector< Item* >				_loginItems;

	EmaString						_loginFailureMsg;

	bool _notifyChannelDownReconnecting;			/* Used for recovery to check if the user has gotten the current status already */

	bool convertRdmLoginToRsslBuffer( RsslReactorChannel*, RsslRDMLoginMsgEvent*, RsslBuffer* );

	LoginCallbackClient( OmmBaseImpl& );
	virtual ~LoginCallbackClient();

	LoginCallbackClient();
	LoginCallbackClient( const LoginCallbackClient& );
	LoginCallbackClient& operator=( const LoginCallbackClient& );
};

struct LoginItemCreationCallbackStruct
{
	LoginItemCreationCallbackStruct( LoginCallbackClient* lcbc, LoginItem* li ) : loginCallbackClient( lcbc ), loginItem( li ) {}
	LoginCallbackClient* loginCallbackClient;
	LoginItem* loginItem;
};

struct NiProviderLoginItemCreationCallbackStruct
{
	NiProviderLoginItemCreationCallbackStruct( LoginCallbackClient* lcbc, NiProviderLoginItem* li ) : loginCallbackClient( lcbc ), loginItem( li ) {}
	LoginCallbackClient* loginCallbackClient;
	NiProviderLoginItem* loginItem;
};

}

}

}

#endif // __refinitiv_ema_access_LoginCallbackClient_h
